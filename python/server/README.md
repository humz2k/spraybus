# spraybus-server Python package

This package installs the native spraybus server as a Python console command:

```sh
pip install spraybus-server
spraybus-server
```

The command runs the bundled C++ server binary. It listens on UDP port `6767` by
default. Set `SPRAYBUS_PORT` to choose a different port:

```sh
SPRAYBUS_PORT=7000 spraybus-server
```

## Build From This Repository

Build wheels from the repository root after generating Conan dependency files:

```sh
python -m pip install build
conan install . --build=missing -s compiler.cppstd=23 -of build/python-server
CMAKE_PREFIX_PATH="$PWD/build/python-server/build/Release/generators" \
  python -m build python/server --wheel
```

Binary wheels should be published for end users. Source builds need CMake to be
able to find ENet and Quill.
