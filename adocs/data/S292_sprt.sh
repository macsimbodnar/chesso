#!/usr/bin/env bash
#
# S292: THE NULL MOVE'S TABLE GUARD, NARROWED TO A WHOLE SEARCH'S BOUND -- no
# pass at a node whose table entry is a mated upper bound or exact score
# written by a node that searched every legal move -- against the tree before
# it, one SPRT at the harness's 8+0.08, Hash 16, {-5, 0} nElo: a correctness fix
# the BUGS rule wants, measured as a non-regression (DEC-309 (3), DEC-171 (c)).
#
#   nohup adocs/data/S292_sprt.sh > .tuning/s292_sprt.log 2>&1 &
#
# Written 2026-10-22 by S292's implementer, in adocs/data/S289_sprt.sh's shape,
# whose harness section -- S283_sprt.sh's before it -- this file relies on and
# does not repeat. Nothing here was run: every figure below is read from a file
# or a git object and names it. The pair does not exist yet; the coordinator
# pins it in the bracketed lines below, and until then the script refuses to
# start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  S292's commit itself -- not a status or pin commit after it, which
#         would make CAND^ a tree that already carries the guard: in
#         `negamax_at`'s null-move block, beside the depth floor,
#         `whole_search_found_mated` -- the node's entry is an upper bound or
#         an exact score at or below -MATE_MIN and its `no_move_skipped` byte
#         is set -- turns the pass off; the byte in `tt_entry_t`, written by
#         `tt_store_entry()` and passed by negamax's own store as "no
#         shallow-depth rule skipped a move here"; the three cases that hold
#         it (tests/test_search.cpp "a node a whole search found mated makes no
#         null move" and "a node that skips a move says so in its entry",
#         tests/test_engine.cpp "a root mated in two reports the mate even
#         where its node could pass"); nine mutants. Bench 6042933 -> 6042933:
#         the bench positions never meet the guard's condition on a whole
#         search's bound.
#   REF   CAND's parent, the tree that ships: `4a3a0e5`'s engine (S289's guard
#         reverted, bench 6042933), at `10f1e42` or a documents-only
#         descendant of it.
#
# The two differ by S292's change alone, which the guards below check: CAND^
# is REF, REF's src/ is byte for byte `4a3a0e5`'s, and CAND's src/ differs in
# src/search.cpp, src/data_structures.hpp, src/transposition_table.hpp and
# src/transposition_table.cpp and nowhere else.
#
# WHAT THE VERDICT PRICES, stated before a game. S289's guard read H0 here
# (`adocs/data/S289_sprt.log`, nElo -10.68 +/- 7.29). S292's census
# (`adocs/data/S292_cost.txt`, positions from that run's own games) found 98 to
# 99 % of its firings on bounds from nodes that skipped moves, which are not
# proofs -- the node's own search refutes the mate behind 78 to 81 % of them --
# and every root best move it moved moved by those. This guard keeps only the
# rest: the firings on bounds of whole searches, where the six proved mates in
# two S289 found hidden are (`mate -2` at 31 of 31 S165 roots, the reverted tree
# 25). At the node it fires at it only removes a pass; elsewhere the entries
# that node then stores can move a score or the iteration a mate first appears
# in either way. Its reach in ordinary play, measured before this run, the
# frozen candidate against the reverted tree, identical row for row to the
# census's mode 2:
#
#   3000 positions of S289's games, depth 14   3 searches moved, 0 best moves,
#                                              1 score, nodes x1.000003
#   12408 replayed moves of 200 of its games,  13 searches moved, 0 best moves,
#   depth 14, the table carried                2 scores, nodes x0.999944
#   the same replays at the game's own nodes   53 searches moved, 10 best
#                                              moves, 32 scores
#   S210's census shape, 11503 S219 A/A        see adocs/data/S292_census_s210.txt
#   positions at depth 10
#
# against S289's guard over the same three: 227, 2556 and 2341 searches moved,
# 6, 670 and 747 best moves. The bench positions are node-identical, every reply
# the same; tools/search_bench.py at 9 and 12 node- and move-identical
# (adocs/data/S292_goldens.txt). The prediction, labelled as one and not a
# measurement: zero (DEC-019). DEC-242's discharge does not apply on its own
# terms -- root scores move in the census, and the change removes passes rather
# than adding exact cuts -- so the change is decided by a run (MEASUREMENT).
#
# WHY THESE BOUNDS AND THIS CONTROL. S289_sprt.sh's, unchanged and for its
# reasons: `{-5, 0}` nElo at alpha = beta = 0.05, the non-regression pair for a
# change the rules want for a reason other than strength (S039's, S210 F22's,
# S289's); a true zero sits on H1's bound and ends there with probability
# 1 - beta, a true cost of 5 at H0 with 1 - alpha (DEC-063). 8+0.08: DEC-286's
# harness control for a search guard. Rejected: `{0, 5}` -- a true zero would
# end at H0 95 times in 100 and revert a correctness fix for want of Elo;
# `{-5, 5}` -- a true zero is its midpoint and ends at either bound about half
# the time; the longer control -- the census gives it no reason. And rejected,
# the same pair re-run on S289's guard: DEC-309 refused it, and this is not it.
#
# WHAT THE PAIR COSTS (DEC-143), S289_sprt.sh's arithmetic for the same width:
# **25591 games** with the truth on a bound -- the expected case, a true zero
# sits on H1's -- 12.13 h at the 2110 games an hour `.moltke.local.md` budgets
# 8+0.08 on `noob_3moves.epd` from (DEC-190); **41861 games** with it at the
# interval's midpoint, 19.84 h. S289's run played 2151.5 games an hour.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S289's rule. Four hours or more, so the
# run starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h), twice the
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
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S289's lines with this
# run's shas, dates and out directory --
#
#   candidate  <CAND, short>  <its commit date>
#   reference  <REF, short>   <its commit date>
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=-5 elo1=0 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s292_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# Pinned by the coordinator 2026-10-22 23:23: CAND `e27d3e5` (S292's fix,
# bench 6042933, gate green) and REF its parent `10f1e42` (bench 6042933,
# S289's guard reverted by `e47eb4a`, src/ equal to `4a3a0e5`'s). The Open list
# at main's pin commit re-read: one DEC-171 filler is open, **S290** (the S170
# budget picker lives outside the tree), tooling only, reachable in no game, so
# it cannot move this run; S291 diagnoses S083's re-landed constants, which
# neither build carries. The harness is S289's run's of 2026-10-22 --
# fastchess alpha 1.8.1 20260720-daa3ea2, `noob_3moves.epd`, `fastchess.sh`
# last changed `769ce7a` (S284), the same workstation and kernel 7.2.6 -- so
# no A/A is owed (DEC-143). Before the run: fast suite 50/50 in both builds,
# Debug self-play 8 games, 0 Assertion and 0 disconnect, gate_extra 5/5 green.
# The run starts at once: about 12 h is the night's run (DEC-155), and nothing
# better needs the machine, since S291 and S083 wait behind S292 by the BUGS
# rule.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the narrowed guard is accepted as not costing 5
#                   nElo: H1's 0 preferred to H0's -5 at alpha = beta = 0.05.
#                   **The guard stays**; S292 completes, S289 completes on it
#                   (its `paused_by` clears), the six proved mates in two are
#                   closed, and S083 re-lands on this tree (DEC-307, DEC-309).
#                   The stopping run's Elo is biased (DEC-063) and is not the
#                   effect size.
#
#   H0           -> the guard is read as costing 5 nElo or more, against a
#                   prediction of zero and a census that moves no root best
#                   move at fixed depth -- an H0 is then first a reason to look
#                   at the run (forfeits, crashes, the banner) before it is a
#                   cost. If the run stands, **the guard is reverted whole** --
#                   code, the entry's byte, cases and mutants, back to REF byte
#                   for byte -- the defect returns to the record as open, and
#                   the owner rules (DEC-309 (3), DEC-265) between the BUGS rule
#                   and two measured costs, with S292's figures beside S289's;
#                   DEC-158's form is the precedent. S083 stays paused until the
#                   ruling.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   nothing was shown to cost 5 nElo, so **the guard stays**,
#                   as on H1, recorded with the games and the interval, and no
#                   follow-up run (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {-5, 0} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S292_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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
# S289_sprt.sh's same block reads them: pinned, refused if set to anything
# else, and passed on the exec line; ROUNDS, BARE_FEN, SRAND and AA refused if
# set at all; RELAY_PYTHON unset; OUT overridden.
ref="10f1e42a0f2b3197d38bce7ae6eef61a19748aab"
cand="e27d3e5e1415b0d7ed5d59561ac2ee9747637fbd"
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=-5 elo1=0 alpha=0.05 beta=0.05"
sprt_rounds="20950"

# THE COORDINATOR'S SENTENCE AND SHAS. While the bracketed marker is anywhere
# in this file the pre-registration is unfinished and the run does not start.
# The marker is assembled in two halves so this line does not match itself.
# Checked before the environment, because an unpinned sha would read there as
# a mismatch and name the wrong fault.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); pin the pair and fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

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

cd /home/max/ws/chesso || fail "cd"

# THE PAIR: REF is CAND's parent, so the run measures S292's guard and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S292"
fi

# The guard is one condition in the search and one byte in the entry: those
# four files must differ in the guard's direction, and nothing else under src/
# may. src/search.cpp has to be among them.
if git diff --quiet "$ref" "$cand" -- src/search.cpp; then
  fail "src/search.cpp is the same at CAND and REF -- not S292's guard"
fi
allowed='^src/(search\.cpp|data_structures\.hpp|transposition_table\.hpp|transposition_table\.cpp)$'
if [[ -n "$(git diff --name-only "$ref" "$cand" -- src/ | grep -vE "$allowed")" ]]; then
  fail "CAND changes src/ beyond the guard's four files -- the pair would measure more than S292"
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s292_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
