# spraybus-cli Python package

This package installs the native spraybus CLI as Python console commands:

```sh
pip install spraybus-cli
spraybus-cli pub test_topic "hello"
spraybus-cli sub test_topic
```

The command runs the bundled C++ CLI binary. It connects to `localhost:6767` by
default. Use flags or environment variables to choose a different endpoint:

```sh
spraybus-cli --host localhost --port 7000 pub test_topic "hello"
SPRAYBUS_HOST=localhost SPRAYBUS_PORT=7000 spraybus-cli sub test_topic
```

## Build From This Repository

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

Binary wheels should be published for end users. Source builds need CMake to be
able to find ENet. The release workflow publishes Linux x86_64, macOS arm64, and
macOS x86_64 wheels.
