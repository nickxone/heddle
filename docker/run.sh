#!/usr/bin/env sh
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
[ -t 0 ] && tty=-it || tty=
exec docker run --rm $tty \
  -v "$ROOT":/work \
  -w /work \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  heddle-dev "$@"