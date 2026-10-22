#!/usr/bin/env bash
#
# S111: THE CONTINUATION CORRECTION TABLES -- the two tables keyed on the move
# two and four plies ago with the move one ply ago, against the tree before
# them, one SPRT at 32+0.32, Hash 64, {0, 10} nElo, pre-registered by the
# coordinator under the owner's standing instruction (DEC-297: "S111 is
# pre-registered the same way").
#
#   cd /home/max/ws/chesso-s111r &&
#     nohup adocs/data/S111_sprt.sh > /home/max/ws/chesso/.tuning/s111_sprt.log 2>&1 &
#
# Written 2026-10-21 in S111's phase 1, in S110_sprt.sh's shape (and so in
# S283's, adocs/data/S283_sprt.sh, whose harness section this file relies on
# and does not repeat), while S110's SPRT held the machine; rebased with S111
# onto S110's revert; switched to WORKING-TREE MODE after phase 2 by DEC-303
# (the coordinator's): the mined mate floor read 162 against `EXACT_FLOOR`
# 163 on S111's tree, a lowered mate floor waits for the owner, so S111 is
# not committed before the ruling and its SPRT plays the uncommitted tree of
# the worktree `/home/max/ws/chesso-s111r` against its HEAD, fastchess.sh's
# dirty-tree mode (S256): the candidate is named `cand-<HEAD>+<hex>` and the
# run saves its diff as `$OUT/candidate.diff`. Every figure below is read
# from a file or a git object and names it. The coordinator fills the
# bracketed sentence below on the day, and until then the script refuses to
# start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  the working tree of `/home/max/ws/chesso-s111r`, uncommitted: its
#         HEAD (REF) plus S111's change, built in that worktree's build/ --
#         `continuation_correction_t`, two tables of the pawn
#         table's kind, 2^18 slots a side each, directly indexed -- one keyed
#         on the move two plies ago with the move one ply ago, one on the
#         move four plies ago with the move one ply ago, each move coded by
#         its piece type and destination, a ply before the root or a null
#         move coded as no move -- read off the game's history at
#         `negamax_at`'s compute-or-read site and joined to the pawn read in
#         DEC-300's weighted average at `CorrHistCont2Weight` and
#         `CorrHistCont4Weight` 100 each, beside the pawn read's 100 -- a
#         third each, one division, rounded, inside CorrHistMax, then the
#         mate band -- and taught beside the pawn table under the same gate.
#         The UCI layer owns them, datagen gives each worker its own. The
#         pair key is DEC-302's.
#   REF   `03fa3ec` (DEC-302), the worktree's HEAD: the tree S110's revert
#         leaves (`eee722c`, S110's H0; src/, tests/ and tools/ byte-identical
#         to `bb3d31d`'s, bench 6042933).
#
# The two differ by S111's change alone, which the guards below check: the
# worktree's HEAD is REF, the tree is dirty, and build/'s binary is no older
# than any source it is built from. What the tree does, phase 1b on the rebased tree (S111's step
# file, the dated rebase section): bench 6042933 -> 7343608, the second reply
# moved (`d5e6` -> `e2a6`); `bench 12` 2339074 -> 2460543, the fifth moved
# (`a7a6` -> `a7a5`); the tune build at `CorrHistCont2Weight` 0 and
# `CorrHistCont4Weight` 0 prints the parent's bench stream, 121 lines, none
# differing but time and nps, and search_bench 9 and 12 read the parent's
# nodes and best moves (DEC-215).
#
# THE FAMILY'S READINGS, quoted and not re-derived:
#   - S099's pawn table at the harness's 8+0.08, `{0, 5}`, Hash 16: H0, nElo
#     -4.71 +/- 6.84 over 9916 games (`adocs/data/S099_sprt.log`).
#   - The same pair at 32+0.32, `{0, 10}`, Hash 64 (S283): H1, nElo +12.56
#     +/- 9.89 over 4738 games (`adocs/data/S283_sprt.log`). Relanded by S286.
#   - S110's non-pawn tables at this run's shape: H0, nElo -4.88 +/- 11.28
#     over 3644 games (record `99a33a4`, `adocs/data/S110_sprt.log`), reverted
#     whole by `eee722c`. DEC-302 reads S111's verdict as S110's was read: the
#     conditioning and the gain the average gives, together.
# DEC-297 books this run at the second's shape for the reason it booked
# S110's: the family's one measured member gained at the longer control and
# not at the harness's, and the Chess Programming Wiki's page on the
# technique says its gains grow with the time control. A record of
# direction, never a prediction (DEC-019).
#
# DEC-221 and the copying rules: S111 is implemented from the Chess
# Programming Wiki's description of continuation correction history (its
# table: "Move on the previous ply and the ply before it") at the step's two
# distances, and S099's own table; its two constants are DEC-134 form (c),
# the range midpoint, argued at their site, and its blend is DEC-300's
# weighted average. No engine's code, constant or table.
#
# THE HARNESS. fastchess.sh has not moved since `be48e02` (`git log
# be48e02..03fa3ec -- fastchess.sh` empty, read 2026-10-21, and S111's diff
# does not touch it), so every line number S283_sprt.sh's HARNESS section
# cites into it holds here; the run uses the worktree's copy.
# The guard below refuses unless fastchess.sh still reads both SPRT names.
#
# BOUNDS. `elo0=0 elo1=10 alpha=0.05 beta=0.05`, `model=normalized` (nElo),
# DEC-297's pair.
#
# WHAT THE PAIR COSTS (DEC-143), S283's arithmetic, the same width: D / (e1 -
# e0)^2 with D = 1046535 at alpha = beta = 0.05 is **10465 games** with the
# truth at the interval's midpoint, 19.6 h at 534 an hour and 19.2 h at 545;
# 639770 / 100, **6398 games**, with it on a bound, 12.0 h and 11.7 h. The two
# throughputs are `.moltke.local.md`'s at 32+0.32, Hash 64, `noob_3moves.epd`,
# concurrency 12, as S110_sprt.sh quotes them; S110's own run is the latest
# reading at this shape and the coordinator compares it on the day.
#
# THE CAP. **5250 rounds, 10500 games**, 19.7 h at 534 an hour: the midpoint
# figure rounded up to a whole hundred, as S283's and S110's. Four hours or
# more, so the run starts by DEC-155's night rule. Watcher ceiling 144000 s
# (40 h), at least twice the cap and DEC-297's abort at 2x, wall-clock only
# while the machine is awake.
#
# REGIME. 32+0.32, Hash 64, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), fastchess.sh's adjudication, the
# governor recorded and not set (DEC-195). If fastchess, the book, the
# adjudication or the machine has moved since the last fixed-rounds A/A, that
# A/A of 1000 games comes first (DEC-143); the coordinator checks on the day.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's PGN in
# `$OUT`), on a crash or a disconnect on either side (`SPRT-RUN-INVALID`,
# S212), if the machine loses mains power (POWER) or another load starts on it
# (MACHINE), if the banner is not what the coordinator's sentence below says
# it must print, or at 40 h of wall clock with neither bound (DEC-297).
# Nothing else stops it.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S283's lines with this
# run's shas, dates and out directory --
#
#   candidate  03fa3ec  2026-10-21  + uncommitted changes
#   diff       /home/max/ws/chesso/.tuning/sprt_s111_<stamp>/candidate.diff  blob <12 hex>
#   reference  03fa3ec  2026-10-21
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 32+0.32  hash 64  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=10 alpha=0.05 beta=0.05
#   cap        5250 rounds, 10500 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s111_<stamp>
#
# with no `relay` or `A/A CALIBRATION` line, fastchess's engine names
# `cand-03fa3ec+<the same 12 hex>` and `ref-03fa3ec`, the candidate's `id name`
# carrying `-dirty`, and fastchess's first `Started game 1 of N` line reading
# N = 10500.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# Pinned by the coordinator 2026-10-21 23:35, started 2026-10-22 00:12 after
# an owner shutdown (the 23:34 launch refused before any game: the worktree
# had no `books/noob_3moves.epd`, which is gitignored, so it was copied in):
# the Open list at main's `3918882`
# re-read, and no DEC-171 filler step is open (S288 closed at `bb3d31d`). The
# harness is S110's run's of 2026-10-21 -- fastchess alpha 1.8.1
# 20260720-daa3ea2, `noob_3moves.epd`, `fastchess.sh` untouched since
# `be48e02`, the same workstation and kernel 7.2.6 -- so no A/A is owed
# (DEC-143; the kernel reasoning in `adocs/data/S110_sprt.sh`). The worktree
# holds nothing unstaged or untracked, and its staged diff is the one the
# coordinator's phase 2 read (`.tuning/coord/S111/s111_rebased.diff`, byte
# for byte, before this sentence was written). Phase 2 before the run: fast
# suite 49/50 in both builds, the one red `EXACT_FLOOR` (162 >= 163,
# DEC-303); Debug self-play 8 games, 0 Assertion; gate_extra prose,
# citations, debug and perft ok, sanitize red on the same floor alone with no
# sanitizer report; the mutation pass waits for the owner's ruling.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063),
# as DEC-297 states the outcomes.
#
#   H1 accepted  -> at 32+0.32, Hash 64, the continuation tables are accepted
#                   as gaining: H1's 10 nElo preferred to H0's 0 at alpha =
#                   beta = 0.05. **S111 ships once the owner has ruled on
#                   `EXACT_FLOOR` (DEC-303)**: its commit then lands, the
#                   tree this run played, with the verdict's block. The
#                   stopping run's Elo is upward-biased (DEC-063) and is not
#                   the effect size.
#
#   H0           -> not shown to gain at this control. **S111 is not
#                   committed** and is recorded as zero, the played diff kept
#                   as the record; the tables before it stay. A
#                   small true gain, under about 5 nElo, reads H0 here more
#                   often than at `{0, 5}`; that is the price of a bounded run,
#                   and it is recorded beside the H0.
#
#   No verdict   -> the 10500-game cap, or the 40 h abort, with neither bound:
#                   read as H0 is -- not committed, recorded as zero, with the
#                   games and the interval -- and no follow-up run (DEC-063,
#                   DEC-297).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand 03fa3ec+<hex>
# vs ref 03fa3ec, 32+0.32, Hash=64, noob_3moves.epd, {0, 10} nElo` and the
# `Elo`, `LLR`, `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S111_sprt.log` by `tools/trim_console.py` and the run's
# `candidate.diff` committed beside it as `adocs/data/S111_candidate.diff`
# (DEC-254: `tools/gate.sh` and `tools/ledger.py` find it by its blob id,
# the `<hex>`); `tools/ledger.py` regenerates plan.md's ledger table from it.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, and this script execs it and adds
# nothing after.

set -uo pipefail

fail()
{
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

# THE BOOKED SETTINGS, AND NOTHING ELSE FROM THE CALLER'S ENVIRONMENT, as
# S283_sprt.sh's same block reads them: pinned, refused if set to anything
# else, and passed on the exec line; CAND, ROUNDS, BARE_FEN, SRAND and AA
# refused if set at all -- CAND above all, which would turn the run into a
# commit's and leave the tree unplayed; RELAY_PYTHON unset; OUT overridden.
ref="03fa3ec47601cca7d56509a0b40d03cd145b403e"
worktree="/home/max/ws/chesso-s111r"
tc="32+0.32"
hash="64"
concurrency="12"
sprt_bounds="elo0=0 elo1=10 alpha=0.05 beta=0.05"
sprt_rounds="5250"

for pair in "REF=$ref" "TC=$tc" "HASH=$hash" \
            "CONCURRENCY=$concurrency" "SPRT_BOUNDS=$sprt_bounds" \
            "SPRT_ROUNDS=$sprt_rounds"; do
  name="${pair%%=*}"
  want="${pair#*=}"
  if [[ -n "${!name+set}" && "${!name}" != "$want" ]]; then
    fail "$name is '${!name}' in the environment, and this run is booked at" \
         "$name=$want; unset it"
  fi
done
for name in CAND ROUNDS BARE_FEN SRAND AA; do
  if [[ -n "${!name+set}" ]]; then
    fail "$name is set in the environment ('${!name}'), and this run is" \
         "booked without it; unset it"
  fi
done

# THE COORDINATOR'S SENTENCE. While the bracketed marker is anywhere in this
# file the pre-registration is unfinished and the run does not start. The
# marker is assembled in two halves so this line does not match itself.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

cd "$worktree" || fail "cd $worktree"

# THE PAIR: the worktree's HEAD is REF, so the reference is the tree the
# candidate's diff is taken against, and the run measures S111 and nothing
# else.
if [[ "$(git rev-parse HEAD)" != "$(git rev-parse "$ref")" ]]; then
  fail "$worktree's HEAD is $(git rev-parse --short HEAD), not REF" \
       "$(git rev-parse --short "$ref") -- the pair would measure more than S111"
fi

# THE TREE IS DIRTY: the candidate is the uncommitted change, so a clean tree
# would be an A/A. `git status --porcelain` counts staged, unstaged and
# untracked alike, as fastchess.sh's own test does through a scratch index
# (which names the candidate); this one only refuses early.
[[ -n "$(git status --porcelain)" ]] \
  || fail "$worktree is clean: there is no candidate change to play"

# BUILD/ WAS BUILT FROM THIS TREE. fastchess.sh plays build/src/chesso as it
# finds it. Checked two ways: `cmake --build` of the engine target first, so a
# stale binary is rebuilt from the tree before the snapshot rather than
# trusted; then no file of the engine's sources -- src/, cmake/ or a
# CMakeLists.txt -- may be newer than the binary, which a build that failed
# part way, or a tree edited after it, would leave behind.
binary="$worktree/build/src/chesso"
cmake --build "$worktree/build" -j "$concurrency" --target chesso > /dev/null \
  || fail "cmake --build $worktree/build --target chesso failed"
[[ -x "$binary" ]] || fail "no engine at $binary after the build"
newer="$(find "$worktree/src" "$worktree/cmake" "$worktree/CMakeLists.txt" \
           -newer "$binary" -type f 2> /dev/null | head -5)"
[[ -z "$newer" ]] || fail "sources newer than $binary: $newer"

[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh"

# THE HARNESS: fastchess.sh must read both names, or it ignores them and plays
# its own `{0, 5}` to 20000 rounds.
for name in SPRT_BOUNDS SPRT_ROUNDS; do
  if ! grep -qE '\$\{?'"$name"'([^A-Za-z0-9_]|$)' ./fastchess.sh; then
    fail "./fastchess.sh does not read $name, so this run's bounds or cap" \
         "would be ignored -- see THE HARNESS in adocs/data/S283_sprt.sh"
  fi
done

shopt -s execfail
exec env -u CAND -u ROUNDS -u BARE_FEN -u SRAND -u AA -u RELAY_PYTHON \
     REF="$ref" \
     TC="$tc" \
     HASH="$hash" \
     CONCURRENCY="$concurrency" \
     SPRT_BOUNDS="$sprt_bounds" \
     SPRT_ROUNDS="$sprt_rounds" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s111_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
