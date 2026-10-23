#!/bin/bash
# S291. Trace one node of the multicut mined row's tree: the node reached from the
# row's root by the given moves, at the ply that many moves deep, in one cold
# search_fen() at DEPTH (adocs/data/S291_row.py), through a scratch build patched by
# adocs/data/S289_trace.py and then adocs/data/S291_trace.py (never shipped).
#
#   adocs/data/S291_trace_line.sh SCRATCH DEPTH [Name=Value ...] -- [move ...]
#
# SCRATCH is the patched worktree; LIB (default build-tune/src/libchesso_engine.a)
# picks its build. The FENs come from python-chess applying the moves
# (adocs/data/S291_fens.py), never a mental board. $S291_NO_RFP, $S291_NO_NULL,
# $S291_CENSUS and $S291_RFP_GUARD pass through to the engine.
REPO=$1; D=$2; shift 2
HERE=$(cd "$(dirname "$0")" && pwd)
ROW="6k1/5pp1/6np/1PP1p3/1Q2P1P1/3r1q1P/1B1R4/6K1 b - - 0 42"
SETS=()
while [ "$1" != "--" ]; do SETS+=(--set "$1"); shift; done; shift
N=$#
F=$(~/.venv/chess/bin/python "$HERE/S291_fens.py" "$ROW" "$@" | awk -F'|' '{print $NF}') || exit 1
echo "# line: ${*:-(root)} -- ply $N, $F"
S289_TRACE="$F" ~/.venv/chess/bin/python "$HERE/S291_row.py" --repo "$REPO" \
  --lib "${LIB:-build-tune/src/libchesso_engine.a}" --tune --lo "$D" --hi "$D" "${SETS[@]}" 2>&1 |
  python3 "$HERE/S291_trace_filter.py" 0 "$N" | grep -v '^ITER'
