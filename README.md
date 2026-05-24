# spraybus

spraybus is a small C++23 pub/sub message bus built on top of
[ENet](http://enet.bespin.org/). It provides a lightweight server, a C++ client
API, and a command-line tool for publishing and subscribing to string messages.

The project is early and intentionally compact. It is useful as a local
messaging primitive, an experiment in topic fanout, and a base for richer
protocol work.

## Dependencies

The default Conan package builds the client library only and depends on:

- ENet

Local server/app builds also require Quill.

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

The package installs the client-facing headers and libraries:

- `spraybus-client`
- `spraybus-networking`

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

The client package does not include the server library, server headers, CLI, or
Quill.

ENet is a public transitive dependency because the client headers expose the
low-level networking wrapper directly.

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

## Local Build

To build the server and CLI for local development:

```sh
conan build . --build=missing -o "&:with_server=True" -o "&:with_apps=True"
```

This writes build artifacts under `build/Release/` and creates the following
executables:

```text
build/Release/main
build/Release/cli
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
