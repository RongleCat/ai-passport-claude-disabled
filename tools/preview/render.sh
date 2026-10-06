#!/bin/sh
set -eu
cd "$(dirname "$0")"

if ! command -v cmake >/dev/null 2>&1; then
    found=""
    if [ -d "$HOME/.espressif/tools/cmake" ]; then
        found=$(find "$HOME/.espressif/tools/cmake" -path '*/CMake.app/Contents/bin/cmake' -type f 2>/dev/null | head -1 || true)
    fi
    if [ -z "$found" ] && [ -x /opt/homebrew/bin/cmake ]; then
        found=/opt/homebrew/bin/cmake
    fi
    if [ -z "$found" ]; then
        echo "cmake not found (PATH, Homebrew, or ~/.espressif/tools/cmake)" >&2
        exit 127
    fi
    PATH="$(dirname "$found"):$PATH"
    export PATH
fi

cmake -S . -B build
cmake --build build -j
./build/ban_preview out
