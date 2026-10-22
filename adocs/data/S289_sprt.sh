#!/usr/bin/env bash
#
# S289: THE NULL MOVE'S TABLE GUARD -- no pass at a node whose table entry
# holds a mated upper bound or exact score -- against the tree before it, one SPRT at the
# harness's 8+0.08, Hash 16, {-5, 0} nElo: a correctness fix the BUGS rule
# wants, measured as a non-regression (DEC-305, DEC-171 (c)).
#
#   nohup adocs/data/S289_sprt.sh > .tuning/s289_sprt.log 2>&1 &
#
# Written 2026-10-22 by S289's implementer, in adocs/data/S110_sprt.sh's and
# S083_sprt.sh's shape, whose harness section -- and S283_sprt.sh's before
# them -- this file relies on and does not repeat. Nothing here was run: every
# figure below is read from a file or a git object and names it. The pair does
# not exist yet; the coordinator pins it in the bracketed sentence below, and
# until then the script refuses to start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  S289's fix commit itself -- not a status or pin commit after it, which
#         would make CAND^ a tree that already carries the guard: in
#         `negamax_at`'s null-move block, beside the depth
#         floor, `table_proves_mated` -- the node's table entry is an upper
#         bound or an exact score at or below -MATE_MIN -- turns the pass off;
#         the two cases that hold it (tests/test_search.cpp "a node the table
#         proves mated makes no null move", tests/test_engine.cpp "a root mated
#         in two reports the mate even where its node could pass"), the S170
#         budgets the rule answered on the fix's grid, four mutants. Bench
#         6042933 -> 6042928.
#   REF   CAND's parent, the tree that ships: `4a3a0e5`'s engine (S286's
#         relanded pawn correction, bench 6042933), or a documents-only
#         descendant of it.
#
# The two differ by S289's change alone, which the guards below check: CAND^
# is REF, REF's src/ is byte for byte `4a3a0e5`'s (so "documents-only" is
# checked and not assumed), and CAND's src/ differs in src/search.cpp alone.
#
# WHAT THE VERDICT PRICES, stated before a game. At the node it fires at, the
# guard only removes a pass, and only where the table holds a mated upper bound
# or exact score from a shallower search; there it adds search where a pass
# would have cut and cuts nothing. Elsewhere in the tree it is not neutral: the
# entries that node then stores, and the ordering they feed, can move a score
# or the iteration a mate first appears in either way -- adocs/data/
# S289_reach.txt has a mate in two reported an iteration later and a score
# moved down, and S289_defender_sweep.txt a mate in three lost beside six mates
# in two gained. Its reach in ordinary play, measured on S210's
# census before this run (adocs/data/S289_census.txt): over the 11503 S219 A/A
# positions at depth 10 it changes 145 searches (1.26 %), nodes x1.000109, and
# moves 4 root best moves and 11 root scores. The bench positions move by 5
# nodes, every reply the same; tools/search_bench.py at 9 and 12 is
# node-identical (adocs/data/S289_goldens.txt). The prediction, labelled as one
# and not a measurement: about zero (DEC-019). DEC-242's discharge does not
# apply on its own terms -- the bench moves and root answers move -- so the
# change is decided by a run (MEASUREMENT).
#
# WHY THESE BOUNDS AND THIS CONTROL. `{-5, 0}` nElo at alpha = beta = 0.05:
# the question is whether the fix costs five nElo or more, not whether it
# gains -- S039's and S210 F22's form (fastchess.sh's `--nonreg` pair) for a
# change the rules want for a reason other than strength. The expected zero
# sits on H1's bound, so a true zero ends at H1 with probability 1 - beta and
# a true cost of 5 at H0 with 1 - alpha, the expectation stated first as
# DEC-063 asks. 8+0.08: DEC-286 kept the harness's control for every verdict
# but the correction family's, and this is a search guard. Rejected: `{0, 5}`
# -- asks for a gain nobody predicts, and a true zero sits on H0's bound and
# ends there 95 times in 100, which for a fix would mean reverting a
# correctness change for want of Elo; `{-5, 5}` -- DEC-063's small-change pair,
# 10465 games at a true zero, but a true zero is its midpoint and ends at either
# bound about half the time; the longer control -- DEC-202's reading names no
# search guard and the census gives it no reason.
#
# WHAT THE PAIR COSTS (DEC-143): **25591 games** with the truth on a bound --
# the expected case, a true zero sits on H1's -- 12.13 h at the 2110 games an
# hour `.moltke.local.md` budgets 8+0.08 on `noob_3moves.epd` from (DEC-190);
# **41861 games** with it at the interval's midpoint, 19.84 h (plan.md's "What
# the formula says before a run"; S039_sprt.sh's arithmetic, the same width).
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S283's, S110's and S083's rule, in
# place of fastchess.sh's 20000-round default. Four hours or more, so the run
# starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h), twice the
# cap's hours and the abort below, wall-clock only while the machine is awake.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
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
# it must print, or at 40 h of wall clock with neither bound. Nothing else
# stops it.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S283's lines with this
# run's shas, dates and out directory --
#
#   candidate  cd50178  2026-10-22 15:05:13 +0200
#   reference  4a3a0e5  2026-10-22 12:24:18 +0200
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=-5 elo1=0 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s289_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# Pinned by the coordinator 2026-10-22 15:10: CAND `cd50178` (S289's fix,
# bench 6042928, gate green) and REF its parent `4a3a0e5` (bench 6042933).
# The Open list at main's pin commit re-read: one DEC-171 filler is open,
# **S290** (the S170 budget picker `.tuning/coord/S099/p2/pick_budgets.sh`
# lives outside the tree; S289's fast check), tooling only, reachable in no
# game, so it cannot move this run. The harness is S111's run's of
# 2026-10-22 -- fastchess alpha 1.8.1 20260720-daa3ea2, `noob_3moves.epd`,
# `fastchess.sh` untouched since `be48e02` (last change `769ce7a`, S284), the
# same workstation and kernel 7.2.6 -- so no A/A is owed (DEC-143). Before
# the run: fast suite 50/50 in both builds, Debug self-play 8 games, 0
# Assertion and 0 disconnect, gate_extra 5/5 green. The run starts after
# S083's re-landing has released the machine (DEC-155's night rule: the
# re-landing is the better work meanwhile).
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the guard is accepted as not costing 5 nElo:
#                   H1's 0 preferred to H0's -5 at alpha = beta = 0.05. **The
#                   guard stays**; S289 completes and S083 re-lands on it
#                   (DEC-305). The stopping run's Elo is biased (DEC-063) and
#                   is not the effect size.
#
#   H0           -> the guard is read as costing 5 nElo or more, against a
#                   prediction of zero and a census that moves 4 root answers
#                   in 11503 -- an H0 is then first a reason to look at the run
#                   (forfeits, crashes, the banner) before it is a cost. If the
#                   run stands, **the guard is reverted whole** -- code, cases,
#                   mutants and the budgets it moved, back to REF byte for byte
#                   -- and the defect it closed returns to the record as open:
#                   the shipped engine reports `cp 0` or about -670 for six
#                   proved mates in two (adocs/data/S289_reach.txt). The BUGS
#                   rule wants that defect fixed and the verdict says this fix
#                   costs; the owner rules between the two, DEC-158's form being
#                   the precedent (RFP's depth ceiling's mate price, accepted on
#                   a verdict), or a narrower guard with its own run. S083 stays
#                   paused until the ruling.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   nothing was shown to cost 5 nElo, so **the guard stays**,
#                   as on H1, recorded with the games and the interval -- S039's
#                   reading of the same pair -- and no follow-up run (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {-5, 0} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S289_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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
ref="4a3a0e5015daaae31a1936bfe444eb93affc10c7"
cand="cd501780111c7d769702a8c85a79358501a31e5c"
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=-5 elo1=0 alpha=0.05 beta=0.05"
sprt_rounds="20950"

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

# THE PAIR: REF is CAND's parent, so the run measures S289's guard and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S289"
fi

# The guard is one condition in the search: src/search.cpp must differ and
# nothing else under src/ may.
if git diff --quiet "$ref" "$cand" -- src/search.cpp; then
  fail "src/search.cpp is the same at CAND and REF -- not S289's guard"
fi
if [[ -n "$(git diff --name-only "$ref" "$cand" -- src/ | grep -v '^src/search.cpp$')" ]]; then
  fail "CAND changes src/ beyond src/search.cpp -- the pair would measure more than S289"
fi

# REF's engine is the one that ships at 4a3a0e5: a documents-only descendant
# passes, a tree with any other src/ change does not.
if ! git diff --quiet 4a3a0e5 "$ref" -- src/; then
  fail "REF's src/ differs from 4a3a0e5's -- REF is not the shipped engine or a" \
       "documents-only descendant of it"
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s289_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
