from __future__ import annotations

import asyncio

import spraybus

HOST = "localhost"
PORT = 6767
SUBSCRIBE_TOPIC = "python_example_in"
PUBLISH_TOPIC = "python_example_out"
PUBLISH_MESSAGE = "hello from the spraybus Python example"


async def print_messages(client: spraybus.AsyncClient) -> None:
    async for message in client.messages():
        print(message.text())


async def main() -> None:
    async with spraybus.AsyncClient(HOST, PORT) as client:
        await client.subscribe(SUBSCRIBE_TOPIC)
        receiver = asyncio.create_task(print_messages(client))

        await client.publish(PUBLISH_TOPIC, PUBLISH_MESSAGE)

        print(f"subscribed to {SUBSCRIBE_TOPIC}")
        print(f"published to {PUBLISH_TOPIC}: {PUBLISH_MESSAGE}")

        await receiver


if __name__ == "__main__":
    asyncio.run(main())
