# spraybus

spraybus is a small C++23 pub/sub message bus built on top of
[ENet](http://enet.bespin.org/). It provides a lightweight server, a C++ client
API, and a command-line tool for publishing and subscribing to string messages.

The project is early and intentionally compact. It is useful as a local
messaging primitive, an experiment in topic fanout, and a base for richer
protocol work.

## Dependencies

The default Conan build includes the client library, server library, command-line
apps, and Python extension. It depends on:

- ENet
- Quill
- pybind11

You also need a C++23 compiler, CMake, and Conan 2.

## Client Package

Create the installable Conan package from the repository root:

```sh
conan create . --build=missing
```

The package reference is:

```text
spraybus-client/0.1
```

The default package build compiles the server, CLI, and Python extension too.
The installed C++ artifacts are the client-facing headers and libraries:

- `spraybus-client`
- `spraybus-networking`

The Python package is installed under `spraybus/` when `with_python=True`.

It exposes the CMake target:

```cmake
find_package(spraybus-client CONFIG REQUIRED)
target_link_libraries(app PRIVATE spraybus::client)
```

Upload the package to a configured Conan remote:

```sh
conan remote add myrepo <repo-url>
conan login myrepo <user>
conan upload "spraybus-client/0.1:*" -r=myrepo -c
```

To build a minimal C++ client-only package, disable the optional outputs:

```sh
conan create . --build=missing -o "&:with_server=False" -o "&:with_apps=False" -o "&:with_python=False"
```

ENet is a public transitive dependency because the client headers expose the
low-level networking wrapper directly.

## Release CI

Pushing a `v*` tag whose commit is reachable from `main` builds a Linux server
binary, creates a GitHub release, and uploads the server archive. See
[docs/release.md](docs/release.md) for the full release flow.

## Python Client

The `python/` directory contains an asyncio client wrapper around the C++
client. The native client lives on one background thread; Python async methods
send commands to that thread and receive fanout messages through an
`asyncio.Queue`.

Install the package from PyPI:

```sh
pip install spraybus
```

Build the native Python extension with Conan:

```sh
conan build . --build=missing -s compiler.cppstd=23 -of build/python
PYTHONPATH=python/src:build/python/build/Release/python python -c "import spraybus"
```

Example:

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

The release workflow publishes `spraybus` wheels for CPython 3.10 through 3.14
on Linux x86_64, macOS arm64, and macOS x86_64.

## Python Server Package

The `python/server` directory packages the native server as `spraybus-server`.
After installing the package, users can run the server with:

```sh
spraybus-server
```

The command runs the bundled C++ server binary. It listens on UDP port `6767` by
default, or `SPRAYBUS_PORT` when set:

```sh
SPRAYBUS_PORT=7000 spraybus-server
```

Build a Linux wheel for PyPI from the repository root with cibuildwheel:

```sh
python -m pip install cibuildwheel
CIBW_BUILD=cp310-manylinux_x86_64 \
CIBW_ARCHS_LINUX=x86_64 \
CIBW_MANYLINUX_X86_64_IMAGE=manylinux_2_28 \
CIBW_BEFORE_ALL_LINUX='python -m pip install --upgrade "conan>=2,<3" cmake ninja && cd /project && conan profile detect --force && conan install . --build=missing -s compiler.cppstd=23 -o "&:with_server=True" -o "&:with_apps=True" -o "&:with_python=False" -of build/python-server' \
CIBW_ENVIRONMENT_LINUX='CMAKE_PREFIX_PATH=/project/build/python-server/build/Release/generators' \
python -m cibuildwheel \
  --output-dir python/server/dist \
  python/server
```

Publish binary wheels for end users; source builds need CMake to find ENet and
Quill. The release workflow publishes Linux x86_64, macOS arm64, and macOS
x86_64 wheels for `spraybus-server`.

## Python CLI Package

The `python/cli` directory packages the native CLI as `spraybus-cli`. After
installing the package, users can run:

```sh
spraybus-cli pub test_topic "hello"
spraybus-cli sub test_topic
```

The command connects to `localhost:6767` by default. Flags take precedence over
environment variables:

```sh
spraybus-cli --host localhost --port 7000 pub test_topic "hello"
SPRAYBUS_HOST=localhost SPRAYBUS_PORT=7000 spraybus-cli sub test_topic
```

Build a Linux wheel for PyPI from the repository root with cibuildwheel:

```sh
python -m pip install cibuildwheel
CIBW_BUILD=cp310-manylinux_x86_64 \
CIBW_ARCHS_LINUX=x86_64 \
CIBW_MANYLINUX_X86_64_IMAGE=manylinux_2_28 \
CIBW_BEFORE_ALL_LINUX='python -m pip install --upgrade "conan>=2,<3" cmake ninja && cd /project && conan profile detect --force && conan install . --build=missing -s compiler.cppstd=23 -o "&:with_server=False" -o "&:with_apps=True" -o "&:with_python=False" -of build/python-cli' \
CIBW_ENVIRONMENT_LINUX='CMAKE_PREFIX_PATH=/project/build/python-cli/build/Release/generators' \
python -m cibuildwheel \
  --output-dir python/cli/dist \
  python/cli
```

Publish binary wheels for end users; source builds need CMake to find ENet. The
release workflow publishes Linux x86_64, macOS arm64, and macOS x86_64 wheels
for `spraybus-cli`.

## Example Client

The `examples/client` directory is a standalone Conan/CMake app that consumes
`spraybus-client/0.1` like an external project.

Build it against the locally created package:

```sh
conan create . --build=missing
conan build examples/client --build=missing -nr
```

Or point Conan at this checkout while iterating:

```sh
conan editable add . --name=spraybus-client --version=0.1
conan build examples/client --build=missing
conan editable remove .
```

The `examples/python` directory contains a small asyncio client example that
uses the in-tree Python package and native extension.

## Local Build

To build the server, CLI, and Python extension for local development:

```sh
conan build . --build=missing -s compiler.cppstd=23
```

This writes build artifacts under `build/Release/` and creates the following
executables and Python extension:

```text
build/Release/main
build/Release/cli
build/Release/python/spraybus/_native.*
```

The build defines three library targets:

- `spraybus-networking`, which wraps ENet process initialization and links ENet.
- `spraybus-client`, which links `spraybus-networking`.
- `spraybus-server`, which links `spraybus-networking` and Quill.

## Run The Server

The server listens on UDP port `6767` by default:

```sh
./build/Release/main
```

Set `SPRAYBUS_PORT` to choose a different port:

```sh
SPRAYBUS_PORT=7000 ./build/Release/main
```

## CLI

The CLI connects to `localhost:6767` by default. You can override that with
flags:

```sh
./build/Release/cli --host localhost --port 6767 pub test_topic "hello"
./build/Release/cli --host localhost --port 6767 sub test_topic
```

Or with environment variables:

```sh
SPRAYBUS_HOST=localhost SPRAYBUS_PORT=6767 ./build/Release/cli sub test_topic
SPRAYBUS_HOST=localhost SPRAYBUS_PORT=6767 ./build/Release/cli pub test_topic "hello"
```

Command-line flags take precedence over environment variables.

### Publish

```sh
./build/Release/cli pub <topic> "<message>"
```

Example:

```sh
./build/Release/cli pub alerts "disk space low"
```

### Subscribe

```sh
./build/Release/cli sub <topic>
```

`sub` blocks until interrupted and prints each received payload to stdout:

```text
disk space low
```

## C++ Client API

Initialize ENet once before creating networking objects:

```cpp
#include <spraybus/client/client.hpp>
#include <spraybus/networking/networking.hpp>

int main() {
    spraybus::networking::init();

    spraybus::client::Client client("localhost", 6767);
    client.publish("alerts", "disk space low");
}
```

Subscribe and poll for fanout messages:

```cpp
#include <spraybus/client/client.hpp>
#include <spraybus/networking/networking.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    spraybus::networking::init();

    spraybus::client::Client client("localhost", 6767);
    client.subscribe("alerts");

    while (true) {
        client.process([](const spraybus::networking::protocol::Message& msg) {
            std::cout << msg.topic() << ": " << msg.payload_as_string() << '\n';
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
```

## Architecture

spraybus has three layers:

1. `spraybus::networking` wraps the ENet host, peer, packet, and event loop
   primitives.
2. `spraybus::networking::protocol` defines the fixed 16-byte header, message
   types, and helpers that construct packet bytes.
3. `spraybus::client` and `spraybus::server` implement topic lookup,
   subscription tracking, publishing, and fanout.

Topic names are resolved through the server. Clients ask for a topic key with a
`topic_request`; the server returns a `topic_response`; later publish and
subscribe packets use the numeric topic key.

## Protocol Overview

Every packet starts with `spraybus::networking::protocol::Header`, followed by
an optional payload.

Current message types:

- `topic_request`: payload is the topic name.
- `topic_response`: header contains the topic key and payload repeats the topic
  name.
- `publish`: header contains the topic key and payload contains user bytes.
- `subscribe`: header contains the topic key.
- `unsubscribe`: header contains the topic key.
- `fanout`: header contains the topic key and payload contains forwarded user
  bytes.

The server drops packets shorter than the protocol header.

## Logging

Server logging is backed by Quill. Call `spraybus::server::init_logging()`
before expecting log output from server code. `spraybus::server::set_logfile()`
can be used before logger creation to route logs to a file.

The client API and CLI do not start or link Quill, so `sub` prints only message
payloads to stdout.

## API Documentation

Generate HTML API documentation with Doxygen from the repository root:

```sh
cd docs
doxygen Doxyfile
```

The generated site is written to:

```text
docs/api/html/index.html
```

Generated documentation output is ignored by git.

## Current Limitations

- The protocol currently serializes the in-memory header layout directly, so it
  assumes compatible endianness and layout between peers.
- Topic keys are process-local and are regenerated after a server restart.
- The server is single-process and keeps subscription state in memory.
- There is no authentication, authorization, persistence, or replay.
