#!/usr/bin/env bash
#
# Use clang-tidy to run over src/ and tests/
# extra arguments go to run-clang-tidy (eg, -fix)

set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
build=$root/build/lint

cmake -S "$root" -B "$build" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON > /dev/null

run-clang-tidy -p "$build" -quiet "$@" "^$root/(src|tests)/.*\.cpp$"
