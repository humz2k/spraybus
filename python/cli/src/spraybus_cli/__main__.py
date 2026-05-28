from __future__ import annotations

import os
import subprocess
import sys
from importlib import resources
from pathlib import Path


def _binary_name() -> str:
    if os.name == "nt":
        return "_spraybus_cli.exe"
    return "_spraybus_cli"


def _binary_path() -> Path:
    return Path(resources.files("spraybus_cli").joinpath("bin", _binary_name()))


def main() -> int:
    binary = _binary_path()
    if not binary.is_file():
        raise RuntimeError(f"Bundled spraybus CLI binary not found: {binary}")

    args = [str(binary), *sys.argv[1:]]
    if os.name != "nt":
        os.execv(args[0], args)

    return subprocess.call(args)


if __name__ == "__main__":
    raise SystemExit(main())
