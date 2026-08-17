#!/usr/bin/env bash
set -euo pipefail

# S075, second pass: does the game-result error dip inside the first 100 epochs?
#
# WHY THIS RUN EXISTS. The sweep reported every 100 epochs, and `--report` sets
# the checkpoint grid as well as the log cadence -- the kept vector can only be
# one of the reported ones. On `adocs/data/S075_lambda_sweep.log` at lambda 0.25
# almost all of the movement toward the blend target happens before the first
# report:
#
#   start:      train 0.074006  validation 0.074249  wdl 0.118457
#   epoch 100   train 0.073714  validation 0.073894  wdl 0.118616
#   epoch 200   train 0.073705  validation 0.073886  wdl 0.118636
#
# The blend's own held-out error moves 0.074249 -> 0.073894 and is then flat to
# six digits, while the wdl column rises monotonically from epoch 100 on. Adam at
# `--lr 1.0` moves each of 817 free parameters by about the step size per epoch,
# so 100 epochs is a long way in parameter space. If any non-zero lambda ever
# beat the incumbent's 0.118457, it did so in epochs 1..99, where the sweep did
# not look. Recording a zero without answering that is recording a zero about the
# grid rather than about the blend.
#
# WHAT IS DIFFERENT, and only this: `--report 5 --epochs 200`. 40 sample points,
# 20 of them inside the first 100 epochs. Everything else -- corpus, seed, split,
# freeze, thread count -- is the sweep's, so the two runs are comparable and the
# lambda 0 row is the same control.
#
# `--patience 40` cannot fire inside a 200-epoch budget by construction, which is
# deliberate: this run is a scan of a region, not a fit, and it must not stop
# early and leave the region half looked at.
#
# WHAT THIS RUN CANNOT DO. 200 epochs is not a converged fit and none of the
# vectors it emits is a shipping candidate. It answers one question -- is there a
# dip -- and the answer decides what happens next:
#
#   a dip at some lambda -> that lambda earns a full fit at a fine grid, and the
#                           winner of THAT is the one vector the SPRT plays.
#                           Still one verdict (DEC-064, section 7's trap).
#   no dip anywhere      -> the sweep's zero is a zero about the blend and not
#                           about the sampling, and it is recorded as one. No
#                           SPRT is spent, per S075_lambda_sweep.sh rule 3.
#
# DEC-061: prints a terminal marker as its last action.

root="$(cd "$(dirname "$0")/../.." && pwd)"
corpus="$root/.tuning/selfplay_v2.tsv"
tuner="$root/build/tools/tuner"
out="$root/adocs/data/S075_fits_fine"
log="$root/adocs/data/S075_lambda_fine_grid.log"

common="--freeze tempo,piece_placement --seed 1 --validation 0.1
        --epochs 200 --report 5 --patience 40 --threads 12"

mkdir -p "$out"

{
  echo "S075 lambda fine grid, epochs 1..200 at report 5"
  echo "started   $(date -Iseconds)"
  echo "engine    $(git -C "$root" rev-parse --short HEAD)$(git -C "$root" diff --quiet || echo '-dirty')"
  echo "corpus    $corpus"
  echo "budget    $common"
  echo "incumbent wdl validation 0.118457 at the constants that ship"
  echo
} > "$log"

for lambda in 0.0 0.25 0.5 0.7 0.85 1.0; do
  tag="l${lambda/./_}"
  started=$(date +%s)

  echo "=== lambda $lambda ===" >> "$log"

  # shellcheck disable=SC2086
  "$tuner" --data "$corpus" --lambda "$lambda" \
      --out "$out/S075_fine_$tag.hpp" $common >> "$log" 2>&1

  echo "elapsed $(( $(date +%s) - started ))s" >> "$log"
  echo >> "$log"
done

{
  echo "finished  $(date -Iseconds)"
  echo "S075_FINE_DONE"
} >> "$log"
