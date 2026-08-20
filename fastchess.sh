#!/usr/bin/env bash
set -euo pipefail

# Plays the working tree against a reference build and runs an SPRT on the
# result.
#
#   ./fastchess.sh                  gainer SPRT, elo0=0 elo1=5, runs for hours
#   ./fastchess.sh --fast           looser bounds, few hundred games
#   ./fastchess.sh --nonreg         non-regression, elo0=-5 elo1=0
#   REF=HEAD~1 ./fastchess.sh       pick what to measure against
#   OUT=<dir> ./fastchess.sh        where the pgn and log land
#
# WHICH BOUNDS, AND WHY THE PAIR IS NOT A DETAIL. The hypothesis pair sets the
# cost of a verdict as much as the hardware does. S068 measured one constant
# twice with the same binaries: elo0=0 elo1=5 ran 6 h 36 m over 9036 games and
# returned nothing, elo0=-5 elo1=5 returned H1 in 1 h 41 m over 2312, both at
# 1371 games/h (DEC-063). So:
#
#   default   elo0=0  elo1=5    a change claimed to gain. The published band
#                               for an engine of this strength (CPW tabulates
#                               {0,5} for top-200, {0,10} below it).
#   --nonreg  elo0=-5 elo1=0    a change that is not expected to gain and must
#                               not cost -- a simplification, a rewrite, a
#                               constant moved for another reason.
#   --fast    elo0=0  elo1=10   a first look, alpha=beta=0.10.
#
# Neither default brackets an effect from one side only: a bound pair that
# cannot contain the truth random-walks to the round limit. When the expected
# effect is genuinely two-sided, pass the bounds by editing this block for the
# run and record which pair ran, as S021 and S076 did.
#
# The reference is built from a git ref into a worktree under .ref-builds/, so
# what the number means is always attributable to a commit range. It used to be
# a binary sitting in ~/.local/bin, which drifted out of date silently and made
# every result measure an unknown amount of unrelated work.
#
# NOTE ON WHAT THIS CAN MEASURE. Games are played at a time control, so a
# change that makes the engine faster shows up here and a change that only
# reorders equal work does not. The reverse holds for fixed-nodes matches. If a
# change leaves the node count identical, this is the only tool that will see
# it; if it changes the tree, expect the result to mix the two effects.

REF="${REF:-7b4d9a4}"
# THE BOOK IS UNBALANCED, AND THAT IS THE POINT. Between two builds of the
# same engine a balanced book draws about 91 % (Pohl's measurement over the
# book class), and a drawn pair carries no information about which side is
# stronger -- so a balanced book spends the night to say less. UHO books are
# human openings filtered to a stated evaluation band; the decisive rate they
# buy is measurement capacity, not strength, and nothing about the engine is
# being flattered. It is 175 MB, gitignored, and fetched by books/fetch_book.sh
# against a pinned digest. DEC-083, S105.
book="$(dirname "$0")/books/UHO_Lichess_4852_v1.epd"
book_format="epd"
candidate="$(dirname "$0")/build/src/chesso"

# 8+0.08, the control the engines this plan reads figures from test at, and
# about 29 s a game against the 52 s of the 10+0.2 that ran until S105 -- the
# same resolution for a little over half the machine time. DEC-083.
tc="8+0.08"

# Every core the machine reports, whatever kind it is: efficiency cores on
# Apple silicon (DEC-048, superseding DEC-042), SMT siblings on the Linux
# machine (DEC-050, where `nproc` answers 12 for 6 physical cores). Measurement
# capacity is the binding constraint on the whole plan and S027 spent about
# twenty hours of it on six verdicts.
#
# What that buys and what it costs. Games are timed, so a game sharing a
# physical core -- or landing on an efficiency one -- is played slower than a
# game that does not. The scheduler decides which, not the match, so over a run
# both engines are hit about equally and the effect inflates variance rather
# than biasing the result. It is still variance neither engine controls and
# nothing corrects, so a run may need more games to reach its bound. Both
# decisions were taken knowingly on that trade.
#
# CONCURRENCY still overrides, and it is the dial to reach for when a run has to
# share the machine or when a result needs to be as clean as this setup can make
# it. Do not set it below the default to be polite: nothing else should be
# running during a match.
all_cores="$(sysctl -n hw.physicalcpu 2> /dev/null || nproc)"
concurrency="${CONCURRENCY:-$all_cores}"

# adjudication cuts dead games, gets to a verdict faster
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"

# A marker on every exit path, success and failure both, because a detached run
# is watched by something that has to be able to stop (AGENTS.md 12, DEC-061).
# `marked` keeps the trap from printing a second one after fail() has spoken.
marked=0
fail()
{
  marked=1
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

case "${1:-}" in
  --fast)
    # few hundred games
    sprt="elo0=0 elo1=10 alpha=0.10 beta=0.10"
    rounds=1500
    tag="fast"
    ;;
  --nonreg)
    # not expected to gain, must not cost
    sprt="elo0=-5 elo1=0 alpha=0.05 beta=0.05"
    rounds=20000
    tag="nonreg"
    ;;
  "")
    # SPRT will stop by it self
    sprt="elo0=0 elo1=5 alpha=0.05 beta=0.05"
    rounds=20000
    tag="full"
    ;;
  *)
    fail "unknown argument '$1'; expected --fast, --nonreg or nothing"
    ;;
esac

# One directory per run, stamped, as rating.sh:72-77 already does. The old
# fixed /tmp/fastchess_<tag>.pgn was appended to by every run that shared a
# tag, so a census over it mixed matches: after S089's 500-game run the file
# held 587 games, 87 of them against an earlier reference, and an unfiltered
# termination count read 421/166 instead of the run's real 359/141. "Filtered
# to the run" is now the filename rather than a grep nobody remembers to write.
stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_sprt_${tag}_${stamp}}"
mkdir -p "$outdir"
logfile="$outdir/fastchess.log"
pgnfile="$outdir/games.pgn"

[[ -x "$candidate" ]] || fail "no candidate at $candidate, build it first"
[[ -r "$book" ]] || fail "no book at $book, fetch it with books/fetch_book.sh"

# Snapshot the candidate before playing a single game.
#
# fastchess spawns the engine binary once per game, so pointing it at build/
# means a rebuild part way through silently swaps the engine under the match
# and the result becomes a mixture of two versions. That has already happened
# once. Copying it makes the match immune to whatever the working tree does
# next.
#
# The template is spelled out rather than passed to `mktemp -t`, which is a
# BSD-ism: GNU mktemp rejects a template with no X's in it and the script died
# here on Linux, before a single game.
snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-candidate.XXXXXX")"
cp "$candidate" "$snapshot"
chmod +x "$snapshot"

# The trap saves and restores the status it was entered with. Left to itself it
# ends in a successful `rm -f`, and a shell that takes the trap's status as the
# script's then reports an abort as a clean exit -- which is how a broken
# harness went a commit unnoticed (2026-08-13_adversarial-F01).
trap 'status=$?; rm -f "$snapshot"
      if ((status != 0 && marked == 0)); then
        echo "SPRT-RUN-FAILED: exited $status" >&2
      fi
      exit $status' EXIT
candidate="$snapshot"

# Build the reference from the ref, in its own worktree, once per ref.
ref_sha="$(git rev-parse --short "$REF")"
ref_dir="$(git rev-parse --show-toplevel)/.ref-builds/$ref_sha"
reference="$ref_dir/build/src/chesso"

if [[ ! -x "$reference" ]]; then
  echo "Building reference $ref_sha ..."
  git worktree add --detach "$ref_dir" "$ref_sha" > /dev/null
  cmake -S "$ref_dir" -B "$ref_dir/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > /dev/null
  cmake --build "$ref_dir/build" --target chesso \
    -j"$(sysctl -n hw.logicalcpu 2> /dev/null || nproc)" > /dev/null
fi

# A busy machine invalidates a timed match, and it is worth knowing before
# spending an hour rather than after.
busy="$(ps -A -o %cpu= | awk '{ total += $1 } END { printf "%.0f", total }')"
if ((busy > 60)); then
  echo "WARNING: about ${busy}% of a core is already busy. A timed match on a"
  echo "         loaded machine measures the load as much as the engine."
  echo "         Check with: ps aux | sort -rnk3 | head"
  echo
fi

echo "candidate  $(git rev-parse --short HEAD)$(git diff --quiet || echo ' + uncommitted changes')"
echo "reference  $ref_sha"
echo "tc $tc  hash 16  concurrency $concurrency of $all_cores cores"
echo "book       $(basename "$book")"
echo "bounds     $sprt"
echo "out        $outdir"
echo

fastchess \
  -engine cmd="$candidate" name=candidate \
  -engine cmd="$reference" name="ref-$ref_sha" \
  -openings file="$book" format="$book_format" order=random \
  -each tc="$tc" option.Hash=16 option.Threads=1 \
  -sprt $sprt model=normalized \
  $adjudication \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -recover \
  -pgnout file="$pgnfile" \
  -log file="$logfile"

# WHAT THE RUN COST, READ FROM THE PGN AND NOT FROM THE LOG. `-log` is
# WARN-and-above by default and comes back empty, and an empty log is
# indistinguishable from one that was never written -- grepping it for time
# losses passes for free (S089). The PGN is the record of what happened.
#
# This census reports, it does not void. Both sides here are chesso, so a thin
# time-management margin costs both about equally and is noise rather than
# bias; rating.sh is the one that voids, because there the margin is the
# opponent's too. A forfeit rate that is not near zero is still a reason to
# stop and look: 8+0.08 leaves MOVE_OVERHEAD_MS 50 about 2.5 times less room
# per move than 10+0.2 did.
terminations()
{
  grep -h '^\[Termination' "$pgnfile" 2> /dev/null | sort | uniq -c | sort -rn
}

count()
{
  grep -hc "$1" "$pgnfile" 2> /dev/null || true
}

# grep prints 0 and exits 1 when a pattern misses, which `|| true` absorbs; a
# missing file prints nothing at all, so the default is what makes that case
# read as zero games rather than as an empty arithmetic expression.
games="$(count '^\[Result ')"
draws="$(count '^\[Result "1/2-1/2"\]')"
forfeits="$(count '^\[Termination "time forfeit"\]')"
games="${games:-0}"
draws="${draws:-0}"
forfeits="${forfeits:-0}"

echo
echo "=== $games games in $pgnfile ==="
terminations || true
if ((games > 0)); then
  # Decisive rate is what the unbalanced book is for. Pohl's floor is about
  # 45 % draws -- below that the opening is simply winning and the pair scores
  # 1:1 with no signal in it -- and a balanced book sits near 91 %.
  awk -v g="$games" -v d="$draws" -v f="$forfeits" 'BEGIN {
    printf "draws     %d of %d, %.1f %%\n", d, g, 100 * d / g
    printf "decisive  %d of %d, %.1f %%\n", g - d, g, 100 * (g - d) / g
    printf "forfeits  %d of %d, %.2f %%\n", f, g, 100 * f / g
  }'
fi

echo "SPRT-RUN-DONE $tag $outdir"
