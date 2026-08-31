#!/usr/bin/env sh

set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
exec docker run --rm -it \
  -v "$ROOT":/work \
  -w /work \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  heddle-dev bash