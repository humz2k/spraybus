# spraybus client example

This is a standalone Conan/CMake consumer of `spraybus-client/0.1`.

From the repository root, first put the local spraybus client package in the
Conan cache:

```sh
conan create . --build=missing
```

Then build the example:

```sh
conan build examples/client --build=missing -nr
```

Run it against a local spraybus server:

```sh
./examples/client/build/Release/spraybus-client-example
```

The example connects to `localhost:6767`, publishes a string to `test_topic`,
subscribes to `test_topic`, and prints received messages.

During active development, you can use the checkout as an editable Conan package
instead of creating a cache package:

```sh
conan editable add . --name=spraybus-client --version=0.1
conan build examples/client --build=missing
conan editable remove .
```
