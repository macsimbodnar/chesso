#!/usr/bin/env bash
# S122 phase 2, DEC-352's re-read: each part's decided pair read on the fixed set
# at K refit at the candidate's constants, beside the K the screen used. A check
# and not a decision (DEC-352, DEC-353): it refits nothing and moves no golden.
#
#   adocs/data/S122_kcheck.sh            # from the worktree that holds the paste
#
# Written before it ran, 2026-11-05.
#
# K. The screen fitted K once at M by a zero-epoch tuner run at the landing's
# constants on the full corpus (S122_screen.sh k_probe, OUT/k/full.k). Here
# the same command runs twice: with the screen's own frozen tuner, which must
# reproduce OUT/k/full.k to four places, and with this tree's build-tune tuner,
# whose constants are the paste's (`.tuning/verify_fit.py` must find every
# parameter the fit's). The tuner's fit_k() is a 40-step ternary search over K
# at fixed constants, and DEC-353 refits nothing, so one round is the fixed
# point: a second round would search the same function again.
#
# THE READING. For each part P1 to P7 its reading's DECIDED line names the fit
# and the fit it was decided against (P1's against its start, the landing's
# shipped constants). Each fit's full vector is read on the fixed set's qs view
# by the margin_census of the tree its tree.txt names (the landing's, or the
# variant's built by the screen), at margin 275 and at each K: `--games`, the
# summed sse over the summed rows. d = mse(against) - mse(fit), at each K.
#
# THE STOP (the brief, DEC-352): a part kept by DEC-353 -- P1, P2, P4, P5 --
# whose d at the refit K is at or under its screen m, or of the other sign from
# its screen d, prints STOP, the coordinator's before any SPRT. P3, P6 and P7
# (dropped by the screen or by DEC-353) are read and decide nothing.
#
# Prints S122-KCHECK-DONE, S122-KCHECK-STOP or S122-KCHECK-FAILED last.

set -uo pipefail

root="$(cd "$(dirname "$0")/../.." && pwd)"
main=$(dirname "$(git -C "$root" rev-parse --path-format=absolute --git-common-dir)")
OUT="${OUT:-$main/.tuning/s122/screen}"
PREPARED="${PREPARED:-$main/.tuning/s122/prepared/prepared.txt}"
WORK="${WORK:-$main/.tuning/s122p2/kcheck}"
FIT="$OUT/fits/1245_e20000/full.hpp"
MARGIN=275
FREE_GROUPS="ks_checks,ks_weak,ks_defenders,ks_storm,ks_curve"
KEPT="P1 P2 P4 P5"

fail() { echo "$*"; echo "S122-KCHECK-FAILED"; exit 1; }
# Inside a command substitution: the reason to stderr, and the caller fails.
die() { echo "$*" >&2; exit 1; }
check_copy() {
  [[ "$(sha256sum "$1" | cut -d' ' -f1)" == "$(cat "$1.sha256")" ]] ||
    die "refused: $1 is not the binary the screen froze"
}

mkdir -p "$WORK" || fail "cannot create $WORK"
corpus=$(sed -n 's/^corpus //p' "$PREPARED" | head -n 1)
holdout=$(awk '$1 == "view" && $2 == "qs" { print $3 }' "$PREPARED")
[[ -f "$corpus" && -f "$holdout" ]] || fail "refused: $PREPARED names no corpus or qs view"
[[ "$(sha256sum "$corpus" | cut -d' ' -f1)" == "$(sed -n 's/^sha256 //p' "$PREPARED" | head -n 1)" ]] ||
  fail "refused: $corpus is not the corpus $PREPARED records"
[[ "$(awk '$1 == "MARGIN" { print $2 }' "$OUT/margin/M.txt")" == "$MARGIN" ]] ||
  fail "refused: the screen's margin is not $MARGIN"

echo "S122 kcheck, $(date -Iseconds), tree $root at $(git -C "$root" rev-parse --short HEAD)"
python3 "$root/.tuning/verify_fit.py" "$FIT" --root "$root" || fail "refused: $root does not hold DEC-353's fit"

# K at the landing's constants by the screen's frozen tuner, then at the paste's.
k_of() {
  local tuner="$1" name="$2"
  "$tuner" --data "$corpus" --holdout "$holdout" --out "$WORK/$name.hpp" --margin "$MARGIN" \
      --epochs 0 --only "$FREE_GROUPS" --threads 12 > "$WORK/$name.log" 2>&1 ||
    die "the K probe $name failed: $WORK/$name.log"
  sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$WORK/$name.log"
}
check_copy "$OUT/bin/tuner" || fail "refused: the screen's tuner"
k_screen=$(cat "$OUT/k/full.k")
k_landing=$(k_of "$OUT/bin/tuner" k_landing) || fail "the landing's K probe failed"
[[ "$k_landing" == "$k_screen" ]] ||
  fail "refused: the screen's tuner reads K $k_landing, and OUT/k/full.k holds $k_screen"
k_cand=$(k_of "$root/build-tune/tools/tuner" k_candidate) || fail "the candidate's K probe failed"
[[ -n "$k_cand" ]] || fail "the candidate's K probe printed no K"
echo "K at the landing's constants (the screen's): $k_screen, reproduced by the screen's tuner"
echo "K refit at the candidate's constants (1245 at $MARGIN): $k_cand, one round (constants fixed)"
echo "corpus $corpus, 40-step ternary search on its training rows; read on $holdout"

census_for() {
  local tree="$1" bin
  case "$tree" in
    landing) bin="$OUT/bin/margin_census" ;;
    p6|p7) bin=$(ls -d "$OUT"/variant_"$tree"_*/bin/margin_census | head -n 1) ;;
    *) die "no census for tree $tree" ;;
  esac
  check_copy "$bin"
  echo "$bin"
}

# mse of one vector (or the landing's shipped constants, "start") at one K.
mse() {
  local dir="$1" k="$2" tag="$3" bin args=()
  if [[ "$dir" == start ]]; then
    bin=$(census_for landing) || exit 1
  else
    bin=$(census_for "$(cat "$dir/tree.txt")") || exit 1
    args=(--vector "$dir/full.vector")
  fi
  "$bin" --data "$holdout" "${args[@]}" --k "$k" --loss-margin "$MARGIN" \
      --games "$WORK/$tag.games" --threads 12 > "$WORK/$tag.census" 2>&1 ||
    die "margin_census failed on $dir: $WORK/$tag.census"
  awk '!/^#/ { rows += $3; sse += $4 } END { printf "%.10f\n", sse / rows }' "$WORK/$tag.games"
}

stop=0
printf '\n%-4s %-8s %-10s %-11s %-12s %-12s %-12s %s\n' part outcome m "screen d" "d at K scr" "d at K cand" "rows" ""
for part in P1 P2 P3 P4 P5 P6 P7; do
  reading="$OUT/read/$part.txt"
  line=$(grep '^DECIDED ' "$reading") || fail "no DECIDED line in $reading"
  fit=$(awk '{ print $3 }' <<< "$line")
  pred=$(awk '{ print $5 }' <<< "$line")
  dscreen=$(awk '{ print $7 }' <<< "$line")
  m=$(sed -n 's/.* s [0-9.]*, m \([0-9.]*\),.*/\1/p' "$reading" | head -n 1)
  outcome=dropped
  [[ " $KEPT " == *" $part "* ]] && outcome=kept
  ds=()
  for k in "$k_screen" "$k_cand"; do
    a=$(mse "$pred" "$k" "${part}_pred_$k") || fail "$part: the census of $pred failed"
    b=$(mse "$fit" "$k" "${part}_fit_$k") || fail "$part: the census of $fit failed"
    ds+=("$(awk -v a="$a" -v b="$b" 'BEGIN { printf "%+.7f", a - b }')")
  done
  rows=$(awk '!/^#/ { r += $3 } END { print r }' "$WORK/${part}_fit_$k_cand.games")
  flag=""
  if [[ "$outcome" == kept ]]; then
    if awk -v d="${ds[1]}" -v m="$m" -v s="$dscreen" \
         'BEGIN { exit !(d <= m || (d > 0) != (s > 0)) }'; then
      flag="STOP"; stop=1
    fi
  else
    flag="(a reading)"
  fi
  printf '%-4s %-8s %-10s %-11s %-12s %-12s %-12s %s\n' "$part" "$outcome" "$m" "$dscreen" "${ds[0]}" "${ds[1]}" "$rows" "$flag"
done

if (( stop )); then
  echo "STOP: a part DEC-353 keeps reads at or under its m, or turns sign, at K refit at the candidate; the coordinator's before any SPRT"
  echo "S122-KCHECK-STOP"
  exit 1
fi
echo "S122-KCHECK-DONE"
