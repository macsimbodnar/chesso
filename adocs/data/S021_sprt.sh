#!/usr/bin/env bash
# S021, aspiration windows. Candidate is the working tree at the shipping
# schedule -- AspirationMinDepth 5, AspirationDelta 50, AspirationMaxDelta 400
# -- against 2b54a4f, which has no windows at all.
#
# BOUNDS, and why they are not fastchess.sh's. DEC-063: bounds straddle the
# effect the change is expected to have. The expectation here is +9 with an
# error bar of +/-17, one engine's reported figure and therefore direction only
# (DEC-019), so elo0=0 elo1=5 leaves the truth inside the interval it does not
# defend and the LLR random-walks -- which is exactly what it did for S068 over
# 9036 games and 6 h 36 m. elo0=-5 elo1=5 can terminate.
#
# WHAT THE SWEEP ALREADY SAID, so it is on the record before the games are
# played: adocs/data/S021_aspiration_sweep.tsv, 300 positions over three
# independent samples, node counts at depth 11. The shipping schedule costs
# 0.9248 of the nodes the feature switched off costs. Nodes at a fixed depth
# are not Elo and a 7.5 % saving is not 5 Elo; that is what this run is for.
#
# PRE-REGISTERED INTERPRETATION, written before the run:
#   H1 accepted -> aspiration windows are not a regression of 5 Elo or more.
#                  With 7.5 % fewer nodes over 300 positions, that is the case
#                  for shipping them at (5, 50, 400).
#   H0 accepted -> they are a regression. Revert the feature; the node saving
#                  did not survive contact with a clock, which is DEC-019's
#                  seventh entry and would be recorded as one.
#   No verdict   -> record as no verdict and revert. Two failure modes share
#                  that outcome and neither ships: the effect is too small to
#                  separate, or the bounds were wrong again. Do not re-run at
#                  other bounds without a decisions.md entry.
#
# Everything except -sprt mirrors fastchess.sh (fastchess.sh:118-134).
# DEC-061: prints a terminal marker as its last action.
set -u
cd /home/max/ws/chesso || { echo "S021_SPRT_FAILED cd"; exit 1; }

SP=/tmp/claude-1000/-home-max-ws-chesso/ee618ff2-3804-4999-b647-4995cd2dd15b/scratchpad
candidate="$SP/chesso-candidate-s021"
reference=".ref-builds/2b54a4f/build/src/chesso"

# Snapshot the candidate so a later rebuild cannot swap the engine mid-match.
cp build/src/chesso "$candidate" && chmod +x "$candidate"

start=$(date +%s)

fastchess \
  -engine cmd="$candidate" name=candidate-s021-asp \
  -engine cmd="$reference" name=ref-2b54a4f-noasp \
  -openings file=books/8moves_v3.pgn format=pgn order=random \
  -each tc=10+0.2 option.Hash=16 option.Threads=1 \
  -sprt elo0=-5 elo1=5 alpha=0.05 beta=0.05 model=normalized \
  -draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400 \
  -rounds 20000 \
  -repeat \
  -concurrency 12 \
  -recover \
  -pgnout file="$SP/S021_sprt.pgn" \
  -log file="$SP/S021_fastchess.log"
status=$?

end=$(date +%s)
echo
echo "elapsed_seconds=$(( end - start ))"
if [[ $status -eq 0 ]]; then
  echo "S021_SPRT_DONE status=0"
else
  echo "S021_SPRT_FAILED status=$status"
fi
