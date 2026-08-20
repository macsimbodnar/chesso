#!/usr/bin/env bash
# S105, the harness regime change. Measures what the change to the instrument
# costs and buys, on the instrument itself, before any verdict is taken with it.
#
# WHY TWO RUNS AND NOT ONE. The accepts asks for games per hour "before and
# after". The recorded before-figure is ~23 games/min, taken from two real
# SPRT runs (DEV_MANUAL "Play games"), and those stopped when their LLR
# crossed rather than at a fixed count -- a throughput number read off a run
# whose length was decided by its own result is not a clean baseline. So the
# old regime is re-run here at a fixed round count, on the same machine, in
# the same hour, against the new one. The ratio is then a measurement.
#
# WHY NO -sprt. Both sides are the same binary: an A/A match. There is no
# effect to detect and an SPRT on one would random-walk to its round limit,
# so the run would end at an unpredictable count and the throughput and
# decisive-rate figures would each come from a different sample size. Fixed
# rounds gives both numbers the same denominator.
#
# WHAT EACH RUN ANSWERS
#   before  10+0.2, books/8moves_v3.pgn (balanced), Hash 16 -- the regime that
#           shipped until this step. Its decisive rate is the balanced-book
#           number for THIS engine, which has never been measured here; Pohl's
#           91 % draws is a figure for engines 600 points stronger.
#   after   8+0.08, UHO_Lichess_4852_v1.epd (unbalanced), Hash 16 -- DEC-083
#           and DEC-088.
#
# Everything else is held fixed and is fastchess.sh's: adjudication, -repeat,
# -recover, concurrency, Threads 1, order=random. One change at a time does
# not apply to a two-variable instrument swap that was decided as one -- but
# the two variables are reported separately below so a later step can tell the
# control from the book.
#
# DEC-061: prints a terminal marker as its last action, success and failure
# both, so the watcher has something to exit on.

set -u

repo=/home/max/ws/chesso
cd "$repo" || { echo "S105-CALIBRATION-FAILED: cd"; exit 1; }

rounds="${ROUNDS:-500}"          # 1000 games per run
concurrency="${CONCURRENCY:-$(nproc)}"
stamp="$(date +%Y%m%d_%H%M%S)"
base="/tmp/chesso_s105_calibration_${stamp}"
mkdir -p "$base" || { echo "S105-CALIBRATION-FAILED: mkdir"; exit 1; }

adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"

# One snapshot, played against itself. Copying it once means a rebuild during
# the run cannot swap the engine under either side of either match, and means
# the two runs are byte-identical engines rather than two builds that happen
# to agree.
snapshot="$base/chesso"
cp "$repo/build/src/chesso" "$snapshot" || { echo "S105-CALIBRATION-FAILED: no candidate"; exit 1; }
chmod +x "$snapshot"

echo "S105 calibration, $(git rev-parse --short HEAD)"
echo "rounds $rounds per run, concurrency $concurrency, out $base"
echo

run_one()
{
  local label="$1" tc="$2" book="$3" fmt="$4"
  local out="$base/$label"
  mkdir -p "$out"

  local started ended elapsed
  started="$(date +%s)"

  echo "=== $label: tc=$tc book=$(basename "$book") ==="
  fastchess \
    -engine cmd="$snapshot" name=chesso-a \
    -engine cmd="$snapshot" name=chesso-b \
    -openings file="$book" format="$fmt" order=random \
    -each tc="$tc" option.Hash=16 option.Threads=1 \
    $adjudication \
    -rounds "$rounds" \
    -repeat \
    -concurrency "$concurrency" \
    -recover \
    -pgnout file="$out/games.pgn" \
    -log file="$out/fastchess.log" \
    > "$out/stdout.txt" 2>&1
  local status=$?

  ended="$(date +%s)"
  elapsed=$((ended - started))

  # The PGN and not the log: -log is WARN-and-above and comes back empty, and
  # an empty log is indistinguishable from one never written (S089). The
  # per-run directory is what makes this the run's own games -- fastchess
  # appends to -pgnout, so a shared filename mixes matches.
  local games draws decisive forfeits
  games="$(grep -hc '^\[Result ' "$out/games.pgn" 2> /dev/null || true)"
  draws="$(grep -hc '^\[Result "1/2-1/2"\]' "$out/games.pgn" 2> /dev/null || true)"
  forfeits="$(grep -hc '^\[Termination "time forfeit"\]' "$out/games.pgn" 2> /dev/null || true)"
  games="${games:-0}"; draws="${draws:-0}"; forfeits="${forfeits:-0}"
  decisive=$((games - draws))

  {
    echo "label        $label"
    echo "tc           $tc"
    echo "book         $(basename "$book")"
    echo "fastchess    exit $status"
    echo "games        $games"
    echo "elapsed      ${elapsed}s"
    echo
    echo "--- terminations ---"
    grep -h '^\[Termination' "$out/games.pgn" 2> /dev/null | sort | uniq -c | sort -rn
    echo
    if ((games > 0)); then
      awk -v g="$games" -v d="$draws" -v c="$decisive" -v f="$forfeits" -v e="$elapsed" 'BEGIN {
        printf "draws        %d of %d, %.1f %%\n", d, g, 100 * d / g
        printf "decisive     %d of %d, %.1f %%\n", c, g, 100 * c / g
        printf "forfeits     %d of %d, %.2f %%\n", f, g, 100 * f / g
        if (e > 0) {
          printf "games/min    %.1f\n", g * 60 / e
          printf "games/hour   %.0f\n", g * 3600 / e
        }
      }'
    fi
  } | tee "$out/summary.txt"
  echo
}

# The forfeit count is the first thing read, per the accepts: 8+0.08 leaves
# MOVE_OVERHEAD_MS 50 about 2.5x less room per move than 10+0.2 did, and a
# regime that loses games on the clock is not a cheaper regime.
run_one before "10+0.2" "$repo/books/8moves_v3.pgn"                pgn
run_one after  "8+0.08" "$repo/books/UHO_Lichess_4852_v1.epd"      epd

echo "=== summaries ==="
cat "$base/before/summary.txt" "$base/after/summary.txt"
echo "S105-CALIBRATION-DONE $base"
