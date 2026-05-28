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

Binary wheels should be published for end users. Source builds need CMake to be
able to find ENet and Quill. The release workflow publishes Linux x86_64, macOS
arm64, and macOS x86_64 wheels.
