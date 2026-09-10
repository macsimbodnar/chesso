#!/usr/bin/env bash
#
# Re-derives S206's answer: the two commits that moved `truncation_scan`'s
# counts between S076 and 2026-09-10, and the mechanism behind each.
#
#   adocs/data/S206_truncation_drift.sh [CORPUS]
#
# CORPUS defaults to .tuning/selfplay_v2_dedup.tsv, which is untracked and
# 706 MB. Without it there is nothing to re-derive and the script says so.
#
# Six readings and two counterfactuals, each a fresh build in a throwaway
# worktree: eight builds and eight 50 s scans, about 20 minutes. Every expected
# number below is what this machine measured on 2026-09-10; a mismatch is a
# finding, not a rounding difference, because the scan is deterministic (two
# runs over the same input printed the same three counts).
#
# The two movers, and why each is the count it is:
#
#   883c255  S104 gave build/ CHESSO_ARCH=native, so -march=native lets GCC
#            contract `mobility[t] * params[...] + sum` in the *model* into an
#            FMA and the model's double moves by an ulp. Only the 2.0 column
#            sees it, because a residual is a multiple of 1/24 and 2.0 = 48/24
#            is a value rows sit exactly on, where 2.8 is not. Proof is the
#            counterfactual: the same commit with -ffp-contract=off reads
#            135399 again.
#
#   21b4a21  S085's SPSA vector raised LAZY_EVAL_MARGIN from 150 to 184.
#            `eval_model::evaluate` clamps the tapered mobility-plus-king-safety
#            sum at that margin and so does `evaluate()`, so while the clamp
#            binds both sides return the margin *exactly* and the taper's
#            truncation residual is not there to measure. Raising the margin
#            unclamps positions and the residual reappears. Proof is the
#            counterfactual: HEAD with the margin put back to 150 reads
#            134408 / 99 / 30, which is this commit's parent's reading.
#
# ecd735e -- S161's change to what `load_FEN` keeps -- was the candidate the
# step was opened on and it moves nothing: it lands after 21b4a21, and 21b4a21
# already reads what HEAD reads.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CORPUS="${1:-$ROOT/.tuning/selfplay_v2_dedup.tsv}"

if [ ! -r "$CORPUS" ]; then
  echo "no corpus at $CORPUS -- nothing to re-derive" >&2
  exit 1
fi

# Resolved in the repository and not in the worktree: `git -C "$WT" checkout
# HEAD` means the worktree's own HEAD, so passing the word through would
# re-measure whatever the previous reading left checked out. It did, on the
# first run of this script, and the row still said "as recorded".
HEAD_SHA="$(git -C "$ROOT" rev-parse HEAD)"

WORK="$(mktemp -d "${TMPDIR:-/tmp}/S206.XXXXXX")"
WT="$WORK/wt"
FAILED=0

cleanup()
{
  git -C "$ROOT" worktree remove --force "$WT" > /dev/null 2>&1 || true
  rm -rf "$WORK"
}
trap cleanup EXIT

git -C "$ROOT" worktree add --detach "$WT" "$HEAD_SHA" > /dev/null 2>&1

# reading SHA LABEL EXPECT_PAST_2 EXPECT_PAST_MIN EXPECT_MAX [EXTRA_CMAKE_FLAGS]
reading()
{
  local sha="$1" label="$2" e2="$3" e8="$4" emax="$5" flags="${6:-}"
  local summary hits

  git -C "$WT" checkout --quiet --force --detach "$sha"
  git -C "$WT" checkout --quiet -- .

  # The margin counterfactual is a source edit, not a flag, so it is applied
  # after the checkout and thrown away by the next one.
  if [ "$label" = "HEAD, margin back at 150" ]; then
    grep -q 'X(LAZY_EVAL_MARGIN,  "LazyEvalMargin",  184,' "$WT/src/search_params.hpp" ||
      { echo "the margin is no longer 184 at $sha: this counterfactual is vacuous" >&2; exit 2; }
    sed -i 's/X(LAZY_EVAL_MARGIN,  "LazyEvalMargin",  184,/X(LAZY_EVAL_MARGIN,  "LazyEvalMargin",  150,/' \
      "$WT/src/search_params.hpp"
  fi

  rm -rf "$WT/build"
  cmake -S "$WT" -B "$WT/build" -DCMAKE_BUILD_TYPE=Release ${flags:+-DCMAKE_CXX_FLAGS="$flags"} > /dev/null 2>&1
  cmake --build "$WT/build" -j"$(nproc)" --target truncation_scan > /dev/null 2>&1

  summary="$("$WT/build/tools/truncation_scan" --data "$CORPUS" --min 2.8 \
    2> "$WORK/summary.txt" > "$WORK/hits.txt"; tail -1 "$WORK/summary.txt")"
  hits="$(awk -F'\t' '$1=="2.875000"' "$WORK/hits.txt" | wc -l | tr -d ' ')"

  local got
  got="$(echo "$summary" | sed -E 's/.* ([0-9]+) past 2\.0, ([0-9]+) past .*/\1 \2/')"
  got="$got $hits"

  printf '%-12s %-34s %-22s' "$(git -C "$WT" rev-parse --short HEAD)" "$label" "$got"

  if [ "$got" = "$e2 $e8 $emax" ]; then
    printf 'as recorded\n'
  else
    printf 'EXPECTED %s %s %s\n' "$e2" "$e8" "$emax"
    FAILED=1
  fi
}

printf '%-12s %-34s %-22s %s\n' sha what "past2.0 past2.8 at2.875" verdict

reading 77d7450 "S076's fit pasted"            135399  99 30
reading 38ad4fe "883c255^, before the arch flag" 135399 99 30
reading 883c255 "S104: CHESSO_ARCH=native"     134408  99 30
reading 3488506 "21b4a21^, margin still 150"   134408  99 30
reading 21b4a21 "S085: margin 150 -> 184"      138331 105 33
reading "$HEAD_SHA" "HEAD"                     138331 105 33

echo
echo "counterfactuals:"
reading 883c255 "S104, -ffp-contract=off"      135399  99 30 "-ffp-contract=off"
reading "$HEAD_SHA" "HEAD, margin back at 150" 134408  99 30

echo
if [ "$FAILED" -eq 0 ]; then
  echo "8 of 8 reproduced"
else
  echo "a reading moved -- see the EXPECTED lines above"
fi
exit "$FAILED"
