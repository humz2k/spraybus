# spraybus-cli Python package

This package installs the native spraybus CLI as Python console commands:

```sh
pip install spraybus-cli
spraybus pub test_topic "hello"
spraybus sub test_topic
```

It also installs `spraybus-cli` as an alias for environments where the shorter
`spraybus` command is already taken.

The command runs the bundled C++ CLI binary. It connects to `localhost:6767` by
default. Use flags or environment variables to choose a different endpoint:

```sh
spraybus --host localhost --port 7000 pub test_topic "hello"
SPRAYBUS_HOST=localhost SPRAYBUS_PORT=7000 spraybus sub test_topic
```

## Build From This Repository

Build wheels from the repository root after generating Conan dependency files:

```sh
python -m pip install build
conan install . --build=missing -s compiler.cppstd=23 -of build/python-cli
CMAKE_PREFIX_PATH="$PWD/build/python-cli/build/Release/generators" \
  python -m build python/cli --wheel
```

Binary wheels should be published for end users. Source builds need CMake to be
able to find ENet.
