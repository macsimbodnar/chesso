#!/usr/bin/env bash
#
# S202: THE CENSUS THE ACCEPTS NAMES. A fixed-rounds match in the `--fast`
# regime between the landing of S202's two reporting fixes and its parent,
# read for how many `Incomplete mating PV` warnings each side prints.
#
#   nohup adocs/data/S202_census.sh > .tuning/census_s202.log 2>&1 &
#
# WHAT IS MEASURED, AND WHY IT IS NOT A VERDICT. The two builds are INV-6
# identical -- `bench` 4081329 with the whole stream, `chesso bench 12`, and
# `tools/search_bench.py` at depths 9 and 12 node for node and move for move
# (the step file's "Phase 2, as built") -- so they play the same moves from
# the same positions and the Elo this run prints is noise around zero by
# construction. It is a census, not an SPRT: `ROUNDS=1500` removes the SPRT
# (fastchess.sh, "FIXED ROUNDS"), so 3000 games are played whatever the
# score, at the `--fast` regime S171's census used (DEC-150): 8+0.08, Hash 16,
# one thread a side, `-check-mate-pvs`, `books/noob_3moves.epd` (DEC-189),
# concurrency every core (MACHINE rule). Both engines play in the same run,
# so the reference's count is measured beside the candidate's and no figure
# crosses machines (DEC-049).
#
# WHAT THE FIXES DO, so the count is read against the right thing (DEC-251).
#   (c1) An aborted iteration's line the walk cannot complete against the last
#        completed score gives way to the last completed line when both start
#        with the move played: S170's third cause, closed where it can be.
#   (c2) The walk takes a move certified at the distance still owed -- the
#        entry's own when exact there, else a certified child -- before a
#        bound entry's move: DEC-150's class, closed where the table certifies
#        a line.
# Over the S170 grid on `fe5d3b3` they took 33 short lines of 873 to 19 of
# 873, the mate-line count unmoved. What is left is DEC-122's residue: a line
# the table does not hold at that moment, left short and visible.
#
# THE READING, WRITTEN BEFORE A GAME IS PLAYED (DEC-063, DEC-150).
#
#   DEC-150's ceiling is **8 `Incomplete mating PV` lines from 1 search in
#   3000 games**, the candidate's own count; it is a ceiling and not a zero.
#   A line is one warning block; a search is one distinct `Position;` +
#   `Moves;` pair among the candidate's warnings.
#
#   candidate at or below 8 lines   -> the accepts' census clause holds. The
#   and at or below 1 search           reference's own count is recorded
#                                      beside it, whatever it is.
#   candidate above the ceiling     -> the clause does not hold. Each warning's
#                                      `Position;` and `Moves;` become a
#                                      replay with adocs/data/S170_replay.py
#                                      and build/tools/mate_trace (stride 2,
#                                      DEC-151) before anything else is read;
#                                      the step does not complete on it.
#   candidate above the reference   -> by itself, nothing: which side meets a
#                                      mate position is the game's, not the
#                                      build's (DEC-150 read 8 against 0 from
#                                      identical builds). A candidate warning
#                                      whose position the reference also met
#                                      and printed complete is a regression and
#                                      is replayed on both binaries first.
#
#   Counted per side from this script's own log, which is fastchess's stdout.
#   With CAND set, fastchess.sh names the sides `cand-<sha>` and `ref-<sha>`
#   (its `cand_name` and the reference's `-engine ... name=`), never
#   `candidate`, so a count on `from candidate` would read 0 whatever the
#   engine printed:
#
#     grep -c 'Incomplete mating PV - from cand-' LOG
#     grep -c 'Incomplete mating PV - from ref-'  LOG
#
#   Both counts are read from the same log and a zero on both sides is checked
#   against the banner's two engine names before it is believed.
#   and searches by the distinct `Position;`/`Moves;` pairs under each.
#
# EXPECTED WALL TIME, stated before the run (RUNS rule): 3000 games at the
# 2110 games an hour `.moltke.local.md` records for this regime is about
# **1 h 25 m**. Under four hours, so it starts when ready, during the day
# (DEC-155).
#
# ABORT RULE. Stop and report nothing if the time-forfeit rate passes 1.0 % on
# either side (`tools/forfeit_report.py` over the run's PGN in `$OUT`). A crash
# or a disconnect on either side voids it outright. Mains, and a second load
# on the machine. Nothing else is an abort.
#
# HARNESS. No harness change is made here. If fastchess, the book, the
# adjudication or the machine has moved since the last fixed-rounds A/A, that
# A/A comes first (DEC-143).
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, fixed rounds included.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR. CAND is the commit that lands S202's two fixes; REF is its parent.
# Both are pinned after the landing commit exists, by editing the defaults:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S202_census.sh before running (CAND is S202's landing" \
         "commit, REF its parent)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     ROUNDS=1500 \
     OUT="/home/max/ws/chesso/.tuning/census_s202_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh --fast
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
