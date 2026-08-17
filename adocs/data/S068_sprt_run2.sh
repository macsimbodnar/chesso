#!/usr/bin/env bash
# S068, second run. Same two binaries as run 1, different SPRT bounds.
#
# Run 1 used elo0=0 elo1=5 and did not converge: 9066 games, Elo 3.19 +/- 5.60,
# LLR drifting 0.59-0.86 against a +/-2.94 bound. The true effect sits BETWEEN
# the two hypotheses, so neither can be accepted and the LLR random-walks.
# S021's step file names this failure mode independently and prescribes the fix.
#
# elo0=-5 elo1=5 can terminate: H0 and H1 now straddle the observed effect
# rather than bracketing it from one side.
#
# PRE-REGISTERED INTERPRETATION, written before the run:
#   H1 accepted -> margin 75 is not a regression. Combined with 14.5 % fewer
#                  nodes at depth 9, that is the case for shipping 75.
#   H0 accepted -> margin 75 is a regression. Keep 100, revert both edits.
#   No verdict   -> record as no verdict, keep 100. Do not re-run a third time
#                  without a decisions.md entry: two inconclusive runs is the
#                  measurement saying the effect is too small to matter here.
#
# Everything except -sprt mirrors fastchess.sh exactly (fastchess.sh:118-134).
# DEC-061: prints a terminal marker as its last action.
set -u
cd /home/max/ws/chesso || { echo "S068_SPRT2_FAILED cd"; exit 1; }

SP=/tmp/claude-1000/-home-max-ws-chesso/15ad9dc1-2aed-4b32-aca7-69494270d848/scratchpad
candidate="$SP/chesso-candidate-m75"
reference=".ref-builds/7f15ac4/build/src/chesso"

# Snapshot the candidate so a later rebuild cannot swap the engine mid-match.
cp build/src/chesso "$candidate" && chmod +x "$candidate"

start=$(date +%s)

fastchess \
  -engine cmd="$candidate" name=candidate-m75 \
  -engine cmd="$reference" name=ref-7f15ac4-m100 \
  -openings file=books/8moves_v3.pgn format=pgn order=random \
  -each tc=10+0.2 option.Hash=16 option.Threads=1 \
  -sprt elo0=-5 elo1=5 alpha=0.05 beta=0.05 model=normalized \
  -draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400 \
  -rounds 20000 \
  -repeat \
  -concurrency 12 \
  -recover \
  -pgnout file="$SP/S068_run2.pgn" \
  -log file="$SP/S068_run2_fastchess.log"
status=$?

end=$(date +%s)
echo
echo "elapsed_seconds=$(( end - start ))"
if [[ $status -eq 0 ]]; then
  echo "S068_SPRT2_DONE status=0"
else
  echo "S068_SPRT2_FAILED status=$status"
fi
