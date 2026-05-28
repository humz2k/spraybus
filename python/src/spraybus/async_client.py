from __future__ import annotations

import asyncio
import queue
import threading
from dataclasses import dataclass
from typing import AsyncIterator, Optional, Union

from . import _native

Payload = Union[str, bytes, bytearray, memoryview]


@dataclass(frozen=True, slots=True)
class Message:
    """Owning fanout message returned by the asyncio client."""

    topic_key: int
    topic: str
    payload: bytes

    def text(self, encoding: str = "utf-8", errors: str = "strict") -> str:
        return self.payload.decode(encoding, errors)


@dataclass(slots=True)
class _Command:
    name: str
    args: tuple[object, ...]
    future: Optional[asyncio.Future[object]]


@dataclass(frozen=True, slots=True)
class _WorkerError:
    error: BaseException


class AsyncClient:
    """Asyncio wrapper around a single-thread-owned native spraybus client."""

    def __init__(
        self,
        host: str = "localhost",
        port: int = 6767,
        *,
        poll_interval: float = 0.01,
    ) -> None:
        self._host = host
        self._port = port
        self._poll_interval = poll_interval
        self._commands: queue.Queue[_Command] = queue.Queue()
        self._messages: asyncio.Queue[Message | _WorkerError] | None = None
        self._loop: asyncio.AbstractEventLoop | None = None
        self._thread: threading.Thread | None = None
        self._started: asyncio.Future[None] | None = None
        self._worker_error: BaseException | None = None
        self._closed = False

    async def __aenter__(self) -> "AsyncClient":
        await self.connect()
        return self

    async def __aexit__(self, *_: object) -> None:
        await self.close()

    async def connect(self) -> None:
        if self._thread is not None:
            await self._started
            return

        self._loop = asyncio.get_running_loop()
        self._messages = asyncio.Queue()
        self._started = self._loop.create_future()
        self._closed = False
        self._worker_error = None
        self._thread = threading.Thread(
            target=self._run,
            name="spraybus-async-client",
            daemon=True,
        )
        self._thread.start()
        await self._started

    async def close(self) -> None:
        thread = self._thread
        if thread is None:
            self._closed = True
            return

        if thread.is_alive():
            loop = self._require_loop()
            future: asyncio.Future[object] = loop.create_future()
            self._commands.put(_Command("close", (), future))
            await future
            await asyncio.to_thread(thread.join)

        self._thread = None
        self._closed = True

    async def publish(
        self, topic: str, payload: Payload, *, encoding: str = "utf-8"
    ) -> None:
        data = self._payload_bytes(payload, encoding)
        await self._call("publish", topic, data)

    async def subscribe(self, topic: str) -> None:
        await self._call("subscribe", topic)

    async def get_topic_key(self, topic: str) -> int:
        return int(await self._call("get_topic_key", topic))

    async def recv(self) -> Message:
        self._ensure_running()
        messages = self._require_messages()
        item = await messages.get()
        if isinstance(item, _WorkerError):
            messages.put_nowait(item)
            raise item.error
        return item

    async def messages(self) -> AsyncIterator[Message]:
        while True:
            yield await self.recv()

    async def _call(self, name: str, *args: object) -> object:
        self._ensure_running()
        loop = self._require_loop()
        future: asyncio.Future[object] = loop.create_future()
        self._commands.put(_Command(name, args, future))
        return await future

    def _run(self) -> None:
        client: _native.Client | None = None
        try:
            client = _native.Client(self._host, self._port)
            self._complete_started()

            while True:
                command = self._next_command()
                if command is not None and command.name == "close":
                    self._complete(command.future, None)
                    break

                if command is not None:
                    self._handle_command(client, command)

                self._poll_messages(client)
        except BaseException as error:
            self._fail_started(error)
            self._publish_worker_error(error)
            self._fail_pending(error)
        finally:
            client = None

    def _next_command(self) -> _Command | None:
        try:
            return self._commands.get(timeout=self._poll_interval)
        except queue.Empty:
            return None

    def _handle_command(self, client: _native.Client, command: _Command) -> None:
        try:
            if command.name == "publish":
                topic, payload = command.args
                client.publish(topic, payload)
                result: object = None
            elif command.name == "subscribe":
                (topic,) = command.args
                client.subscribe(topic)
                result = None
            elif command.name == "get_topic_key":
                (topic,) = command.args
                result = client.get_topic_key(topic)
            else:
                raise RuntimeError(f"Unknown spraybus command: {command.name}")
        except BaseException as error:
            self._complete(command.future, error=error)
        else:
            self._complete(command.future, result)

    def _poll_messages(self, client: _native.Client) -> None:
        while True:
            native_message = client.poll()
            if native_message is None:
                return

            message = Message(
                topic_key=native_message.topic_key,
                topic=native_message.topic,
                payload=native_message.payload,
            )
            self._post_message(message)

    def _complete_started(self) -> None:
        started = self._started
        if started is not None:
            self._complete(started, None)

    def _fail_started(self, error: BaseException) -> None:
        started = self._started
        if started is not None:
            self._complete(started, error=error)

    def _publish_worker_error(self, error: BaseException) -> None:
        loop = self._loop
        messages = self._messages
        if loop is None or messages is None:
            return

        def publish() -> None:
            self._worker_error = error
            messages.put_nowait(_WorkerError(error))

        loop.call_soon_threadsafe(publish)

    def _fail_pending(self, error: BaseException) -> None:
        while True:
            try:
                command = self._commands.get_nowait()
            except queue.Empty:
                return
            self._complete(command.future, error=error)

    def _post_message(self, message: Message) -> None:
        loop = self._require_loop()
        messages = self._require_messages()
        loop.call_soon_threadsafe(messages.put_nowait, message)

    def _complete(
        self,
        future: Optional[asyncio.Future[object]],
        result: object = None,
        *,
        error: BaseException | None = None,
    ) -> None:
        if future is None:
            return

        loop = self._require_loop()

        def complete() -> None:
            if future.cancelled() or future.done():
                return
            if error is not None:
                future.set_exception(error)
            else:
                future.set_result(result)

        loop.call_soon_threadsafe(complete)

    def _ensure_running(self) -> None:
        if self._closed:
            raise RuntimeError("spraybus client is closed")
        if self._worker_error is not None:
            raise self._worker_error
        if self._thread is None:
            raise RuntimeError("spraybus client is not connected")

    def _require_loop(self) -> asyncio.AbstractEventLoop:
        if self._loop is None:
            raise RuntimeError("spraybus client is not connected")
        return self._loop

    def _require_messages(self) -> asyncio.Queue[Message | _WorkerError]:
        if self._messages is None:
            raise RuntimeError("spraybus client is not connected")
        return self._messages

    @staticmethod
    def _payload_bytes(payload: Payload, encoding: str) -> bytes:
        if isinstance(payload, str):
            return payload.encode(encoding)
        if isinstance(payload, bytes):
            return payload
        return bytes(payload)
