# Release CI

GitHub Actions runs `.github/workflows/release.yml` whenever a `v*` tag is
pushed. The workflow rejects tags that do not point to a commit reachable from
`main`.

To cut a release:

```sh
git checkout main
git pull
git tag v0.1.0
git push origin v0.1.0
```

The tag workflow:

1. Builds the server app with `with_server=True` and `with_apps=True`.
2. Creates a GitHub release for the tag.
3. Uploads `spraybus-server-<tag>-linux-x86_64.tar.gz` and a SHA-256 file.
