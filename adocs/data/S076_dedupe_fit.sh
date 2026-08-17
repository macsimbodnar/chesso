#!/usr/bin/env bash
set -euo pipefail

# S076's refit on the deduplicated corpus. One fit, one candidate, and the
# budget, seed, split and freeze are S065's and S075's -- so the only thing that
# differs from the fit that produced the shipping weights is the corpus.
#
# THE COUNT THAT DECIDED THIS RUN HAPPENS. Pre-registered in the step file
# before `corpus_dedupe` was pointed at the corpus: under 1 % of rows dropped,
# no match is spent. Measured: **11003693 rows in, 10795695 out, 207998 dropped,
# 1.8903 %**, so the threshold is met and exactly one candidate plays one SPRT.
#
# WHAT DECIDES WHAT, WRITTEN BEFORE THE FIT. DEC-065, DEC-019.
#
#   1. **One fit and one candidate.** No sweep over dedupe variants, no second
#      fit at another budget. Held-out error ranks candidates and an SPRT
#      decides one, and there is only one to rank.
#   2. **The SPRT is the verdict, not the loss.** The emitted table's held-out
#      error is over the deduplicated corpus's own rows, so it is not comparable
#      with the 0.118457 the shipping weights score on the full corpus -- a
#      different row set is a different number. What is comparable is the
#      `start` and `best` pair *inside this run*: both are measured on the same
#      held-out rows, and `start` is the shipping constants.
#   3. **If nothing moved, no match.** If `.tuning/diff_fit.py` reports no
#      constant changed between the emitted table and what ships, there is
#      nothing for a verdict to measure and the step's recorded result is the
#      dedupe count.
#   4. **The attribution is already paid for.** This candidate is a refit *and*
#      a changed corpus, and the incumbent is neither. S075's lambda 0 control
#      ran this exact budget on the *full* corpus and moved held-out error by
#      3e-06 with a dozen weights shifting by one
#      (`adocs/data/S075_fits/S075_fit_l0_0.hpp`), against a 4e-05 noise floor.
#      So whatever this run buys or costs, "5000 more epochs" is not the reason
#      and no second control is run for it.
#
# THE FREEZE IS NOT OPTIONAL. `--freeze tempo,piece_placement` is DEC-057's
# regime and the shipping constants were fitted inside it. Without it the fit
# re-fires the tempo tolerance and `POSITIONAL_ROOM` guards, and the verdict
# stops being about the corpus at all.
#
# Detached, and it prints its own terminal marker as its last action so a
# watcher exits on the run rather than on a turn boundary. DEC-061.

root="$(cd "$(dirname "$0")/../.." && pwd)"
corpus="$root/.tuning/selfplay_v2_dedup.tsv"
tuner="$root/build/tools/tuner"
out="$root/adocs/data/S076_fits"
log="$root/adocs/data/S076_dedupe_fit.log"

# S065's budget and seed, unchanged, and K is fitted from the data rather than
# pinned: the deduplicated corpus is a different sample and its own scale is
# what belongs to it.
common="--freeze tempo,piece_placement --seed 1 --validation 0.1
        --epochs 5000 --report 100 --patience 20 --threads 12 --lambda 0"

mkdir -p "$out"

if [[ ! -r "$corpus" ]]; then
  echo "no corpus at $corpus; run build/tools/corpus_dedupe first" >&2
  exit 1
fi

if [[ ! -x "$tuner" ]]; then
  echo "no tuner at $tuner; build it first" >&2
  exit 1
fi

{
  echo "S076 dedupe refit"
  echo "started   $(date -Iseconds)"
  echo "engine    $(git -C "$root" rev-parse --short HEAD)$(git -C "$root" diff --quiet || echo '-dirty')"
  echo "corpus    $corpus"
  echo "rows      $(wc -l < "$corpus")"
  echo "sha256    $(sha256sum "$corpus" | cut -d' ' -f1)"
  echo "source    .tuning/selfplay_v2.tsv, 11003693 rows, 207998 dropped, 1.8903 %"
  echo "budget    $common"
  echo
} > "$log"

started=$(date +%s)

# shellcheck disable=SC2086
"$tuner" --data "$corpus" --out "$out/S076_fit_dedup.hpp" $common >> "$log" 2>&1

{
  echo
  echo "elapsed $(( $(date +%s) - started ))s"
  echo
  echo "=== emitted header ==="
  sed -n '1,12p' "$out/S076_fit_dedup.hpp"
  echo
  echo "finished  $(date -Iseconds)"
  echo "S076_FIT_DONE"
} >> "$log"
