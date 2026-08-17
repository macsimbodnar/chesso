#!/usr/bin/env bash
# S076, the corpus deduplicated by zobrist key. Candidate is the working tree
# carrying the ONE fitted weight vector `adocs/data/S076_dedupe_fit.sh` emitted,
# pasted over src/eval_tables.hpp and src/evaluation.cpp. Reference is the
# commit whose constants ship before the paste, set in REF below.
#
# ONE VECTOR, AND THE COUNT THAT BOUGHT IT THE MATCH. `corpus_dedupe` dropped
# 207998 of 11003693 rows, 1.8903 %, clearing the 1 % the step file
# pre-registered before the tool was ever pointed at the corpus. One fit on the
# 10795695-row result, one candidate, one run. No sweep, so no multiple
# comparison: eval_tuning_strategy.md section 7.
#
# BOUNDS, and why they are not fastchess.sh's. DEC-063: bounds straddle the
# expected effect. S065 refitted 827 constants on this corpus and measured
# +21.10 +/- 10.47, but that was a fit against hand-written constants; this one
# is a refit of already-fitted weights with 1.9 % of the rows removed, so the
# expectation is small and of unknown sign. elo0=0 elo1=5 cannot terminate on an
# effect that sits between the hypotheses -- 6 h 36 m and 9036 games for nothing,
# S068 run 1. elo0=-5 elo1=5 can.
#
# WHAT IS ALREADY ON THE RECORD, before a game is played: the drop count, the
# repeat histogram, the byte-identical second pass, the zero collisions the
# --verify pass found, and the fit's own start-and-best pair over the
# deduplicated corpus's held-out rows. adocs/data/S076_dedupe_fit.log and the
# step file.
#
# PRE-REGISTERED INTERPRETATION, written before the run:
#   H1 accepted -> the deduplicated fit is not a regression of 5 Elo or more.
#                  Keep the paste and make the dedupe pass part of corpus
#                  preparation, which is what S082 and S083 inherit. The
#                  argument for keeping it is then the modelling one -- a
#                  repeated position stops carrying repeated weight -- with a
#                  verdict saying it costs nothing, and the step records exactly
#                  that rather than a gain it did not measure.
#   H0 accepted -> it is a regression. Revert the paste in full, keep
#                  `corpus_dedupe` as a tool and record the verdict as DEC-019's
#                  next entry: a filter the literature lists as mattering, and
#                  it did not survive a clock here.
#   No verdict   -> record as no verdict and revert the paste. Two failure modes
#                  share that outcome and neither ships: the effect is too small
#                  to separate, or the bounds were wrong again. Do not re-run at
#                  other bounds without a decisions.md entry.
#
# WHAT THIS RUN CANNOT ATTRIBUTE, stated before it is read. The candidate is a
# 5000-epoch refit AND a changed corpus, and the incumbent is neither. The
# refit half is already priced: S075's lambda 0 control ran this budget on the
# full corpus and moved held-out error by 3e-06 against a 4e-05 noise floor,
# with a dozen weights shifting by one. So the epochs are not the reason,
# whatever this verdict says.
#
# Everything except -sprt mirrors fastchess.sh (fastchess.sh:118-134).
# DEC-061: prints a terminal marker as its last action.
set -u
cd /home/max/ws/chesso || { echo "S076_SPRT_FAILED cd"; exit 1; }

REF="${REF:?set REF to the commit whose constants ship before the paste}"

SP=/tmp/claude-1000/-home-max-ws-chesso/3389f1f7-453e-442c-9f55-59f5c4c2f457/scratchpad
candidate="$SP/chesso-candidate-s076"
reference=".ref-builds/$REF/build/src/chesso"

if [[ ! -x "$reference" ]]; then
  echo "S076_SPRT_FAILED no reference build at $reference" >&2
  exit 1
fi

# Snapshot the candidate so a later rebuild cannot swap the engine mid-match.
cp build/src/chesso "$candidate" && chmod +x "$candidate"

start=$(date +%s)

fastchess \
  -engine cmd="$candidate" name=candidate-s076-dedupe \
  -engine cmd="$reference" name="ref-$REF" \
  -openings file=books/8moves_v3.pgn format=pgn order=random \
  -each tc=10+0.2 option.Hash=16 option.Threads=1 \
  -sprt elo0=-5 elo1=5 alpha=0.05 beta=0.05 model=normalized \
  -draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400 \
  -rounds 20000 \
  -repeat \
  -concurrency 12 \
  -recover \
  -pgnout file="$SP/S076_sprt.pgn" \
  -log file="$SP/S076_fastchess.log"
status=$?

end=$(date +%s)
echo
echo "elapsed_seconds=$(( end - start ))"
if [[ $status -eq 0 ]]; then
  echo "S076_SPRT_DONE status=0"
else
  echo "S076_SPRT_FAILED status=$status"
fi
