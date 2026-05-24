# spraybus

## CLI

Run the server:

```sh
SPRAYBUS_PORT=6767 ./build/Release/main
```

Publish a string message:

```sh
./build/Release/cli --host localhost --port 6767 pub test_topic "hello"
```

Subscribe to a topic:

```sh
./build/Release/cli --host localhost --port 6767 sub test_topic
```

If `--host` or `--port` are omitted, the CLI reads `SPRAYBUS_HOST` and
`SPRAYBUS_PORT`, falling back to `localhost` and `6767`.
