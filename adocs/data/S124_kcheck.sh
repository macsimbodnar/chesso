#!/usr/bin/env bash
# S124 phase 2b, DEC-352's re-read: each part's decided pair read on the fixed set at K
# refit at the candidate's constants, beside the K the screen read it at; and the
# tablebase guard re-read for the candidate against the landed tree at one K (DEC-357).
# Checks and not decisions (DEC-352): nothing is refit and no golden moves.
# adocs/data/S122_kcheck.sh's form.
#
#   adocs/data/S124_kcheck.sh            # from the worktree that holds the paste
#
# Written before it ran, 2026-11-11.
#
# THE CANDIDATE. The screen's decided fit for F2 (OUT/fits/f1_f2/full, its decided
# round's fit.hpp, K 1.3248), F1's and F2's ten cells free, with mop-up's six from P1
# (adocs/data/S124_p1_final.txt, DEC-358). The header is composed here from those two
# files and `.tuning/verify_fit.py` must find all 1190 parameters of it in the tree.
#
# K. K_0, the screen's K at the identity (OUT/k/full.k, 1.2171), is reproduced first by
# the screen's frozen tuner: a zero-epoch run at the landed constants on the full corpus,
# the qs view held out, `--only endgame_scale`, the screen's `k` stage's command. Then the
# same command with this tree's build/ tuner, whose constants are the candidate's, gives
# K refit at the candidate; this tree's S124_k_refit (the screen's instrument, built
# against this tree's trace library, no --vector) must agree with it within 1e-4. The
# tuner's fit_k() is a 40-step ternary search at fixed constants and nothing is refit,
# so one round is the fixed point.
#
# THE READING. For each part F1 to F3 its reading's DECIDED line names the fit, its
# predecessor and the part's decided K. Each vector is read on the fixed set's qs view by
# the screen's frozen margin_census at margin 275 (`--games`, the summed sse over the
# summed rows), at the part's decided K and at the candidate's: d = mse(pred) - mse(fit).
# The census reads the engine's exact score where the screen read the tuner's smooth
# one, so d at the part's K is not the screen's d (S122_kcheck.sh says the same).
# Beside, deciding nothing: mop-up's own d, F2's decided vector against the candidate
# (this tree's census, its shipped weights), at the candidate's K.
#
# THE STOP (the brief, DEC-352): a part kept by the screen -- F1, F2 -- whose d at the
# candidate's K is at or under its screen m, or of the other sign from its screen d,
# prints STOP, the coordinator's before any SPRT. F3 (dropped by the guard) is read and
# decides nothing.
#
# THE GUARD, DEC-357: the screen's guard rows (OUT/guard/guard.tsv, adocs/data/S124_guard.py's
# rows of five men or fewer, labelled by the tables at each row's own clock), the landed
# tree (the screen's census, its shipped weights) against the candidate (this tree's),
# both at the candidate's K, by the screen's frozen S124_screen.py guard-read; the same
# at K_0 beside, deciding nothing. A refusal prints STOP.
#
# Prints S124-KCHECK-DONE, S124-KCHECK-STOP or S124-KCHECK-FAILED last.

set -uo pipefail

root="$(cd "$(dirname "$0")/../.." && pwd)"
main=$(dirname "$(git -C "$root" rev-parse --path-format=absolute --git-common-dir)")
OUT="${OUT:-$root/.tuning/s124/screen}"
PREPARED="${PREPARED:-$main/.tuning/s122/prepared/prepared.txt}"
WORK="${WORK:-$root/.tuning/s124/kcheck}"
P1FINAL="$root/adocs/data/S124_p1_final.txt"
MARGIN=275
THREADS=12
KEPT="F1 F2"

fail() { echo "$*"; echo "S124-KCHECK-FAILED"; exit 1; }
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
for b in tuner margin_census S124_screen.py; do check_copy "$OUT/bin/$b" || fail "refused: $b"; done

echo "S124 kcheck, $(date -Iseconds), tree $root at $(git -C "$root" rev-parse --short HEAD)"

# The candidate's header: F2's decided round's fit.hpp with P1's two lines.
read -r r2 _ _ < <(sed 's/^r\([0-9]*\) K /\1 K /' "$OUT/fits/f1_f2/full/decided")
fit_hpp="$OUT/fits/f1_f2/full/r$r2/fit.hpp"
python3 - "$fit_hpp" "$P1FINAL" "$WORK/candidate.hpp" <<'EOF' || fail "refused: the candidate's header was not composed"
import re, sys
fit, final, out = sys.argv[1:]
text = open(fit).read()
lines = [l for l in open(final).read().splitlines() if l.startswith("constexpr int mopup_")]
assert len(lines) == 2, lines
for l in lines:
    name = re.match(r"constexpr int (\w+)\[", l).group(1)
    pat = re.compile(r"^constexpr int %s\[MOPUP_FEATURE_COUNT\] = \{[^}]*\};" % name, re.M)
    text, n = pat.subn(lambda m: l, text)
    assert n == 1, (name, n)
open(out, "w").write(text)
EOF
python3 "$root/.tuning/verify_fit.py" "$WORK/candidate.hpp" --root "$root" ||
  fail "refused: $root does not hold the candidate, F2's fit with P1's mop-up"

# K at the landed constants by the screen's frozen tuner, then at the candidate's.
k_probe() {
  local tuner="$1" name="$2"
  "$tuner" --data "$corpus" --holdout "$holdout" --out "$WORK/$name.hpp" --epochs 0 \
      --only endgame_scale --threads "$THREADS" > "$WORK/$name.log" 2>&1 ||
    die "the K probe $name failed: $WORK/$name.log"
  sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$WORK/$name.log"
}
k_zero=$(cat "$OUT/k/full.k")
k_landing=$(k_probe "$OUT/bin/tuner" k_landing) || fail "the landing's K probe failed"
[[ "$k_landing" == "$k_zero" ]] ||
  fail "refused: the screen's tuner reads K $k_landing, and OUT/k/full.k holds $k_zero"
k_cand=$(k_probe "$root/build/tools/tuner" k_candidate) || fail "the candidate's K probe failed"
[[ -n "$k_cand" ]] || fail "the candidate's K probe printed no K"
g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I "$root/src" -I "$root/tools" -I "$root/tests" \
    "$root/adocs/data/S124_k_refit.cpp" "$root/build/src/libchesso_engine_trace.a" -lpthread \
    -o "$WORK/S124_k_refit" || fail "S124_k_refit did not build against this tree"
"$WORK/S124_k_refit" --data "$corpus" --threads "$THREADS" > "$WORK/k_candidate.refit" 2>&1 ||
  fail "S124_k_refit at the candidate failed: $WORK/k_candidate.refit"
k_refit=$(sed -n 's/^K \([0-9.]*\)$/\1/p' "$WORK/k_candidate.refit")
awk -v a="$k_cand" -v b="$k_refit" 'BEGIN { d = a - b; if (d < 0) d = -d; exit !(a > 0 && d <= 1e-4 + 1e-12) }' ||
  fail "refused: at the candidate the tuner's K $k_cand and S124_k_refit's $k_refit disagree"
echo "K at the identity (the screen's K_0): $k_zero, reproduced by the screen's tuner"
echo "K refit at the candidate's constants: $k_cand (S124_k_refit $k_refit), one round (constants fixed)"
echo "corpus $corpus, the tuner's ternary search on its training rows; read on $holdout"

# mse of one vector (empty: the binary's own weights) at one K, by the census BIN.
mse() {
  local bin="$1" vector="$2" k="$3" tag="$4" args=()
  [[ -n "$vector" ]] && args=(--vector "$vector")
  "$bin" --data "$holdout" "${args[@]}" --k "$k" --loss-margin "$MARGIN" \
      --games "$WORK/$tag.games" --threads "$THREADS" > "$WORK/$tag.census" 2>&1 ||
    die "margin_census failed on $tag: $WORK/$tag.census"
  awk '!/^#/ { rows += $3; sse += $4 } END { printf "%.10f\n", sse / rows }' "$WORK/$tag.games"
}
vector_of() {
  local name="$1" r
  [[ "$name" == landed ]] && { echo ""; return 0; }
  read -r r _ _ < <(sed 's/^r\([0-9]*\) K /\1 K /' "$OUT/fits/$name/full/decided")
  echo "$OUT/fits/$name/full/r$r/fit.vector"
}

stop=0
printf '\n%-4s %-8s %-10s %-11s %-8s %-12s %-12s %-9s %s\n' part outcome m "screen d" "K part" "d at K part" "d at K cand" rows ""
for part in F1 F2 F3; do
  reading="$OUT/read/$part.txt"
  line=$(grep '^DECIDED ' "$reading") || fail "no DECIDED line in $reading"
  fit=$(awk '{ print $3 }' <<< "$line")
  pred=$(awk '{ print $5 }' <<< "$line")
  dscreen=$(awk '{ print $7 }' <<< "$line")
  kpart=$(awk '{ print $9 }' <<< "$line")
  m=$(sed -n 's/.* s [0-9.]*, m \([0-9.]*\),.*/\1/p' "$reading" | head -n 1)
  outcome=$(sed -n 's/^PART-OUTCOME //p' "$reading" | head -n 1)
  vfit=$(vector_of "$fit"); vpred=$(vector_of "$pred")
  ds=()
  for k in "$kpart" "$k_cand"; do
    a=$(mse "$OUT/bin/margin_census" "$vpred" "$k" "${part}_pred_$k") || fail "$part: the census of $pred failed"
    b=$(mse "$OUT/bin/margin_census" "$vfit" "$k" "${part}_fit_$k") || fail "$part: the census of $fit failed"
    ds+=("$(awk -v a="$a" -v b="$b" 'BEGIN { printf "%+.7f", a - b }')")
  done
  rows=$(awk '!/^#/ { r += $3 } END { print r }' "$WORK/${part}_fit_$k_cand.games")
  flag=""
  if [[ " $KEPT " == *" $part "* ]]; then
    [[ "$outcome" == kept ]] || fail "refused: $part is in KEPT and its reading says $outcome"
    if awk -v d="${ds[1]}" -v m="$m" -v s="$dscreen" \
         'BEGIN { exit !(d <= m || (d > 0) != (s > 0)) }'; then
      flag="STOP"; stop=1
    fi
  else
    flag="(a reading)"
  fi
  printf '%-4s %-8s %-10s %-11s %-8s %-12s %-12s %-9s %s\n' "$part" "$outcome" "$m" "$dscreen" "$kpart" "${ds[0]}" "${ds[1]}" "$rows" "$flag"
done

# Beside, deciding nothing: mop-up's own d on the fixed set, F2's vector against the
# candidate's shipped weights, both at the candidate's K.
a=$(mse "$OUT/bin/margin_census" "$(vector_of f1_f2)" "$k_cand" mopup_pred) || fail "mop-up: the census of f1_f2 failed"
b=$(mse "$root/build/tools/margin_census" "" "$k_cand" mopup_cand) || fail "mop-up: the candidate's census failed"
echo
echo "beside, deciding nothing: mop-up, F2's decided vector against the candidate at K $k_cand: d $(awk -v a="$a" -v b="$b" 'BEGIN { printf "%+.7f", a - b }')"

# The guard, DEC-357: the landed tree against the candidate on the guard's rows, at the
# candidate's K; at K_0 beside.
guard_census() {
  local bin="$1" k="$2" tag="$3"
  "$bin" --data "$OUT/guard/guard.tsv" --k "$k" --games "$WORK/$tag.games" \
      --threads "$THREADS" > "$WORK/$tag.census" 2>&1 ||
    die "margin_census on the guard failed: $WORK/$tag.census"
}
[[ "$(sha256sum "$OUT/guard/guard.tsv" | cut -d' ' -f1)" == "$(cat "$OUT/guard/guard.tsv.done")" ]] ||
  fail "refused: $OUT/guard/guard.tsv is not the file the screen built"
guard_census "$OUT/bin/margin_census" "$k_cand" guard_landed_kcand &&
  guard_census "$root/build/tools/margin_census" "$k_cand" guard_cand_kcand &&
  guard_census "$OUT/bin/margin_census" "$k_zero" guard_landed_k0 &&
  guard_census "$root/build/tools/margin_census" "$k_zero" guard_cand_k0 || fail "the guard's census failed"
echo
python3 "$OUT/bin/S124_screen.py" guard-read "$WORK/guard_landed_kcand.games" "$WORK/guard_cand_kcand.games" \
    --label "the candidate (F1, F2 and mop-up) against the landed tree, both at the candidate's K $k_cand" \
    | tee "$WORK/guard.txt"
(( PIPESTATUS[0] == 0 )) || fail "guard-read failed"
python3 "$OUT/bin/S124_screen.py" guard-read "$WORK/guard_landed_k0.games" "$WORK/guard_cand_k0.games" --beside \
    --label "the candidate against the landed tree, both at K_0 $k_zero" | tee -a "$WORK/guard.txt"
grep -q '^GUARD refused ' "$WORK/guard.txt" && stop=1

if (( stop )); then
  echo "STOP: a kept part reads under its m or with the other sign at the candidate's K, or the guard refuses the candidate; the coordinator's before any SPRT"
  echo "S124-KCHECK-STOP"
  exit 0
fi
echo "S124-KCHECK-DONE"
