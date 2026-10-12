#!/usr/bin/env bash
#
# S119: THE TRANSPOSITION TABLE AS CACHE-LINE CLUSTERS, WITH AN AGED
# REPLACEMENT. Part C of the step; Parts A (the prefetch) and B (huge pages)
# are node-identical and land before this run as commits of their own, so the
# reference already has them and this run prices C alone.
#
#   nohup adocs/data/S119_sprt.sh > .tuning/s119_sprt.log 2>&1 &
#
# THE CANDIDATE. `tt_entry_t` is 20 bytes: the hash's low 32 bits as the key,
# score, move, evaluation, depth, a 16-bit generation and the type.
# `tt_cluster_t` is three of them in one 64-byte line, `alignas(64)`, and the
# allocation is aligned to the line; both are `static_assert`ed. A hash picks
# its cluster by its high bits through a 64 x 64 multiply, so every megabyte
# of `Hash` is used and nothing is rounded to a power of two. A store of a
# position its cluster holds keeps the old rule (depth-preferred inside a
# search, always across searches). Any other store takes a never-written way,
# else the way worth least, **depth less 2 plies per search of age**, age being
# a distance on the 16-bit counter's 65535-search cycle, so a wrap cannot make
# an old entry read as current (S271's finding on the 8-bit counter). `info`
# lines carry `hashfull`. Files: `src/transposition_table.{hpp,cpp}`,
# `src/data_structures.hpp`, `src/chesso.cpp`; the step file's "As built" has
# the rest.
#
# DEC-221: implemented from the step file's description, which quotes the Chess
# Programming Wiki's Transposition Table page (bucket systems sized to a cache
# line, ageing; https://www.chessprogramming.org/Transposition_Table), and from
# nothing else. No other engine's code was opened and no constant of one is
# behind any value here (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134). The age weight, 2 plies a search, is form (b), a
# derivation over this engine's own play, stated at its site
# (`src/transposition_table.cpp` `TT_AGE_PLIES`): one search per own move, so
# the next root is two plies below this one and a node both searches reach is
# two plies nearer it, searched about two plies deeper there if the iteration
# depth holds from move to move. Three entries a cluster is not a seed: it is
# what 20-byte entries leave in a 64-byte line. The `hashfull` sample (334
# clusters, 1002 entries) is reporting only.
#
# THE CAPACITY CHANGE IS PART OF WHAT THIS PRICES (DEC-258). The parent floors
# 16 MB / 24 bytes to a power of two: **524288 entries in 12 MiB**,
# direct-mapped. The candidate uses all 16 MiB: **262144 clusters, 786432
# entries, +50 %**, three to a cluster. The verdict is layout, replacement and
# capacity together, and no outcome below attributes it to one of them.
#
# THE PRESSURE RATIO (DEC-088), counted from the harness's own PGNs with
# `.tuning/coord/S119/pressure.py` (the `n=` comment fastchess writes per
# move): S272's run (719 games, 2026-10-11, `a7bb10e` against `16c8883`, both
# through the bare-FEN relay) wrote a mean **2.08e7 nodes per side per game**
# (median 2.07e7, p10 1.43e7, p90 2.73e7); S238's (32577 games, 2026-09-26)
# 1.82e7. Against the parent's 524288 entries that is **39.6 overwrites per
# entry** (S238: 34.6); against the candidate's 786432, **26.4** (S238: 23.1).
# DEC-088 fixed 16 MB to reproduce the rating list's 60 to 120 at 2'+1"; on
# these counts the harness sat under that before this step and sits about a
# third further under it after. That is the regime the verdict is taken in;
# DEC-088's Consequences line is why it is taken here and not at Hash 128.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES (the step file's "As built"
# has every figure and its run). Parent = A + B, node-identical to `b3a93aa`.
#
#   `bench` 4081329 -> 3872032 (-5.13 %), `bench 12` 1860699 -> 1730486
#   (-7.00 %); the eight replies the parent's but the fifth (d8d6 -> d8e7).
#   `tools/search_bench.py` depth 9: 32932 -> 32829, 70095 -> 69892, 25178 ->
#   25174; depth 12: 67792 -> 68090, 280873 -> 246712, 137893 -> 114555; best
#   moves c3d5 e2a6 d7c8q and c3d5 d5e6 d7c8q on both.
#
#   NPS AGAINST NODES-TO-DEPTH (S270's interleaved counters and hyperfine,
#   2026-10-12, `adocs/data/S119_timing.txt`). Hash 16: `bench 16` -5.71 %
#   nodes, -3.86 % nps, -1.64 % wall; S020's 300 positions at depth 11 -3.69 %
#   nodes, about -5.2 % nps, +1.62 % wall. Hash 128: `bench 16` +4.54 % nodes,
#   +8.27 % wall; the 300 positions -8.67 % nodes, -2.69 % wall. About 2.2 to
#   2.6 % more instructions a node. Time to depth goes both ways by workload.
#
#   HASHFULL in self-play of the candidate (median / p90 per move): 8+0.08 at
#   Hash 16 about 330 / 680; 10+0.2 at Hash 128 about 64 / 170; 2'+1" at Hash
#   128 about 430 / 790, one search at 988.
#
#   The counts move by construction; INV-6 does not discharge this change and
#   nothing here is Elo (DEC-019).
#
# THE PREDICTION, labelled as one and not a measurement. The step file's
# "5 to 15 Elo for bucket-plus-aging over always-replace" is **unsourced**: CPW
# states no figure, and two searches (2026-09-04, 2026-09-13) found no source
# for the range. This engine's own published-to-measured transfers read 0, 0,
# wrong sign, 0.10 and 0.33 (DEC-222's F01). No number is predicted here; the
# direction is: positive, from capacity (+50 % entries at a pressure of 26 to
# 40 overwrites an entry) and from the associativity, against the time the
# 3-way probe and the multiply cost.
#
# BOUNDS. elo0=0 elo1=5 nElo, alpha=beta=0.05, `model=normalized` --
# fastchess.sh's default, the gainer pair. DEC-063: the pair straddles the
# expected effect.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint, **25591** on a bound (`adocs/testing_strategy.md`
# section 1.1, D / (e1 - e0)^2 with D = 1046535, and 639770 / 25), at **2110
# games an hour** (`.moltke.local.md`): **19.8 h** and **12.1 h**. The cap is
# fastchess.sh's 20000 rounds, 40000 games, 19.0 h. Four hours or more, so a
# night (DEC-155).
#
# REGIME. 8+0.08, Hash 16 (DEC-088), one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 (DEC-050), fastchess.sh's
# adjudication, whatever governor the machine is under, recorded and not set
# (DEC-195). No harness change since the last fixed-rounds A/A (S272's, through
# the relay, and S219's without it); if fastchess, the book, the adjudication or
# the machine moves before this starts, the A/A of 1000 fixed rounds comes
# first (DEC-143).
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Stop it and report nothing, too, if
# the machine loses mains power (POWER) or another load starts on it while the
# run plays (MACHINE). Nothing else stops it -- not the length.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171), read
# off `adocs/status.md` at `b3a93aa`: **S259**, the S097 multicut row and the
# S113 ProbCut row in "pruning does not hide a forced mate" no longer separate
# their mutants E21 and B04 -- test-side, nothing reachable in play; this
# candidate moves the tree, and the rows were not re-mined here (both cases are
# green on it). No defect reachable in ordinary play, on the UCI surface, or
# able to move a reported score, move or line is open. The coordinator re-reads
# the list on the day it pins the pair.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> the cluster table -- layout, replacement and capacity
#                   together (DEC-258) -- gains at least 5 nElo at the harness
#                   setting. It stays. The stopping run's Elo is upward-biased
#                   (DEC-063) and is not the effect size. A reading at
#                   rating.sh's Hash 128 is a different tool, named as one
#                   (the step file's "verdict is taken at the harness setting").
#
#   H0           -> not shown to gain 5 nElo here. **Revert C** -- the cluster,
#                   the multiply, the replacement and `hashfull` -- by
#                   reverting its commit; A and B stay, node-identical and
#                   timed on their own. Unless a reason to keep it is stated
#                   and ruled on: `hashfull` alone is a UCI nicety that could
#                   be re-added on the direct-mapped table in a step of its
#                   own. The tests and mutants written for C leave with it.
#
#   No verdict   -> the 40000-game cap with neither bound: a zero, read as H0
#                   is, with the games and the interval recorded. No follow-up
#                   run and no second pair (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block before its `Bench:` line, in
# fastchess's own line names, with the log trimmed into
# `adocs/data/S119_sprt.log` by `tools/trim_console.py`.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path; this script execs it and adds nothing
# after.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR -- a COMMITTED candidate, not a working-tree one. The coordinator
# commits A and B (node-identical, `No functional change` against the bench
# both print, 4081329) and then C, and pins CAND to C's commit and REF to its
# parent, the commit that lands B (or A, if B is not kept). Committed rather
# than played from the working tree because the parent is itself two commits
# that have to exist before the match, DEC-220's block then names two plain
# shas that `tools/gate.sh` checks against the log, and nothing in a dirty tree
# needs a `candidate.diff` blob (DEC-254). Pinned by editing the defaults
# below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # C's landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates, and each side's padding, before the first game.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S119_sprt.sh before running (CAND is C's landing" \
         "commit, REF its parent: the commit that lands B, or A)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s119_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
