#!/usr/bin/env bash
set -euo pipefail

# S219 -- which opening book gives this harness the cheapest verdict.
#
# PRE-REGISTRATION. Written before the run started, 2026-09-11 evening, and
# not edited after it; DEC-182 is the decision this run serves and the rule
# below is fixed here so the reading cannot be chosen after the numbers are
# seen.
#
# WHAT IS PLAYED. One binary -- the HEAD build of chesso, snapshotted before
# the first game -- against itself at a fixed time handicap, on each of four
# books, for a fixed number of games. No SPRT: this is a calibration in the
# DEC-143 sense and never a verdict about the engine. The books:
#
#   uho4852     books/UHO_Lichess_4852_v1.epd   unbalanced, what the harness plays today
#   popularpos  books/popularpos_lichess_v3.epd balanced-leaning, 200000 positions
#   noob3       books/noob_3moves.epd           balanced, shallow, 150932 positions
#   uho4060v4   books/UHO_4060_v4.epd           unbalanced, 241670 positions
#
# All four are CC0 from official-stockfish/books (adocs/data/S219_book_survey.md).
#
# THE HANDICAP IS ONE DOUBLING, NOT FISHTEST'S 30 %. fishtest compares books by
# playing a build against itself at about 30 % time odds and reading the
# normalized Elo each book returns for that same, known handicap: the book
# that returns the larger nElo resolves the same real difference in fewer
# games. The method is kept; the handicap is larger because of power. At
# 3000 games a book this harness resolves logistic Elo to about +/- 11 (S065's
# +/- 10.47 over 3396 games scaled by 1/sqrt(N)), about +/- 16 nElo. A 30 %
# handicap is roughly half a doubling; a full doubling gives roughly twice
# the signal for the same error bar, and the selection metric below goes as
# nElo squared, so the relative error of the metric halves. The full side
# plays 8+0.08, the harness control since S105; the half side plays 4+0.04.
# Both sides Hash=16, Threads=1, as every verdict here.
#
# THE SELECTION RULE. For each book B, pooled over both passes:
#
#   nElo_B    normalized Elo of `full` over `half`, from the pentanomial
#   G_B       games an hour, from wall time
#   M_B  =  nElo_B^2 * G_B
#
# An SPRT at fixed nElo bounds needs games in proportion to 1/nElo^2 of the
# true difference, and hours in proportion to that over throughput, so M_B is
# proportional to verdicts per hour and 1/M_B to hours per verdict. The pick is
# the book with the largest M_B. Ties: two books whose M differ by less than
# one combined standard error are tied, and a tie is broken toward the
# balanced book (the owner's stated leaning, DEC-182). If no candidate beats
# uho4852 by more than one standard error and the tied candidate is not
# balanced, uho4852 stays: no harness change on noise.
#
# Reported beside the metric, for interpretation and not for selection: draw
# rate, the share of pairs the opening decided (1:1), forfeits, and the
# logistic Elo of the doubling itself -- which is a figure S132 and S151 can
# use, labelled as measured at this control on this book.
#
# TWO PASSES, COUNTERBALANCED. Each book plays 750 rounds (1500 games) twice:
# pass 1 in the order above, pass 2 in reverse, so a machine that drifts over
# the night (thermal, a background job) hits the books at different hours and
# the two halves of each book are a reproducibility check on each other. One
# seed per pass, the same seed for every book in that pass.
#
# ABORT RULES. fastchess exiting non-zero ends the run with the FAILED marker.
# A match with time forfeits above 1 % of its games voids that book's reading
# for that pass -- recorded in results.tsv and the run continues. More than
# 60 % of a core busy before the first game refuses to start.
#
# COST. 12000 games. At the S198 throughput of 2277 an hour on the UHO book,
# and less on a balanced book whose games are longer (S105 measured x1.20),
# about 5.5 to 6.5 hours. Ceiling for the watcher: 14 hours.
#
# MARKERS. `S219-MATCH-DONE <pass> <book>` after each match,
# `S219-COMPARE-DONE <outdir>` at the end, `S219-COMPARE-FAILED <why>` on any
# other exit, so a detached run can be watched by something that terminates
# (AGENTS.md WATCHERS).
#
# OVERRIDES, for a smoke test only: ROUNDS (750), PASSES (2), BOOKS (all
# four, space separated keys), OUT (the output directory). A run with any of
# them set is not the pre-registered run and results.tsv says so in its
# header.

marked=0
completed=0
fail()
{
  marked=1
  echo "S219-COMPARE-FAILED: $*" >&2
  exit 1
}
trap 'status=$?
      [[ -n "${snapshot:-}" ]] && rm -f "$snapshot"
      if ((marked == 0 && (status != 0 || completed == 0))); then
        echo "S219-COMPARE-FAILED: exited $status" >&2
        ((status != 0)) || status=1
      fi
      exit $status' EXIT

repo="$(cd -- "$(dirname -- "$0")/../.." && pwd)"
candidate="$repo/build/src/chesso"
[[ -x "$candidate" ]] || fail "no binary at $candidate"

declare -A book_file=(
  [uho4852]="$repo/books/UHO_Lichess_4852_v1.epd"
  [popularpos]="$repo/books/popularpos_lichess_v3.epd"
  [noob3]="$repo/books/noob_3moves.epd"
  [uho4060v4]="$repo/books/UHO_4060_v4.epd"
)
order_default="uho4852 popularpos noob3 uho4060v4"
books="${BOOKS:-$order_default}"
for b in $books; do
  [[ -n "${book_file[$b]:-}" ]] || fail "unknown book key '$b'"
  [[ -r "${book_file[$b]}" ]] || fail "no book at ${book_file[$b]}"
done

rounds="${ROUNDS:-750}"
passes="${PASSES:-2}"
case "$rounds$passes" in *[!0-9]*) fail "ROUNDS and PASSES must be unsigned integers" ;; esac
# Zero passes the digit check and plays nothing, and a detached run that played
# nothing would still print the DONE marker its watcher waits for.
((rounds > 0 && passes > 0)) || fail "ROUNDS and PASSES must be at least 1, got rounds=$rounds passes=$passes"

tc_full="8+0.08"
tc_half="4+0.04"
concurrency="${CONCURRENCY:-$(nproc)}"
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"
mate_pv_check="-check-mate-pvs"

busy="$(ps -A -o %cpu= | awk '{ total += $1 } END { printf "%.0f", total }')"
((busy <= 60)) || fail "about ${busy}% of a core is already busy; a timed match on a loaded machine measures the load"

stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_s219_book_compare_${stamp}}"
mkdir -p "$outdir"
results="$outdir/results.tsv"
[[ ! -e "$results" ]] || fail "$results exists; one directory per run"

snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-s219.XXXXXX")"
cp "$candidate" "$snapshot"
chmod +x "$snapshot"

head_sha="$(git -C "$repo" rev-parse --short HEAD)"
dirty=""
git -C "$repo" diff --quiet HEAD || dirty="  + uncommitted changes"
governor="$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2> /dev/null || echo unknown)"
preregistered="yes"
# CONCURRENCY is in this test with the three the header names, because it sets
# games an hour and games an hour is half the selection metric.
[[ -z "${ROUNDS:-}${PASSES:-}${BOOKS:-}${CONCURRENCY:-}" ]] || preregistered="NO -- overrides set, not the pre-registered run"

{
  echo "# S219 book comparison  $stamp"
  echo "# engine     $head_sha$dirty  ($(git -C "$repo" show -s --date=short --format=%cd HEAD))"
  echo "# fastchess  $(fastchess --version 2>&1 | head -1)"
  echo "# tc         full $tc_full  half $tc_half   Hash=16 Threads=1  concurrency $concurrency"
  echo "# rounds     $rounds per match, $passes pass(es), books: $books"
  echo "# governor   $governor"
  echo "# preregistered  $preregistered"
  printf 'pass\tbook\tseed\tstart_epoch\tend_epoch\twall_s\tgames\tdraws\tforfeits\tgames_per_hour\tvoided\tpgn\n'
} > "$results"

echo "S219 book comparison  $stamp"
echo "engine     $head_sha$dirty"
echo "tc         full $tc_full  half $tc_half  hash 16  concurrency $concurrency of $(nproc)"
echo "rounds     $rounds per match, $passes pass(es)"
echo "books      $books"
echo "governor   $governor"
echo "out        $outdir"
echo

count()
{
  grep -hc "$1" "$2" 2> /dev/null || true
}

for ((pass = 1; pass <= passes; pass++)); do
  seed="2026091100$pass"
  if ((pass % 2 == 1)); then
    sequence="$books"
  else
    sequence="$(printf '%s\n' $books | tac | tr '\n' ' ')"
  fi
  for b in $sequence; do
    mdir="$outdir/pass${pass}_${b}"
    mkdir -p "$mdir"
    pgn="$mdir/games.pgn"
    # fastchess appends. A book named twice in BOOKS would write two matches
    # into one file with colliding round numbers, which the pair reader then
    # pairs across matches. fastchess.sh refuses the same way.
    [[ ! -e "$pgn" ]] || fail "$pgn exists; two matches would be mixed into one file"
    echo "=== pass $pass  $b  seed $seed  $(date '+%F %T') ==="
    start="$(date +%s)"
    fastchess \
      -engine cmd="$snapshot" name=full tc="$tc_full" \
      -engine cmd="$snapshot" name=half tc="$tc_half" \
      -openings file="${book_file[$b]}" format=epd order=random \
      -srand "$seed" \
      -each option.Hash=16 option.Threads=1 \
      $adjudication \
      $mate_pv_check \
      -rounds "$rounds" \
      -repeat \
      -concurrency "$concurrency" \
      -recover \
      -report penta=true \
      -event "chesso S219 book=$b pass=$pass srand=$seed" \
      -pgnout file="$pgn" nodes=true timeleft=true \
      -log file="$mdir/fastchess.log" \
      > "$mdir/stdout.log" 2>&1 \
      || fail "fastchess exited $? on pass $pass $b; see $mdir/stdout.log"
    end="$(date +%s)"
    wall=$((end - start))
    games="$(count '^\[Result ' "$pgn")"; games="${games:-0}"
    draws="$(count '^\[Result "1/2-1/2"\]' "$pgn")"; draws="${draws:-0}"
    forfeits="$(count '^\[Termination "time forfeit"\]' "$pgn")"; forfeits="${forfeits:-0}"
    gph="$(awk -v g="$games" -v w="$wall" 'BEGIN { if (w > 0) printf "%.1f", 3600 * g / w; else print "0" }')"
    # `-check-mate-pvs` and the option warnings ("doesn't have option Hash",
    # which would silently void the regime) are fastchess's only report that
    # something was wrong, and they land in a per-match log nobody opens. A
    # count on the console is what makes them visible while the run is going.
    warnings="$(count 'Warning[;:]' "$mdir/stdout.log")"; warnings="${warnings:-0}"
    voided="no"
    if ((games > 0)) && ((forfeits * 100 > games)); then voided="yes-forfeits"; fi
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
      "$pass" "$b" "$seed" "$start" "$end" "$wall" "$games" "$draws" "$forfeits" "$gph" "$voided" "$pgn" >> "$results"
    # The last summary block fastchess printed, for the eye; the reader uses the
    # PGN. Exactly the five lines of one block: fastchess reprints the block
    # every rating interval -- 50 times over S198's 1000 games -- so a wider
    # tail puts the previous interval's Ptnml line above the final one, which is
    # a number waiting to be quoted as the result.
    grep -E "^(Results of |Elo:|LOS:|Games:|Ptnml)" "$mdir/stdout.log" | tail -5 || true
    echo "games $games  draws $draws  forfeits $forfeits  warnings $warnings  wall ${wall}s  $gph games/h  voided $voided"
    echo "S219-MATCH-DONE $pass $b"
    echo
  done
done

completed=1
echo "S219-COMPARE-DONE $outdir"
