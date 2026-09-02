id:         S147
goal:       a mate score is reported with a principal variation long enough to reach the mate it claims, so fastchess's -check-mate-pvs stops warning on a truncation
accepts:    an `info` line reporting `score mate N` carries a `pv` of at least `2N - 1` plies for a mate the engine delivers and `2|N|` plies for one it receives, and the last position of that line is checkmate; asserted over the constructed set in `adocs/data/S145_mate_set.tsv` and over the mined set in `adocs/data/S145_mined_set.tsv`, at every iteration from the first that reports a mate rather than at the last one only, because the truncation measured is a shallow-iteration effect; a run of `fastchess.sh --fast` produces no `Incomplete mating PV` line, which is the check `-check-mate-pvs` already performs on every SPRT since S145; behaviour-neutral for play, proven the S145 way -- identical node counts and identical best moves from `tools/search_bench.py`, since only what is printed changes; `tests/test_uci_surface.cpp` refreshed after MANUAL.md describes the new output; the known-bug entry S145 added to MANUAL.md removed in the same commit that removes the bug
touches:    src/chesso.cpp, src/search.cpp, tests/test_engine.cpp, tests/test_uci_surface.cpp, MANUAL.md
excludes:   changing any search behaviour, any default or any score -- the distances are already right and this step must not move them; extending the principal variation through quiescence as a search feature rather than as a reporting one, if the measurement says a table walk is enough; `go mate N`, which is a separate missing feature listed in MANUAL.md
decisions:  DEC-061
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-02

## What was measured, S145, 2026-08-21

Found by turning on `-check-mate-pvs` in `fastchess.sh`, which fired on the
first four-game smoke match. Then measured properly, over 400 late-game
positions drawn from `.spsa/S085/games.pgn` and driven through iterative
deepening to depth 8, with `stockfish` at 300000 nodes as the truth:

| what | count |
|---|---|
| positions where chesso reported a mate at all | 8 |
| `info` lines carrying a mate score | 31 |
| lines whose `pv` is shorter than the distance claimed | **2** |
| lines whose distance disagrees with stockfish | **0** |

**So the score is right and the line is short.** That is the whole finding, and
it is why this is a reporting step and not a search step. Both short lines are
the same shape: `1Q6/pp3pk1/5b2/2P3p1/8/P5Pb/2p2P1P/5NK1 b - - 0 35` reports
`mate 3` at depth 4 with a 4-ply line where 5 are needed, and its successor
reports `mate -3` at depth 5 with 5 plies where 6 are needed. In both the
principal variation is exactly as long as the iteration is deep.

**The mechanism follows from that.** `state->pv_table` is filled from the main
search, so it can hold at most one entry per main-search ply. A mate found
inside quiescence -- which happens, and `tests/test_search.cpp`'s "mate is
recognised at depth zero" is the case that pins it -- puts a correct mate score
at a node whose line continues below the deepest main-search ply, and the line
stops where the table stops.

**What it costs today: nothing in play, and one class of noise in every SPRT
log.** The move played is the mating move and the distance is right, so no
measurement is contaminated and this did not jump the queue. What it does is
put `Warning; Incomplete mating PV` lines into the output of every match run
since S145, at a rate of roughly 2 in 31 mate reports, and a warning that is
always there is a warning nobody reads.

## Why it is worth a step rather than an accepted limitation

Because the check is free and it is now armed. `-check-mate-pvs` runs on every
SPRT and it is the only mate check that sees the positions the engine actually
meets -- tens of thousands a night against the 32 a constructed set can hold.
Its value is entirely in the signal-to-noise: with a known constant warning it
reports nothing, and with the truncation fixed the next `Incomplete mating PV`
line is a real defect the moment it appears. That is the same argument S106 made
for clearing the bound-sign findings before trusting the bound assertions.

Two candidate fixes, and the step is to measure which is enough rather than to
assume. Walking the transposition table from the end of the stored line until
the position is checkmate is what most engines do and touches nothing in the
search. Extending the line out of quiescence is the complete answer and is more
work. The excludes above allow the first and do not require the second.

## What was built, 2026-09-02

**The truncation reproduced first, from the step file's own position.**
`1Q6/pp3pk1/5b2/2P3p1/8/P5Pb/2p2P1P/5NK1 b - - 0 35` at `go depth 8`:

```
info score mate 3 ... depth 4 ... pv c2c1q b8f8 g7f8 g1h1        4 plies, 5 needed
info score mate 3 ... depth 5 ... pv c2c1q b8f8 g7f8 g1h1 c1f1   5 plies, correct
```

The line is exactly as long as the iteration is deep, which is what S145 said
it was.

**The measurement that decided the shape of the fix.** `tests/test_mate_pv.cpp`
was written first and observed red, over both sets at depth 8, reading **every**
`info` line that carries a mate score and not the last one only:

| set | mate lines | short of the mate |
|---|---|---|
| `S145_mate_set.tsv`, constructed | 182 | **0** |
| `S145_mined_set.tsv`, mined | 524 | **54** |

The constructed set never trips it: its mates are proved at distances two and
three and are reported at iterations deep enough to hold them. Every one of the
54 is the mined set, and every one is the same shape — a line whose length
equals the iteration depth.

**The table walk is enough, and the last ply is not.** `extend_mate_pv()`
(`src/search.cpp:1139`) replays the stored line, then follows the move each
position's table entry recorded until the position has no legal reply. That
alone took 54 to **1**. The one that survived is
`k2r4/Pq4p1/4p3/Q1ppP2r/3P3p/1R3b1P/5BP1/1R4K1 w - - 1 37` at depth 4, and
instrumenting the walk said why: at depth 3 the walk built the whole five-ply
line out of the table from a one-move principal variation, and at depth 4 the
entry for the position after four plies was **gone** — `tt_get_entry` returned
`nullptr` for a slot that the same iteration had written. A direct-mapped table
evicts, and the mating ply is written from quiescence at `TT_DEPTH_QS`, which
loses every collision.

So the last ply is looked for rather than read: when one ply is missing and the
table offers nothing, the walk scans the legal moves for one that leaves the
opponent with no legal reply while in check. That is the generator deciding
checkmate, the same way every other site in the tree decides it, and it is
bounded by one move list. **54 to 0** over both sets.

**All or nothing is the safety argument.** The walk builds its moves aside and
the principal variation is extended only when it ends in checkmate at exactly
the claimed distance. An evicted entry, a bound node with no move, or a walk
that wanders off the line leaves the reported line exactly as the search
produced it — short, and visible to the same two checks that found this. It
cannot publish a line that does not deliver the mate it claims.

**Reporting only.** Nothing in it searches a node, counts one, or writes to the
table; it runs once per `search()` return and only when the score is a mate.

## Verification

- **INV-6 discharged.** `tools/search_bench.py` at depth 9 gives
  121512 / 800769 / 62907 and at depth 12 gives 639228 / 3430710 / 367858,
  best moves `c3d5` / `e2a6` / `d7c8q` at both — identical to the figures
  `.moltke.local.md` records for this machine and to `8736aec`. No SPRT is owed
  and none is claimed.
- **Both builds green.** `build` 23 tests in 44.6 s, `build-tune` 23 in 48.8 s,
  `./clang-format.sh --check` clean. The suite was 22 tests before; `test_mate_pv`
  is 3.1 s of the new figure.
- **`test_mate_pv` is the guard.** Its own binary and in the fast label, for the
  reason `test_mate_breadth` is: 400 positions at depth 8 is 3.1 s in Release and
  a cost that size is reported rather than absorbed into another test's line.

## One thing changed beyond the `info` line

`bestmove ... ponder X` is printed when the principal variation has more than
one move (`src/chesso.cpp:821`), and it reads the same line this step extends.
A mate whose line was one move long now carries a ponder move where it carried
none — the opponent's reply on the mating line, legal and from the same walk.
It alters no play: `go ponder` is ignored and no match runs with pondering on.
`test_uci_surface` is unmoved by it, the surface being the command and option
set rather than the search output.

## The first smoke run was contaminated, and it is recorded rather than used

The first `./fastchess.sh --fast` was launched and then the working tree was
rebuilt while it played: `fastchess.sh:145` points the candidate at
`build/src/chesso` itself, so the binary under test was replaced at game ~50 of
3000. The run was killed and relaunched from the final tree. DEC-020 is the
class.

Its log is kept as `.tuning/S147_fast_void.log` for one reading that does not
depend on the swap, both binaries having been fixed builds by then: over the 60
games it played, `-check-mate-pvs` reported **16 `Incomplete mating PV` lines
and every one of them was `- from ref-8736aec`**, none from the candidate. The
reference is the unfixed engine and the check names which engine tripped it, so
the same log carries the defect present and absent side by side. It is
supporting evidence and not the accepts: the clean run is.

## The clean run does not meet the accepts, and here is what is left

`./fastchess.sh --fast` against `8736aec`, 3000 games in 1 h 55 m 30 s, 0 time
forfeits, `LLR 0.95` at 3000 -- no bound, which is what two engines that play
identically produce and not a strength reading; INV-6 was already discharged on
node counts and nothing here adds to it.

What the run was for is the mate check, and it reports:

| engine | `Incomplete mating PV` lines |
|---|---|
| `ref-8736aec`, unfixed | **138** |
| candidate, this step | **10** |

Same 3000 games, same book, same opponent. **92.8 % of them are gone and the
accepts asked for all of them.** The ten are:

| mate | iteration depth | nodes | pv plies | needed |
|---|---|---|---|---|
| 8 | 1, 2, 3, 4 | 39 to 243 | 1, 2, 3, 4 | 15 |
| 6 | 1, 2, 3, 5 | 48 to 5730 | 1, 2, 3, 5 | 11 |
| 7 | 11 | 326882 | 11 | 13 |
| -6 | 10 | 1158874 | 10 | 12 |

Every one has a line exactly as long as its iteration is deep, which is the
same signature as the defect this step removed -- and it is a different cause.

**Measured, not argued.** The first of them was reproduced by replaying its game
move by move through one engine process at `go movetime 150`, which is what
gives the transposition table the contents a game gives it: the position after
71 plies reports `mate -8` from depth 3 with a 3-ply line, three lines short of
the 16 it needs. The same position and the same time with a **cold** table --
`ucinewgame` first -- reports no mate at any depth: `cp 113` at depth 1 down to
`cp -725` at depth 15, best move `g2g3`.

So the score is **inherited from the table**, written by a search of an earlier
move in the same game, and by the time it is read back the line that proved it
has been overwritten. A 16 MB table is about a million entries and a 150 ms
search visits more nodes than that, so a previous move's line is gone while the
one slot carrying its score survives. The walk cannot invent plies the table no
longer holds, and all-or-nothing correctly refuses to publish a partial line.

**This is not the truncation S147 was written against.** That one was a mate the
search had just proved, whose line ran below the deepest main-search ply; it is
fixed, and `test_mate_pv` holds it at zero over 706 mate lines. This one is a
mate score with no proof left anywhere in the engine, reported at an iteration
that did not find it. No table walk closes it, and neither would extending the
line through quiescence -- the step file's other candidate -- because the plies
are not below the search, they are not in the engine at all.

`test_mate_pv` does not see it because every case there is `ucinewgame` and one
search, which is a cold table by construction. The reproduction above is what a
test for this would have to do.

**It needs a decision and the step stops here.** The accepts says no
`Incomplete mating PV` line, and that is not met.

## Four options, for the owner

The agent's recommendation is **A**, with **B** raised as its own step if zero
is what is wanted.

**A. Amend the accepts to what a reporting-only fix can carry, and close.** The
guarantee becomes: a mate line the search itself produced reaches its mate,
asserted at zero by `test_mate_pv` over 706 lines; a mate score inherited from
the table at an iteration too shallow to hold its line is a named residual,
counted at 10 in 3000 games against the reference's 138 and re-counted whenever
a run is taken. The residual becomes a new step. Cheapest, honest, and it stops
`-check-mate-pvs` being a warning nobody reads -- 10 in 3000 games is a number
that moves when something breaks, where 138 was not.

**B. Carry the mating line across searches, as its own step.** When a search
proves a mate it also holds the line; store it with the position it was proved
from, and when a later search reports the same mate and the walk stalls, take
the remainder of the stored line after the moves actually played. Validated by
the same all-or-nothing gate -- legal from the current root, ending in
checkmate at exactly the claimed distance -- so it cannot publish a wrong line
either. This is the option that reaches zero. It is new engine state, it is
reporting state and not search state, and it is not what this step's `excludes`
scoped.

**C. Do not report a mate score whose line cannot be shown.** Report the last
non-mate score instead. Rejected on its face: the score is right and the line is
not, so this throws away the true half. It also changes what is printed, which
this step's `excludes` forbids.

**D. Complete the line with a bounded mate search in the reporting path.** For
the two deep cases -- 11 plies of 13, 10 of 12 -- a two-ply completion is a
move list and a mate test, and it would close them. For the eight shallow ones
it is a mate-in-8 solver run inside a time-controlled search. Rejected as
priced: it buys 2 of 10 at a cost that is small, and the other 8 at a cost that
is not.
