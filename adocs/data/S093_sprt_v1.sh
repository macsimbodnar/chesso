#!/usr/bin/env bash
#
# S093 verdict 1: quiet history malus, gravity ageing and butterfly indexing.
# Candidate is the working tree at `6e0afa0` plus one change in
# `src/search.cpp`, `src/evaluation.cpp`, `src/data_structures.hpp` and
# `src/search_params.hpp`. Reference is `6e0afa0` itself, the commit
# immediately before the change, built into .ref-builds/ the way fastchess.sh
# builds it.
#
# WHY THE THREE ARE ONE RUN, and this is the one place "one change at a time"
# yields. DEC-087 (j): they are one published mechanism. Gravity's whole reason
# to exist is that a signed table needs a bound and a decay that a saturating
# counter does not, and butterfly is the indexing every surveyed description of
# that mechanism is written on. Measured apart, each is measured against a table
# shape it will not ship with, and the number would not transfer to the
# combination that does ship.
#
# WHAT IT CHANGES, measured before the games rather than argued. History was a
# monotone counter: a `depth*depth` bonus to the quiet that cut off, nothing to
# any other quiet, and a `std::min` at 600000 standing in for ageing. It is now
# `int16_t quiet_history[2][64][64]` updated by
#
#   entry += clamp(b) - entry * |clamp(b)| / QuietHistoryMax
#
# with `+b` to the move that cut off and `-b` to every quiet the node searched
# before it. Three consequences, each asserted in the suite rather than argued:
# the interval [-MAX, MAX] is closed under the update, so the saturation is
# deleted rather than kept next to the clamp; every update shrinks the old value
# by (1 - |b|/MAX), so ageing is by construction; and the malus span is appended
# to after the cutoff test, so the cutoff move is structurally absent from it.
#
# `tools/search_bench.py` at depth 13, deterministic across two interleaved
# passes:
#
#                    reference 6e0afa0     candidate         delta
#   midgame                     863774        944870        +9.39 %
#   kiwipete                   7248224       5202441       -28.22 %
#   tactical                    529142        533229        +0.77 %
#   total                      8641140       6680540       -22.69 %
#
# Best move unchanged at all three, c3d5 / e2a6 / d7c8q. Node throughput is down
# about 2 to 3 % -- the malus loop and a 16 KB table against 3 KB -- and wall
# time over the three positions falls 1.194 s to 0.939 s anyway. So it alters
# play by construction, INV-6 is not available, and this run is the verdict.
#
# BOUNDS, and why they are fastchess.sh's default here where S149 edited them.
#
#   elo0=0 elo1=5 alpha=0.05 beta=0.05
#
# This is the documented form for "a change claimed to gain", and unlike S149
# the prior is strong and doubly sourced: Weiss measured the quiet history malus
# at 37.49 +- 12.19 over 1228 games (commit c5d4921, PR #296) and Lynx at
# 28.0 +/- 10.3, LOS 100 % (PR #610), both SPRT-verified in independent engines
# at about this strength. S149 used the two-sided pair because the sign of its
# effect was genuinely unknown; this one is not in that position. If the effect
# is anywhere near either figure the gainer bounds terminate quickly and the
# verdict means "gains 5 Elo or more", which is the keepable claim.
#
# The hazard is real and is why all three readings are written down first:
# DEC-063 measured elo0=0 elo1=5 random-walking 6 h 36 m over 9036 games for
# nothing on S068's constant, where elo0=-5 elo1=5 returned a verdict in
# 1 h 41 m over 2312 on the same binaries.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> the mechanism gains 5 Elo or more. Keep it, ship it, and
#                  record the point estimate as biased upward by the early stop
#                  (DEC-063): what the run establishes is the pre-registered
#                  claim and not the magnitude.
#   H0 accepted -> it does not. That is DEC-019's fifth entry -- a published
#                  figure that did not transfer to this search -- and it is
#                  recorded as one, with the number, in the step file. Do not
#                  keep it on the strength of +37.5 and +28.
#   No verdict   -> recorded as no verdict, which is a real result (S005, S006,
#                  S015). Re-running at the two-sided pair is a recorded
#                  decision and not an automatic action: DEC-063, and S021 is
#                  the precedent for writing the entry first.
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
ref_sha="6e0afa0"

all_cores="$(sysctl -n hw.physicalcpu 2> /dev/null || nproc)"
concurrency="${CONCURRENCY:-$all_cores}"

stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_sprt_s093_v1_${stamp}}"
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
snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-candidate-s093v1.XXXXXX")" \
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

echo "step       S093 verdict 1, quiet history malus + gravity + butterfly"
echo "candidate  $(git rev-parse --short HEAD)$(git diff --quiet || echo ' + uncommitted changes')"
echo "reference  $ref_sha"
echo "tc $tc  hash 16  concurrency $concurrency of $all_cores cores"
echo "book       $(basename "$book")"
echo "bounds     $sprt"
echo "out        $outdir"
echo

start=$(date +%s)

fastchess \
  -engine cmd="$snapshot" name=candidate-s093-history-malus \
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

echo "SPRT-RUN-DONE s093v1 $outdir"
