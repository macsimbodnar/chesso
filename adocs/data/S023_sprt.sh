#!/usr/bin/env bash
#
# S023 VERDICT 1: CAPTURE HISTORY IN THE TWO CAPTURE MARGINS. The step file's
# order note puts the margins first and the ordering key second, and this run
# prices the first alone; `score_move` does not read the table in CAND, and
# the ordering key with its band bound is verdict 2, pre-registered after this
# one reads (see the H1 row below).
#
#   nohup adocs/data/S023_sprt.sh > .tuning/s023_sprt.log 2>&1 &
#
# THE CANDIDATE. `search_state_t::capture_history`, int16_t
# `[moving piece][target square][victim type]` (12 x 64 x 6, 9 KiB, per
# thread, zeroed per `go`), reached only through `capture_history_entry()`
# (src/data_structures.hpp). At every fail-high in `negamax_at` the capture
# that cut off is credited and every capture the node searched before the
# cutoff move -- a capture's or a quiet's -- is charged, through
# `history_gravity_update` on a bound of its own, `CAPT_HIST_BOUND` = the
# int16_t ceiling, graded quadratically in the remaining depth in S222's unit
# (`CaptHistBonus`, `CaptHistMalus`, thousandths of the band at depth 11).
# Quiescence reads the table and never writes it. Two readers, both margins:
#
#   S091's capture SEE margin   max(0, SeeCaptureCoeff * lmr_depth
#                                      + CaptHistSeeWeight * entry / BOUND)
#   S112's quiescence futility  max(0, QsFutilityMargin
#                                      + CaptHistQsWeight * entry / BOUND)
#
# The floors are the rules' own classes kept: the capture rule still skips
# only a capture the exchange evaluation says loses material, and a quiescence
# skip still fires only below the alpha the node was entered with. Files:
# src/data_structures.hpp, src/search.{hpp,cpp}, src/search_params.hpp; the
# step file's "As built" has the rest.
#
# DEC-221: implemented from the step file's description -- "history indexed by
# piece, target and victim", and its order note -- and the Chess Programming
# Wiki's History Heuristic page as `adocs/data/2026-09-19_search_technique_study.md`
# quotes it: "a history table indexed by moved piece, target square and
# captured piece type, with bonus on cutoff and malus otherwise". No other
# engine's code was opened and no constant of one is behind any value here
# (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), each stated at its site in src/search_params.hpp.
# CaptHistBonus 106 and CaptHistMalus 25: (b), plain history's own fitted
# grading at depth 11 -- 6*121 + 19*11 + 2 = 937 and 17*11 + 36 = 223 against
# its bound of 8831 -- so the capture table learns at the quiet table's rate.
# CaptHistSeeWeight 100: (b), one pawn of `see_value`, the unit
# SeeCaptureCoeff's own derivation reads. CaptHistQsWeight 94: (b), one pawn
# of src/eval_tables.hpp. Every one is a seed for S127, not a fit.
# Re-read on `5a04ffa` (2026-10-14), S099's revert, whose `src/` is S261's
# landing `bacead1`'s: S261's vector moved QsFutilityMargin 188 -> 226 and
# SeeCaptureMaxLmrDepth 8 -> 9 and left SeeCaptureCoeff at 50; no seed's form
# reads a moved value -- each weight is a pawn of its own scale, not a share
# of a margin -- and none of S261's thirty-one axes is a coefficient or the
# bound of plain history, so all four stand.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, on `7e36271` plus S023
# against the parent's engine (`.ref-builds/bacead1`, `src/` equal to
# `7e36271`'s), 2026-10-14:
#
#   `bench` 5636835 -> 7045606 (+25.0 %), the eight replies unchanged; `bench
#   12` 1937969 -> 2314722, the second reply d5e6 -> e2a6.
#   `tools/search_bench.py` depth 9: 43496 / 160453 / 30816 -> 43477 / 151552
#   / 30816, best moves `c3d5 e2a6 d7c8q` on both; depth 12: 180917 / 509438
#   / 147594 -> 282734 / 465448 / 236166, kiwipete's d5e6 -> e2a6. The off
#   values prove themselves (DEC-215): the tune build at `CaptHistSeeWeight`
#   0 and `CaptHistQsWeight` 0 prints the parent's whole bench stream, 0 of
#   121 lines differing, and so it does at `CaptHistBonus` 0 and
#   `CaptHistMalus` 0. The reach census was not taken: phase one wrote no
#   instrument and it informs nothing the games do not. No timing is owed:
#   DEC-266 reads a behaviour-neutral speed verdict, and this change alters
#   play, so the cell quiescence reads per capture is priced by the games,
#   as a play-altering change's cost always is here (DEC-271's reasoning).
#
#   The counts move by construction; INV-6 does not discharge this change and
#   nothing above is Elo (DEC-019).
#
# THE PREDICTION, labelled as one and not a measurement. The step file prices
# the ordering use (Weiss, -4.17 +/- 4.83 at 10+0.1, +3.66 +/- 3.29 at
# 60+0.6) and a reduction use (Ethereal, +7.22 / +2.37, which is S265's), and
# neither is this run's. Its order note -- "capture history's measured value
# is in the capture futility and capture SEE margins" -- carries no source and
# no figure. This engine's own published-to-measured transfers read 0, 0,
# wrong sign, 0.10 and 0.33 (DEC-222's F01). No number is predicted; the
# direction is small and positive, from captures the record spares that the
# fixed margins skip and futile ones it condemns, against the cell read in
# quiescence. A zero is an expected outcome (the step file says so of the
# family) and is recorded as one.
#
# BOUNDS. elo0=0 elo1=5 nElo, alpha=beta=0.05, `model=normalized` --
# `fastchess.sh`'s default, the gainer pair the step file names. DEC-063: the
# pair straddles the expected effect.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint, **25591** on a bound (`adocs/testing_strategy.md`
# section 1.1, D / (e1 - e0)^2 with D = 1046535, and 639770 / 25), at **2110
# games an hour** (`.moltke.local.md`): **19.8 h** and **12.1 h**. The cap is
# `fastchess.sh`'s 20000 rounds, 40000 games, 19.0 h. Four hours or more, so a
# night (DEC-155).
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
# re-read against plan.md's Open list at `bf7e8e8` (2026-10-14): **S282**,
# open as filler under DEC-171 -- `adocs/data/S192_node_budget.py`'s middle
# half reads its own derivation count as outside (a band from count `c`
# starts its middle half at 1.15c), a tool and documentation defect that
# moves no score, move or line and is not on the UCI surface. Nothing else:
# S283 (DEC-286, S099's pair at 32+0.32 after this run) is a run and not a
# finding, and the rest of the list is strength steps. S099 completed as
# `7e36271`, this run's REF. Closed since this was first written on
# `c3b7b5e`: S279 (`go nodes` one over, `870d210`) and S280 (the mutation
# recipe's file transport, `6fc7f23`). The coordinator re-reads the list on
# the day it pins the pair and names anything else here.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> the table and its two margin readers gain at least 5 nElo
#                   at the harness setting. They stay. The stopping run's Elo
#                   is upward-biased (DEC-063) and is not the effect size. The
#                   table is kept, so **S025 and S265 become eligible**
#                   (DEC-258 (8)), and **verdict 2** -- the record let into
#                   `score_move` as a bounded tie-break inside the capture
#                   band, its bound a constant with a range in
#                   src/search_params.hpp, held by the band case S023 already
#                   carries -- is pre-registered as its own `{0, 5}` run
#                   against this landing.
#
#   H0           -> not shown to gain 5 nElo here. **Revert the landing** --
#                   the table, its update, both readers, the four parameters,
#                   the tests and the mutants that hold them -- by reverting
#                   its commit, **except the band case** in
#                   tests/test_evaluation.cpp, "a king capturing a pawn
#                   outranks a killer at the same ply", the accepts' own guard
#                   over the bands, which hold without the table (the
#                   coordinator's ruling at S023's fast check): it is kept
#                   with its capture-history record loop taken out, a written
#                   edit in the revert commit and not part of `git revert`'s
#                   output -- the case as `adocs/data/S023_h0_band_case.diff`
#                   adds it to 7e36271's file, built and green there at S023's
#                   fast check -- and is observed green on the reverted tree
#                   before that commit. **S025 and S265 return to the reserve**
#                   (DEC-258 (8)). Verdict 2 is not run: the ordering key alone
#                   is the use the step file's own record prices negative at
#                   short control. Unless the owner rules otherwise.
#                   The revert also takes out what landed with the candidate:
#                   DEC-287's ceilings and the budgets, `EXACT_FLOOR` 160 and
#                   both re-mined mate rows. S261's E21 row comes back and
#                   separates on that tree; the extra-ply row comes back
#                   stale -- it had stopped separating on the parent already
#                   -- so a filler step re-mines it there (DEC-142, DEC-288).
#
#   No verdict   -> the 40000-game cap with neither bound: a zero, read as H0
#                   is and reverted as H0 is -- the band case kept without its
#                   record loop, as above -- with the games and the interval
#                   recorded. No follow-up run and no second pair (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block before its `Bench:` line, in
# fastchess's own line names, with the log trimmed into
# `adocs/data/S023_sprt.log` by `tools/trim_console.py`.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path; this script execs it and adds nothing
# after. Watcher ceiling 136800 s (38 h), twice the 19.0 h cap.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR -- a COMMITTED candidate, not a working-tree one, so DEC-220's block
# names two plain shas that `tools/gate.sh` checks against the log and no
# `candidate.diff` blob is needed (DEC-254):
#
#   REF   `7e36271`, S099's completing commit, docs-only on its H0's revert
#         `5a04ffa`, whose `src/` is S261's landing `bacead1`'s: the tree
#         before S023
#   CAND  S023 verdict 1, one commit, landed on REF or on a docs-only
#         descendant of it, pinned by the coordinator once committed
#
# What makes REF the right reference is that the engine is the one CAND was
# built on, so the script refuses unless `git diff REF CAND^ -- src/` is
# empty, as S099's did. CAND is pinned by editing its default below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # S023 v1's landing, CAND
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates, and each side's padding, before the first game.
REF="${REF:-7e36271}"
CAND="${CAND:-PIN_ME_CAND}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == PIN_ME* ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S023_sprt.sh before running (REF is 7e36271, S099's" \
         "completing commit; CAND is the commit that lands S023 verdict 1" \
         "on REF or a docs-only descendant)" >&2
    exit 1
  fi
done

if ! git diff --quiet "$REF" "$CAND^" -- src/; then
  echo "SPRT-RUN-FAILED: CAND's parent differs from REF in src/ -- REF is" \
       "the engine the candidate was built on, or the pair measures two" \
       "changes" >&2
  exit 1
fi

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s023_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
