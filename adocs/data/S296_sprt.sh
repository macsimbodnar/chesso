#!/usr/bin/env bash
#
# S296: A NODE THAT SKIPPED MOVES CLAIMS NO MATE IT DID NOT SEARCH -- where a
# shallow-depth rule skipped a move and every move the node searched came back
# mated, the node stores and returns alpha instead of its fail-soft mated best
# (DEC-318's form (a), DEC-319) -- against the tree before it, one SPRT at the
# harness's 8+0.08, Hash 16, {-5, 0} nElo: a correctness fix the BUGS rule
# wants, measured as a non-regression (DEC-318, DEC-171 (c)).
#
#   nohup adocs/data/S296_sprt.sh > .tuning/s296_sprt.log 2>&1 &
#
# Written 2026-10-26 by S296's implementer, in adocs/data/S293_sprt.sh's shape,
# whose harness section -- S283_sprt.sh's before it -- this file relies on and
# does not repeat. Nothing here was run: every figure below is read from a file
# or a git object and names it. The pair does not exist yet; the coordinator
# pins it in the bracketed lines below, and until then the script refuses to
# start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  S296's commit itself -- not a status or pin commit after it, which
#         would make CAND^ a tree that already carries the rule: at the end of
#         `negamax_at`'s move loop, `move_skipped && best_so_far <= -MATE_MIN`
#         makes the node store and return `alpha0` (`node_value`); the probe's
#         `loop_ended` and `loop_best` (src/data_structures.hpp, written only
#         in the probed instantiation, test instrumentation); the case that
#         holds it (tests/test_search.cpp "a node that skipped moves claims no
#         mate it did not search"); two mutants. Bench 5184358 -> 5069022.
#   REF   CAND's parent, the tree that ships: `c8f1744`'s engine (S295's
#         reporting fix, bench 5184358), at `b477459` or a documents-only
#         descendant of it.
#
# The two differ by S296's change alone, which the guards below check: CAND^
# is REF, REF's src/ is byte for byte `c8f1744`'s, and CAND's src/ differs in
# src/search.cpp and src/data_structures.hpp and nowhere else.
#
# WHAT THE VERDICT PRICES, stated before a game. The rule fires in ordinary
# play: S292's census shape (11503 S219 A/A positions at depth 10,
# `adocs/data/S296_census.py`, `adocs/data/S296_forms.txt` section 5) counts
# 5715 firings in 1532 searches, and the rule moves 189 root best moves, 504
# root scores and 954 node counts there, every one in a search where it fired,
# at a median score change of 11 cp; nodes x1.000063. Over the mate sets it
# removes every reported mate shorter than the oracle's (the shipped tree's 2
# iterations on S289's four sets, 16 searches on S294's grid, 28 on the S170
# grid), keeps both mate floors (mined 160, mates in three 13), gains and loses
# exact mates by tree shape (S296_forms.txt sections 1 to 4) and loses S145
# root 76 through depth 21 to the null move and reverse futility where the rule
# does not fire (`adocs/data/S296_root76.txt`). S295's chain reads the oracle's
# mate 5 at depth 8 where the shipped tree reads mate 4. **The expectation**,
# labelled as one and not a measurement: zero (DEC-019) -- a bound at alpha in
# place of a mate distance no search established changes which subtrees the
# table cuts, either way. DEC-242's discharge does not apply: root moves and
# scores move in ordinary play (MEASUREMENT).
#
# WHY THESE BOUNDS AND THIS CONTROL. S293_sprt.sh's, unchanged and for its
# reasons: `{-5, 0}` nElo at alpha = beta = 0.05, the non-regression pair for a
# change the rules want for a reason other than strength (S039's, S210 F22's,
# S289's, S292's, S293's); a true zero sits on H1's bound and ends there with
# probability 1 - beta, a true cost of 5 at H0 with 1 - alpha (DEC-063).
# 8+0.08: DEC-286's harness control for a search guard. Rejected: `{0, 5}` -- a
# true zero would end at H0 95 times in 100 and revert a correctness fix for
# want of Elo; `{-5, 5}` -- a true zero is its midpoint and ends at either
# bound about half the time; the longer control -- nothing in the census gives
# it a reason.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic for this width
# (Van den Bergh, `adocs/testing_strategy.md` section 1.1; S289_sprt.sh,
# S292_sprt.sh and S293_sprt.sh carry the same figures): **25591 games** with
# the truth on a bound -- the expected case, a true zero sits on H1's --
# 12.13 h at the 2110 games an hour `.moltke.local.md` budgets 8+0.08 on
# `noob_3moves.epd` from (DEC-190); **41861 games** with it at the interval's
# midpoint, the worst case, 19.84 h. S292's run played 2140.9 games an hour,
# S293's 2139.1.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S289's, S292's and S293's rule. Four
# hours or more, so the run starts by DEC-155's night rule. Watcher ceiling
# 144000 s (40 h), twice the cap's hours and the abort below, wall-clock only
# while the machine is awake.
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
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S293's lines with this
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
#   out        /home/max/ws/chesso/.tuning/sprt_s296_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# none expected -- at S296's start the plan's todo list holds no DEC-171
# filler (adocs/plan_todo/, 2026-10-26). The harness is S293's and S135's
# runs': `fastchess.sh` last changed `769ce7a` (S284, 2026-10-15), the same
# fastchess, book and workstation, so no A/A is owed (DEC-143).
# Pinned by the coordinator 2026-10-26 16:45: CAND `25b332b` (S296's rule,
# bench 5069022, gate green) and REF its parent `b477459` (bench 5184358, src/
# equal to `c8f1744`'s, S295's reporting fix). The Open list at main's pin
# commit re-read: no DEC-171 filler is open (adocs/plan_todo/ holds none). The
# harness is S135's run's of 2026-10-26 -- fastchess alpha 1.8.1
# 20260720-daa3ea2, `noob_3moves.epd`, `fastchess.sh` last changed `769ce7a`
# (S284), the same workstation and kernel 7.2.6 -- so no A/A is owed
# (DEC-143). Before the run: fast suite 50/50 in both builds, Debug self-play
# 8 games on S296's tree, 0 Assertion and 0 disconnect, gate_extra 5/5 green
# on S296's tree (src/ changed in comments only after both). Root 76's ruling
# and the form's are DEC-319's. The run starts when the workstation is back
# on (the owner shut it down before it began): 12 to 20 h is the night's run
# and the day's (DEC-155), and nothing better needs the machine, since S136
# waits behind S296 by the BUGS rule.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the rule is accepted as not costing 5 nElo: H1's
#                   0 preferred to H0's -5 at alpha = beta = 0.05. **The rule
#                   stays**; S296 completes and S136 is next (DEC-318). S145
#                   root 76's mate stays lost through depth 21 as the null
#                   move's and reverse futility's price on the changed tree,
#                   with S296's attribution (adocs/data/S296_root76.txt). The
#                   stopping run's Elo is biased (DEC-063) and is not the
#                   effect size.
#
#   H0           -> the rule is read as costing 5 nElo or more, against an
#                   expectation of zero -- an H0 is first a reason to look at
#                   the run (forfeits, crashes, the banner) before it is a
#                   cost. If the run stands, **the rule is reverted whole** --
#                   code, the probe's two fields, case and mutants, E09's
#                   anchor re-pointed back, every golden S296 moved back by its
#                   script, to REF byte for byte -- the defect returns to the
#                   record as open (MANUAL.md's Known bugs), and the owner
#                   rules (DEC-265) between the BUGS rule and the measured
#                   cost, DEC-158's form the precedent; form (b) is not tried
#                   instead without that ruling (it drops EXACT_FLOOR, DEC-319).
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   nothing was shown to cost 5 nElo, so **the rule stays**, as
#                   on H1, recorded with the games and the interval, and no
#                   follow-up run (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {-5, 0} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S296_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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
ref="b477459e0372ad83839cdd3de1ba88a500980fcb"
cand="25b332b6e0d00b817588e33da20a20f25027f46a"
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

# THE PAIR: REF is CAND's parent, so the run measures S296's rule and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S296"
fi

# The rule is in src/search.cpp, and the probe's two fields it fills in
# src/data_structures.hpp: src/search.cpp must differ, and nothing under src/
# beyond those two may.
if git diff --quiet "$ref" "$cand" -- src/search.cpp; then
  fail "src/search.cpp is the same at CAND and REF -- not S296's rule"
fi
changed="$(git diff --name-only "$ref" "$cand" -- src/)"
if [[ -n "$(grep -vxF -e 'src/search.cpp' -e 'src/data_structures.hpp' <<< "$changed")" ]]; then
  fail "CAND changes src/ beyond src/search.cpp and src/data_structures.hpp --" \
       "the pair would measure more than S296"
fi

# REF's engine is the one that ships at c8f1744: a documents-only descendant
# passes, a tree with any other src/ change does not.
if ! git diff --quiet c8f1744 "$ref" -- src/; then
  fail "REF's src/ differs from c8f1744's -- REF is not the shipped engine or a" \
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s296_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
