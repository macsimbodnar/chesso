#!/usr/bin/env bash
#
# S149, the killer slot distinctness guard. Candidate is the working tree at
# `ac4c588` plus two lines in `src/search.cpp`: the killer store shifts slot 0
# into slot 1 only for a move slot 0 does not already hold. Reference is
# `ac4c588` itself, the commit immediately before the change, built into
# .ref-builds/ the way fastchess.sh builds it.
#
# WHAT IT FIXES, measured before the games rather than argued.
# 2026-08-21_adversarial-F01. The unguarded store copies slot 0 onto itself
# whenever the same quiet fails high twice at the same ply, and killers survive
# every iteration of one `go` (`src/chesso.cpp:647` builds the state once), so
# the repeat is the common case. Both slots then hold the same move and
# `src/evaluation.cpp:1160-1161` tests slot 0 first, so ORDER_KILLER_1 (800000)
# can never be awarded to a distinct move -- the second refutation falls to the
# countermove band or into history.
#
# Re-counted on the audit's own instrumentation, four counters at the audit's
# own placement, over its 11 positions at depths 12-22 and ~19 M nodes:
#
#                                                   before        after
#   stores that re-store the move in slot 0    351422/532133  330666/498618
#                                                     66.0 %        66.3 %
#   negamax nodes with both slots equal      5115505/11531069        0/11146351
#                                                     44.4 %         0.0 %
#
# The store rate is a property of the search and does not move; what moves is
# what the repeat costs. Those stores are now no-ops instead of slot-1 kills,
# and no node in the run sees a duplicated pair. Negamax nodes -3.34 %, total
# nodes over the 11 positions -2.87 % (18985913 -> 18440510), spread -73 % to
# +33 % per position, and two positions change best move with a changed score.
# So it alters play, INV-6 is not available, and this run is the verdict.
#
# BOUNDS, and why they are not fastchess.sh's default. The effect is genuinely
# two-sided. The mechanism argues positive -- a band that cannot fire now can
# -- but DEC-019's ledger is three published figures that measured 0, 0 and
# *slower*, and capture ordering, reported at about 150 Elo, measured slower on
# this code. A bound pair that cannot contain the truth random-walks: DEC-063
# measured elo0=0 elo1=5 running 6 h 36 m over 9036 games for nothing where
# elo0=-5 elo1=5 returned a verdict in 1 h 41 m over 2312, same binaries.
# Measurement capacity is the binding constraint on the plan, so:
#
#   elo0=-5 elo1=5 alpha=0.05 beta=0.05
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> the guard is not a regression of 5 Elo or more, and the sign
#                  is positive. Keep it, ship it, and record the point estimate
#                  as biased upward by the early stop (DEC-063): what the run
#                  establishes is the pre-registered claim, not the magnitude.
#   H0 accepted -> the guard costs 5 Elo or more. It is then DEC-019's fourth
#                  entry -- a published rule that measured negative here -- and
#                  it is recorded as one. Do not keep it on the strength of the
#                  argument; revert the two lines in src/search.cpp, keep the
#                  re-targeted test red-flagged as documenting a property the
#                  engine deliberately does not have, and write the decision.
#   No verdict   -> record as zero. A zero is a real result (S005, S006, S015).
#                  The guard is then kept anyway, and the reason is stated
#                  rather than implied: it is two lines, it removes a state the
#                  design does not intend, it is what CPW's Killer Heuristic
#                  page specifies, and it must be in the tree before S093
#                  rewrites this block -- a history verdict taken over a dead
#                  second slot attributes to history whatever the dead slot was
#                  costing. Do not re-run at other bounds without a
#                  decisions.md entry.
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
sprt="elo0=-5 elo1=5 alpha=0.05 beta=0.05"
rounds=20000
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"
mate_pv_check="-check-mate-pvs"
ref_sha="ac4c588"

all_cores="$(sysctl -n hw.physicalcpu 2> /dev/null || nproc)"
concurrency="${CONCURRENCY:-$all_cores}"

stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_sprt_s149_${stamp}}"
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
snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-candidate-s149.XXXXXX")" \
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

echo "step       S149, killer slot distinctness guard"
echo "candidate  $(git rev-parse --short HEAD)$(git diff --quiet || echo ' + uncommitted changes')"
echo "reference  $ref_sha"
echo "tc $tc  hash 16  concurrency $concurrency of $all_cores cores"
echo "book       $(basename "$book")"
echo "bounds     $sprt"
echo "out        $outdir"
echo

start=$(date +%s)

fastchess \
  -engine cmd="$snapshot" name=candidate-s149-killerdedupe \
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

echo "SPRT-RUN-DONE s149 $outdir"
