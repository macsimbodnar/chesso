id:         S171
goal:       a reported mate line reaches its mate even where the table has lost a slot the walk needs, closing the 5 `Incomplete mating PV` lines from 1 search in 3000 games S170 left -- **rescoped by DEC-127**, the original goal having assumed a wrong mate distance that measurement refuted
accepts:    the case S170 measured is reproduced and its mechanism established from the code rather than argued -- **done, and it refuted the premise this step was created on (DEC-127)**: the reported `mate -9` is deliverable, an 18-ply line from that root is legal throughout and ends in checkmate, and the same warm replay at `Hash=256` prints it and warns about nothing, so what fails at `Hash=16` is the walk and not the score; a minimized failing test observed red before any fix, driving the engine the way `tests/test_mate_carry.cpp` does because a cold table does not reproduce it; the fix decided by SPRT if it alters play at all and by identical `tools/search_bench.py` node counts and best moves if it does not (INV-6); a `fastchess.sh --fast` run reports **0** `Incomplete mating PV` lines from the candidate, which is the residual S170 could not close and the reason this step exists; `MANUAL.md`, `adocs/specs.md` and `DEV_MANUAL.md`'s instrument 3 lose the residual they state today in the same commit that removes it
touches:    src/search.cpp, tools/, tests/, adocs/data/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md
excludes:   **amended by DEC-127** -- the reporting side is now this step's ground, because the residual is a line the walk could not build and not a score; anything that alters play, including making the table keep entries longer, resizing it or changing its replacement policy, each of which owes an SPRT of its own; improving mate *finding*, which is S148's and S154's ground, and which is where a search naming a longer mate than the position's own value belongs
decisions:  DEC-122, DEC-123, DEC-125, DEC-127
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-03

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

## What was measured, 2026-09-03

Everything above this line was written before the measurement. It assumed the
score was the defect. It is not, and DEC-127 is the correction.

**The instrument.** `tools/mate_trace.cpp`, built for this step: it replays a
game through the real UCI layer -- one process, one table -- then walks a
reported line over a board of its own and prints the entry behind every
position, following the table's own best move past the end of the line. At a
stall it prints every legal move and what the table holds one ply on. Nothing
it does searches a node or writes an entry.

**What it showed.** On the reproduction at `Hash=16`, `go depth 11`, node count
353576, identical to the in-game search:

| ply | entry |
|---|---|
| 0 to 9 | exact, generation 12 (this search), depths 11 down to 2, every one claiming the same root-relative score |
| 10 | **miss** -- covered by the reported `pv`, which is 11 plies long |
| 11 | exact, generation 11 (the previous search), depth 7, black mates in 4 |
| 12 | exact, generation 11, depth 6, best move `g2g4` |
| 13 | **miss** -- the walk stalls here, five plies from the mate |

The generation-11 entry at ply 11 is honest: `stockfish` at depth 30 gives
`Mate(+4)` for black on that position. And the claimed distance is deliverable:

```
h1g1 d2e3 f1f2 h3g3 g1h1 e3f2 b1f1 g3f3 f1g2 f3d1 h1h2
f6g4 h2h3 d1d3 h3g4 d3d7 g4f4 d7f5
```

18 plies, legal throughout, ending in checkmate -- replayed move by move with
python-chess. So `mate -9` is **sound and not optimal**; the position's own
value is `mate -7`, and DEC-125's "no line of the claimed length exists" was
never measured and is wrong.

**The confirmation that it is table pressure and nothing else.** The same warm
replay at three hash sizes, depths 8 to 13:

| Hash | what the engine reports |
|---|---|
| 16 MB | `mate -7` at 8 to 10, `mate -9` at 11 and 12 with an 11- and 12-ply line -- short, warns |
| 64 MB | `mate -7` at 8 only, centipawns from 9 on |
| 256 MB | `mate -9` at 9 to 12 with a **complete 18-ply line** -- no warning |

At 256 MB the engine publishes the line by itself. What differs is only which
entries survive.

**The fix.** `certified_mate_move()` in `src/search.cpp`. When the walk has
nothing to read -- neither the proven line nor an entry with a move -- it looks
one ply down and takes the move whose child carries an **exact** score at
exactly the distance the line still owes. A collision takes one slot at a time
and a mating line's nodes are scattered across the table, so the children of a
lost position usually still have theirs. Exact entries only: a lower bound of
"mate in n" leaves a faster mate open, and the all-or-nothing gate would catch a
walk that fails to reach the mate but not one that reaches it by a road the
position would not take.

**Red first, then green.** `adocs/data/S170_cases.tsv`'s `E_mate_minus9` row
becomes `guard: yes`. Without the fix `tests/test_mate_carry.cpp` reports
`6 of 9 mate lines do not reach their mate` -- the four `mate 8` lines at ply 49
at 5, 6, 7 and 9 plies of 15, and the two `mate -9` lines at ply 50 at 11 and 12
plies of 18. With it, 0 of 9, and the mate-line count is 9 either way, which is
what says the change is reporting-only. 7.9 s.
