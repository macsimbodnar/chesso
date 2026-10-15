#!/usr/bin/env bash
#
# S039: THE LAZY SHORTCUT AND THE CLAMP RETIRE TOGETHER. One non-regression
# verdict for both, as the owner ruled on 2026-10-08 (DEC-258 (5)): two
# separate verdicts cost 24 to 38 hours for two near-zero answers, and the
# clamp is what made the shortcut's bound hold, so neither half is a question
# on its own.
#
#   nohup adocs/data/S039_sprt.sh > .tuning/s039_sprt.log 2>&1 &
#   echo $! > .tuning/s039_sprt.pid
#
# THE CANDIDATE. One commit, with every weight frozen (accepts (1)):
# `evaluate_lazy()` is deleted and quiescence calls `evaluate()` where it did,
# so its stand pat is the exact static score at every window and every
# quiescence entry carries it; `evaluate_expensive()` adds mobility plus king
# safety whole, where it clamped the sum to +/-LAZY_EVAL_MARGIN (184); the
# `LAZY_EVAL_MARGIN` / `LazyEvalMargin` parameter leaves
# `src/search_params.hpp` with both. The tuner's trace loses the clamp in step
# (`exact_score()`, the smooth score, the gradient), which no game reads.
# Files: `src/evaluation.{hpp,cpp}`, `src/search.cpp`, `src/search_params.hpp`;
# the step file's "As built" has the rest.
#
# S023 verdict 1 read H0 and was reverted (`0cdd39a`), so neither side carries
# capture history: quiescence's per-move futility base is the stand pat plus
# `QsFutilityMargin` (S112), and what S039 changes there is the stand pat --
# the exact static score at every window, where the lazy shortcut's bound
# stood at the windows the cheap stage cleared. S099's correction was reverted
# on its H0 (`5a04ffa`) and is in neither side; on an H1 for S283, S099
# relands onto S039's outcome by its own step, after this run (DEC-290).
#
# DEC-221: nothing here is a technique taken from the 2026-09-19 analysis. The
# change removes one -- lazy evaluation, as the Chess Programming Wiki's "Lazy
# Evaluation" page describes it -- on this project's own ruling (DEC-258), and
# no other engine's code was opened for it. No constant is introduced, so
# there is no seed (DEC-134); the one constant the change deletes, 184, was
# this project's own SPSA output (S085).
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES (accepts (3)). INV-6 does not
# discharge this change: the tree moves by construction. Against REF below,
# whose engine is the candidate's parent's, on the workstation:
#
#   `bench`: 5636835 (REF) -> **6100949** (candidate), +8.23 %; the eight
#   replies unchanged (c3d5 e2a6 d7c8q g7h8q a7a6 a1b2 e5e6 e5e6).
#   `tools/search_bench.py` depth 9: 234765 -> 192267 nodes over the three,
#   the three best moves unchanged; depth 12: 837949 -> 873087, kiwipete's
#   best move d5e6 -> e2a6, the other two unchanged.
#
#   NPS AGAINST NODES TO DEPTH 11 over S103's 300 positions, S120's method
#   (`nps_rotate.py`, 8 rounds, every binary once a round, order rotated,
#   paired per round), REF, a byte copy of REF and the candidate, load
#   average 3.25 before and 1.29 after: nodes 38381506 -> **36684997**
#   (-4.42 %), nps **-2.79 %** (CI -3.20 .. -2.38), wall **-1.68 %** (CI
#   -2.09 .. -1.26); the copy read nps +0.01 % (CI -0.41 .. +0.43), the A/A.
#   `adocs/data/S039_measurements.txt`.
#   S120 measured the shortcut alone switched off with the clamp kept, on
#   `1eaa776`: -7.5 % nodes, -5.01 % nps, -2.64 % wall
#   (`adocs/data/S120_measurements.txt`, "Exact everywhere").
#
#   How often the removed clamp would have bound at the shipped weights: 36503
#   of the dedup corpus's 10795695 rows, 0.338 % (S262).
#
#   None of this is Elo (DEC-019).
#
# THE PREDICTION, labelled as one and not a measurement: no direction is
# predicted. DEC-258 read each half alone as near zero -- the shortcut's
# switch-off buys a tree 7.5 % smaller for 5 % of nps, and the clamp binds on
# a third of a percent of positions -- and no located source prices a lazy
# margin in Elo at any band (the step file, section 1). Why it is run at all:
# S121 and S122 fit the unclamped sum, and a play-altering change is decided by
# SPRT and not by argument (AGENTS.md MEASUREMENT).
#
# BOUNDS. `--nonreg`, fastchess.sh's own mode: elo0=-5 elo1=0 nElo,
# alpha=beta=0.05, `model=normalized` -- the `{-5, 0}` pair accepts (4) and
# DEC-258 (5) name. It asks whether the change costs 5 nElo or more, not
# whether it gains. Neither `SPRT_BOUNDS` nor `SPRT_ROUNDS` is set here: S284
# (DEC-289, `769ce7a`) added both to fastchess.sh and left `--nonreg` as it
# was, `{-5, 0}` at alpha = beta = 0.05 with its 20000-round cap. Either one
# beside a mode argument is refused there, so one left in the calling shell's
# environment fails this run before a game rather than changing it.
#
# WHAT THE PAIR COSTS (DEC-143). **25591 expected games** with the truth on a
# bound and **41861** at the interval's midpoint (`adocs/testing_strategy.md`
# section 1.1, 639770 / 25 and D / (e1 - e0)^2 with D = 1046535, the figures
# the step file states), at **2110 games an hour** (`.moltke.local.md`):
# **12.1 h** and **19.8 h**. The cap is fastchess.sh's 20000 rounds, 40000
# games, 19.0 h, so a truth near the midpoint is likelier to reach the cap
# than a bound. Four hours or more, so a night (DEC-155).
#
# REGIME. 8+0.08, Hash 16 (DEC-088), one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 (DEC-050), fastchess.sh's
# adjudication, whatever governor the machine is under, recorded and not set
# (DEC-195). If fastchess, the book, the adjudication or the machine has moved
# since the last fixed-rounds A/A, the A/A of 1000 fixed rounds comes first
# (DEC-143).
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Stop it and report nothing, too, if
# the machine loses mains power (POWER) or another load starts on it while the
# run plays (MACHINE). Nothing else stops it -- not the length.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171), read
# off `adocs/status.md` at `c88a961`, off `adocs/plan.md`'s Open list at
# `8ef820f` (none), at `4e6ca63` (S282), at `769ce7a` (S282, S285) and at
# `be48e02`: **none**. S282 (`2bdc7cc`, the node band's middle half) and S285
# (`be48e02`, the extra-ply row re-mined on the reverted tree) both landed
# before this step's phase 2, which read the node-budget golden by S282's
# rule and re-mined S285's row on S039's tree. The coordinator re-reads the
# list on the day it pins the pair and names here whatever is open then.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063),
# the H1 and H0 actions in the words of the step file's accepts (4), the
# no-verdict action DEC-274's.
#
#   H1 accepted  -> not a regression of 5 nElo or more. **S039 lands**: the
#                   shortcut and the clamp are gone, and S121 onward fit the
#                   unclamped sum. No magnitude is claimed; the stopping run's
#                   Elo is biased (DEC-063).
#
#   H0 accepted  -> it costs 5 nElo or more. **Both are kept**: the candidate
#                   commit is reverted, shortcut, clamp and `LazyEvalMargin`
#                   with it, and the tests, mutant and documents written for
#                   it leave with it -- every golden phase 2 re-derived on
#                   S039's tree included: `MATE_IN_THREE_FLOOR` 12 -> 11,
#                   `EXACT_FLOOR` 161 -> 159, the three mined rows back to
#                   S285's (extra ply, depth 8, mate in 4) and S261's (E21 at
#                   10, mate in 5; B04 `8/5Qk1/...` at 10, mate in 5), the
#                   `capture_mates` rows back to 9, 8, 11, 12 and S261's
#                   labels, DEC-293's E21 row (`r6k/...` at 11) with them;
#                   DEC-291's values, in S039's commit only, leave too --
#                   `tests/test_mate_carry.cpp`'s ceilings A 2 -> 0 and C 6
#                   -> 4, `adocs/data/S170_cases.tsv`'s budgets A 500000 ->
#                   1000000 and D 500000 -> 2000000, S039's grid out of the
#                   `--ceilings` command -- and DEC-292's,
#                   `tests/test_tuner_gradient.cpp`'s gauge-walk floor 0.01
#                   -> 0.1, back where the reverted tree's walks, 0.41 and
#                   0.89, clear it. The evidence files under `adocs/data/`
#                   stay as evidence. **Three edits the
#                   revert keeps, written into the revert commit**, because
#                   the tree before this step carried three stale statements
#                   it corrected in passing, and a revert would put them
#                   back again: `DEV_MANUAL.md`'s `golden_defaults` row reads
#                   **73** -- the reverted tree's count, `LazyEvalMargin`
#                   back -- and not the stale 71 (S116's two rows made 73);
#                   `tests/test_mate_carry.cpp`'s GOLDEN line names the six
#                   `short_line_ceiling` ships on the reverted tree, **0, 5,
#                   4, 0, 0 and 1**, and not S116's stale 6, 15, 5, 2, 11 and
#                   5, with its history sentence; and `adocs/specs.md`'s
#                   search row states both mate floors at the reverted tree's
#                   readings, not `be48e02`'s stale "a floor of 143 exact" and
#                   mate-in-three weakened end of 10: `EXACT_FLOOR` **159**,
#                   its ends 160 shipping and 156 weakened, and
#                   `MATE_IN_THREE_FLOOR` **11**, its ends 12 and **6** --
#                   each re-read on `be48e02` by its script in S039's phase 2
#                   (`adocs/data/S039_floors.log`, the reverted tree's
#                   section). **S122's design is then
#                   amended by an owner decision before S121 starts** --
#                   never by S122 claiming unclamped scores under a kept
#                   clamp.
#
#   No verdict   -> the 40000-game cap with neither bound (DEC-274). **S039
#                   lands as on H1**, recorded as a zero with the interval
#                   stated: nothing was shown to cost 5 nElo, and the fits
#                   that follow need the unclamped sum (DEC-258). A cost inside
#                   the bounds can land unmeasured; S121's and S122's own
#                   verdicts are taken on top of it. No follow-up run and no
#                   second pair (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block before its `Bench:` line, in
# fastchess's own line names, with the log trimmed into
# `adocs/data/S039_sprt.log` by `tools/trim_console.py`.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path; this script execs it and adds nothing
# after.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR -- a COMMITTED candidate, not a working-tree one, as S119's and
# S099's were:
#
#   REF   `be48e02`, S285's landing: documents and tests only on S023
#         verdict 1's revert `0cdd39a` (its H0; bench 5636835, the engine
#         `7e36271` carried), S282's and S285's filler landings among them,
#         none of which touches `src/`. DEC-290 puts S039 first among the
#         engine landings after S023 verdict 1, so no other engine change
#         sits between
#   CAND  S039 -- the shortcut and the clamp out, one commit carrying `Bench:`
#
# CAND's parent is REF or a docs-only descendant of it -- a status or pin
# commit may sit between -- so, as S099's script did, this one refuses unless
# `git diff REF CAND^ -- src/` is empty: what makes REF the right reference is
# that its engine is the one the candidate was built on. DEC-220's block then
# names two plain shas `tools/gate.sh` checks against the log, and no
# `candidate.diff` blob is needed (DEC-254). Pinned by editing the defaults
# below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # S039's landing, CAND
#   git -C /home/max/ws/chesso log --oneline -3           # the commit below, REF
#
# Until both are pinned and the measurements above are filled, this script
# refuses (DEC-020). The banner prints both shas with their commit dates
# before the first game.
REF="${REF:-be48e02}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S039_sprt.sh before running (REF is the last commit on" \
         "achesso before the candidate, on S023 verdict 1's revert; CAND is" \
         "S039 committed on REF or a docs-only descendant)" >&2
    exit 1
  fi
done

if ! git diff --quiet "$REF" "$CAND^" -- src/; then
  echo "SPRT-RUN-FAILED: CAND's parent differs from REF in src/ -- REF is" \
       "the engine the candidate was built on, or the pair measures two" \
       "changes" >&2
  exit 1
fi

# The marker is spelled split here so that this check does not find itself: a
# pre-registration with a hole in it is not one.
if grep -q "P2_""PENDING" "${BASH_SOURCE[0]}"; then
  echo "SPRT-RUN-FAILED: adocs/data/S039_sprt.sh still carries P2_""PENDING --" \
       "fill the measurements first" >&2
  exit 1
fi

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s039_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh --nonreg
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh --nonreg" >&2; exit 127
