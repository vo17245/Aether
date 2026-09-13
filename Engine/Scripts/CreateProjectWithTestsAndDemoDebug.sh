#!/usr/bin/env bash

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR/.." || exit 1

# Use Unix Makefiles by default; override with CMAKE_GENERATOR when needed.
CMAKE_GENERATOR="${CMAKE_GENERATOR:-Unix Makefiles}"

cmake -B Build -S . \
    -DCMAKE_BUILD_TYPE=Debug \
    -G "$CMAKE_GENERATOR" \
    -DAETHER_RUNTIME_CHECK=1 \
    -DAETHER_ENABLE_DEBUG_LOG=1 \
    -DCMAKE_INSTALL_PREFIX=Packages \
    -DAETHER_BUILD_DEMO=1 \
    -DAETHER_BUILD_TESTS=1

cmake -B DummyBuild -S . \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -G "$CMAKE_GENERATOR" \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DAETHER_RUNTIME_CHECK=1 \
    -DAETHER_ENABLE_DEBUG_LOG=1 \
    -DAETHER_BUILD_DEMO=1 \
    -DAETHER_BUILD_TESTS=1

cp DummyBuild/compile_commands.json ./compile_commands.json


