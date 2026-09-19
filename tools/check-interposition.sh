#!/usr/bin/env bash
#
# Verify a shim never calls a function it interposes

set -euo pipefail

usage() {
    echo "usage: $0 <shim.so> [allowlist]" >&2
    exit 2
}

[ $# -ge 1 ] || usage
shim=$1
allowlist=${2:-}
[ -f "$shim" ] || { echo "$0: no such file: $shim" >&2; exit 2; }

# Symbol names carry version suffixes (foo@GLIBC_2.17, foo@@GLIBC_2.17)
strip_version() { sed 's/@.*//'; }

exported_symbols() {
    nm -D --defined-only --format=posix "$shim" \
        | awk '$2 ~ /^[TWi]$/ { print $1 }' \
        | strip_version | sort -u
}

plt_called_symbols() {
    readelf -rW "$shim" \
        | awk '/JUMP_SLO/ { print $5 }' \
        | strip_version | sort -u
}

permitted_symbols() {
    [ -n "$allowlist" ] && [ -f "$allowlist" ] || return 0
    grep -vE '^[[:space:]]*(#|$)' "$allowlist" | sort -u
}

unexpected_dependencies() {
    readelf -dW "$shim" \
        | awk '/\(NEEDED\)/ { gsub(/[][]/, "", $5); print $5 }' \
        | grep -vE '^(libc|libstdc\+\+|libgcc_s|libm|ld-linux)' || true
}

exported=$(exported_symbols)
called=$(plt_called_symbols)

self_calls=$(comm -12 <(echo "$exported") <(echo "$called"))
self_calls=$(comm -23 <(echo "$self_calls") <(permitted_symbols | sort -u))
self_calls=$(echo "$self_calls" | grep -v '^$' || true)

deps=$(unexpected_dependencies)
status=0

if [ -n "$self_calls" ]; then
    echo "FAIL: $shim calls functions it interposes" >&2
    echo "$self_calls" | sed 's/^/  /' >&2
    status=1
fi

# This check cannot see interposed calls made from inside other libraries
# -> the dependency set has to stay small to be meaningful 
if [ -n "$deps" ]; then
    echo "FAIL: $shim depends on libraries outside the trusted set" >&2
    echo "$deps" | sed 's/^/  /' >&2
    status=1
fi

if [ $status -eq 0 ]; then
    echo "ok: $shim interposes $(echo "$exported" | grep -c .) symbols, calls none of them"
fi

exit $status
