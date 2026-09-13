#!/usr/bin/env bash
#
# The S109 block read at four times the control it was verified at. DEC-202's
# block-boundary rule, second instance, in S151's shape. Written before a
# single game is played, which is what makes everything below a
# pre-registration rather than a reading chosen after seeing a number.
#
# THIS RUN IS NOT A VERDICT AND CANNOT BECOME ONE. It is the longer-control
# reading of DEC-202 (2) and (3): one fixed 1000-pair match, no SPRT, no
# bounds, read as an estimate with its own interval. `fastchess.sh` prints
# `bounds none -- fixed 1000 rounds, a calibration or drift reading, NOT a
# verdict` where the reader of the log meets it. The `8+0.08` SPRT decides
# whether S109 ships; this decides what may be written about its magnitude, and
# **a regression here opens a decision rather than an automatic revert**
# (DEC-202 (3)). Nothing in the engine changes on any outcome of this script.
#
# BOTH SIDES ARE COMMITS AND NEITHER IS THE WORKING TREE. `CAND=<ref>` is
# S151's harness addition: both binaries are built from their commit into
# `.ref-builds/<sha>` by the same function, and the candidate is snapshotted
# before the first game like any other (DEC-020). The banner carries both shas
# with their own commit dates and no dirty flag, because the tree is neither
# read nor played.
#
#   candidate  600f448  2026-09-13  "Land S109: four shallow-depth pruning
#                                    rules enter the move loop together"
#   reference  50e3661  2026-09-13  "Start S109: the shallow-depth pruning
#                                    block moves to plan_current"
#
# WHY 600f448 AND NOT THE TESTS-ONLY FOLLOW-UP 1952c56. The pair has to be the
# block's src/ and nothing else, and the candidate is therefore the commit
# whose src/ **is** the block:
#
#   git diff 600f448 1952c56 -- src/          -> empty
#       `1952c56` ("Drop <regex> from the surface test so the sanitizer build
#       compiles again") changes tests and documents only, so it plays the
#       identical engine and adds nothing to the comparison. S151 chose the
#       same way for the same reason -- `git diff 21b4a21 43bf189 -- src/` was
#       empty there and the earlier sha was the candidate.
#   git diff 600f448 HEAD -- src/             -> empty at HEAD 690f7db
#       everything since the landing is documents, so this run's candidate is
#       also the engine the `8+0.08` SPRT played: that run's banner reads
#       `candidate 1952c56` and its binary answered `Chesso 1952c56 native`,
#       and `1952c56`'s src/ is `600f448`'s by the check above. The two runs
#       play the same two engines at two controls, which is what makes reading
#       (2) below a comparison and not an analogy.
#
# VERIFIED AT HEAD 690f7db, 2026-09-13, all three checks:
#
#   git rev-parse --short 600f448^            -> 50e3661
#       the reference is exactly the parent of the landing commit, so nothing
#       else rides along in the comparison.
#   git diff --stat 50e3661 600f448 -- src/
#       src/data_structures.hpp |  29 ++
#       src/search.cpp          | 348 ++++++++++++++++++++++++++++++++++++-
#       src/search.hpp          |   9 ++
#       src/search_params.hpp   | 137 +++++++++++++++
#       4 files changed, 520 insertions(+), 3 deletions(-)
#   `adocs/data/S109_sprt.sh` runs `exec env REF="${REF:-50e3661}"`, so the
#       `8+0.08` verdict and this reading share a reference sha as well as a
#       candidate.
#
# THE TEN DEFAULTS THE BLOCK ADDS, re-read from
# `git diff 50e3661 600f448 -- src/search_params.hpp` at HEAD and not from an
# earlier document. Every one of them is DEC-202's bound class -- "a margin,
# depth bound, reduction coefficient or divisor in src/search_params.hpp that
# decides whether a node or move is searched at all" -- so the block is inside
# the rule's scope without a judgement call:
#
#   LMP_BASE               "LmpBase"                733   late move pruning
#   LMP_DEPTH_COEFF        "LmpDepthCoeff"            0
#   LMP_MAX_LMRDEPTH       "LmpMaxLmrDepth"           8
#   FUT_BASE               "FutBase"                147   futility pruning
#   FUT_SLOPE              "FutSlope"               170
#   FUT_MAX_LMRDEPTH       "FutMaxLmrDepth"           8
#   HP_COEFF               "HistPruneCoeff"         576   history pruning
#   HP_MAX_LMRDEPTH        "HistPruneMaxLmrDepth"     8
#   SEE_QUIET_COEFF        "SeeQuietCoeff"           50   quiet SEE pruning
#   SEE_QUIET_MAX_LMRDEPTH "SeeQuietMaxLmrDepth"      8
#
# All four rules are gated on the reduction-adjusted depth
# `depth - lmr_reduction(depth, move_number)`, which is a quantity the time
# control moves: at four times the clock the same position is searched deeper,
# so the same rule fires at different places in the tree. That is the whole
# reason the reading exists. **The run measures the four rules jointly and
# cannot attribute anything to one of them, or to one of the ten constants.**
#
# WHAT THE 8+0.08 VERDICT SAID -- the figure this run is read against, filled
# in from the SPRT's own final block, verbatim, before this script is launched:
#
# S109 SPRT at 8+0.08: H1, LLR 2.97, Elo 46.90 +/- 15.43, nElo 56.76 +/- 18.44, 1364 games, 0 forfeits (adocs/data/S109_sprt.log; filled by the coordinator 2026-09-13 09:10, before launch)
#
#   That run finished at 08:49 on 2026-09-13, while this script was being
#   written, and its evidence is `adocs/data/S109_sprt.log`: **H1 accepted at
#   `{0, 5}`, `Elo: 46.90 +/- 15.43, nElo: 56.76 +/- 18.44`, 1364 games in
#   37 m 51 s, `Ptnml(0-2): [55, 116, 214, 185, 112]`, 0 forfeits either
#   side.** The line above is still filled in by the coordinator at launch and
#   from that file rather than from this sentence, because one copy of a
#   figure is the record and a second copy is a thing that drifts.
#
#   Copy the printed `Elo: ... +/- ..., nElo: ... +/- ...` line and the
#   `Ptnml(0-2)` line as they appear, with the game count and the bounds pair.
#   An SPRT's early-stopped point estimate is biased upward (DEC-063: S068's
#   pooled figure fell from +12.18 to +5.02 under that correction), so the
#   number this run is compared to is itself an upper reading, and that
#   asymmetry is stated here rather than discovered afterwards.
#
# WHY A LONGER CONTROL AT ALL. vondele's `nevergrad4sf`: "the optimal
# parameters are often time sensitive, i.e. can be verified to be a gain at the
# VSTC used for tuning, but regress at STC or LTC". Stockfish issue #2600, of
# the last 40 LTC tests with gaining bounds: "23 reds, 16 yellows, 1 green".
# The wiki's own phrasing for a depth-bounded rule is that it "may only be
# effectively tested on time controls where this new condition is triggered
# frequently enough". S109 is four depth-bounded pruning rules at once, which
# is the class the record names; a smaller reading here than at `8+0.08` is the
# expected shape and is recorded as it comes.
#
# THE REGIME, each part with its origin:
#
#   control      32+0.32   four times 8+0.08, the ratio DEC-202 fixed: the
#                          cheapest admissible one, fishtest's being 6 and
#                          48+0.48 costing 1.5x again.
#   hash         64        four times the clock is about four times the nodes a
#                          game writes, so Hash=16 at 32+0.32 would be four
#                          times the table pressure every verdict was taken at.
#                          64 holds DEC-088's invariant and is fishtest's own
#                          LTC setting.
#   rounds       1000      1000 pairs, 2000 games, `-repeat`, no `-sprt`.
#   book         noob_3moves.epd   the harness's book (DEC-189, DEC-190), the
#                          same book S151's reading and every current verdict
#                          run on.
#   seed                   derived from the run stamp and printed on the
#                          banner, which is its only record (S198). Read it off
#                          the log when the run is written up.
#   concurrency  12        every core (MACHINE, DEC-050). Governor is recorded,
#                          never set or waited on (DEC-195).
#   power                  the workstation has no battery, so the POWER rule's
#                          adapter check does not apply here; it does not
#                          hibernate either, and the watcher's pid exit is what
#                          catches a killed run.
#
# WHAT IT COSTS, from the measured throughput and not from a guess (DEC-155,
# DEC-143). **545 games an hour at `32+0.32` with `Hash=64`** is now a measured
# figure on this machine and this book, not an extrapolation: S151 played 2000
# games in 3 h 40 m on 2026-09-12/13 under governor `performance`, 110.6 plies
# and 78.2 s a game (`.moltke.local.md`, `adocs/data/S151_ltc.log`). So
# **2000 / 545 = 3.7 h**, just under DEC-155's four-hour line -- a daytime run
# while the agent writes the next step's code, and it is budgeted from the
# measurement rather than from S151's own pre-registered 528.
#
# One effect is not in the estimate and it points both ways: S109's candidate
# prunes more, so its games may be shorter than S151's pair at the same
# control, while a stronger side adjudicates earlier. **The check is the first
# hour's `Finished game` count** -- about 545 at the estimate. The watcher's
# ceiling is twice the estimate, **27360 s (7 h 36 m)**, and it is wall-clock
# only while the machine is awake (S024's first run hibernated for 42 hours
# under a 9 h ceiling and the watcher did not fire).
#
# ABORT RULE, and one class that is explicitly not one:
#
#   STOP for a time-forfeit rate over 1.0 % on a side:
#
#       python3 tools/forfeit_report.py <outdir>/games.pgn --max-pct 1.0
#
#   `8+0.08` leaves MOVE_OVERHEAD_MS 50 the least room of any control this
#   harness has run and `32+0.32` leaves four times more, so a forfeit here
#   would mean something other than a thin margin. S151's run at this control
#   had 0 on either side.
#
#   A CRASH OR A DISCONNECT VOIDS THE RUN and `fastchess.sh` does that itself:
#   any termination outside normal, adjudication and time forfeit prints
#   `SPRT-RUN-INVALID` and then the `SPRT-RUN-FAILED` marker (S212,
#   2026-09-10_adversarial-F31). A void is not a reading.
#
#   COUNT `Incomplete mating PV` PER SIDE, AND IT IS NOT A STOP. Both commits
#   post-date S147, S170 and S171, which is the difference from S151's run
#   (57 candidate against 67 reference there, both sides predating all three).
#   **A non-zero count on either side here is therefore worth a sentence in the
#   step file and a look afterwards, not an abort of a run already played** --
#   and the candidate is the side carrying four new pruning rules, which is
#   where a mate-carrying defect would show. Record both counts either way.
#
#   Nothing else stops the run. There is no bound to cross and no cap to reach:
#   the stopping rule is the round count and it is written above.
#
# THE THREE READINGS, all three written before the first game:
#
#   1. THE RUN'S OWN INTERVALS, AGAINST ZERO. Quote the printed `Elo` and
#      `nElo` lines with their half-widths. An interval clear of zero says the
#      block's gain is present at four times the control; one straddling zero
#      says this run cannot separate the block from its parent there, which is
#      a statement about the run's resolution and not a verdict of zero; one
#      clear of zero and negative is the regression the published record says
#      to expect from this class, and it opens a decision (DEC-202 (3)) rather
#      than a revert.
#
#   2. AGAINST THE `8+0.08` FIGURE filled in above. A logistic gain is expected
#      to read *smaller* at a longer control even when it transfers perfectly
#      (the Komodo doubling series runs 49.20 % to 76.63 % draws), and
#      Stockfish's own data says that compression is not universal -- so **the
#      nElo pair is the one to lead with**. The comparison is a difference over
#      a combined error: with half-widths h1 and h2 the two readings agree when
#      |d| < sqrt(h1^2 + h2^2), which is how S151 read its own 9.0 on 15.2 as
#      agreement within noise.
#
#   3. THE PAIR VARIANCE AT 32+0.32, WHICH IS AN OUTPUT OF THIS RUN AND ONLY
#      THE SECOND EVER TAKEN AT THIS CONTROL. Read it off the PGN:
#
#        python3 -c "import sys; sys.path.insert(0, 'adocs/data'); import S105_pairs; \
#          S105_pairs.report('<outdir>/games.pgn', engine='cand-600f448')"
#
#      **The engine name is not optional.** `S105_pairs.py`'s own `main`
#      defaults to `chesso-a`, and a name matching neither side is not an
#      error: every game then scores as Black's points, a pair one side won
#      twice reads 0.0 instead of 2.0, and the variance comes back inflated
#      with nothing printed to say so (S198's header is where that trap is
#      written down). This run names its sides `cand-600f448` and
#      `ref-50e3661`. It is not a DEC-143 A/A -- the two sides are different
#      engines, so the number carries a strength difference as well as the
#      harness's noise -- and it is read against S151's 0.2876 at the same
#      control as a first band, not as a verdict on either.
#
# THE RESOLUTION THIS BUYS, derived here and checkable against a printed run.
# With pair score on the 0-to-2 scale `adocs/data/S105_pairs.py` uses, the nElo
# half-width is `1.96 C / sqrt(2N)` with `C = 800 / ln 10 = 347.44` -- it
# depends on the pair count alone, not on the book and not on the draw rate --
# and the logistic half-width is `1.96 * 694.8 * sd_pair / (2 sqrt N)`:
#
#   at N = 1000 pairs, v = 0.29  ->  +/- 15.2 nElo, +/- 11.6 logistic Elo
#
# S151's run at this exact regime printed `+/- 11.57` and `+/- 15.23` at a
# measured pair variance of 0.2876, so this is an arithmetic result checked
# against a printed run at the same control and not an assertion. The reading
# separates "the gain transfers" from "it is gone" and cannot separate -4 from
# 0, which is why DEC-143's "an estimate is not a verdict" is the sentence that
# governs it.
#
# WHERE THIS SITS. It is one of the block boundary's **two** numbers (F30,
# DEC-172): this run is transfer to four times the control, and
# `adocs/data/S199_drift.sh` is drift at the regime, the engine now against the
# pinned early-S105 commit. Neither is a verdict and neither is read without
# the other.
#
# WATCHERS, DEC-061: `fastchess.sh` prints `SPRT-RUN-DONE` or `SPRT-RUN-FAILED`
# on every exit path, and the three places this script can die before reaching
# it -- the `cd`, a missing `fastchess.sh`, a failed `exec` -- each print one of
# their own, so a watcher has a terminal marker and four exits. Launch detached
# and never hold it open:
#
#   nohup adocs/data/S109_ltc.sh > .tuning/s109_ltc.log 2>&1 &
#   echo $! > .tuning/s109_ltc.pid
#   # poll .tuning/s109_ltc.log for SPRT-RUN-(DONE|FAILED), ceiling 27360 s,
#   # pid exit from the pid file, armed through Monitor with persistent: true.
#
# Before launching: `.ref-builds/50e3661` already exists -- it is the reference
# the `8+0.08` SPRT is playing while this is written -- and `ref_cache_ok`
# checks it on every run, rebuilding rather than playing it if it is dirty,
# moved or configured unlike the candidate (S212). **Do not clear it by hand
# while that SPRT is running**: the reference is played straight out of the
# worktree and is the one thing this harness does not snapshot. If it ever does
# need clearing, `git worktree remove --force .ref-builds/50e3661` and never a
# bare `rm -rf`, which leaves the `.git/worktrees/` entry behind and makes the
# next `git worktree add` fail as "missing but already registered".

set -uo pipefail

cd /home/max/ws/chesso || {
  echo "SPRT-RUN-FAILED: cd" >&2
  exit 1
}

# A FAILED EXEC MUST STILL LEAVE A MARKER. `set -e` is not on above, and an
# `exec` that cannot start its program dies with `env`'s own message on stderr
# -- which is not `SPRT-RUN-FAILED`, so the watcher this header arms would have
# nothing to break on and would spin to its 27360 s ceiling (DEC-061; S160's
# lost marker is the same failure one script up).
#
# Two guards, because they catch different things. The check names a missing or
# non-executable `fastchess.sh` outright; the lines after the `exec` catch the
# exec itself failing -- a missing `env`, an exhausted process table. They do
# not catch `env` failing to start `fastchess.sh` (a broken interpreter line,
# say): by then bash is gone and `env` exits 126 or 127 with no marker, which
# is what the watcher's pid-death exit is for.
#
# AN EXIT TRAP DOES NOT WORK HERE AND WAS TRIED (S151's header has the
# measurement on bash 5.2.21): a non-interactive bash whose `exec` fails exits
# from inside the builtin without running EXIT traps. `shopt -s execfail` is
# what makes the shell survive the failure instead, and with it the two lines
# after the exec run in the ordinary way.
[[ -x ./fastchess.sh ]] || {
  echo "SPRT-RUN-FAILED: no executable ./fastchess.sh in $PWD" >&2
  exit 1
}
shopt -s execfail

# OUT under .tuning/ rather than the /tmp default: the PGN is read after the
# run for the forfeit census and the pair variance, and /tmp is wiped at boot.
# .tuning/ is gitignored, and the games are not committed -- every per-game
# result is in the log (the S068 run-1 reasoning).
exec env \
  CAND=600f448 \
  REF=50e3661 \
  TC=32+0.32 \
  HASH=64 \
  ROUNDS=1000 \
  OUT="/home/max/ws/chesso/.tuning/s109_ltc_$(date +%Y%m%d_%H%M%S)" \
  ./fastchess.sh

# Reached only when the exec above failed, which `shopt -s execfail` is what
# makes possible. 127 is the status a shell gives a program it could not find.
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2
exit 127
