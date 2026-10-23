#!/usr/bin/env bash
#
# S293: REVERSE FUTILITY'S TABLE GUARD -- no reverse futility at a node whose
# table entry is a mated upper bound or exact score written by a node that
# searched every legal move, S292's condition asked of the null move's static
# form -- against the tree before it, one SPRT at the harness's 8+0.08, Hash
# 16, {-5, 0} nElo: a correctness fix the BUGS rule wants, measured as a
# non-regression (DEC-311 (2), DEC-171 (c)).
#
#   nohup adocs/data/S293_sprt.sh > .tuning/s293_sprt.log 2>&1 &
#
# Written 2026-10-23 by S293's implementer, in adocs/data/S292_sprt.sh's shape,
# whose harness section -- S283_sprt.sh's before it -- this file relies on and
# does not repeat. Nothing here was run: every figure below is read from a file
# or a git object and names it. The pair does not exist yet; the coordinator
# pins it in the bracketed lines below, and until then the script refuses to
# start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  S293's commit itself -- not a status or pin commit after it, which
#         would make CAND^ a tree that already carries the guard: in
#         `negamax_at`, `whole_search_mated_entry` -- the node's entry is an
#         upper bound or an exact score at or below -MATE_MIN and its
#         `no_move_skipped` byte is set -- is one more conjunct of reverse
#         futility's condition; the case that holds it (tests/test_search.cpp
#         "reverse futility does not fire at a node a whole search found
#         mated"); five mutants. Bench 6042933 -> 6042933: the bench positions
#         never meet the guard's condition.
#   REF   CAND's parent, the tree that ships: `e27d3e5`'s engine (S292's
#         narrowed null-move guard, bench 6042933), at `6ebeadc` or a
#         documents-only descendant of it.
#
# The two differ by S293's change alone, which the guards below check: CAND^
# is REF, REF's src/ is byte for byte `e27d3e5`'s, and CAND's src/ differs in
# src/search.cpp and nowhere else.
#
# WHAT THE VERDICT PRICES, stated before a game. S291 found reverse futility
# returning a fail-high, beta outside the mate band, at nodes whose entry is a
# whole search's mated bound: 90 returns over S289's four mate sets
# (`adocs/data/S291_reach.txt` section 3) and **0 in 11503 ordinary-play
# positions at depth 10** (`adocs/data/S291_census.txt`, 981078145 nodes; the
# scratch guard moved no node, score or best move there). Over the mate sets
# the guard moves eight reported scores and two best moves, gains no exact mate
# and loses mined row 171's at depth 10 -- traced to reverse futility at
# another node of the changed tree, with no entry at all, DEC-158's price
# (`adocs/data/S293_row171.txt`). The shipped form reads the same, row for
# row, and the same census, Release against Release
# (`adocs/data/S293_reach.txt`). At the node it fires at it only removes a cut;
# elsewhere the entries that node then stores can move a score or the
# iteration a mate first appears in, either way. The bench positions are
# node-identical, every reply the same; tools/search_bench.py at 9 and 12 node-
# and move-identical (`adocs/data/S293_goldens.txt`). **The expectation**,
# labelled as one and not a measurement: zero (DEC-019). With no firing in
# ordinary play at depth 10 the two engines play nearly the same games, so the
# run is expected to sit near a true zero and may well run to its cap rather
# than end at a bound; that is the honest cost of measuring a change that rarely
# fires, and DEC-311 accepted it before the run. DEC-242's discharge does not
# apply on its own terms -- root scores move in the mate sets, and the change
# removes cuts rather than adding exact ones -- so the change is decided by a
# run (MEASUREMENT).
#
# WHY THESE BOUNDS AND THIS CONTROL. S292_sprt.sh's, unchanged and for its
# reasons: `{-5, 0}` nElo at alpha = beta = 0.05, the non-regression pair for a
# change the rules want for a reason other than strength (S039's, S210 F22's,
# S289's, S292's); a true zero sits on H1's bound and ends there with
# probability 1 - beta, a true cost of 5 at H0 with 1 - alpha (DEC-063).
# 8+0.08: DEC-286's harness control for a search guard. Rejected: `{0, 5}` -- a
# true zero would end at H0 95 times in 100 and revert a correctness fix for
# want of Elo; `{-5, 5}` -- a true zero is its midpoint and ends at either
# bound about half the time; the longer control -- nothing in the census gives
# it a reason.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic for this width
# (Van den Bergh, `adocs/testing_strategy.md` section 1.1; S289_sprt.sh and
# S292_sprt.sh carry the same figures): **25591 games** with the truth on a
# bound -- the expected case, a true zero sits on H1's -- 12.13 h at the 2110
# games an hour `.moltke.local.md` budgets 8+0.08 on `noob_3moves.epd` from
# (DEC-190); **41861 games** with it at the interval's midpoint, the worst case,
# 19.84 h. S292's run played 2140.9 games an hour.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S289's and S292's rule. Four hours or
# more, so the run starts by DEC-155's night rule. Watcher ceiling 144000 s
# (40 h), twice the cap's hours and the abort below, wall-clock only while the
# machine is awake.
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
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S292's lines with this
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
#   out        /home/max/ws/chesso/.tuning/sprt_s293_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# [COORDINATOR: the pin -- CAND and REF with their benches and gate, the Open
# list re-read at the pin commit, the harness against the last A/A, the Debug
# self-play and gate_extra figures, and when the run starts.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the guard is accepted as not costing 5 nElo: H1's
#                   0 preferred to H0's -5 at alpha = beta = 0.05. **The guard
#                   stays**; S293 completes, S083's `paused_by` clears, and
#                   S083 re-lands on this tree (DEC-311). Mined row 171's
#                   depth-10 mate stays lost as DEC-158's price, with S293's
#                   attribution. The stopping run's Elo is biased (DEC-063) and
#                   is not the effect size.
#
#   H0           -> the guard is read as costing 5 nElo or more, against an
#                   expectation of zero and a census in which it never fires
#                   -- an H0 is then first a reason to look at the run
#                   (forfeits, crashes, the banner) before it is a cost. If the
#                   run stands, **the guard is reverted whole** -- code, case
#                   and mutants, and the two S191 anchors re-pointed back, to
#                   REF byte for byte -- the defect returns to the record as
#                   open (MANUAL.md's Known bugs), and the owner rules (DEC-265)
#                   between the BUGS rule and the measured cost, DEC-158's form
#                   the precedent. S083 stays paused until the ruling.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   nothing was shown to cost 5 nElo, so **the guard stays**,
#                   as on H1, recorded with the games and the interval, and no
#                   follow-up run (DEC-063). Given the census, the expected
#                   outcome is either H1 or this one.
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {-5, 0} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S293_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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
# S292_sprt.sh's same block reads them: pinned, refused if set to anything
# else, and passed on the exec line; ROUNDS, BARE_FEN, SRAND and AA refused if
# set at all; RELAY_PYTHON unset; OUT overridden.
ref="[COORDINATOR: REF, the full sha of CAND's parent]"
cand="[COORDINATOR: CAND, the full sha of S293's commit]"
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

# THE PAIR: REF is CAND's parent, so the run measures S293's guard and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S293"
fi

# The guard is one conjunct of reverse futility's condition: src/search.cpp
# must differ, and nothing else under src/ may.
if git diff --quiet "$ref" "$cand" -- src/search.cpp; then
  fail "src/search.cpp is the same at CAND and REF -- not S293's guard"
fi
if [[ -n "$(git diff --name-only "$ref" "$cand" -- src/ | grep -vxF 'src/search.cpp')" ]]; then
  fail "CAND changes src/ beyond src/search.cpp -- the pair would measure more than S293"
fi

# REF's engine is the one that ships at e27d3e5: a documents-only descendant
# passes, a tree with any other src/ change does not.
if ! git diff --quiet e27d3e5 "$ref" -- src/; then
  fail "REF's src/ differs from e27d3e5's -- REF is not the shipped engine or a" \
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s293_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
