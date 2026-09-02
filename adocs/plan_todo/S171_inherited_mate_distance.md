id:         S171
goal:       a mate distance the engine reports is one the position holds, so a score read back from the transposition table cannot claim a mate at a distance a deeper search of the same position contradicts
accepts:    the case S170 measured is reproduced and its mechanism established from the code rather than argued -- `adocs/data/S170_replay.py --only E_mate_minus9 --go "nodes 1500000" --start-override 40` reports `mate -9` at depths 9 to 13 where the same position on a cold table reports `mate -7` at depth 18 and holds it to depth 24, and `stockfish` says `#-7` at depth 30 and 36; a minimized failing test observed red before any fix, driving the engine the way `tests/test_mate_carry.cpp` does because a cold table does not reproduce it; the fix decided by SPRT if it alters play at all and by identical `tools/search_bench.py` node counts and best moves if it does not (INV-6); a `fastchess.sh --fast` run reports **0** `Incomplete mating PV` lines from the candidate, which is the residual S170 could not close and the reason this step exists; `MANUAL.md`, `adocs/specs.md` and `DEV_MANUAL.md`'s instrument 3 lose the residual they state today in the same commit that removes it
touches:    src/search.cpp, src/transposition_table.cpp, tests/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md
excludes:   changing what is *printed* -- S170 settled the reporting side and its mechanisms are not this step's to revisit; making the table keep entries longer, resizing it or changing its replacement policy, each of which alters play and owes an SPRT of its own; improving mate *finding*, which is S148's and S154's ground
decisions:  DEC-122, DEC-123, DEC-125
closes:
blocks:
paused_by:
author:

## What this is

S170 removed three ways a true mate score could be reported with a line too
short to reach it, and one `Incomplete mating PV` line survived its 3000-game
run -- five `info` lines from a single search. That search's **score** is the
defect, not its line.

**The measurement, from S170's run, 2026-09-02.** Position
`8/4ppk1/2p2np1/p7/NPP1p3/P6q/3b4/1Q3R1K w - - 0 34`, reached 50 plies into a
game whose root and moves are the `E_mate_minus9` row of
`adocs/data/S170_cases.tsv`:

| asked | answer |
|---|---|
| the engine, in the game, at depths 9 to 13 | `mate -9`, 18 plies, with lines of 9 to 13 plies |
| the engine, cold table, depth 18 | `mate -7`, 14 plies, complete line |
| the engine, cold table, depths 19 to 24 | `mate -7`, held over 7.3 billion nodes |
| `stockfish`, depth 30 and depth 36 | `#-7` |

So the distance reported in the game is not the distance the position holds,
and no 18-ply line exists for S170's completion to publish. The all-or-nothing
rule (DEC-122) did the right thing by refusing; what is left is the score.

**Why it matters beyond the warning.** A mate score is not only printed. It is
stored, it answers later nodes through `tt_entry_answers()`, and a distance
that is wrong by two moves is a score that orders and cuts on a claim the
position does not support. Whether that costs anything in play is unmeasured
and is one of the things this step is for -- a verdict of zero is a valid
outcome and is recorded as zero (DEC-019, and S005, S006 and S015 are the
precedents).

## Where to look first, and what has already been ruled out

`normalize_score()` and `de_normalize_score()` (`src/search.cpp`) are the
distance's ply arithmetic and they round-trip by construction; the aspiration
loop in `iterative_deepening_search()` (`src/chesso.cpp`) was checked while
S170 was measured and is **not** the cause -- it re-searches with the full
window whenever a mate score falls outside the band and loops until the score
is inside it, so a non-aborted iteration's root score is not a bound.

That leaves the entry itself: which search wrote `mate -9` for that position,
at what depth, and whether it was exact or a bound when it was written. Three
candidates worth separating before choosing a fix:

- **A bound stored as one and read as exact.** `tt_entry_answers()` returns a
  `TT_BETA_NODE` or `TT_ALPHA_NODE` score to the caller when it passes the
  window test, which is ordinary alpha-beta; a mate *bound* carried up to the
  root and printed as an exact distance would look exactly like this.
- **A key collision.** `tt_get_entry()` compares the full 64-bit key and a
  64-bit key still collides; S147's `extend_mate_pv()` comment already names
  this as the reason a stored move is re-validated against the generator.
- **A mate proved on a reduced tree.** Late move reduction re-searches on a
  fail-high, so a mate that survives a reduction should be re-proved -- but
  "should" is an argument and this step does not run on arguments.

The first thing to build is the instrument that says which: a way to see the
entry that answered, its depth, its type and the search that wrote it, on the
reproduction above. `tools/truncation_scan.cpp` and `tools/probe_cost.cpp` are
the precedents for a tool that reads the engine's own tables.

## Why it is first in the Open list

The BUGS rule: a bug that has been found gets fixed before anything else
starts, because a known defect in the tree contaminates every measurement taken
after it. This one is found, reproducible in about twelve seconds, and it sits
in the score -- which is the input every later measurement in the search block
is taken against.
