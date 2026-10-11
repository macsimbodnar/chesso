#!/usr/bin/env bash
#
# S272: WHAT S271 BOUGHT A CLIENT THAT SENDS THE BOARD. S271 (`a7bb10e`) stopped
# a new base FEN from clearing the transposition table (DEC-264); its parent
# `16c8883` clears it on every `position` whose FEN differs from the last base.
# fastchess holds the base fixed for a game, so in the harness the two play
# node for node alike (INV-6) and S271 landed on node counts. Here both engines
# are driven through `tools/bare_fen_relay.py`, which sends every position as a
# bare FEN -- the regime of a GUI that sends the board, and the one regime where
# S271 changes play: the parent searches every move on a cold table, the
# candidate on a warm one. DEC-264 recorded this as validation, run on the
# workstation when it is free, and S271 does not wait on it.
#
#   nohup adocs/data/S272_sprt.sh > .tuning/s272_sprt.log 2>&1 &
#
# ONLY AFTER THE A/A. The relay is a harness change, so `adocs/data/S272_aa.sh`
# -- 1000 fixed-rounds games, `a7bb10e` against itself through the relay -- is
# run and read by its own reading rule first (DEC-143). If it does not pass,
# this does not start. The relay blob this banner prints must be the A/A's and
# HEAD's.
#
# THE PAIR. CAND `a7bb10e`, S271's landing; REF `16c8883`, its parent. Both are
# built by fastchess.sh into `.ref-builds/` and played as commits (`CAND=`), so
# the log's result line reads `cand-a7bb10e vs ref-16c8883` and DEC-220's
# block is checkable by `tools/gate.sh` against it.
#
# REGIME. `BARE_FEN=1`: both sides through the relay (python-chess computes each
# board from fastchess's base and moves; castling, en passant and both counters
# are python-chess's, `tests/test_bare_fen_relay.py`). Otherwise the default:
# 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd` (DEC-189),
# concurrency 12 (DEC-050), fastchess.sh's adjudication, `model=normalized`.
# The relay is a different regime and this is the one run that sets it; no
# other verdict's figure is comparable to this one. What it costs a move:
# fastchess starts the clock at `go`, after the `readyok` that follows
# `position`, so the rewrite is off the clock, and the hop of `go` in and
# `bestmove` out through it measured a median of 0.05 ms and a 99th percentile
# of 0.28 ms with nothing else running (load average 0.3), 0.02 ms and 2.4 ms
# beside twelve busy engines
# (`adocs/data/S272_relay_latency.txt`), against an 80 ms increment, on both
# sides alike. The reference's own table clear, which runs inside `position`,
# is off the clock for the same reason: this prices the cold table, not the
# clear.
#
# BOUNDS. elo0=0 elo1=5 nElo, alpha=beta=0.05 -- fastchess.sh's default, the
# gainer pair: the change is claimed to gain in this regime.
#
# WHAT THE PAIR COSTS (DEC-143). The worst case, the truth at the interval's
# midpoint: **41861 expected games**, and **25591** with it on a bound
# (`adocs/testing_strategy.md` section 1.1, `D / (e1 - e0)^2` with
# D = 1046535, and 639770 / 25). At 2110 games an hour (`.moltke.local.md`;
# the A/A's own rate through the relay replaces it when read): **19.8 h** and
# **12.1 h**. The cap is fastchess.sh's 20000 rounds, 40000 games, 19.0 h.
#
# THE PREDICTION, stated here and never quoted as the result (accepts 4).
# `adocs/data/S271_replay.md`: over 8653 positions of 200 self-play games at
# depth 12 and Hash 16, the parent sent a bare FEN each move searched x1.218 the
# nodes of base plus moves (median x1.294), and S271 x1.001. Read as the same
# share of thinking time lost and priced at S219's +174.85 Elo per doubling,
# that is **about +50 Elo** for S271 in this regime (about +53 at depth 14).
# In nElo, at the 33.9 % draws of S212's A/A, about +61 (e_n = e / sqrt(1 - d),
# the balanced-book approximation; the relay's A/A draw rate replaces it). It
# is a conversion, not a measurement: it assumes the extra nodes cost what the
# same share of time costs, at a depth 8+0.08 does not always reach, and the
# replay's ratio grew with depth and hash.
#   If the prediction held, the run would be short: with the truth far above
#   the interval the expected length is about ln(19) * C^2 / ((e1 - e0) *
#   (theta - (e0 + e1) / 2)) games, C^2 = 1046535 / ln(19)^2 = 120712 -- **about
#   1200 games, 35 minutes** at +61 nElo; 2600 games, 1 h 14 m at +30. That is
#   what puts the run under four hours (RUNS, DEC-155) and lets it start in the
#   day; the worst case above is what it costs if the prediction is wrong, and
#   the run is let go to its verdict either way.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect voids it outright,
# and so does a position the relay passed unrewritten (both `SPRT-RUN-INVALID`,
# fastchess.sh). Mains, and a second load on the machine. Nothing else -- not
# the length.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171), read
# off `adocs/plan.md` on 2026-10-11: **S259**, the S097 multicut row and the
# S113 ProbCut row in "pruning does not hide a forced mate" no longer separate
# their mutants E21 and B04 -- test-side, nothing reachable in play. No defect
# reachable in ordinary play, on the UCI surface, or able to move a reported
# score, move or line is open. The coordinator re-reads the list on the day.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
# S271 is not reverted by any outcome: it is the UCI-correctness change the
# owner ruled on (DEC-264), and the default regime, which every other verdict
# plays in, is node-identical across it.
#
#   H1 accepted  -> for a client that sends the board, S271's kept table is
#                   worth Elo against the parent's cold one: the replay's
#                   direction, measured in games. The stopping run's Elo is
#                   upward-biased and is not the effect size; it is stated
#                   beside the prediction above and not as a check of it.
#
#   H0           -> the kept table is not shown to be worth 5 nElo to such a
#                   client at 8+0.08 and Hash 16. S271 stands. What an H0 does
#                   mean: the replay's conversion -- extra nodes to a fixed
#                   depth read as lost time, priced per doubling -- does not
#                   carry to games at this control, and "about +50 Elo" is not
#                   quoted again without that. Its readings: the searches at
#                   8+0.08 are shallower than the replay's depth 12, where the
#                   cost was already smaller than at 14; a cold table may cost
#                   fewer moves than nodes. A reading at a longer control or a
#                   larger hash, where the replay read more, is a new question
#                   for the owner, not a re-run.
#
#   No verdict   -> the 40000-game cap with neither bound: a zero, read as H0
#                   is, with the games and the interval recorded. No follow-up
#                   run and no second pair (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block before its `Bench:` line, and the
# log it names is this run's console TRIMMED, not copied:
#
#   python3 tools/trim_console.py --summary < .tuning/s272_sprt.log > adocs/data/S272_sprt.log
#
# A bare FEN hides the game's history from both sides, so their PVs run past
# repetitions and fastchess prints a four-line "PV continues after threefold
# repetition" block for each: 93 % of the smoke consoles' bytes, about 8.4 KB
# a game -- some 22 MB at the 2600 games of the slower prediction, hundreds at
# the cap, against the 20 MB `tools/gate.sh` takes (DEC-235). `--summary`
# drops the blocks and prints their count per rule and engine at the end; every
# other line passes, the `Results of cand-a7bb10e vs ref-16c8883` line the gate
# matches among them. Proved on this script's smoke console, S272's step file:
# the trimmed log carried that line and `tools/gate.sh --message` passed a dry
# block against it. The untrimmed console goes to the owner's archive with the
# PGN (DATA rule).
#
# Proposed, the coordinator's call: the tc field written `8+0.08 bare-FEN`,
# which both `tools/gate.sh` and `tools/ledger.py` read as the free-text field
# it is, so the ledger row says which regime this verdict was taken in.
#
# MARKERS (WATCHERS, DEC-061). fastchess.sh prints `SPRT-RUN-DONE` or
# `SPRT-RUN-FAILED` on every exit path; this script execs it and adds nothing
# after. THE RELAY'S NOTES FILE, for a watcher to poll while the run plays:
# `<out>/relay_notes.txt`, emptied at the start, `<out>` being
# `/home/max/ws/chesso/.tuning/sprt_s272_<YYYYmmdd_HHMMSS>` from the main
# checkout; the banner's `notes to` line prints the whole path. Any line in it
# voids the run at its end; a watcher may stop it early. SMOKE=1 plays 4 games at 1+0.01 on 2 threads with no SPRT, into a
# `smoke_` directory, to prove the plumbing; its output is not a reading.

set -uo pipefail

repo="$(cd -- "$(dirname -- "$0")/../.." && pwd)" \
  || { echo "SPRT-RUN-FAILED: cannot find the repository" >&2; exit 1; }
cd "$repo" || { echo "SPRT-RUN-FAILED: cd $repo" >&2; exit 1; }
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }

# Nothing the caller's environment holds decides the regime: fastchess.sh
# reads each of these, and a ROUNDS left exported would remove the SPRT.
unset ROUNDS TC HASH CONCURRENCY SRAND AA RELAY_PYTHON

CAND=a7bb10e
REF=16c8883
stamp="$(date +%Y%m%d_%H%M%S)"

smoke=()
out="$repo/.tuning/sprt_s272_$stamp"
if [[ -n "${SMOKE:-}" ]]; then
  smoke=(ROUNDS=2 TC=1+0.01 CONCURRENCY=2)
  out="$repo/.tuning/coord/S272/smoke_sprt_$stamp"
fi

shopt -s execfail
exec env CAND="$CAND" REF="$REF" BARE_FEN=1 ${smoke[@]+"${smoke[@]}"} OUT="$out" ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2
exit 127
