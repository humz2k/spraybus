# spraybus Python example

This example uses the in-tree asyncio client wrapper. It connects to
`localhost:6767`, subscribes to `python_example_in`, publishes a string to
`python_example_out`, and prints received messages as strings.

From the repository root, build spraybus with the Python extension:

```sh
conan build . --build=missing -s compiler.cppstd=23 -of build/python
```

In one terminal, run the server:

```sh
./build/python/build/Release/main
```

In another terminal, run the example:

```sh
PYTHONPATH=python/src:build/python/build/Release/python python examples/python/client.py
```

To send something to the subscribed topic from the CLI:

```sh
./build/python/build/Release/cli pub python_example_in "hello python subscriber"
```
