#!/usr/bin/env bash
#
# Run a program under the shim once per seed and check the exit codes:
# every run must exit with one of <codes>, and every code must occur

set -euo pipefail

usage() {
    echo "usage: $0 <shim.so> <seed-count> <codes> <program> [args...]" >&2
    echo "  codes: comma-separated exit codes, e.g. 0,120" >&2
    exit 2
}

[ $# -ge 4 ] || usage
shim=$1 seeds=$2 codes=$3
shift 3

IFS=, read -ra expected <<< "$codes"
declare -A seen=()

for seed in $(seq 1 "$seeds"); do
    status=0
    HEDDLE_SEED=$seed LD_PRELOAD=$shim timeout 10 "$@" > /dev/null 2>&1 || status=$?
    if [[ ! " ${expected[*]} " =~  $status  ]]; then
        echo "seed $seed: exit $status, expected one of {$codes}" >&2
        exit 1
    fi
    seen[$status]=1
done

for code in "${expected[@]}"; do
    if [ -z "${seen[$code]:-}" ]; then
        echo "exit $code never occurred in $seeds seeds" >&2
        exit 1
    fi
done