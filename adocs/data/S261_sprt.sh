#!/usr/bin/env bash
#
# S261: THE SEARCH BLOCK'S TUNED VECTOR, VERIFIED. The independent gainer SPRT
# that decides whether the vector `adocs/data/S261_spsa.sh`'s lane returned
# ships (accepts (5), DEC-222 (8)). Written before the lane plays a game, which
# is what makes everything below a pre-registration.
#
#   nohup adocs/data/S261_sprt.sh > .tuning/s261_sprt.log 2>&1 &
#   echo $! > .tuning/s261_sprt.pid
#
# THE CANDIDATE. One commit whose src/ change is the lane's vector and nothing
# else: the defaults in `src/search_params.hpp`, written from the driver's own
# rounded JSON (the last JSON block of the lane's log, never retyped), less any
# axis the end-of-range rule or the mate suites held at its incumbent (step
# file, item (4)). No rule of the search changes; what moves is integers. The
# same commit carries `tests/test_search_params.cpp`'s pins and `MANUAL.md`'s
# default column, which describe the values and do not change play.
#
# THE REFERENCE IS THE UNTUNED TREE, AND IT IS THE TREE THE LANE FITTED. REF is
# the candidate's parent. Before pinning:
#
#   git diff <lane launch head> <REF> -- src/      -> must be empty
#
# where the launch head is the `head` line of the lane's own banner. Empty
# means the vector was fitted on exactly the engine it is measured against.
# S278 (trace-only, bench-identical) lands **before** the lane launches, so the
# launch head already carries it and it is not in this diff.
#
# ONE EXCEPTION, written before the lane: a src/ change that landed between the
# launch and the vector **in a commit of its own that proved INV-6** -- `chesso
# bench`'s total and all eight replies identical to its parent's, and
# `tools/search_bench.py` identical in node counts and best moves at depths 9
# and 12, both recorded in that commit or its step file, with a
# `No functional change` trailer -- does not count: the engine the lane fitted
# and the engine REF builds search identically. Every other src/ difference
# means the lane priced a tree that is not the reference; stop and put it to
# the owner.
#
# INDEPENDENT, AND WHAT THAT WORD BUYS. The lane tuned at 2+0.02 on
# `books/UHO_4060_v3.epd` against itself; this run plays 8+0.08 on
# `books/noob_3moves.epd` against the untuned tree. No opening, no control and
# no game is shared, so the SPSA's own score -- which is not a measurement of
# anything -- cannot leak into the verdict (DEC-019, `adocs/eval_tuning_strategy.md`
# par.7, DEC-209 clause 1). S085 is the precedent: +21.02 at a control and book
# its run never saw.
#
# THE VECTOR, incumbent -> fitted, appended below this paragraph by the
# coordinator when CAND is pinned, from the driver's JSON and marking every
# held axis; nothing above this paragraph is edited then.
#
#   (PIN_ME: thirty-one rows -- the reachability check removed `SePlyFactor`
#   on 2026-10-13. `LmrBase` and `LmrDivisor` are among them by DEC-269, so
#   this verdict can move the reduction table itself.)
#
# BOUNDS. elo0=0 elo1=5 nElo, alpha=beta=0.05, `model=normalized` --
# `fastchess.sh`'s default, the gainer pair the step's accepts names. No
# published expectation sizes a vector of first settings fitted on this engine,
# so the bound stays where the step put it (DEC-063). S085's +21.02 and S222's
# +11.13 are this project's two lanes that read H1 and S231's -2.65 the one that
# read H0; records, not a prior.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint, **25591** on a bound (`adocs/testing_strategy.md`
# section 1.1, D / (e1 - e0)^2 with D = 1046535, and 639770 / 25), at **2110
# games an hour** (`.moltke.local.md`): **19.8 h** and **12.1 h**. The cap is
# `fastchess.sh`'s 20000 rounds, 40000 games, 19.0 h. Four hours or more, so a
# night (DEC-155). An effect well outside the interval ends far sooner: S085's
# took 2946 games and S222's 6278.
#
# REGIME. 8+0.08, Hash 16 (DEC-088), one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 (DEC-050), `fastchess.sh`'s
# adjudication, whatever governor the machine is under, recorded and not set
# (DEC-195). If fastchess, the book, the adjudication or the machine moved since
# the last fixed-rounds A/A, that A/A of 1000 games comes first (DEC-143).
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Stop it and report nothing, too, if
# the machine loses mains power (POWER) or another load starts on it while the
# run plays (MACHINE). Nothing else stops it -- not the length.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171),
# re-read at `1edb3b1`: **S278** unless it lands first -- the sanitizer build's
# trace tests fail under UBSan on `eval_trace::sink`'s store, red since S262;
# tooling, compiled only into the trace library and never into `chesso`, so
# nothing reachable in play. S259 (the multicut and ProbCut mate rows) is
# closed. S099 is in progress and lands after this verdict (DEC-271). The
# coordinator re-reads the Open list on the day it pins the pair and names
# anything else.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> the vector gains at least 5 nElo over the untuned tree at
#                   the harness setting. It ships as CAND carries it. The
#                   stopping run's Elo is upward-biased (DEC-063) and is not
#                   the effect size; what may be written is "at least 5 nElo".
#                   The block-boundary readings follow (step file, item (6)).
#
#   H0           -> not shown to gain 5 nElo. **Nothing ships**: CAND is
#                   reverted by its own revert commit and `src/` returns to
#                   REF's defaults. The lane's log, trajectory and frozen config
#                   stay in `adocs/data/` as the record of the night spent, and
#                   the zero is recorded as a zero (S005, S006, S015). S127
#                   starts from the untuned values.
#
#   No verdict   -> the 40000-game cap with neither bound: a zero, read as H0
#                   is and reverted as H0 is, with the games and the interval
#                   recorded. No follow-up run and no second pair (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block before its `Bench:` line, in
# fastchess's own line names, with the log trimmed into
# `adocs/data/S261_sprt.log` by `tools/trim_console.py`.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path; this script execs it and adds nothing
# after. Watcher ceiling 136800 s (38 h), twice the 19.0 h cap.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR -- a COMMITTED candidate, not a working-tree one, so DEC-220's block
# names two plain shas that `tools/gate.sh` checks against the log and no
# `candidate.diff` blob is needed (DEC-254). Pinned by editing the defaults
# below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the vector's landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates, and each side's padding, before the first game.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S261_sprt.sh before running (CAND is the commit that" \
         "lands the lane's vector, REF its parent, the untuned tree)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s261_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
