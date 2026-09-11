#!/usr/bin/env bash
set -euo pipefail

# Plays the working tree against a reference build and runs an SPRT on the
# result.
#
#   ./fastchess.sh                  vs HEAD, gainer SPRT, elo0=0 elo1=5, hours
#   ./fastchess.sh --fast           vs HEAD, looser bounds, few hundred games
#   ./fastchess.sh --nonreg         vs HEAD, non-regression, elo0=-5 elo1=0
#   REF=HEAD~1 ./fastchess.sh       measure against some other commit instead
#   OUT=<dir> ./fastchess.sh        where the pgn and log land
#   AA=1 ./fastchess.sh             play an identical build against itself on
#                                   purpose, to calibrate the harness
#   SRAND=<n> ./fastchess.sh        replay another run's opening sequence, the
#                                   seed read off its banner
#   ROUNDS=<n> AA=1 ./fastchess.sh  fixed rounds and no SPRT: a calibration or
#                                   a drift reading, never a verdict
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
#
# THE BOUNDS ARE NORMALIZED ELO, NOT LOGISTIC ELO. `model=normalized` below is
# fastchess's default and its --help says so in as many words: "normalized -
# Uses nElo (default)". So elo0/elo1 above are nElo, and a verdict against
# elo0=-5 excludes a regression of 5 *nElo*, which is smaller in logistic Elo
# -- how much smaller depends on the draw rate and is read off each run's own
# printed pair, not from a constant: measured 3.54 at 44.75 % draws (S165's
# 18598 games) and 3.92 to 3.99 at the lower draw rates of S107, S085 and S021.
#
# The setting is deliberate and must not be changed to `logistic` to make the
# numbers read as Elo. The CPW table cited above states no scale, but its rows
# are Stockfish STC {0,2} and LTC {0.5,2.5}, which are fishtest's own bounds,
# and fishtest expresses bounds in normalized Elo. `model=normalized` is what
# makes {0,5} mean what the source it was taken from means. 2026-08-21
# adversarial F09, S157.
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

# A MARKER ON EVERY EXIT PATH, SUCCESS AND FAILURE BOTH, because a detached run
# is watched by something that has to be able to stop (AGENTS.md 12, DEC-061).
# `marked` keeps the trap from printing a second one after fail() has spoken.
#
# Armed here, before the first git call, and not after the candidate snapshot
# where it used to sit. S160 put `git rev-parse --short HEAD` above the old
# trap, so a run launched outside a git checkout died `fatal: not a git
# repository` with no marker at all -- and the fallback watcher DEV_MANUAL.md
# teaches breaks only on SPRT-RUN-(DONE|FAILED), so it would have spun until
# its ceiling. The snapshot is guarded by `-n` rather than removed
# unconditionally: `rm -f ""` is itself an error on GNU coreutils, which would
# make the trap fail on every path that exits before the snapshot exists.
marked=0
# Set beside SPRT-RUN-DONE at the end of the file. Read by the trap, which asks
# whether the script reached that line rather than asking the shell how it died.
completed=0
fail()
{
  marked=1
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

#
# THE TRAP DOES NOT TRUST THE STATUS IT IS HANDED. macOS ships bash 3.2.57, and
# there an EXIT trap reads `$? == 0` when the shell aborts on an unbound
# variable under `set -u`; bash 5 reads 1. The `set -e` shape is unaffected, so
# only the shape this trap was written for was masked -- the script printed no
# marker and exited 0, and the watcher of a detached run would have spun to its
# ceiling on that silence. Asking `completed` instead is version-independent.
# S167.
trap 'status=$?
      [[ -n "${snapshot:-}" ]] && rm -f "$snapshot"
      if ((marked == 0 && (status != 0 || completed == 0))); then
        echo "SPRT-RUN-FAILED: exited $status" >&2
        ((status != 0)) || status=1
      fi
      exit $status' EXIT

# EVERY GIT CALL IS ANCHORED TO THE SCRIPT, NOT TO THE CALLER'S DIRECTORY. They
# read cwd by default, so an absolute-path launch from inside another checkout
# stamped the banner with *that* repository's sha and date, resolved the
# reference against it and built a worktree from it -- while playing chesso's
# binary. The paths derived from $0 were already immune; the git calls were not.
#
# Done with a wrapper rather than by adding -C at each site, so a git call added
# later inherits the anchor instead of reintroducing the bug. Every `git ...`
# below is this function; `$real_git` is the real one.
#
# The binary is resolved once instead of being reached through the `command`
# builtin at each call, because bash 3.2 does not suppress errexit for `command`
# on the left of `||`. The dirty-tree probe below is `git diff --quiet HEAD ||
# diff_status=$?`, and git exits 1 there on every working-tree run -- which is
# every --fast run -- so the script died at that line, before its banner, with
# the trap above reporting success. A plain external command in the same
# position is suppressed correctly. S167.
repo="$(cd -- "$(dirname -- "$0")" && pwd)"
real_git="$(command -v git)"
git()
{
  "$real_git" -C "$repo" "$@"
}

# THE DEFAULT REFERENCE IS HEAD, AND THE BANNER PRINTS IT. It used to be the
# fixed sha 7b4d9a4 -- the 2026-08-08 pre-achesso baseline, set that day and
# never moved -- which by the time it was found was several hundred Elo stale
# (S028's evaluation fit alone measured +188.74 since it). A run that omitted
# REF reached H1 at --fast bounds in minutes whatever the change under test
# was: the DEC-020 class, armed in the default of the project's own per-change
# instrument, while CLAUDE.md taught the bare form. HEAD keeps the bare
# invocation meaningful -- it measures the uncommitted diff against the commit
# the tree sits on, which is the working-tree-versus-reference contract this
# header already describes. S160, 2026-08-22_adversarial-F02.
REF="${REF:-HEAD}"

# THE BOOK IS PICKED BY MEASUREMENT. S219 (2026-09-12) compared four CC0
# books -- one HEAD binary against itself at a fixed time handicap, pooled
# over two counterbalanced passes -- by M = nElo^2 x games/hour: noob3
# 154101610, uho4060v4 132307458, popularpos 128933263, uho4852 (the
# incumbent) 123838572 -- adocs/data/S219_book_compare.md. noob_3moves.epd
# won outright, more than one combined standard error clear of the rest, and
# it is balanced (DEC-189). The old argument for an unbalanced book (DEC-083)
# was that a balanced book draws about 91 % between engines of equal strength
# (Pohl's measurement) and a drawn pair carries no signal; S105 measured
# chesso itself drawing only 40.3 % on a balanced book, already under Pohl's
# own 45 % floor, so that argument did not hold at this engine's strength. It
# is 9.4 MB unpacked, gitignored, and fetched by books/fetch_book.sh against a
# pinned digest -- the zip digest is not yet pinned, and that script's header
# says why.
book="$repo/books/noob_3moves.epd"
book_format="epd"
candidate="$repo/build/src/chesso"

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

# THE FREE MATE CHECK. `-check-mate-pvs` makes fastchess verify, for every info
# line that reports a mate score, that the principal variation has the length
# the score claims and ends in checkmate. It costs nothing, it needs no
# position file, and it is the one check on mate handling that a unit test
# cannot reach: it runs over every position both engines actually meet, which
# is tens of thousands a night rather than the twenty-odd a constructed set can
# hold. S145.
#
# What it does and does not catch. It is a *consistency* check on what the
# engine says about the mates it does find -- a wrong distance, a wrong sign, a
# PV that does not end in mate -- and it is silent about a mate the engine
# never reported at all. That second failure is the one reverse futility
# causes, and it is what the constructed set in adocs/data/S145_mate_set.tsv is
# for. The two do not overlap and neither substitutes for the other.
#
# Failures arrive as fastchess output on the run's stdout and stderr, which the
# detached form of this script redirects into its own log. Verified against the
# installed binary, alpha 1.8.1 20260720-daa3ea2: `-check-mate-pvs` is
# accepted and documented as "Check that PVs for mate scores have the correct
# length and end in checkmate."
mate_pv_check="-check-mate-pvs"

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

# FIXED ROUNDS, AND WHY THAT IS NEVER A VERDICT. ROUNDS=<n> replaces the mode's
# round limit and removes the SPRT entirely, so the run stops where it was told
# to rather than where a likelihood ratio happened to cross a bound.
#
# That is what a calibration needs and what an SPRT cannot give it. In an A/A
# the true difference is zero by construction, so the pair distribution that
# comes back is the harness's alone -- but an SPRT on it accepts H1 with
# probability alpha exactly, stopping at a game count the result itself chose,
# and variance read over that denominator is read over a number the answer
# picked. Fixed rounds give a known denominator and a known error bar,
# v * sqrt(2/(n-1)) as adocs/data/S105_pairs.py prints it, about 6.3 % of v at
# 500 pairs. S199's drift readings want the same thing for the same reason.
#
# The bounds do not merely go unused here, they go unprinted: a bounds line
# over a run nothing tested against is the DEC-020 class of banner. The
# MEASUREMENT rule is on the screen instead. S198, DEC-143.
sprt_args="-sprt $sprt model=normalized"
if [[ -n "${ROUNDS:-}" ]]; then
  case "$ROUNDS" in
    '' | *[!0-9]*) fail "ROUNDS must be an unsigned integer, got '$ROUNDS'" ;;
  esac
  rounds="$ROUNDS"
  sprt_args=""
fi

head_sha="$(git rev-parse --short HEAD)"
ref_sha="$(git rev-parse --short "$REF")"

# IS THE TREE DIRTY, ASKED ONCE. `git diff --quiet HEAD` and not a bare
# `git diff --quiet`: the bare form compares the tree against the index, so a
# fully staged diff read as clean -- while the binary being played was built
# from that tree. Exit 1 is "dirty" and anything above it is git failing, which
# must not read as clean: a fail-open here silently disarms the guard below.
diff_status=0
git diff --quiet HEAD || diff_status=$?
((diff_status <= 1)) \
  || fail "git diff --quiet HEAD exited $diff_status, so whether the tree is clean is unknown"

# CANDIDATE AND REFERENCE AT THE SAME COMMIT, NOTHING UNCOMMITTED, ARE THE SAME
# ENGINE. An SPRT between identical engines does not return zero: it
# random-walks until a bound is crossed by luck, and at --fast alpha 0.10 that
# is one run in ten reporting a gain that does not exist.
#
# The test is on state, not on who chose the reference. S160 asked whether REF
# was passed, which let REF=<HEAD's own sha>, REF=master and REF=$(git rev-parse
# HEAD) -- the form saved runner scripts use -- play the full A/A anyway, and
# the refusal message said "pass REF=<sha>", walking the reader straight into
# it. AA=1 is the opt-in instead, because an A/A calibration of the harness is a
# thing somebody asks for by name, not something a ref spelling falls into.
if [[ "$ref_sha" == "$head_sha" ]] && ((diff_status == 0)); then
  [[ -n "${AA:-}" ]] \
    || fail "reference $ref_sha is HEAD and the tree is clean, so both sides are the same build and there is nothing to measure; change something, or set AA=1 to calibrate the harness against itself on purpose"
  echo "A/A CALIBRATION: both sides are $ref_sha with a clean tree, so this run"
  echo "                 measures the harness and not the engine. AA is set."
  echo
fi

stamp="$(date +%Y%m%d_%H%M%S)"

# THE SEED, BECAUSE FASTCHESS RECORDS IT NOWHERE. `-openings ... order=random`
# has always been passed, so which openings a run played -- and in which order,
# which decides the pairs -- was drawn from an unrecorded seed and could not be
# replayed. fishtest passes `-srand` with `order=random` for exactly this
# reason.
#
# Measured on alpha 1.8.1 20260720-daa3ea2: the seed appears on no stream, in
# no log and in no PGN header, and the parser is a 64-bit unsigned integer (a
# 20-digit value is refused with `stoull: out of range`, and seed + 2^32 gives
# a different sequence, so nothing is truncated to 32 bits). The banner below
# and the -event header are therefore the seed's only records.
#
# The default is derived from the run stamp, so it is unique per run and reads
# as the time the run started; SRAND overrides it to replay a sequence. What a
# replay reproduces is the openings and not the games: search under a clock is
# not deterministic, and the seed-to-sequence mapping is fastchess's own, so it
# needs the same book, the same fastchess and the same -openings options.
#
# Validated before the output directory is made and before the reference is
# built, so a typo costs a message rather than a directory and a compile. S198.
seed="${SRAND:-${stamp//_/}}"
case "$seed" in
  '' | *[!0-9]*) fail "SRAND must be an unsigned integer, got '$seed'" ;;
esac

# One directory per run, stamped, as rating.sh:72-77 already does. The old
# fixed /tmp/fastchess_<tag>.pgn was appended to by every run that shared a
# tag, so a census over it mixed matches: after S089's 500-game run the file
# held 587 games, 87 of them against an earlier reference, and an unfiltered
# termination count read 421/166 instead of the run's real 359/141. "Filtered
# to the run" is now the filename rather than a grep nobody remembers to write.
outdir="${OUT:-/tmp/chesso_sprt_${tag}_${stamp}}"
mkdir -p "$outdir"
logfile="$outdir/fastchess.log"
pgnfile="$outdir/games.pgn"

# The stamp makes the default unique, but OUT is a name the caller chose and
# can choose twice -- and that walks straight back into the trap above, since
# fastchess appends. Refuse rather than mix two matches into one census.
[[ ! -e "$pgnfile" ]] \
  || fail "$pgnfile already exists; fastchess appends, so this run's census would mix it with an earlier match"

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

# The trap that removes this is armed at the top of the script, before the first
# git call, and it saves and restores the status it was entered with: left to
# itself it would end in a successful `rm -f` and report an abort as a clean
# exit, which is how a broken harness went a commit unnoticed
# (2026-08-13_adversarial-F01).
candidate="$snapshot"

# Build the reference from the ref, in its own worktree, once per ref.
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

# The commit date beside each sha is what makes a stale reference visible
# before an hour is spent on it: a pair of shas says nothing about how far
# apart the two sides are, and the old fixed default was two weeks and several
# hundred Elo behind without one line of this banner saying so. S160.
#
# The dirty flag is the one computed above, not a second `git diff`: two calls
# can disagree if the tree changes mid-run, and the one the guard judged is the
# one the banner has to report.
commit_date()
{
  git show -s --date=short --format=%cd "$1"
}

dirty=""
((diff_status == 0)) || dirty="  + uncommitted changes"

echo "candidate  $head_sha  $(commit_date HEAD)$dirty"
echo "reference  $ref_sha  $(commit_date "$ref_sha")"
echo "tc $tc  hash 16  concurrency $concurrency of $all_cores cores"
echo "book       $(basename "$book")"
echo "seed       $seed"
if [[ -n "$sprt_args" ]]; then
  echo "bounds     $sprt"
else
  echo "bounds     none -- fixed $rounds rounds, a calibration or drift reading, NOT a verdict"
fi
echo "out        $outdir"
echo

fastchess \
  -engine cmd="$candidate" name=candidate \
  -engine cmd="$reference" name="ref-$ref_sha" \
  -openings file="$book" format="$book_format" order=random \
  -srand "$seed" \
  -each tc="$tc" option.Hash=16 option.Threads=1 \
  $sprt_args \
  $adjudication \
  $mate_pv_check \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -recover \
  -event "chesso $tag $stamp srand=$seed" \
  -pgnout file="$pgnfile" nodes=true timeleft=true \
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
  # Decisive rate is reported for interpretation, not because the book is
  # chosen to keep it high: Pohl's floor is about 45 % draws -- below that the
  # opening is simply winning and the pair scores 1:1 with no signal in it --
  # and chesso runs under that floor on every book measured so far, book
  # choice included (S219, DEC-189).
  awk -v g="$games" -v d="$draws" -v f="$forfeits" 'BEGIN {
    printf "draws     %d of %d, %.1f %%\n", d, g, 100 * d / g
    printf "decisive  %d of %d, %.1f %%\n", g - d, g, 100 * (g - d) / g
    printf "forfeits  %d of %d, %.2f %%\n", f, g, 100 * f / g
  }'
fi

completed=1
echo "SPRT-RUN-DONE $tag $outdir"
