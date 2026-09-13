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
#   TC=32+0.32 ./fastchess.sh       play at another control; default 8+0.08
#   HASH=64 ./fastchess.sh          another hash per engine; default 16 MB
#   CAND=<ref> REF=<ref> ./fastchess.sh
#                                   play two commits against each other. The
#                                   working tree is not played and not read
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
# AND ONE READING THAT IS NOT A BOUND PAIR: THE LONGER CONTROL, ONCE PER BLOCK.
# A verdict that moves a pruning or reduction parameter -- a margin, depth
# bound, reduction coefficient or divisor in src/search_params.hpp deciding
# whether a node or move is searched at all, or how much shallower -- has its
# class read once more at `TC=32+0.32 HASH=64`, as one fixed 1000-pair match
# beside S199's drift point at the block boundary, never a re-take per verdict:
# an estimate with its own interval, never a verdict. The 8+0.08 SPRT decides
# whether a change ships; the longer control decides what may be written about
# its magnitude. Per verdict the rule would cost more than the whole plan --
# 13 re-takes at {-5, 0} -- which is the reason it is per block. Outside it:
# evaluation weights, ordering tables, TM_*, hash and table layout, and any
# change proved behaviour-neutral on node counts. S151, DEC-202.
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

# A CANDIDATE THAT IS A COMMIT INSTEAD OF THE WORKING TREE. CAND=<ref> builds
# that ref the same way the reference is built and plays it, so a run can
# measure two commits neither of which is checked out -- which is what a
# reading taken long after the change landed needs (S151 replays S085's
# shipped vector against its own parent). The working tree is then neither
# read nor played: no build/ is required, the dirty flag does not apply, and
# the A/A guard below keys on the two commits.
CAND="${CAND:-}"

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
#
# TC overrides it, and the default is what every verdict this project has taken
# was taken at, so a run that sets it is a different regime and says so in its
# pre-registration. The longer-control reading above is the one use there is a
# rule for; anything else is a new question and needs its own decision. S151.
tc="${TC:-8+0.08}"

# 16 MB, because what transfers across time controls is table *pressure* and
# not table size: at the rating list's 2'+1" a game writes on the order of
# 660 M nodes against 5.6 to 11 M entries, 60 to 120 overwrites per entry, and
# 16 MB at 8+0.08 reproduces that ratio where 128 MB undershoots it about
# eightfold. DEC-088.
#
# HASH overrides it, and the reason the two knobs exist together is that they
# are not independent: four times the clock is about four times the nodes a
# game writes, so holding Hash at 16 while TC goes to 32+0.32 quadruples the
# pressure instead of holding it. `TC=32+0.32 HASH=64` is the pair that keeps
# DEC-088's invariant, and it is fishtest's own LTC practice. S151.
hash="${HASH:-16}"

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

# Adjudication cuts dead games and gets to a verdict faster.
#
# RESIGNATION IS TWO-SIDED, AND THAT IS NOT THE DEFAULT. `twosided=true` makes
# a resignation need *both* engines to agree for `movecount` moves. Without it
# the losing side's own score alone ends the game -- the installed harness says
# so in its own --help, "twosided - if true, enables two-sided resignation.
# Defaults to false" (alpha 1.8.1 20260720-daa3ea2) -- and this script passed
# `-resign movecount=3 score=400` and nothing else until S212, while
# rating.sh's comment claimed the opposite property in the sentence that
# justified the setting.
#
# What that was worth, counted from the tracked PGNs rather than argued
# (2026-09-10 adversarial F04): 76 % of the A/A's games and 84 % of the S088
# rating run's ended by adjudication, and of the decisive ones 11 of 676 were
# one-sided in self-play (1.6 %) against 514 of 2627 (19.6 %) in the rating
# run, where chesso conceded alone 311 times and its opponents 203. In
# self-play with one evaluation scale the exposure is that 1.6 % floor, which
# is why the SPRT ledger is not corrupted by it -- and why the flag costs this
# harness almost nothing in throughput while removing the hazard the evaluation
# block creates the moment a candidate's scale moves (S039, S122, S126): under
# one-sided adjudication the side with the larger scale resigns first in equal
# positions, and that is the candidate.
#
# `score=400` and `movecount=3` stay. fastchess's own example and fishtest both
# use 600, so this truncates games earlier than the practice it was taken from;
# moving it is a throughput trade that gets its own decision, and two
# adjudication changes under one A/A cannot be priced apart. DEC-174.
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400 twosided=true"

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

# Empty unless CAND was set, and that emptiness is what every branch below
# reads to tell the two modes apart. Resolved beside the other two so a typo
# costs a message rather than a build.
cand_sha=""
if [[ -n "$CAND" ]]; then
  cand_sha="$(git rev-parse --short "$CAND")"
fi

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
#
# WITH CAND SET THE GUARD ASKS A DIFFERENT QUESTION, because the tree is not
# the candidate. Two commits that resolve to the same sha are the same build
# whatever the working tree holds, so a dirty tree must not rescue the run --
# under the working-tree guard it would, and the run would be an unannounced
# A/A between two copies of one commit.
same_build=0
if [[ -n "$cand_sha" ]]; then
  if [[ "$cand_sha" == "$ref_sha" ]]; then
    same_build=1
  fi
elif [[ "$ref_sha" == "$head_sha" ]] && ((diff_status == 0)); then
  same_build=1
fi

if ((same_build == 1)); then
  if [[ -n "$cand_sha" ]]; then
    aa_why="candidate $cand_sha and reference $ref_sha are the same commit"
  else
    aa_why="reference $ref_sha is HEAD and the tree is clean"
  fi
  [[ -n "${AA:-}" ]] \
    || fail "$aa_why, so both sides are the same build and there is nothing to measure; change something, or set AA=1 to calibrate the harness against itself on purpose"
  if [[ -n "$cand_sha" ]]; then
    echo "A/A CALIBRATION: both sides are the commit $ref_sha, so this run"
  else
    echo "A/A CALIBRATION: both sides are $ref_sha with a clean tree, so this run"
  fi
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

[[ -r "$book" ]] || fail "no book at $book, fetch it with books/fetch_book.sh"

# WHAT THE CANDIDATE WAS CONFIGURED AS, AND WHAT THE REFERENCE IS CONFIGURED
# WITH. `build/` is whatever it was last configured as; the reference used to
# be configured with bare defaults and never compared. They agreed by luck and
# not by check -- cmake/arch.cmake defaults CHESSO_ARCH to `native`, so both
# got -march=native on this machine -- and 11 of the 30 cached worktrees on
# disk predate S104 and hold binaries with no hardware popcount, which S104
# measured at +12.62 % nps on its own (2026-09-10 adversarial F06).
#
# One pair is read here and used twice: the banner prints it, and `build_ref`
# both configures with it and judges a cached worktree against it. Configuring
# with it is what keeps the judgement from looping -- a reference configured
# with bare defaults and judged against `build/`'s would be stale on every run
# and rebuilt on every run.
#
# The fallback is cmake's own default rather than a refusal, because `build/`
# does not have to exist: under CAND neither side is the working tree.
cache_value()
{
  awk -v key="$2" 'index($0, key ":") == 1 {
                     sub(/^[^=]*=/, "", $0); print $0; exit
                   }' "$1" 2> /dev/null || true
}

want_arch="native"
want_tune="OFF"
cand_cache="$repo/build/CMakeCache.txt"
if [[ -r "$cand_cache" ]]; then
  cache_arch="$(cache_value "$cand_cache" CHESSO_ARCH)"
  cache_tune="$(cache_value "$cand_cache" CHESSO_TUNE)"
  if [[ -n "$cache_arch" ]]; then want_arch="$cache_arch"; fi
  if [[ -n "$cache_tune" ]]; then want_tune="$cache_tune"; fi
fi

tune_label="off"
case "$want_tune" in
  [Oo][Nn] | 1 | [Tt][Rr][Uu][Ee] | [Yy][Ee][Ss]) tune_label="on" ;;
esac

# IS A CACHED WORKTREE STILL THE THING IT IS NAMED AFTER. It used to be played
# whenever its binary existed, and nothing asked anything else of it: not that
# the worktree is still at that commit, not that it is clean, not that it was
# built like the candidate. `.ref-builds/2b54a4f` was dirty on the day the
# audit looked.
#
# Three questions, and any `no` rebuilds rather than refuses. A rebuild is
# nearly free with ccache warm; a refusal would make a stale cache something
# somebody has to clear by hand before the run they wanted, which is the state
# that produced the dirty worktree in the first place.
#
#   1. is it a worktree at all, and is it clean -- `git status --porcelain`
#      empty, which `build/` does not disturb because .gitignore covers it;
#   2. is it at <sha>;
#   3. was it configured like the candidate.
#
# Every reason goes to stderr: the caller captures this function's stdout, and
# `build_ref`'s is the binary path.
#
# `git -C "$dir"` through the wrapper above becomes `git -C "$repo" -C "$dir"`.
# git's -C is cumulative and an absolute path wins outright, so these ask the
# worktree and not the repository -- which is also why the wrapper is safe to
# reach for here rather than a second bare `git`.
ref_cache_ok()
{
  local dir="$1" sha="$2"
  local cache="$dir/build/CMakeCache.txt"
  local at cached_arch cached_tune

  if ! git -C "$dir" rev-parse --is-inside-work-tree > /dev/null 2>&1; then
    echo "Cached reference $sha: $dir is not a git worktree." >&2
    return 1
  fi

  if [[ -n "$(git -C "$dir" status --porcelain 2> /dev/null)" ]]; then
    echo "Cached reference $sha: its worktree is dirty." >&2
    return 1
  fi

  at="$(git -C "$dir" rev-parse --short HEAD 2> /dev/null || true)"
  if [[ "$at" != "$sha" ]]; then
    echo "Cached reference $sha: its worktree is at '$at'." >&2
    return 1
  fi

  if [[ ! -r "$cache" ]]; then
    echo "Cached reference $sha: no CMakeCache.txt, so how it was built is unknown." >&2
    return 1
  fi

  cached_arch="$(cache_value "$cache" CHESSO_ARCH)"
  cached_tune="$(cache_value "$cache" CHESSO_TUNE)"
  if [[ "$cached_arch" != "$want_arch" || "$cached_tune" != "$want_tune" ]]; then
    echo "Cached reference $sha: built arch '$cached_arch' tune '$cached_tune', candidate is arch '$want_arch' tune '$want_tune'." >&2
    return 1
  fi

  return 0
}

# BUILD A COMMIT INTO ITS OWN WORKTREE, ONCE PER COMMIT, AND PRINT THE BINARY.
# One function for both sides, called twice, because the two sides of a CAND
# run have to be built the same way or the difference between them is the
# build and not the change. It was the reference's block alone until S151.
#
# The progress line goes to stderr: stdout is the binary path this function
# returns, and a `Building ...` on it would be captured into the caller's
# variable instead of read. Both streams land in the same log for a detached
# run, which is the only place either is read.
#
# EVERY COMMAND IN HERE IS CHECKED BY HAND, BECAUSE `set -e` DOES NOT REACH
# INSIDE A COMMAND SUBSTITUTION. Both call sites are `x="$(build_ref ...)"`,
# and bash does not apply errexit to the subshell that runs: a `cmake` exiting
# 1 left the function running on to its final `echo` and the caller taking a
# path to a file that was never produced. Measured 2026-09-12 on the reference
# side: the banner printed, fastchess was handed `cmd=<nonexistent>`, every
# game failed to start and the script reached `SPRT-RUN-DONE` -- a run that
# looks finished and played nothing. The inline block this function replaced
# was not exposed, because it ran in the script's own shell where errexit
# applies; the factoring is what moved it into a subshell.
#
# So each command ends in `|| return 1` and the binary is re-checked before the
# path is printed. Returning without printing is deliberate: the caller then
# gets an empty string, and its own `[[ -x ]]` check is the second line of
# defence rather than the first. Case 17 of tests/test_fastchess_script.sh.
build_ref()
{
  local sha="$1"
  local dir binary
  dir="$(git rev-parse --show-toplevel)/.ref-builds/$sha" || return 1
  binary="$dir/build/src/chesso"

  # A cached binary that does not answer ref_cache_ok's three questions is
  # cleared rather than played. `git worktree remove` and not a bare `rm -rf`:
  # the directory is a registered worktree, and a bare removal leaves the
  # registration behind, which makes the `git worktree add` below refuse the
  # path. `prune` covers the fallback and any registration a hand-removed
  # directory left. If the directory somehow survives, this returns rather than
  # falling through to play what it just judged unplayable.
  if [[ -x "$binary" ]] && ! ref_cache_ok "$dir" "$sha"; then
    echo "Clearing that worktree and rebuilding $sha." >&2
    git worktree remove --force "$dir" > /dev/null 2>&1 || rm -rf "$dir" || return 1
    git worktree prune > /dev/null 2>&1 || true
    [[ ! -e "$dir" ]] || return 1
  fi

  if [[ ! -x "$binary" ]]; then
    echo "Building reference $sha ..." >&2
    git worktree add --detach "$dir" "$sha" > /dev/null || return 1
    # Configured with the candidate's own arch and tune rather than with bare
    # defaults, so the two sides of the match differ by the commit and not by
    # the build. An older commit that has no such option takes the -D as an
    # unused variable and keeps it in its cache, which is what makes the
    # judgement above stable there too.
    cmake -S "$dir" -B "$dir/build" -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
      -DCHESSO_ARCH="$want_arch" -DCHESSO_TUNE="$want_tune" > /dev/null \
      || return 1
    cmake --build "$dir/build" --target chesso \
      -j"$(sysctl -n hw.logicalcpu 2> /dev/null || nproc)" > /dev/null || return 1
    # A build that exits 0 and produces nothing is the same failure as one that
    # exits 1, and it is the one a wrong target name or a moved output path
    # gives.
    [[ -x "$binary" ]] || return 1
  fi

  echo "$binary"
}

# The candidate is the working tree unless CAND named a commit. In that mode
# build/ is neither read nor required, so the readability check that guards the
# working-tree binary would be asking about a file nothing plays.
if [[ -n "$cand_sha" ]]; then
  candidate="$(build_ref "$cand_sha")" \
    || fail "building the candidate $cand_sha failed; its output is above"
  [[ -x "$candidate" ]] \
    || fail "building the candidate $cand_sha left no binary at '$candidate'"
else
  [[ -x "$candidate" ]] || fail "no candidate at $candidate, build it first"
fi

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
#
# A CAND candidate is snapshotted too, deliberately: one code path, and a
# `.ref-builds/` worktree rebuilt or removed during a run swaps the engine
# under the match exactly as a `build/` rebuild does.
#
# WHAT IS NOT SNAPSHOTTED, IN EITHER MODE, IS THE REFERENCE. It is played
# straight from `.ref-builds/$ref_sha/build/src/chesso`, so clearing or
# rebuilding *that* worktree while a match runs does swap the reference under
# it. Nothing in this script does so -- `build_ref` skips a worktree whose
# binary is already there -- and the exposure is an outside hand, which is why
# a stale cached reference is cleared before a run starts and never during one.
snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-candidate.XXXXXX")"
cp "$candidate" "$snapshot"
chmod +x "$snapshot"

# The trap that removes this is armed at the top of the script, before the first
# git call, and it saves and restores the status it was entered with: left to
# itself it would end in a successful `rm -f` and report an abort as a clean
# exit, which is how a broken harness went a commit unnoticed
# (2026-08-13_adversarial-F01).
candidate="$snapshot"

reference="$(build_ref "$ref_sha")" \
  || fail "building the reference $ref_sha failed; its output is above"
[[ -x "$reference" ]] \
  || fail "building the reference $ref_sha left no binary at '$reference'"

# A busy machine invalidates a timed match, and it is worth knowing before
# spending an hour rather than after.
#
# THE ONE-MINUTE LOAD AVERAGE, AGAINST THE CORE COUNT. This summed
# `ps -A -o %cpu=` until S212, which is each process's average over its own
# lifetime and not what the machine is doing now: measured here at a one-minute
# load average of 0.79 that sum read 255, over the 60 it warned at, so the
# guard fired on every run and carried no information (2026-09-10 adversarial
# F32). The load average is the number of tasks runnable or in uninterruptible
# sleep, which is the quantity a match competes with.
#
# THE THRESHOLD IS A QUARTER OF THE MACHINE -- 3.00 here, of 12 cores. A match
# already books every core it can see (DEC-050), so the question is not whether
# anything else is running -- something always is -- but whether enough is
# running that the games queue behind it rather than behind each other, and a
# quarter of the cores is where that starts. An idle machine here reads 0.00 to
# 1.00 one-minute, so the guard stays quiet, and that is what makes it worth
# reading when it does not. It warns and refuses nothing, as before: the
# judgement is the reader's.
#
# Neither readable is a stated non-check and not a silent pass. macOS has no
# /proc/loadavg and answers `sysctl -n vm.loadavg` with `{ 0.79 0.91 0.95 }`,
# whose second field is the one-minute figure; a machine with neither gets a
# line saying the guard did not run.
load_1min=""
if [[ -r /proc/loadavg ]]; then
  read -r load_1min _ < /proc/loadavg || load_1min=""
elif load_line="$(sysctl -n vm.loadavg 2> /dev/null)"; then
  load_1min="$(echo "$load_line" | awk '{ print $2 }')"
fi

if [[ -z "$load_1min" ]]; then
  echo "NOTE: neither /proc/loadavg nor 'sysctl -n vm.loadavg' could be read here,"
  echo "      so whether the machine is already busy has not been checked."
  echo
else
  busy_threshold="$(awk -v c="$all_cores" 'BEGIN { printf "%.2f", 0.25 * c }')"
  if awk -v l="$load_1min" -v t="$busy_threshold" 'BEGIN { exit !(l > t) }'; then
    echo "WARNING: the one-minute load average is $load_1min over $all_cores cores,"
    echo "         past the $busy_threshold this guard warns at -- a quarter of the"
    echo "         machine. A timed match on a loaded machine measures the load as"
    echo "         much as the engine. Check with: ps aux | sort -rnk3 | head"
    echo
  fi
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

# With CAND the candidate line carries that commit and NO dirty flag: the flag
# describes the working tree, the working tree is not being played, and a run
# that reported `+ uncommitted changes` beside a commit candidate would be
# saying the opposite of what it measured. The engine name follows the same
# rule, so the PGN says which commit it held.
cand_name="candidate"
if [[ -n "$cand_sha" ]]; then
  cand_name="cand-$cand_sha"
  echo "candidate  $cand_sha  $(commit_date "$cand_sha")"
else
  echo "candidate  $head_sha  $(commit_date HEAD)$dirty"
fi
echo "reference  $ref_sha  $(commit_date "$ref_sha")"
# What the candidate was built as, which the reference is built to match. Two
# shas say which commits were compared and nothing about whether the two
# binaries were compiled for the same machine; this is the other half.
echo "config     arch $want_arch  tune $tune_label"
echo "tc $tc  hash $hash  concurrency $concurrency of $all_cores cores"
echo "book       $(basename "$book")"
echo "seed       $seed"
if [[ -n "$sprt_args" ]]; then
  echo "bounds     $sprt"
else
  echo "bounds     none -- fixed $rounds rounds, a calibration or drift reading, NOT a verdict"
fi
echo "out        $outdir"
echo

# WHAT EACH SIDE SAYS IT IS, ASKED BEFORE THE FIRST GAME. The two names on the
# argv below -- `candidate` or `cand-<sha>`, and `ref-<sha>` -- are this
# script's own variables, so until S212 every archived PGN's engine names were
# an assertion about what was meant to be built rather than a property of what
# played (2026-09-10 adversarial F05). The banner does not close it either: it
# reports the same intention. `rating.sh` has refused an opponent whose
# `id name` disagrees with the manifest since DEC-068, after exactly this class
# of mistake put a development build where a tagged one was expected, and
# nothing did it for chesso's own binary. DEC-020's +301 Elo is what the class
# costs when it fires.
#
# The engine answers `id name Chesso <sha>[-dirty] <arch>[ tune]`, stamped
# through a header regenerated on every build (S212), so the sha is the commit
# the binary was compiled from.
#
# THE THREE ANSWERS, AND WHAT EACH ONE MEANS.
#
#   `Chesso <sha>` agreeing with the label   plays.
#
#   `Chesso` with nothing after it           plays, with a line saying the
#     check did not happen. Every commit before S212 answers the bare literal,
#     and most of what the reference mechanism exists to reach is one of them:
#     refusing would make `REF=<any older sha>` unrunnable, which is a worse
#     instrument than one that says what it could not verify.
#
#   anything else                            refused.
#
# `-dirty` is ALLOWED on the candidate side when the banner printed
# `+ uncommitted changes`, and never on the reference -- a `.ref-builds/<sha>`
# worktree is a clean detached checkout, and a dirty one was rebuilt above.
# Allowed and not required, deliberately: the dirty flag is raised by any
# tracked file, `adocs/` included, so requiring it would refuse a run whose
# binary is correct because a step file was edited after the build. The
# converse is refused on both sides -- a clean tree and a `-dirty` binary means
# that binary was built from a state that no longer exists on disk, and nothing
# can say what is in it.
#
# The probe is rating.sh's, guard for guard, including why `|| true` is the
# point rather than a swallowed error: not every engine exits on `quit`, so
# `timeout` returns 124 for a probe that in fact answered, and under
# `set -o pipefail` that status would kill this script with no message. The
# caller checks the string.
if command -v timeout > /dev/null; then
  timeout_cmd=timeout
elif command -v gtimeout > /dev/null; then
  timeout_cmd=gtimeout
else
  fail "neither timeout nor gtimeout on PATH, so neither side can be asked what it is -- brew install coreutils on macOS"
fi

engine_id()
{
  { printf 'uci\nquit\n' | "$timeout_cmd" -k 1 5 "$1" 2> /dev/null \
    | sed -n 's/^id name //p' | head -1 | tr -d '\r'; } || true
}

check_identity()
{
  local binary="$1" label="$2" want="$3" allow_dirty="$4"
  local got name rest token bare

  got="$(engine_id "$binary")"
  [[ -n "$got" ]] \
    || fail "the $label printed no 'id name' line within 5s of 'uci'"

  echo "id name    $label  $got"

  name="${got%% *}"
  [[ "$name" == "Chesso" ]] \
    || fail "the $label says 'id name $got', which is not this engine"

  rest="${got#"$name"}"
  rest="${rest# }"
  token="${rest%% *}"

  if [[ -z "$token" ]]; then
    echo "           ^ no build stamp: every commit before S212 answers the bare"
    echo "             literal, so what this binary holds cannot be checked here."
    return 0
  fi

  bare="${token%-dirty}"
  if [[ "$bare" != "$token" ]] && ((allow_dirty == 0)); then
    fail "the $label says '$token', but nothing is uncommitted on that side, so it was built from a state that no longer exists; rebuild it -- 'cmake --build build -j12' for the working tree, 'git worktree remove --force .ref-builds/<sha>' and rerun for a cached reference"
  fi

  # An abbreviation against an abbreviation: the engine's comes from
  # `git rev-parse --short` at build time and the label's from the same command
  # in this script, so they are the same length on the same repository.
  # Compared as prefixes anyway, so a changed core.abbrev is a longer answer
  # and not a refused run.
  if [[ "$bare" != "$want"* && "$want" != "$bare"* ]]; then
    fail "the $label says it was built from '$bare', but this run labelled that side '$want' -- the binary is not the commit the banner claims"
  fi
}

if [[ -n "$cand_sha" ]]; then
  check_identity "$candidate" "$cand_name" "$cand_sha" 0
else
  check_identity "$candidate" "$cand_name" "$head_sha" "$((diff_status != 0))"
fi
check_identity "$reference" "ref-$ref_sha" "$ref_sha" 0
echo

fastchess \
  -engine cmd="$candidate" name="$cand_name" \
  -engine cmd="$reference" name="ref-$ref_sha" \
  -openings file="$book" format="$book_format" order=random \
  -srand "$seed" \
  -each tc="$tc" option.Hash="$hash" option.Threads=1 \
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
# On forfeits this census reports and does not void. Both sides here are
# chesso, so a thin time-management margin costs both about equally and is
# noise rather than bias; rating.sh voids on forfeits because there the margin
# is the opponent's too. A forfeit rate that is not near zero is still a reason
# to stop and look: 8+0.08 leaves MOVE_OVERHEAD_MS 50 about 2.5 times less room
# per move than 10+0.2 did. A crash or a disconnect is a different matter and
# the block below voids the run on it (S212, 2026-09-10_adversarial-F31).
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

# A CRASH OR A DISCONNECT VOIDS THE RUN. A TIME FORFEIT DOES NOT, AND THE TWO
# ARE NOT THE SAME KIND OF THING. A forfeit is a real game result -- the loser
# ran out of clock and the points are genuinely the opponent's -- and here it
# costs both sides about equally, because both sides are chesso. A crash does
# not divide: only one side carries the change under test, and a run whose
# games ended because an engine died measures the death. `rating.sh` has voided
# on this since DEC-075 and this script only counted it, with a justification
# that covered forfeits alone (2026-09-10 adversarial F31). Given the two
# `position fen` crashes the same audit found, worth closing.
#
# The set of expected terminations is asserted rather than a guessed spelling
# grepped for, exactly as rating.sh does it, so a failure cannot hide behind a
# word nobody anticipated.
#
# THE MARKER STILL COMES LAST, AND THAT IS NOT DECORATION. Every watcher of a
# detached run breaks on SPRT-RUN-(DONE|FAILED) and on nothing else (DEC-061,
# AGENTS.md WATCHERS), so a void that printed INVALID and stopped there would
# leave the watcher spinning to its ceiling -- the S167 and S177 failure,
# reintroduced by the fix for a different finding. fail() prints the marker.
unexpected="$(grep -h '^\[Termination' "$pgnfile" 2> /dev/null \
  | grep -cvE '"(normal|adjudication|time forfeit)"' || true)"
unexpected="${unexpected:-0}"

if ((unexpected > 0)); then
  echo
  echo "SPRT-RUN-INVALID: $unexpected crashes/disconnects -- $unexpected game(s) ended"
  echo "                  outside normal, adjudication and time forfeit. Only one"
  echo "                  side carries the change, so this run measured the"
  echo "                  failure and not the difference. Nothing is reported."
  fail "the run is void: $unexpected crash/disconnect termination(s) in $pgnfile"
fi

completed=1
echo "SPRT-RUN-DONE $tag $outdir"
