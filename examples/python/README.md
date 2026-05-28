# spraybus Python example

This example uses the published asyncio client wrapper. It connects to
`localhost:6767`, subscribes to `python_example_in`, publishes a string to
`python_example_out`, and prints received messages as strings.

Install the example dependencies:

```sh
uv sync --prerelease=allow
```

In one terminal, run the server:

```sh
uvx --prerelease=allow spraybus-server
```

In another terminal, run the example:

```sh
uv run python main.py
```

To send something to the subscribed topic from the CLI:

```sh
uvx --prerelease=allow spraybus-cli pub python_example_in "hello python subscriber"
```
