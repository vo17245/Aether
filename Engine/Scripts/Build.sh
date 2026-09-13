#!/usr/bin/env bash

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR/depot_tools" || exit 1
python3 build.py
