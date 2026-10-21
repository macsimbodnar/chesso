#!/usr/bin/env bash
#
# S110: THE NON-PAWN CORRECTION TABLES -- the two non-pawn keys and the two
# tables keyed on them against the tree before them, one SPRT at 32+0.32, Hash
# 64, {0, 10} nElo, pre-registered by the coordinator under the owner's
# standing instruction (DEC-297).
#
#   nohup adocs/data/S110_sprt.sh > .tuning/s110_sprt.log 2>&1 &
#
# Written 2026-10-21 in S110's phase 1, in S283's shape (adocs/data/S283_sprt.sh,
# whose harness section this file relies on and does not repeat), while S082's
# `compare` held the machine. Nothing here was run: every figure below is read
# from a file or a git object and names it. The pair does not exist yet; the
# coordinator pins it in the bracketed sentence below, and until then the
# script refuses to start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  S110's commit: `board_t::nonpawn_hash`, one key per colour over that
#         colour's knights, bishops, rooks, queens and king, kept by
#         add_piece, remove_piece and move_piece and undone by xor in
#         unmake_move_impl; `nonpawn_correction_t`, two tables of the pawn
#         table's kind, 16384 slots a side each (the census,
#         `S110_nonpawn_key_census.txt`), read with the pawn table's at
#         `negamax_at`'s compute-or-read site and averaged with it at
#         `CorrHistNonPawnWeight` 100 -- the pawn read and the pair's mean
#         half each, rounded, inside CorrHistMax (DEC-300) -- and taught
#         beside it under the same gate. The UCI layer owns them, datagen
#         gives each worker its own.
#   REF   CAND's parent: S286's relanded pawn table (`816d70c`) with S082's
#         leaf walks (`5b0e170`, read by datagen alone) and S287's comments
#         after it, bench 6042933 at `a8c7e3b`, S110's phase-1 base (src/
#         as at `3da69c5`).
#
# The two differ by S110's change alone, which the guard below checks (CAND^
# is REF). What the tree does at phase 1 (S110's step file, the DEC-300
# section): bench 6042933 -> 6570375, the second and fifth `bench 12` replies
# and the second `bench` reply moved; the tune build at
# `CorrHistNonPawnWeight` 0 prints the parent's bench stream, 121 lines, none
# differing but nps (DEC-215).
#
# THE FAMILY'S READINGS, quoted and not re-derived:
#   - S099's pawn table at the harness's 8+0.08, `{0, 5}`, Hash 16: H0, nElo
#     -4.71 +/- 6.84 over 9916 games (`adocs/data/S099_sprt.log`).
#   - The same pair at 32+0.32, `{0, 10}`, Hash 64 (S283): H1, nElo +12.56
#     +/- 9.89 over 4738 games (`adocs/data/S283_sprt.log`). Relanded by S286.
# DEC-297 books this run at the second's shape for that reason: the family's
# one measured member gained at the longer control and not at the harness's,
# and the Chess Programming Wiki's page on the technique says its gains grow
# with the time control. A record of direction, never a prediction (DEC-019).
#
# DEC-221 and the copying rules: S110 is implemented from the Chess
# Programming Wiki's description of the non-pawn key ("the location of all
# pieces belonging to one color, excluding Pawns", in pairs, one per side)
# and S099's own table; its two constants are DEC-134 forms (b) and (c),
# argued at their sites, and its blend is DEC-300's weighted average. No engine's code, constant or table.
#
# THE HARNESS. fastchess.sh is unchanged since `be48e02` (`git log
# be48e02..3da69c5 -- fastchess.sh` empty, 2026-10-21), so every line number
# S283_sprt.sh's HARNESS section cites into it -- REF and CAND, TC, HASH,
# SPRT_BOUNDS, SPRT_ROUNDS, OUT, the banner -- holds here. The guard below
# refuses unless fastchess.sh still reads both SPRT names.
#
# BOUNDS. `elo0=0 elo1=10 alpha=0.05 beta=0.05`, `model=normalized` (nElo),
# DEC-297's pair.
#
# WHAT THE PAIR COSTS (DEC-143), S283's arithmetic, the same width: D / (e1 -
# e0)^2 with D = 1046535 at alpha = beta = 0.05 is **10465 games** with the
# truth at the interval's midpoint, 19.6 h at 534 an hour and 19.2 h at 545;
# 639770 / 100, **6398 games**, with it on a bound, 12.0 h and 11.7 h. The two
# throughputs are `.moltke.local.md`'s at 32+0.32, Hash 64, `noob_3moves.epd`,
# concurrency 12; S283's own 4738 games read 8 h 48 m, 538 an hour, between
# them.
#
# THE CAP. **5250 rounds, 10500 games**, 19.7 h at 534 an hour: the midpoint
# figure rounded up to a whole hundred, as S283's. Four hours or more, so the
# run starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h), at
# least twice the cap and DEC-297's abort at 2x, wall-clock only while the
# machine is awake.
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
#   candidate  <CAND>  <its committer date>
#   reference  <REF>  <its committer date>
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 32+0.32  hash 64  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=10 alpha=0.05 beta=0.05
#   cap        5250 rounds, 10500 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s110_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 10500.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171),
# from DEC-297: S288 (tools/eval_spread's margins, tooling only, reachable in
# no game). [COORDINATOR, on the day it pins: CAND's and REF's shas into
# `ref=` and `cand=` below, their dates into the banner lines above, the
# Open list re-read, and this bracketed sentence replaced by what was found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063),
# as DEC-297 states the outcomes.
#
#   H1 accepted  -> at 32+0.32, Hash 64, the non-pawn tables are accepted as
#                   gaining: H1's 10 nElo preferred to H0's 0 at alpha = beta
#                   = 0.05. **S110 ships.** The stopping run's Elo is
#                   upward-biased (DEC-063) and is not the effect size. S111
#                   is pre-registered the same way unless this reading says
#                   otherwise, its own verdict.
#
#   H0           -> not shown to gain at this control. **S110 is reverted
#                   whole** and recorded as zero; the pawn table stays (S286).
#                   S111 still runs its own verdict (DEC-297). A small true
#                   gain, under about 5 nElo, reads H0 here more often than at
#                   `{0, 5}`; that is the price of a bounded run, and it is
#                   recorded beside the H0.
#
#   No verdict   -> the 10500-game cap, or the 40 h abort, with neither bound:
#                   read as H0 is -- reverted whole, recorded as zero, with the
#                   games and the interval -- and no follow-up run (DEC-063,
#                   DEC-297).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 32+0.32, Hash=64, noob_3moves.epd, {0, 10} nElo` and the `Elo`,
# `LLR`, `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S110_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
# regenerates plan.md's ledger table from it.
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
# else, and passed on the exec line; ROUNDS, BARE_FEN, SRAND and AA refused if
# set at all; RELAY_PYTHON unset; OUT overridden.
ref="[COORDINATOR: REF's sha]"
cand="[COORDINATOR: CAND's sha]"
tc="32+0.32"
hash="64"
concurrency="12"
sprt_bounds="elo0=0 elo1=10 alpha=0.05 beta=0.05"
sprt_rounds="5250"

for pair in "REF=$ref" "CAND=$cand" "TC=$tc" "HASH=$hash" \
            "CONCURRENCY=$concurrency" "SPRT_BOUNDS=$sprt_bounds" \
            "SPRT_ROUNDS=$sprt_rounds"; do
  name="${pair%%=*}"
  want="${pair#*=}"
  if [[ -n "${!name+set}" && "${!name}" != "$want" ]]; then
    fail "$name is '${!name}' in the environment, and this run is booked at" \
         "$name=$want; unset it"
  fi
done
for name in ROUNDS BARE_FEN SRAND AA; do
  if [[ -n "${!name+set}" ]]; then
    fail "$name is set in the environment ('${!name}'), and this run is" \
         "booked without it; unset it"
  fi
done

# THE COORDINATOR'S SENTENCE AND SHAS. While the bracketed marker is anywhere
# in this file the pre-registration is unfinished and the run does not start.
# The marker is assembled in two halves so this line does not match itself.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); pin the pair and fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

cd /home/max/ws/chesso || fail "cd"

# THE PAIR: REF is CAND's parent, so the run measures S110 and nothing else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S110"
fi

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
exec env -u ROUNDS -u BARE_FEN -u SRAND -u AA -u RELAY_PYTHON \
     REF="$ref" \
     CAND="$cand" \
     TC="$tc" \
     HASH="$hash" \
     CONCURRENCY="$concurrency" \
     SPRT_BOUNDS="$sprt_bounds" \
     SPRT_ROUNDS="$sprt_rounds" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s110_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
