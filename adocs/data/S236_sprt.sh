#!/usr/bin/env bash
#
# S236: S098 verdict 1's history-scaled reduction returns as a **fraction of a
# ply**, carried by an accumulator that sums the reduction in ticks and rounds
# it once. Judged against the tree without the term.
#
#   nohup adocs/data/S236_sprt.sh > .tuning/sprt_s236.log 2>&1 &
#
# **THE CANDIDATE IS ONE CHANGE, AND THAT IS A MEASUREMENT AND NOT A PLAN**
# (DEC-082, DEC-222 clause 6). The step was written as one block of two parts,
# with per-part attribution deliberately forfeited, because a fractional term
# cannot exist without a fractional accumulator. The accumulator's own seed then
# did not survive the fast suite: `LmrRoundBias` was to ship at 512,
# round-to-nearest, and at 512 -- and at every bias from one tick upward with
# the term live -- the engine loses the mate in tests/test_search.cpp "pruning
# does not hide a forced mate", which this step's `accepts` requires to stay
# green. So the bias ships at 0, the parent's own truncation, the accumulator is
# **bit-identical to the parent** at that setting, and what plays differently
# here is the history term alone. An H1 therefore attributes to that term and
# the forfeit costs nothing; leg 1 of the bisection below is an identity a
# rebuild proves rather than a run that costs a night. The six builds behind
# that sentence -- ten builds, one per pair of defaults -- are in the step file.
#
# **WHAT IS IN `src/`, WHETHER OR NOT IT MOVES A GAME.**
# `build_lmr_table` stores `r * LMR_SCALE` truncated instead of `r` truncated,
# so the table holds ticks of a ply (1024 to the ply, a compile-time constant
# beside it, DEC-134 (c) -- a power of two so the rounding divide is an
# arithmetic shift and therefore an exact floor at both signs, and the largest
# one that keeps `hist_sum * LMR_SCALE` three-fold inside `int32_t` at the
# widest vector a tune build can be driven to). `lmr_node_adjustment` returns
# its five terms in those ticks. `lmr_adjusted_reduction` sums table, node
# terms and the history term in ticks and rounds once at its return, as
# `(ticks + LmrRoundBias) >> 10`. And `lmr_history_ticks` is the returning
# term: `clamp(hist_sum * 1024 / LmrHistDiv, +/-LmrHistClamp)`, subtracted, on
# the same `quiet_history_sum` S098 verdict 1 read, at the same two sites --
# the reduction, and S109's shallow-depth gate through `lmr_depth_of`, which
# has read one number with the reduction since verdict 2.
#
# DEC-221: implemented from the description in
# `adocs/data/2026-09-19_search_technique_study.md` section 3.3 and its review
# `adocs/audit/2026-09-19_study_review.md` F13, from this step's own file, and
# from DEC-213's record of what verdict 1 measured. No other project's code was
# opened and no constant of one is behind any value here (DEC-016, DEC-104,
# DEC-105, DEC-134).
#
# THE SEEDS, and each is one of DEC-134's three forms:
#
#   LMR_SCALE 1024      (c) a design constant, stated as such: chosen for the
#                       integer range and the shift, not fitted, and not a
#                       resolution any record suggested.
#   LmrRoundBias 0      the parent's own truncation, and **not** the (c)
#                       midpoint the seed rule asked for: 512 loses the mate row
#                       named above, and so does every bias from one tick up
#                       with the term live. It ships at its off value with the
#                       rule live behind it, the shape LmpDepthCoeff has had
#                       since S109, and whether rounding to nearest is worth
#                       anything is left untested rather than rejected.
#   LmrHistDiv 734      (b) twice the p90 of |hist_sum| at the reduction's own
#                       sites, from this step's census over the eight bench
#                       positions at depth 12 (`adocs/data/S236_hist_census.py`
#                       and the .txt beside it), taken on the tree the term is
#                       added to. **Never S098 verdict 1's fitted 699 and 3**,
#                       which were fitted against a term whose smallest step
#                       was a whole ply.
#   LmrHistClamp 2048   the same rule asked for the p99's contribution under
#                       that divisor, which is 6812 ticks -- 6.7 plies -- so
#                       **the declared top binds and this value is a range
#                       bound and not a fit**. The rule assumed S098's tail
#                       (p99 / p90 3.7); this tree's is 15. The cap was
#                       pre-registered for exactly this case and the census
#                       file says it bound. What it costs is in the next
#                       section and it is the largest number in this file.
#
# THE PRIOR, recorded before the run (DEC-019, DEC-222).
#
#   **No published number prices fixed point itself.** The 2026-09-19 study's
#   review establishes that (F13): the open-source record's fractional
#   reduction predates its own measurement ledger, so its two commits carry a
#   bench line and nothing else. What that record does carry is **+9.09 over
#   4970 games** for history-scaled reduction *on a reduction that was already
#   fractional*, which is a direction and not an expectation -- and this
#   project's own three transfers of a published figure read 0, 0 and the wrong
#   sign (DEC-019).
#
#   **This engine has measured the same idea twice, at zero.** S098 verdict 1
#   read `Elo -2.92 +/- 5.02` over 11524 games and its bisection leg
#   `-2.34 +/- 4.73` over 13078, both intervals mostly below zero, and the term
#   left the tree (DEC-213). The hypothesis this step tests is that the unit
#   was the reason: a term that can only say "nothing" or "a whole ply" is not
#   the term the record measured. That is a hypothesis about a recorded chesso
#   failure and not a claim about a number, which is why the pair below is the
#   gainer pair and not a non-regression one.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-23:
#
#   THE OFF VALUE IS PROVED ON THE TREE and not declared (DEC-215). The tune
#   build at `LmrHistClamp` 0 prints the reference binary's own bench total,
#   **4493659, with all eight `bestmove` replies identical**
#   (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), and a release build with that
#   default at 0 reproduces `tools/search_bench.py` exactly at both depths:
#   21479 / 102462 / 33148 with g5f6 / e2a6 / d7c8q at depth 9 and
#   149688 / 459216 / 219544 with c3d5 / e2a6 / d7c8q at depth 12. That is the
#   equality an H0's revert returns to, it is what makes the accumulator a
#   refactor rather than a rule, and it is INV-6's own form.
#
#   AND THE SHIPPED TREE AGAINST THE PARENT AT THE SAME FIXED DEPTHS, which is
#   where the term's cost is visible per position rather than in one total:
#     depth  9   21479 / 102462 / 33148  ->  25248 / 104157 / 28867
#                best moves g5f6 / e2a6 / d7c8q, unchanged
#     depth 12  149688 / 459216 / 219544 ->  235756 / 664570 / 163705
#                best moves c3d5 / e2a6 / d7c8q -> c3d5 / d5e6 / d7c8q
#   The third position **shrinks** at both depths while the first two grow: a
#   term that reduces the quiets history likes does not grow every tree, and a
#   single bench total hides that. Nothing here is Elo (DEC-019).
#
#   THE SHIPPED PAIR SEARCHES A MUCH BIGGER TREE. `bench` goes
#   **4493659 -> 6858745, +52.6 %**, the candidate's eight replies reading
#   c3d5 d5e6 d7c8q g7h8q d8d6 a1b2 e5e6 e5e6 against the parent's
#   c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 -- two of the eight moved.
#   The census says why: the term moves the whole-ply reduction at 12.54 % of
#   sites and 5.42 % of them by two plies, because the clamp is at its cap. The
#   same divisor at a clamp of 1024 searches 5193174 (+15.6 %) and moves the
#   same 12.54 % of sites by one ply each; at 512, 5406957 (+20.3 %). **The
#   candidate therefore pays about half again as many nodes per depth**, and
#   the SPRT is being asked whether the term's judgement is worth that. Nothing
#   here is Elo (DEC-019) and no argument from these numbers is a verdict.
#
#   THE ACCUMULATOR ITSELF IS FREE. Twelve interleaved bench pairs of the parent
#   against the candidate at its off value -- identical trees, 4493659 nodes
#   both -- give 3826388 against 3845136 nodes per second: the candidate half a
#   per cent **faster**, +0.49 % on the means and +0.52 % paired, against a
#   run-to-run spread of 1.8 % and 1.2 % and a paired spread of 2.1 %, at a
#   one-minute load of 1.3 on twelve cores. The reduction table's growth from
#   4 KB to 16 KB shows no cost and no speed-up: it is below this machine's
#   noise floor (CLAUDE.md's 3 % rule). Recorded in
#   adocs/data/S236_nps_interleaved.txt.
#
#     the census's own distribution and seeds:  adocs/data/S236_hist_census.txt
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the block is proposed as a
# gain and a `{-5, 0}` pair would accept H1 on a truth of zero and ship a
# rewritten accumulator that does nothing. The legs below use the pair that
# matches what each leg is: leg 1 is a non-regression question, leg 2 is a
# gainer one.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** at the interval's
# midpoint and **25591** with the truth on a bound (`adocs/testing_strategy.md`
# section 1.1, `adocs/plan.md`'s cost table), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. A night, scheduled (DEC-155).
# The candidate's tree size moves with the rounding, so the throughput is read
# off the banner and recorded rather than assumed.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). **No harness change since the last
# fixed-rounds A/A**; if fastchess, the book, the adjudication or the machine
# moves before this starts, the A/A of 1000 fixed rounds comes first and is
# read with `adocs/data/S105_pairs.py` (DEC-143).
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# read off `adocs/plan.md`'s Open list and amended 2026-09-23 with what this
# step's own measurements found. **No defect reachable in ordinary play, on the
# UCI surface, or able to move a reported score, move or line is open.** What is
# open is test-side, and the first entry below is this candidate's own doing:
#
#   1. **`E21_multicut_mate_band_gate_dropped` survives on this candidate and is
#      killed on its parent** -- measured on both, 2026-09-23, in
#      `.tuning/coord/S236_mutation_E.log` and `S236_E21_parent.log`. The row
#      that kills it, "mate the multicut hides" in "pruning does not hide a
#      forced mate", was mined on the parent's tree; this candidate searches
#      half again as many nodes and the row stops separating, which is the
#      lesson S188 recorded when its own check extension did the same. Test-side
#      under DEC-171: it cannot move a reported score, move or line. **On an H1
#      the row is re-mined on the tree that then ships and the new row is proved
#      by a targeted `--only E21`; on an H0 the kill returns with the revert and
#      the finding closes.** Mining it before the verdict would be mining on a
#      tree that may not ship.
#   2. **`test_mate_breadth`'s headroom.** S188's 2.6 % finding was about the
#      tree that step's H0 reverted; on this one the case runs 20.1 s Release
#      and 20.5 s tune against its 120 s ceiling, and this candidate costs it
#      about a second. Re-read at the landing, ceiling untouched (TESTS).
#   3. **S239 is closed**, not open: `tools/mutation_check.py`'s two refusals --
#      a zero-test baseline and a fixture dirty outside `src/` -- landed on
#      `achesso` at `3b0fbe6` before this step's own pass ran, and that pass ran
#      on the repaired tool. Its header is quoted in the step file: fixture
#      `bd90054` clean at its own HEAD, the mutant list read from the working
#      tree with three files named dirty, baseline green over 40 tests.
#   4. The three S097 named and re-checked on 2026-09-21, each still open and
#      none of them touched here: S231 phase one's node-budget label of 17321
#      against the reverted tree's 16256; `adocs/data/S192_node_budget.py`
#      reporting OUTSIDE for a band it had just derived; and
#      `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pinning depth 6 where `adocs/data/S231_research_witness.py`
#      answers 4.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
# **There is no bisection.** The block was written as two parts with per-part
# attribution forfeited; the rounding then measured out of the tree, the
# accumulator is a proved-neutral refactor at the shipped bias, and what plays
# is the history term alone. Leg 1 is struck -- an identity INV-6 proves in a
# rebuild owes no games -- and DEC-082's forfeit is void with it: whatever this
# run reads, it reads about the term.
#
#   H1 accepted  -> the term gains at least 5 nElo at (734, 2048), and it gains
#                   it while paying 52.6 % more nodes per depth, which is what
#                   makes the result worth having. **Keep the seeds**, both of
#                   them, for S127 to fit with LmrBase and LmrDivisor -- the
#                   clamp first, because it ships on its cap. And **before the
#                   step completes, re-mine the E21 row**: this candidate's tree
#                   is not the one "mate the multicut hides" was mined on, the
#                   row stops separating here, and the fix is
#                   `adocs/data/S097_mine_mate_row.py` run on the shipping tree
#                   with the new row proved by a targeted
#                   `tools/mutation_check.py --only E21` scoring it killed. The
#                   stopping run's Elo is upward-biased and is not the effect
#                   size (DEC-063).
#
#   H0, interval -> **a loss: the term leaves the tree again.** Its two
#   wholly         settings, its cases and its mutants go with it, and DEC-213
#   below zero     is reaffirmed on the form it did not have -- the whole-ply
#                  term measured zero twice and the fractional one loses, which
#                  is three readings of one idea and enough (DEC-194's shape).
#                  **The accumulator stays**, at `LmrRoundBias` 0: it is
#                  behaviour-neutral by the identity proved above, S237 and
#                  S238 both need a reduction that can carry a fraction, and
#                  reverting a refactor that plays no differently would cost
#                  those steps their unit for nothing. The reason is stated in
#                  the completion stamp so that a later reader does not read
#                  the surviving code as an unmeasured rule.
#                  **And the mate row goes back with the term.** This step
#                  re-mined S095's row because the candidate made the old one
#                  false -- the old position reports its mate as a 3 at depth
#                  11 on this tree -- and a revert restores the tree the old
#                  row was mined on. The old row's line is kept verbatim in the
#                  case's GOLDEN block for exactly that: restoring it is a
#                  revert and not a second re-mine, which is how S188's own
#                  instance closed.
#
#   H0 whose      -> **one pre-registered follow-up, and only one**: the same
#   interval         term at `LmrHistClamp` **1024**, its own pinned pair
#   reaches above    against the same parent, at **{0, 5}**. If that reads H0
#   zero, or a       the term leaves as above; there is no third run.
#   stalled walk
#   (DEC-063)        **Why this is pre-registered now and not chosen now.** The
#                    shipped clamp is not a measured value: the census rule
#                    asked for 6812 ticks and the declared range top of 2048
#                    bound it, so what ships at the cap is a **bound**. The
#                    census also says what the cheaper clamp would do, and it
#                    was read before any game: the same 12.54 % of sites move,
#                    every one of them by one ply instead of 5.42 % of them by
#                    two, for a tree of +15.6 % instead of +52.6 %. That makes
#                    "the term is right and the cap is too expensive" a
#                    hypothesis the walk itself would make worth a night --
#                    which is a different thing from picking a number off the
#                    bench after a verdict, and the difference is that this
#                    paragraph exists before the first game. Nothing else is
#                    tried: a value chosen after two H0s is a search for a
#                    passing number (DEC-213's own rejection), and the pair
#                    then belongs to S127's lane, which fits it against games
#                    rather than against one census.
#
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero
#                   with the games played and the interval, and read it by the
#                   third row above -- said in advance so that a long run is
#                   not read as encouragement (DEC-063).
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands the block; REF is its parent, the tree with
# neither the accumulator nor the term. Both are pinned by the coordinator
# after the landing commit exists, by editing the defaults below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S236_sprt.sh before running (CAND is the block's" \
         "landing commit, REF is its parent)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s236_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
