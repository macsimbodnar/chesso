id:         S170
goal:       a mate score read back from the transposition table is reported with a line that reaches it, so the last ten `Incomplete mating PV` lines in a match log go too
accepts:    a `fastchess.sh --fast` run reports **0** `Incomplete mating PV` lines from the candidate, against the 10 in 3000 games S147 left and the 138 the engine produced before it; a test that replays a game move by move through one engine process -- which is the only way to give the transposition table the contents a game gives it, and is why `test_mate_pv` cannot see this -- asserts the property on the reproduction S147 recorded and on at least one more game; behaviour-neutral for play, proven the S145 way with identical node counts and best moves from `tools/search_bench.py`, since only what is printed changes; whatever line is published passes the same all-or-nothing gate DEC-122 sets -- legal from the current root, ending in checkmate at exactly the claimed distance -- so a wrong line is still impossible; `MANUAL.md`, `adocs/specs.md` and `DEV_MANUAL.md`'s instrument 3 lose the bound they state today in the same commit that removes it
touches:    src/search.cpp, src/chesso.cpp, tests/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md
excludes:   changing any search behaviour, any default or any score -- the distances are right and this step must not move them; making the transposition table keep entries longer, resizing it, or changing its replacement policy, any of which alters play and owes an SPRT of its own; `go mate N`, still a separate missing feature
decisions:  DEC-122, DEC-123
closes:
blocks:
paused_by:
author:

## What this is, and why it is not S147

S147 removed the truncation of a mate line the search had just proved: the line
ran below the deepest main-search ply, and walking the transposition table from
the end of it put the rest back. Measured at **54 short of 706 mate lines
before and 0 after**, over both S145 sets at depth 8.

What it could not reach is the other way a short mate line arises, and DEC-123
is the ruling that separates them. The engine reports `mate N` at an iteration
that never found the mate: the score comes out of the transposition table,
written by a search of an **earlier move in the same game**, and the entries
that carried the line have been overwritten since. A 16 MB table holds about a
million entries and one 150 ms search visits more nodes than that, so a
previous move's line is gone while the single slot carrying its score survives.

**The reproduction, from S147.** Replay the game move by move through one
engine process at `go movetime 150`, root
`r2qk1nr/pp1bbppp/2npp3/2p5/8/4PP1P/PPPPB1PN/RNBQ1RK1 b kq - 0 7`, and the
position after 71 plies reports `mate -8` from depth 3 with a 3-ply line where
16 are needed. The same position at the same time after `ucinewgame` reports no
mate at any depth -- `cp 113` at depth 1 down to `cp -725` at depth 15, best
move `g2g3`. That difference is the whole mechanism.

## The measured size of it

One clean `--fast` run, 3000 games at 8+0.08, 1 h 55 m 30 s, 0 forfeits:

| engine | `Incomplete mating PV` |
|---|---|
| `ref-8736aec`, before S147 | 138 |
| after S147 | **10** |

The ten, every one with a line exactly as long as its iteration is deep:

| mate | iteration depth | nodes | pv plies | needed |
|---|---|---|---|---|
| 8 | 1, 2, 3, 4 | 39 to 243 | 1, 2, 3, 4 | 15 |
| 6 | 1, 2, 3, 5 | 48 to 5730 | 1, 2, 3, 5 | 11 |
| 7 | 11 | 326882 | 11 | 13 |
| -6 | 10 | 1158874 | 10 | 12 |

Eight are shallow iterations of a search that inherited the score. Two are deep
searches two plies short, which is a different shape inside the same family and
may have a different cause; establish it before assuming.

## The leading candidate, and what it must not become

**Carry the line, not the search.** When a search proves a mate it also holds
the line that proves it. Store that line with the position it was proved from;
when a later search reports the same mate and the table walk stalls, take the
remainder of the stored line after the moves actually played. It is reporting
state and not search state, and it publishes nothing the all-or-nothing gate
would not accept: legal from the current root, ending in checkmate at exactly
the claimed distance.

Two things it must not do. It must not keep the engine from playing what it
would otherwise play -- the stored line is printed, never followed. And it must
not become a reason to hold transposition table entries longer, which alters
play and belongs to a step with an SPRT.

Priced and rejected in S147 for this step to reconsider only with evidence: a
bounded mate search in the reporting path closes the two deep cases for a move
list and a mate test, and the eight shallow ones only with a mate-in-8 solver
running inside a time-controlled search.
