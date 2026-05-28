# spraybus Python client

The Python client exposes an asyncio API over the native C++ client. The native
client is created and used on a single background thread. Async methods enqueue
commands to that thread, and received fanout messages are delivered back to the
event loop.

```python
import asyncio
import spraybus


async def main():
    async with spraybus.AsyncClient("localhost", 6767) as client:
        await client.publish("test_topic", "hello from python")
        await client.subscribe("test_topic")

        async for message in client.messages():
            print(message.topic, message.text())


asyncio.run(main())
```

## Build

From the repository root:

```sh
conan build . --build=missing -s compiler.cppstd=23 -of build/python
PYTHONPATH=python/src:build/python/build/Release/python python -c "import spraybus"
```

The package also includes a `pyproject.toml` for wheel builds with
scikit-build-core. Wheel builds still need ENet and pybind11 to be discoverable
by CMake.
