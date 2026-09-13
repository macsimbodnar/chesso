#!/usr/bin/env bash
#
# S199, the drift instrument: the engine as it stands now against a pinned
# early-S105 commit, one fixed 1000-pair match at the harness's own regime,
# repeated at every block boundary and read as a trend. Written before a single
# game is played, which is what makes everything below a pre-registration
# rather than a reading chosen after seeing a number.
#
# THIS RUN IS NOT A VERDICT AND CANNOT BECOME ONE, AND IT IS NOT A VERDICT ON
# ANY ONE CHANGE INSIDE IT. It is R14 of `adocs/testing_strategy.md` under
# DEC-139: fixed rounds, no SPRT, no bounds, read as an estimate with its own
# interval. `fastchess.sh` prints `bounds none -- fixed 1000 rounds, a
# calibration or drift reading, NOT a verdict` where the reader of the log
# meets it. Nothing in the engine changes on any outcome -- the step's
# `excludes:` says so, and per-patch attribution is what a fixed match cannot
# give (S199 `excludes:`, DEC-108).
#
# WHY IT EXISTS, IN ONE PARAGRAPH. A one-stage `{0,5}` SPRT passes a true zero
# one run in twenty, and the fishtest FAQ's own sentence is that "the SPRT Elo
# estimates are only unbiased if one takes all patches into account, both
# passed and non-passed ones". With about 45 pending verdicts the sum of the
# kept point estimates drifts up by construction, which is the mechanism S183
# is discounting for. fishtest's answer is the regression test: a fixed match
# against a pinned reference, repeated, read as a trend. This is that, at this
# project's scale.
#
# THE PINNED REFERENCE, AND IT IS PINNED FOR GOOD (S199 `excludes:`):
#
#   reference  f548ff4  2026-08-20  "Refuse a reused OUT, and date the 2559
#                                    rating to its hash (S105)"
#
# The accepts names "the first commit after S105 landed the harness regime".
# S105's regime landed in `21c1949` (2026-08-20, "Move the SPRT harness to the
# surveyed engines' regime"), and `f548ff4` is the commit directly after it.
#
# VERIFIED AT HEAD 690f7db, 2026-09-13, all three checks:
#
#   git rev-parse --short f548ff4^            -> 21c1949
#       the pin is exactly the child of the regime commit, so "the first commit
#       after the regime landed" is a sha and not a judgement.
#   git diff 21c1949 f548ff4 -- src/          -> empty
#       the pin's src/ IS the engine as it stood when the regime landed:
#       `f548ff4` changes `fastchess.sh` and `DEV_MANUAL.md` and nothing else.
#       So the pin is both the first commit after the landing and the engine of
#       the landing, and the two readings of the rule agree on one sha.
#   git log --oneline --reverse 21c1949~1..HEAD | head -3
#       21c1949  Move the SPRT harness to the surveyed engines' regime (S105)
#       f548ff4  Refuse a reused OUT, and date the 2559 rating to its hash
#       ec4d1dd  Sweep the transposition bound signs and mate round trip (S106)
#       `ec4d1dd` is the first commit after the landing that touches src/ at
#       all, so nothing of the engine has moved between the regime and the pin.
#
# THAT IT STILL BUILDS HERE WAS CHECKED BY READING, NOT BY BUILDING (a timed
# SPRT held the machine while this was written):
#
#   - `git diff --stat f548ff4 3488506 -- src/` is 5 files and 166 insertions,
#     and `3488506` (2026-08-20) is S151's reference, built fresh by `build_ref`
#     and played for 3 h 40 m on this machine on 2026-09-12 under gcc 13.3. The
#     pin is 166 lines of the same tree.
#   - the only include the range adds is `<charconv>`, added in the *newer*
#     commit; the pin needs nothing the pin does not have.
#   - `git show f548ff4:src/CMakeLists.txt` has `add_executable(chesso ...)`,
#     which is the target `build_ref` builds, and `git show
#     f548ff4:CMakeLists.txt` already includes `cmake/arch.cmake` -- S104
#     (`883c255`) predates the pin, so the reference is built with
#     `CHESSO_ARCH=native` and its hardware popcount, the same way the
#     candidate is. The 2026-09-10 audit's "11 of 30 cached worktrees predate
#     S104" is about older references and not this one.
#   - S167's dead `MAX` constant in `src/search.cpp` fails `-Werror` under
#     Apple clang only; gcc never warned on it, and gcc 13.3 is this machine's
#     reference compiler (DEC-049, `.moltke.local.md`).
#   - `git show f548ff4:src/chesso.cpp` answers `option name Hash type spin`
#     and `option name Threads type spin default 1 min 1 max 1`, so the
#     `-each option.Hash=16 option.Threads=1` this harness passes is understood
#     on both sides.
#
# THE REFERENCE ANSWERS THE BARE `id name Chesso`, AND THAT IS EXPECTED.
# `git show f548ff4:src/chesso.cpp` is `uci_reply("id name Chesso")`: the pin
# predates S212's build stamp, so `fastchess.sh`'s identity check prints
#
#   id name    ref-f548ff4  Chesso
#              ^ no build stamp: every commit before S212 answers the bare
#                literal, so what this binary holds cannot be checked here.
#
# and plays it. That is DEC-204 (b), decided for exactly this class of run --
# "the reference builds S151 and the drift matches play are exactly those".
# The CANDIDATE side is post-S212 and is checked: its stamp must be HEAD's sha,
# which is the next paragraph's whole point.
#
# THE CANDIDATE IS THE WORKING TREE, DELIBERATELY, AND THE COORDINATOR REBUILDS
# BEFORE LAUNCHING. There is no `CAND=` here: the drift point is "the engine
# now", so the candidate is `build/src/chesso` at whatever HEAD the coordinator
# runs this from, and the run is attributed to that sha on the banner and in
# `adocs/data/S199_drift.tsv`. A binary stamped `<sha>-dirty` is refused once
# HEAD has moved past it, so
#
#   cmake --build build -j12
#
# comes first, every time. That refusal is the identity check doing its job
# (S212, DEC-204 (a)), not an obstacle: the binary that plays must be the code
# the row names.
#
# WHEN THE FIRST POINT IS TAKEN. After the S109 block lands (DEC-139: "R14
# (S199) takes its first point after the S109 block"), on the workstation,
# beside the block's other reading -- `adocs/data/S109_ltc.sh`, the DEC-202
# longer-control estimate. A block boundary produces two numbers: drift at the
# regime, and transfer to four times the control. F30 of the 2026-09-10 audit,
# recorded in the step file.
#
# THE REGIME, each part with its origin. Every one of them is `fastchess.sh`'s
# current default and this script overrides none of them, which is the point:
# the drift point is taken at the regime verdicts are taken at, so the trend
# and the ledger are on one footing.
#
#   control      8+0.08    the control every verdict this project has taken was
#                          taken at (DEC-083, DEC-190). NOT overridden here.
#   hash         16        table pressure, not table size (DEC-088). NOT
#                          overridden here.
#   book         noob_3moves.epd   the harness's book since DEC-189/DEC-190.
#   rounds       1000      1000 pairs, 2000 games, `-repeat`, no `-sprt`
#                          (ROUNDS removes the SPRT entirely).
#   seed                   derived from the run stamp and printed on the
#                          banner, which is its only record (S198). Read it off
#                          the log when the point is written up.
#   concurrency  12        every core (MACHINE, DEC-050). Governor is recorded,
#                          never set or waited on (DEC-195).
#   power                  the workstation has no battery, so the POWER rule's
#                          adapter check does not apply here; it does not
#                          hibernate either, and the watcher's pid exit is what
#                          catches a killed run.
#
# IF THE REGIME EVER MOVES, THE SERIES BREAKS AND THE TSV SAYS SO. Two points
# are comparable only if the control, the hash and the book are the same; the
# book has already moved once (DEC-189). `S199_drift.py` therefore reads the
# regime off each run's own banner into a `regime` column, and `--check`
# refuses to difference two rows whose regime or reference disagree. A regime
# change is a harness change and owes DEC-143's A/A anyway; what it does to
# this instrument is that the trend restarts, and the rows say where.
#
# WHAT IT COSTS, from the measured throughput and not from a guess (DEC-155,
# DEC-143). The workstation does **2133 games an hour at 8+0.08** on this book
# with concurrency 12 -- S212's DEC-143 A/A of 2026-09-13, 1000 games in
# 28 m 08 s under governor `performance`, agreeing with DEC-190's 2110
# (`.moltke.local.md`), and with S109's own SPRT the same morning at 2162
# (1364 games in 37 m 51 s). So 2000 games is **56 minutes**, far under
# DEC-155's four-hour line: a daytime run, started when the machine is free.
#
# One effect pushes it the other way and is not in the estimate: the candidate
# is the stronger side by construction, and a lopsided match adjudicates
# earlier rather than later, which if anything shortens it. **The check is the
# first half hour's `Finished game` count** -- about 1070 at the estimate. The
# watcher's ceiling is twice the estimate, **6750 s (1 h 52 m 30 s)**, and it
# is wall-clock only while the machine is awake (S024's first run hibernated
# for 42 hours under a 9 h ceiling).
#
# ABORT RULE, and the two classes that are explicitly not one:
#
#   STOP for a time-forfeit rate over 1.0 % on a side:
#
#       python3 tools/forfeit_report.py <outdir>/games.pgn --max-pct 1.0
#
#   Both sides are chesso, so a thin margin costs both about equally; a rate
#   that is not near zero means something other than a thin margin, and the
#   two sides are three weeks of search changes apart in how they spend a
#   clock.
#
#   A CRASH OR A DISCONNECT VOIDS THE RUN, and `fastchess.sh` does that itself:
#   any termination outside normal, adjudication and time forfeit prints
#   `SPRT-RUN-INVALID` and then the `SPRT-RUN-FAILED` marker (S212,
#   2026-09-10_adversarial-F31). A void is not a reading and nothing is
#   appended to the TSV for it.
#
#   DO NOT STOP for `Incomplete mating PV` from the REFERENCE side. The pin
#   predates S147, S170 and S171, so `-check-mate-pvs` is expected to report
#   the class from it, exactly as S148's and S151's runs did (7/13 and 57/67
#   per side). Count the lines per side from the log and record both counts. A
#   non-zero count on the CANDIDATE side is a different matter -- that side
#   carries all three fixes -- and it is worth a sentence in the step file and
#   a look, not an abort of a run that has already been played.
#
#   Nothing else stops the run. There is no bound to cross and no cap to reach:
#   the stopping rule is the round count and it is written above.
#
# HOW THE POINT IS READ, all of it written before the first game. The rule is
# in `adocs/plan_current/S199_drift_match_against_pinned_reference.md` under
# "The reading rule" and it is not restated here so the two cannot drift apart.
# In one sentence: a point inside the previous point's interval plus the
# verdicts and conversions landed since is as expected; a point below that band
# names those verdicts as the suspects for S183's discount; no point is a
# verdict on any one of them. The first point has no previous point and is read
# against zero and against the sum of the kept verdicts since the pin, which
# the step file lists with their nElo point estimates.
#
# THE RESOLUTION THIS BUYS, the same arithmetic S151's header derives and
# checks against a printed run. With pair score on the 0-to-2 scale
# `adocs/data/S105_pairs.py` uses, the nElo half-width is `1.96 C / sqrt(2N)`
# with `C = 800 / ln 10 = 347.44` -- it depends on the pair count alone, not on
# the book and not on the draw rate -- and the logistic half-width is
# `1.96 * 694.8 * sd_pair / (2 sqrt N)`:
#
#   at N = 1000 pairs, v = 0.2905  ->  +/- 15.2 nElo, +/- 11.6 logistic Elo
#
# 0.2905 +/- 0.0184 is the pair-score variance measured on this book at 8+0.08
# (DEC-190's band, re-measured at 0.2939 +/- 0.0186 by S212's A/A). Both are
# A/A figures, at zero difference; this match has a real difference in it,
# which spends some of the pair spread on decisive pairs one way, so +/- 11.6
# is an upper reading of the logistic half-width and the nElo one does not
# move. Every point in the series carries its own measured variance in the TSV
# rather than this one.
#
# THE READER. `adocs/data/S199_drift.py <outdir> <runlog>` appends one row to
# `adocs/data/S199_drift.tsv` -- date, candidate sha, reference sha, regime,
# games, Elo and its 95 % interval, nElo and its interval, the pentanomial, the
# pair variance, and two columns the coordinator fills by hand: the verdicts
# landed since the previous point and the conversions landed since it (F30).
# `--check` re-reads the file and prints the trend. It cross-checks the
# pentanomial it reads from the PGN against the one fastchess printed and
# refuses the row if they disagree, so a row is never half from one run.
#
# WATCHERS, DEC-061: `fastchess.sh` prints `SPRT-RUN-DONE` or `SPRT-RUN-FAILED`
# on every exit path, and the three places this script can die before reaching
# it -- the `cd`, a missing `fastchess.sh`, a failed `exec` -- each print one of
# their own, so a watcher has a terminal marker and four exits. Launch detached
# and never hold it open:
#
#   cmake --build build -j12          # the identity check needs HEAD's binary
#   nohup adocs/data/S199_drift.sh > .tuning/s199_drift.log 2>&1 &
#   echo $! > .tuning/s199_drift.pid
#   # poll .tuning/s199_drift.log for SPRT-RUN-(DONE|FAILED), ceiling 6750 s,
#   # pid exit from the pid file, armed through Monitor with persistent: true.
#
# Before launching: there is no `.ref-builds/f548ff4` on this machine today, so
# the first run builds it (about a minute with ccache warm) and every later
# point reuses it -- validated on each run by `ref_cache_ok`, which rebuilds it
# rather than playing it if it is dirty, moved or configured unlike the
# candidate (S212, 2026-09-10_adversarial-F06). Never `rm -rf` a cached
# worktree: `git worktree remove --force .ref-builds/f548ff4`, because a bare
# removal leaves the `.git/worktrees/` registration behind and the next
# `git worktree add` refuses the path.

set -uo pipefail

cd /home/max/ws/chesso || {
  echo "SPRT-RUN-FAILED: cd" >&2
  exit 1
}

# A FAILED EXEC MUST STILL LEAVE A MARKER. `set -e` is not on above, and an
# `exec` that cannot start its program dies with `env`'s own message on stderr
# -- which is not `SPRT-RUN-FAILED`, so the watcher this header arms would have
# nothing to break on and would spin to its 6750 s ceiling (DEC-061; S160's
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
# measurement): a non-interactive bash whose `exec` fails exits from inside the
# builtin without running EXIT traps. `shopt -s execfail` is what makes the
# shell survive the failure instead, and with it the two lines after the exec
# run in the ordinary way -- which is why they are written out rather than left
# to a trap.
[[ -x ./fastchess.sh ]] || {
  echo "SPRT-RUN-FAILED: no executable ./fastchess.sh in $PWD" >&2
  exit 1
}
shopt -s execfail

# OUT under .tuning/ rather than the /tmp default: the PGN is read after the
# run for the forfeit census and by S199_drift.py for the pentanomial and the
# pair variance, and /tmp is wiped at boot. .tuning/ is gitignored, and the
# games are not committed -- every per-game result is in the log.
#
# TC and HASH are deliberately not set: the regime is `fastchess.sh`'s own
# default and the banner records what that was on the day (see "IF THE REGIME
# EVER MOVES" above).
exec env \
  REF=f548ff4 \
  ROUNDS=1000 \
  OUT="/home/max/ws/chesso/.tuning/s199_drift_$(date +%Y%m%d_%H%M%S)" \
  ./fastchess.sh

# Reached only when the exec above failed, which `shopt -s execfail` is what
# makes possible. 127 is the status a shell gives a program it could not find.
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2
exit 127
