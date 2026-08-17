#!/usr/bin/env bash
set -euo pipefail

# S075's lambda sweep. Six fits over the same corpus, the same split and the
# same budget, differing only in --lambda.
#
#   target = lambda * sigma(K * score) + (1 - lambda) * result
#
# WHAT DECIDES WHAT, WRITTEN BEFORE THE RUN. DEC-064, DEC-019.
#
#   1. The sweep is decided on **held-out error against the game result** -- the
#      `wdl` column, printed at every report and stamped into each emitted
#      header. Not on the training objective: each lambda trains against a
#      different target, so those numbers are not comparable with each other.
#      At lambda 0 the two are the same function.
#   2. The winner is the lambda whose emitted vector has the lowest `wdl`
#      validation error. **Exactly one** vector then goes to an SPRT against the
#      weights that ship. A sweep of six SPRTs is six chances at a false
#      positive at alpha 0.05, which is the trap `eval_tuning_strategy.md`
#      section 7 names and the step file's own argument against it.
#   3. **If lambda 0 wins, that is the verdict and there is no SPRT.** The
#      recorded outcome is that the blend does not predict held-out outcomes
#      better than the game outcome alone does, at these weights and on this
#      corpus. A zero is recorded as zero (AGENTS.md section 0).
#   4. A lambda 0 refit of the shipping weights is not itself a shipping
#      candidate under this step. It continues S065's unexhausted budget, which
#      is a different change from the blend and is not what S075 asks about.
#   5. Loss is not Elo. The winner's held-out figure decides which vector is
#      measured, never whether it is kept.
#
# THE FREEZE IS NOT OPTIONAL. `--freeze tempo,piece_placement` is DEC-057's
# regime and the shipping constants were fitted inside it. Without it the fit
# re-fires the tempo tolerance and `POSITIONAL_ROOM` guards, and the verdict
# stops being about lambda at all.
#
# THE SCORES ARE NOT THIS ENGINE'S. `.tuning/selfplay_v2.tsv` was written by
# 08:13 on 2026-08-14 at `dd57c3c`, the last commit before it finished and the
# one that carries `9c947d1`'s `--allow-tactical`, at 100000 nodes per move with
# seed 20260814. Its evaluator is `a2f0065`'s to the constant -- `git diff
# dd57c3c a2f0065 -- src/eval_tables.hpp src/evaluation.cpp` is empty -- which is
# the weight set S065's fit replaced. That is the only thing keeping
# lambda 1 off the fixed point `eval_tuning_strategy.md` section 1 describes.
# S082 and S083 regenerate the corpus and the mitigation expires with them.
#
# Detached, and it prints its own terminal marker as its last action so a watcher
# exits on the run rather than on a turn boundary. DEC-061.

root="$(cd "$(dirname "$0")/../.." && pwd)"
corpus="$root/.tuning/selfplay_v2.tsv"
tuner="$root/build/tools/tuner"
out="$root/adocs/data/S075_fits"
log="$root/adocs/data/S075_lambda_sweep.log"

# Same for every run, so lambda is the only difference between them. S065's
# budget and seed, and K is fitted per run rather than pinned: fit_k() reads the
# game result whatever --lambda says (DEC-064), so six runs fitting the same K
# from the same rows is the rule holding rather than a wasted pass.
common="--freeze tempo,piece_placement --seed 1 --validation 0.1
        --epochs 5000 --report 100 --patience 20 --threads 12"

mkdir -p "$out"

if [[ ! -r "$corpus" ]]; then
  echo "no corpus at $corpus" >&2
  exit 1
fi

if [[ ! -x "$tuner" ]]; then
  echo "no tuner at $tuner; build it first" >&2
  exit 1
fi

{
  echo "S075 lambda sweep"
  echo "started   $(date -Iseconds)"
  echo "engine    $(git -C "$root" rev-parse --short HEAD)$(git -C "$root" diff --quiet || echo '-dirty')"
  echo "corpus    $corpus"
  echo "rows      $(wc -l < "$corpus")"
  echo "budget    $common"
  echo
} > "$log"

for lambda in 0.0 0.25 0.5 0.7 0.85 1.0; do
  tag="l${lambda/./_}"
  started=$(date +%s)

  echo "=== lambda $lambda ===" >> "$log"

  # shellcheck disable=SC2086
  "$tuner" --data "$corpus" --lambda "$lambda" \
      --out "$out/S075_fit_$tag.hpp" $common >> "$log" 2>&1

  echo "elapsed $(( $(date +%s) - started ))s" >> "$log"
  echo >> "$log"
done

{
  echo "=== summary, best wdl validation per lambda ==="
  grep -H "^// lambda\|^// wdl error\|^// error" "$out"/S075_fit_*.hpp || true
  echo
  echo "finished  $(date -Iseconds)"
  echo "S075_SWEEP_DONE"
} >> "$log"
