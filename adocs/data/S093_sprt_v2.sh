#!/usr/bin/env bash
#
# S093 verdict 2: quiet history carried from one `go` to the next within a game.
# Candidate is the working tree at `40f5b56` plus the hoist and the two clear
# rules in `src/chesso.cpp`, `src/search.cpp`, `src/data_structures.hpp` and
# `src/uci.hpp`. Reference is `40f5b56`, verdict 1's commit -- not `6e0afa0`, or
# the run would measure both verdicts at once.
#
# WHAT IT CHANGES. The table left `search_state_t`, which is built fresh per
# `go`, for a `quiet_history_t` the UCI layer owns beside the transposition
# table and wires in by pointer. Killers, countermoves and the PV stay per-`go`;
# only quiet history is carried. Two things clear it and nothing else does:
# `ucinewgame`, and a `go` whose root does not descend from the root the last
# search ran from.
#
# THE SECOND RULE IS NOT WHAT THE PUBLISHED RECORD PRICED, and this is stated
# here rather than after the fact. Lynx PR #637 measured **+12.5 +/- 6.6 over
# 6419 games, LOS 100 %** for "stop clearing quiet history" and nothing about
# descendancy; the same project's PR #457 measured the alternatives to keeping,
# all worse -- decay to 50 % between searches -16.6, decay to 90 % -6.3 and H0,
# always clear -12.1 and H0. The descendant rule is chesso's own hardening
# against a GUI that reuses one process across games without `ucinewgame`. It
# should be inert in a fastchess match -- fastchess replays
# `position startpos moves ...` in full before every `go`, so the previous root
# is always in the chain and the keep path always engages, which
# tests/test_engine.cpp drives directly -- but "should be inert" is an argument
# and not a measurement, so it is written down as a candidate explanation for a
# zero before any game is played.
#
# NODE COUNTS ARE UNCHANGED AND THAT IS THE POINT, NOT AN ABSENCE OF EFFECT.
# `tools/search_bench.py` at depth 13 reads 944870 / 5202441 / 533229, identical
# to `40f5b56`, because the tool gives each of its three positions as a bare FEN
# and each therefore fails the descendant test and starts from an empty table.
# The effect this run measures only exists across the moves of one game, which
# no fixed-position benchmark can see. INV-6 is not available either way: the
# change alters play by construction inside a game.
#
# BOUNDS.
#
#   elo0=0 elo1=5 alpha=0.05 beta=0.05
#
# fastchess.sh's default for a change claimed to gain, and the coordinator's
# decision rather than this script's. The published prior is +12.5 +/- 6.6 at
# LOS 100 % over 6419 games, which this pair resolves comfortably if it
# transfers. Note that verdict 1's prior was +37.5 and +28 and measured +10.73,
# so a third of +12.5 would be about +4 -- inside the indifference region and a
# real risk of the no-verdict branch below. That risk is accepted knowingly; it
# is the cost of a bound pair whose H1 means something keepable.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> persistence gains 5 Elo or more. Keep it, and record the
#                  point estimate as biased upward by the early stop (DEC-063):
#                  the claim the run establishes is the pre-registered one.
#   H0 accepted -> it does not. Recorded as DEC-019's next entry with the
#                  number. Do not keep it on the strength of +12.5.
#   No verdict   -> recorded as no verdict, which is a real result (S005, S006,
#                  S015). **Two candidate explanations, both named now.** First,
#                  the effect may simply be smaller here than +12.5, as verdict
#                  1's was -- and `elo0=0 elo1=5` cannot separate a small gain
#                  from zero, which is DEC-063's whole hazard and cost S068
#                  6 h 36 m over 9036 games for nothing. Second, the descendant
#                  rule is this step's own addition and could be clearing a
#                  table the published change would have kept; the fastchess
#                  path argues it cannot, and instrumenting how often the keep
#                  path engages in a real match is the way to settle it rather
#                  than reasoning about it further. Re-running at other bounds,
#                  and dropping or keeping the descendant rule on its own, are
#                  both recorded decisions and not automatic actions (DEC-063,
#                  S021's precedent).
#
# Everything except -sprt mirrors fastchess.sh's live engine block: tc 8+0.08,
# Hash 16, Threads 1, the UHO book, -repeat, -recover, the adjudication pair,
# and -check-mate-pvs (S145). AGENTS.md 12 and DEC-061: a terminal marker on
# every exit path, success and failure both.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

marked=0
fail()
{
  marked=1
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

book="books/UHO_Lichess_4852_v1.epd"
book_format="epd"
candidate="build/src/chesso"
tc="8+0.08"
sprt="elo0=0 elo1=5 alpha=0.05 beta=0.05"
rounds=20000
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"
mate_pv_check="-check-mate-pvs"
ref_sha="40f5b56"

all_cores="$(sysctl -n hw.physicalcpu 2> /dev/null || nproc)"
concurrency="${CONCURRENCY:-$all_cores}"

stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_sprt_s093_v2_${stamp}}"
mkdir -p "$outdir" || fail "cannot create $outdir"
logfile="$outdir/fastchess.log"
pgnfile="$outdir/games.pgn"

# fastchess appends, so a second run into the same directory would mix two
# matches into one census. Refuse instead.
[[ ! -e "$pgnfile" ]] \
  || fail "$pgnfile already exists; fastchess appends and this run's census would mix two matches"

[[ -x "$candidate" ]] || fail "no candidate at $candidate, build it first"
[[ -r "$book" ]] || fail "no book at $book, fetch it with books/fetch_book.sh"

# Snapshot the candidate before the first game. fastchess spawns the binary
# once per game, so pointing it at build/ lets a rebuild swap the engine under
# the match -- which has already happened once and reported +301 Elo that meant
# nothing. DEC-020.
snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-candidate-s093v2.XXXXXX")" \
  || fail "mktemp for the candidate snapshot"
cp "$candidate" "$snapshot" || fail "cannot snapshot $candidate"
chmod +x "$snapshot"

# The trap preserves the status it was entered with: left to itself it would
# end in a successful `rm -f` and report an abort as a clean exit.
trap 'status=$?; rm -f "$snapshot"
      if ((status != 0 && marked == 0)); then
        echo "SPRT-RUN-FAILED: exited $status" >&2
      fi
      exit $status' EXIT

# The reference, from the ref and in its own worktree, exactly as fastchess.sh
# builds it. Already present when this launches; the block is here so the run
# is reproducible from the script alone.
ref_dir="$(git rev-parse --show-toplevel)/.ref-builds/$ref_sha"
reference="$ref_dir/build/src/chesso"

if [[ ! -x "$reference" ]]; then
  echo "Building reference $ref_sha ..."
  git worktree add --detach "$ref_dir" "$ref_sha" > /dev/null \
    || fail "git worktree add for $ref_sha"
  cmake -S "$ref_dir" -B "$ref_dir/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > /dev/null \
    || fail "cmake configure for $ref_sha"
  cmake --build "$ref_dir/build" --target chesso \
    -j"$(sysctl -n hw.logicalcpu 2> /dev/null || nproc)" > /dev/null \
    || fail "cmake build for $ref_sha"
fi

busy="$(ps -A -o %cpu= | awk '{ total += $1 } END { printf "%.0f", total }')"
if ((busy > 60)); then
  echo "WARNING: about ${busy}% of a core is already busy. A timed match on a"
  echo "         loaded machine measures the load as much as the engine."
  echo
fi

echo "step       S093 verdict 2, quiet history carried across go"
echo "candidate  $(git rev-parse --short HEAD)$(git diff --quiet || echo ' + uncommitted changes')"
echo "reference  $ref_sha"
echo "tc $tc  hash 16  concurrency $concurrency of $all_cores cores"
echo "book       $(basename "$book")"
echo "bounds     $sprt"
echo "out        $outdir"
echo

start=$(date +%s)

fastchess \
  -engine cmd="$snapshot" name=candidate-s093-history-persist \
  -engine cmd="$reference" name="ref-$ref_sha" \
  -openings file="$book" format="$book_format" order=random \
  -each tc="$tc" option.Hash=16 option.Threads=1 \
  -sprt $sprt model=normalized \
  $adjudication \
  $mate_pv_check \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -recover \
  -pgnout file="$pgnfile" \
  -log file="$logfile"
status=$?

end=$(date +%s)

# The census is read from the PGN and not from the log: -log is WARN-and-above
# and comes back empty, and an empty log is indistinguishable from one that was
# never written (S089).
count()
{
  grep -hc "$1" "$pgnfile" 2> /dev/null || true
}

games="$(count '^\[Result ')"
draws="$(count '^\[Result "1/2-1/2"\]')"
forfeits="$(count '^\[Termination "time forfeit"\]')"
games="${games:-0}"
draws="${draws:-0}"
forfeits="${forfeits:-0}"

echo
echo "elapsed_seconds=$(( end - start ))"
echo "=== $games games in $pgnfile ==="
grep -h '^\[Termination' "$pgnfile" 2> /dev/null | sort | uniq -c | sort -rn || true
if ((games > 0)); then
  awk -v g="$games" -v d="$draws" -v f="$forfeits" 'BEGIN {
    printf "draws     %d of %d, %.1f %%\n", d, g, 100 * d / g
    printf "decisive  %d of %d, %.1f %%\n", g - d, g, 100 * (g - d) / g
    printf "forfeits  %d of %d, %.2f %%\n", f, g, 100 * f / g
  }'
fi

if ((status != 0)); then fail "fastchess exited $status"; fi

echo "SPRT-RUN-DONE s093v2 $outdir"
