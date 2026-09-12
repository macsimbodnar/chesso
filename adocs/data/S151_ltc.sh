#!/usr/bin/env bash
#
# S151, S085's shipped vector read at four times the control it was verified
# at. Written before a single game is played, which is what makes everything
# below a pre-registration rather than a reading chosen after seeing a number.
#
# THIS RUN IS NOT A VERDICT AND CANNOT BECOME ONE. It is design (iii) of the
# step file's section 7, chosen by DEC-172: a fixed 1000-pair match, no SPRT,
# no bounds, read as an estimate with its own interval. `fastchess.sh` prints
# `bounds none -- fixed 1000 rounds, a calibration or drift reading, NOT a
# verdict` where the reader of the log meets it. Nothing in the engine changes
# on any outcome -- the step's `excludes:` says so, and which axis to revert,
# hold or re-tune is a new step opened from the stamp, never this one.
#
# BOTH SIDES ARE COMMITS AND NEITHER IS THE WORKING TREE. `CAND=<ref>` is
# S151's own harness addition: both binaries are built from their commit into
# `.ref-builds/<sha>` by the same function, and the candidate is snapshotted
# before the first game like any other (DEC-020). The banner carries both shas
# with their own commit dates and no dirty flag, because the tree is neither
# read nor played.
#
#   candidate  21b4a21  2026-08-21  "Ship S085's tuned vector, eleven axes of twelve"
#   reference  3488506  2026-08-20  "Regenerate status after DEC-094 amended S085's goal"
#
# VERIFIED AT HEAD 1fc0beb, 2026-09-12, all three checks:
#
#   git rev-parse --short 21b4a21^        -> 3488506
#       the reference is exactly the parent of the vector commit, so nothing
#       else rides along in the comparison.
#   git diff --stat 3488506 21b4a21 -- src/
#       src/evaluation.hpp    |  9 ++++++---     (comment only)
#       src/search_params.hpp | 52 ++++++++------
#       2 files changed, 42 insertions(+), 19 deletions(-)
#   git diff 21b4a21 43bf189 -- src/      -> empty
#       S085's completing commit changes no source, so 21b4a21 is the candidate
#       and 43bf189 is not.
#
# THE TEN DEFAULTS THAT MOVED, re-read from
# `git diff 3488506 21b4a21 -- src/search_params.hpp` at HEAD and not from an
# earlier document:
#
#   MAX_QSEARCH_DEPTH    "MaxQsearchDepth"       8 -> 19
#   RFP_MARGIN           "RfpMargin"            75 -> 63
#   RFP_MAX_DEPTH        "RfpMaxDepth"           6 -> 15
#   NULL_MOVE_BASE       "NullMoveBase"          2 -> 3
#   LMR_BASE             "LmrBase"              75 -> 52
#   LMR_DIVISOR          "LmrDivisor"          225 -> 182
#   LAZY_EVAL_MARGIN     "LazyEvalMargin"      150 -> 184
#   ASPIRATION_MIN_DEPTH "AspirationMinDepth"    5 -> 2
#   ASPIRATION_DELTA     "AspirationDelta"      50 -> 21
#   ASPIRATION_MAX_DELTA "AspirationMaxDelta"  400 -> 437
#
# `NULL_MOVE_DIVISOR` came back unchanged at 6 and `RFP_MIN_PLY` was held at 3
# (the twelfth axis, DEC-094). The run measures the ten jointly and **cannot
# attribute anything to one axis**; the only other change in the range is a
# comment in `src/evaluation.hpp`, which compiles to nothing.
#
# WHAT THE 8+0.08 VERIFICATION SAID, verbatim from
# `adocs/plan_done/S085_spsa_first_run.md`, "Verdict: H1 accepted, 2026-08-21
# 05:08" -- the figure this run is read against:
#
#   Elo: 21.02 +/- 9.86, nElo: 26.81 +/- 12.55
#   LOS: 100.00 %, DrawRatio: 35.78 %, PairsRatio: 1.31
#   Games: 2946, Wins: 1088, Losses: 910, Draws: 948, Points: 1562.0 (53.02 %)
#   Ptnml(0-2): [121, 289, 527, 363, 173], WL/DD Ratio: 2.56
#   LLR: 2.95 (100.0%) (-2.94, 2.94) [0.00, 5.00]
#   SPRT ([0.00, 5.00]) completed - H1 was accepted
#
# 2946 games in 1 h 15 m 42 s, 0 forfeits, `8+0.08`, `Hash=16`, concurrency 12,
# book `UHO_Lichess_4852_v1.epd`. +21.02 is an SPRT's early-stopped point
# estimate and is biased upward (DEC-063): S068's pooled figure fell from
# +12.18 to +5.02 under that correction. So the number this run is compared to
# is itself an upper reading, and that asymmetry is stated here rather than
# discovered afterwards.
#
# WHY A LONGER CONTROL AT ALL. vondele's `nevergrad4sf`: "the optimal
# parameters are often time sensitive, i.e. can be verified to be a gain at the
# VSTC used for tuning, but regress at STC or LTC". Stockfish issue #2600, of
# the last 40 LTC tests with gaining bounds: "23 reds, 16 yellows, 1 green".
# S085 tuned at `2+0.02` and verified once at `8+0.08`; every axis it moved
# went toward more pruning and more reduction, and `RFP_MAX_DEPTH` 15 and
# `MAX_QSEARCH_DEPTH` 19 are exactly the depth-bounded kind the wiki says "may
# only be effectively tested on time controls where this new condition is
# triggered frequently enough". A regression here is the outcome the published
# record says to expect, and it is recorded as one if it comes.
#
# THE REGIME, each part with its origin:
#
#   control      32+0.32   four times 8+0.08, the accepts' floor and the
#                          cheapest admissible ratio; fishtest's is 6 and
#                          48+0.48 costs 1.5x again. DEC-172 question 6.
#   hash         64        four times the clock is about four times the nodes a
#                          game writes, so Hash=16 at 32+0.32 would be four
#                          times the table pressure every verdict was taken at.
#                          64 holds DEC-088's invariant, and it is fishtest's
#                          own LTC setting. DEC-172 question 2.
#   rounds       1000      1000 pairs, 2000 games, `-repeat`, no `-sprt`.
#   book         noob_3moves.epd   the harness's current book (DEC-189,
#                          DEC-190, DEC-191). The step's guide names the UHO
#                          book because it was written before the switch; the
#                          run uses the harness's book, and the reading is
#                          attributed to it as DEC-049 attributes a figure to
#                          its machine.
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
# DEC-143). The workstation does **2110 games an hour at 8+0.08** on this book
# with concurrency 12 -- S219's DEC-143 A/A of 2026-09-12, 1000 games in
# 28 m 26 s, `.moltke.local.md` and `adocs/data/S219_aa_calibration.md`. Time
# management spends a share of the clock plus part of the increment, so seconds
# a ply scale with the control: at a control ratio of 4 that is about **528
# games an hour** and **about 3.8 h for 2000 games**. Under DEC-155's four-hour
# line, so it is a daytime run.
#
# Two effects push it the other way and neither is in the estimate: the draw
# rate rises with the control (the Komodo doubling series runs 49.20 % to
# 76.63 %), and the draw adjudication needs eight moves inside 10 cp after move
# 40, so a drawn game may run longer rather than ending sooner. **The check is
# the first hour's `Finished game` count** -- about 528 at the estimate. The
# watcher's ceiling is twice the estimate, **27360 s (7 h 36 m)**, and it is
# wall-clock only while the machine is awake.
#
# ABORT RULE, and one class that is explicitly not one:
#
#   STOP for a time-forfeit rate over 1.0 % on a side. 8+0.08 leaves
#   MOVE_OVERHEAD_MS 50 the least room of any control this harness has run and
#   32+0.32 leaves four times more, so a forfeit here would mean something
#   other than a thin margin:
#
#       python3 tools/forfeit_report.py <outdir>/games.pgn --max-pct 1.0
#
#   DO NOT STOP for `Incomplete mating PV`. Both commits predate S147, S170 and
#   S171, so `-check-mate-pvs` will report the class from **both** sides. It is
#   not a finding against either engine here. Count the lines per side from the
#   log and record the two counts, as S148's run did (7 candidate against 13
#   reference there).
#
#   Nothing else stops the run. There is no bound to cross and no cap to reach:
#   the stopping rule is the round count and it is written above.
#
# THE THREE READINGS, all three written before the first game:
#
#   1. THE RUN'S OWN INTERVALS. Quote the printed `Elo` and `nElo` lines with
#      their half-widths, and read the interval's position against two marks:
#      zero, and the `8+0.08` figure of `Elo 21.02 +/- 9.86` / `nElo 26.81
#      +/- 12.55`. An interval clear of zero and overlapping +21 says the gain
#      transfers; one clear of zero and well below it says part of it does;
#      one straddling zero says this run cannot separate the vector from its
#      parent at four times the control, which is a statement about the run's
#      resolution and not a verdict of zero. A logistic gain is expected to
#      read *smaller* at a longer control even when it transfers perfectly
#      (the Komodo compression), and Stockfish's own data says that compression
#      is not universal -- so the nElo pair is the one to lead with.
#
#   2. THE RESOLUTION THIS BUYS, derived here and checkable against a printed
#      run. With pair score on the 0-to-2 scale that `adocs/data/S105_pairs.py`
#      uses, sd of the mean per-game score over N pairs is `sd_pair / (2 sqrt
#      N)`, the logistic slope at 50 % is `400 / (ln 10 * 0.25) = 694.8` Elo per
#      unit score, and `nElo = C sqrt(2) (p - 0.5) / sd_pair` with `C = 800 /
#      ln 10 = 347.44`, so the nElo half-width is `1.96 C / sqrt(2N)` and does
#      not depend on the book or the draw rate at all.
#
#        at N = 1000 pairs, v = 0.2905  ->  +/- 11.6 logistic Elo, +/- 15.2 nElo
#
#      0.2905 +/- 0.0184 is the pair-score variance measured on this book at
#      8+0.08 (DEC-190's band). The draw rate at 32+0.32 is expected higher,
#      which lowers the pair variance, so +/- 11.6 is an upper reading of the
#      logistic half-width and the nElo one does not move.
#
#      Both formulas reproduce S085's own printed run to the digit: at its 1473
#      pairs and `Ptnml [121, 289, 527, 363, 173]` they give nElo 26.81 and a
#      half-width of 12.55 against the printed `26.81 +/- 12.55`, and 9.82
#      against the printed Elo half-width of 9.86. That check is what makes the
#      +/- 11.6 above an arithmetic result rather than an assertion.
#
#      The step file's section 7 states +/- 10.5 Elo at v = 0.2395, the old
#      book's figure; this is the same arithmetic at the current book's
#      variance and supersedes it for this run.
#
#   3. THE PAIR VARIANCE AT 32+0.32, WHICH IS AN OUTPUT OF THIS RUN. Nothing
#      has ever measured it at a longer control. Read it off the PGN:
#
#        python3 -c "import sys; sys.path.insert(0, 'adocs/data'); import S105_pairs; \
#          S105_pairs.report('<outdir>/games.pgn', engine='cand-21b4a21')"
#
#      **The engine name is not optional.** `S105_pairs.py`'s own `main`
#      defaults to `chesso-a`, and a name matching neither side is not an
#      error: every game then scores as Black's points, a pair one side won
#      twice reads 0.0 instead of 2.0, and the variance comes back inflated
#      with nothing printed to say so (S198's header is where that trap is
#      written down). This run names its sides `cand-21b4a21` and
#      `ref-3488506`. It is not a DEC-143 A/A -- the two sides are different
#      engines, so the number carries a strength difference as well as the
#      harness's noise -- and it is recorded as a first reading at this
#      control, not as a band.
#
# WHAT THE RULE THIS STEP WRITES DOES NOT DEPEND ON. The block-boundary rule in
# `fastchess.sh`'s header and `DEV_MANUAL.md` "Which bounds" is written whatever
# this run returns: its wording is about when a longer-control reading is taken,
# not about what this one said.
#
# WATCHERS, DEC-061: `fastchess.sh` prints `SPRT-RUN-DONE` or `SPRT-RUN-FAILED`
# on every exit path, and the three places this script can die before reaching
# it -- the `cd`, a missing `fastchess.sh`, a failed `exec` -- each print one of
# their own, so a watcher has a terminal marker and four exits. Launch detached
# and never hold it open:
#
#   nohup adocs/data/S151_ltc.sh > .tuning/sprt_s151.log 2>&1 &
#   echo $! > .tuning/sprt_s151.pid
#   # poll .tuning/sprt_s151.log for SPRT-RUN-(DONE|FAILED), ceiling 27360 s,
#   # pid exit from the pid file, armed through Monitor with persistent: true.
#
# Before launching: if `.ref-builds/3488506` is still the 2026-08-21 cached
# worktree, remove it with `git worktree remove --force .ref-builds/3488506`
# (never a bare `rm -rf`, which leaves the `.git/worktrees/` entry behind and
# makes the next `git worktree add` fail as "missing but already registered"),
# so both sides are built fresh by the same `build_ref` under today's compiler
# and flags. A cached reference built under an older configuration is a
# difference between the sides that is not the change. Done by the coordinator
# on 2026-09-12 before this run.

set -uo pipefail

cd /home/max/ws/chesso || {
  echo "SPRT-RUN-FAILED: cd" >&2
  exit 1
}

# A FAILED EXEC MUST STILL LEAVE A MARKER. `set -e` is not on above, and an
# `exec` that cannot start its program dies with `env`'s own message on stderr
# -- which is not `SPRT-RUN-FAILED`, so the watcher this header arms would have
# nothing to break on and would spin to its 27360 s ceiling (DEC-061, and
# S160's lost marker is the same failure one script up).
#
# Two guards, because they catch different things. The check names a missing or
# non-executable `fastchess.sh` outright; the lines after the `exec` catch the
# exec itself failing -- a missing `env`, an exhausted process table. They do
# not catch `env` failing to start `fastchess.sh` (a broken interpreter line,
# say): by then bash is gone and `env` exits 126 or 127 with no marker, which
# is what the watcher's pid-death exit is for. After a successful exec neither
# guard exists any more, the process image is gone, and from there on the
# markers are `fastchess.sh`'s own.
#
# AN EXIT TRAP DOES NOT WORK HERE AND WAS TRIED. A non-interactive bash whose
# `exec` fails exits from inside the builtin without running EXIT traps:
# measured on bash 5.2.21, `trap 'echo fired' EXIT; exec /nonexistent/env`
# prints only the shell's own `No such file or directory` and exits 127.
# `shopt -s execfail` is what makes the shell survive the failure instead, and
# with it the two lines below run in the ordinary way -- which is also why they
# are written out rather than left to a trap.
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
  CAND=21b4a21 \
  REF=3488506 \
  TC=32+0.32 \
  HASH=64 \
  ROUNDS=1000 \
  OUT="/home/max/ws/chesso/.tuning/s151_ltc_$(date +%Y%m%d_%H%M%S)" \
  ./fastchess.sh

# Reached only when the exec above failed, which `shopt -s execfail` is what
# makes possible. 127 is the status a shell gives a program it could not find.
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2
exit 127
