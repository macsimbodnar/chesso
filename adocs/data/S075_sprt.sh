#!/usr/bin/env bash
# S075, the tuner's score/result blend. Candidate is the working tree carrying
# the ONE fitted weight vector the lambda sweep selected, pasted over
# src/eval_tables.hpp and src/evaluation.cpp. Reference is the commit whose
# constants ship before the paste, set in REF below.
#
# ONE VECTOR, NOT SIX. adocs/data/S075_lambda_sweep.sh fitted six lambdas and
# ranked them on held-out error against the game result (DEC-064). Only the
# winner is played. Six SPRTs at alpha 0.05 expect one false positive by chance,
# which is eval_tuning_strategy.md section 7 and the step file's own argument.
# Held-out error selects; this run decides. Loss is not Elo.
#
# BOUNDS, and why they are not fastchess.sh's. DEC-063: bounds straddle the
# expected effect. S065 refitted 827 constants on this corpus and measured
# +21.10 +/- 10.47, but that was a fit against hand-written constants; this one
# is a refit of already-fitted weights with one term of the label changed, so the
# expectation is small and of unknown sign. elo0=0 elo1=5 cannot terminate on an
# effect that sits between the hypotheses -- 6 h 36 m and 9036 games for nothing,
# S068 run 1. elo0=-5 elo1=5 can.
#
# WHAT IS ALREADY ON THE RECORD, before a game is played: the winning lambda,
# its held-out error against the game result, and the same figure at the
# constants it started from, all three in the emitted header's `// lambda` and
# `// wdl error` lines and in adocs/data/S075_lambda_sweep.log.
#
# PRE-REGISTERED INTERPRETATION, written before the run:
#   H1 accepted -> the blended fit is not a regression of 5 Elo or more. Keep
#                  the paste, keep the winning lambda as the tuner's documented
#                  setting for this corpus, and record the verdict.
#   H0 accepted -> it is a regression. Revert the paste in full, keep --lambda
#                  as a flag that ships at 0, and record it as DEC-019's next
#                  entry: a held-out improvement that did not survive a clock.
#   No verdict   -> record as no verdict and revert the paste. Two failure modes
#                  share that outcome and neither ships: the effect is too small
#                  to separate, or the bounds were wrong again. Do not re-run at
#                  other bounds without a decisions.md entry.
#
# WHAT THIS RUN CANNOT ATTRIBUTE, stated before it is read. The candidate is a
# 5000-epoch refit of the shipping weights AND a changed label. The incumbent had
# neither, so a positive verdict does not separate the blend from the extra
# training. The separation is carried by the sweep's own lambda 0 row, at the same
# budget and the same split, as loss and not as Elo. A second verdict against that
# row would settle it and costs another night; DEC-064 records why it was not
# spent.
#
# Everything except -sprt mirrors fastchess.sh (fastchess.sh:118-134).
# DEC-061: prints a terminal marker as its last action.
set -u
cd /home/max/ws/chesso || { echo "S075_SPRT_FAILED cd"; exit 1; }

REF="${REF:?set REF to the commit whose constants ship before the paste}"

SP=/tmp/claude-1000/-home-max-ws-chesso/31b4760d-5d41-4338-925e-72f6bbd4ba8f/scratchpad
candidate="$SP/chesso-candidate-s075"
reference=".ref-builds/$REF/build/src/chesso"

if [[ ! -x "$reference" ]]; then
  echo "S075_SPRT_FAILED no reference build at $reference" >&2
  exit 1
fi

# Snapshot the candidate so a later rebuild cannot swap the engine mid-match.
cp build/src/chesso "$candidate" && chmod +x "$candidate"

start=$(date +%s)

fastchess \
  -engine cmd="$candidate" name=candidate-s075-blend \
  -engine cmd="$reference" name="ref-$REF" \
  -openings file=books/8moves_v3.pgn format=pgn order=random \
  -each tc=10+0.2 option.Hash=16 option.Threads=1 \
  -sprt elo0=-5 elo1=5 alpha=0.05 beta=0.05 model=normalized \
  -draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400 \
  -rounds 20000 \
  -repeat \
  -concurrency 12 \
  -recover \
  -pgnout file="$SP/S075_sprt.pgn" \
  -log file="$SP/S075_fastchess.log"
status=$?

end=$(date +%s)
echo
echo "elapsed_seconds=$(( end - start ))"
if [[ $status -eq 0 ]]; then
  echo "S075_SPRT_DONE status=0"
else
  echo "S075_SPRT_FAILED status=$status"
fi
