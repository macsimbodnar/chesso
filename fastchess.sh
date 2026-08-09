#!/usr/bin/env bash
set -euo pipefail

# Plays the working tree against a reference build and runs an SPRT on the
# result.
#
#   ./fastchess.sh                  full SPRT, elo0=0 elo1=5, runs for hours
#   ./fastchess.sh --fast           looser bounds, few hundred games
#   REF=HEAD~1 ./fastchess.sh       pick what to measure against
#
# The reference is built from a git ref into a worktree under .ref-builds/, so
# what the number means is always attributable to a commit range. It used to be
# a binary sitting in ~/.local/bin, which drifted out of date silently and made
# every result measure an unknown amount of unrelated work.
#
# NOTE ON WHAT THIS CAN MEASURE. Games are played at a time control, so a
# change that makes the engine faster shows up here and a change that only
# reorders equal work does not. The reverse holds for fixed-nodes matches. If a
# change leaves the node count identical, this is the only tool that will see
# it; if it changes the tree, expect the result to mix the two effects.

REF="${REF:-7b4d9a4}"
book="$(dirname "$0")/books/8moves_v3.pgn"
candidate="$(dirname "$0")/build/src/chesso"
tc="10+0.2"

# Games are timed, so a game that lands on an efficiency core is a game played
# at the wrong speed. Only the performance cores count, and one is left free
# for the operating system and for whatever else is running.
perf_cores="$(sysctl -n hw.perflevel0.physicalcpu 2> /dev/null || sysctl -n hw.physicalcpu)"
concurrency="${CONCURRENCY:-$((perf_cores > 2 ? perf_cores - 1 : 1))}"

# adjudication cuts dead games, gets to a verdict faster
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"

if [[ "${1:-}" == "--fast" ]]; then
  # few hundred games
  sprt="elo0=0 elo1=10 alpha=0.10 beta=0.10"
  rounds=1500
  tag="fast"
else
  # SPRT will stop by it self
  sprt="elo0=0 elo1=5 alpha=0.05 beta=0.05"
  rounds=20000
  tag="full"
fi

logfile="/tmp/fastchess_${tag}.log"
pgnfile="/tmp/fastchess_${tag}.pgn"

if [[ ! -x "$candidate" ]]; then
  echo "No candidate at $candidate. Build it first." >&2
  exit 1
fi

# Snapshot the candidate before playing a single game.
#
# fastchess spawns the engine binary once per game, so pointing it at build/
# means a rebuild part way through silently swaps the engine under the match
# and the result becomes a mixture of two versions. That has already happened
# once. Copying it makes the match immune to whatever the working tree does
# next.
snapshot="$(mktemp -t chesso-candidate)"
cp "$candidate" "$snapshot"
chmod +x "$snapshot"
trap 'rm -f "$snapshot"' EXIT
candidate="$snapshot"

# Build the reference from the ref, in its own worktree, once per ref.
ref_sha="$(git rev-parse --short "$REF")"
ref_dir="$(git rev-parse --show-toplevel)/.ref-builds/$ref_sha"
reference="$ref_dir/build/src/chesso"

if [[ ! -x "$reference" ]]; then
  echo "Building reference $ref_sha ..."
  git worktree add --detach "$ref_dir" "$ref_sha" > /dev/null
  cmake -S "$ref_dir" -B "$ref_dir/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > /dev/null
  cmake --build "$ref_dir/build" --target chesso -j"$(sysctl -n hw.logicalcpu)" > /dev/null
fi

# A busy machine invalidates a timed match, and it is worth knowing before
# spending an hour rather than after.
busy="$(ps -A -o %cpu= | awk '{ total += $1 } END { printf "%.0f", total }')"
if ((busy > 60)); then
  echo "WARNING: about ${busy}% of a core is already busy. A timed match on a"
  echo "         loaded machine measures the load as much as the engine."
  echo "         Check with: ps aux | sort -rnk3 | head"
  echo
fi

echo "candidate  $(git rev-parse --short HEAD)$(git diff --quiet || echo ' + uncommitted changes')"
echo "reference  $ref_sha"
echo "tc $tc  concurrency $concurrency  ($perf_cores performance cores)"
echo

fastchess \
  -engine cmd="$candidate" name=candidate \
  -engine cmd="$reference" name="ref-$ref_sha" \
  -openings file="$book" format=pgn order=random \
  -each tc="$tc" option.Hash=16 option.Threads=1 \
  -sprt $sprt model=normalized \
  $adjudication \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -recover \
  -pgnout file="$pgnfile" \
  -log file="$logfile"
