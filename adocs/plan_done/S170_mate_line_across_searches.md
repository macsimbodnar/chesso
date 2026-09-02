id:         S170
goal:       a mate score read back from the transposition table is reported with a line that reaches it, so the last ten `Incomplete mating PV` lines in a match log go too
accepts:    a `fastchess.sh --fast` run reports **0** `Incomplete mating PV` lines from the candidate, against the 10 in 3000 games S147 left and the 138 the engine produced before it -- **amended 2026-09-02 by DEC-125**, the run having found that the guarantee a reporting step can carry is over a mate score whose distance is the position's own value: 12 lines over 3 searches from the reference against 5 over 1 from this build, and that one search reports `mate -9` where the engine's own cold search says `mate -7` at depth 18 and holds it to 24 and `stockfish` says `#-7` at depth 30 and 36, so no line of the claimed length exists to publish and the residual is carried to S171; a test that replays a game move by move through one engine process -- which is the only way to give the transposition table the contents a game gives it, and is why `test_mate_pv` cannot see this -- asserts the property on the reproduction S147 recorded and on at least one more game; behaviour-neutral for play, proven the S145 way with identical node counts and best moves from `tools/search_bench.py`, since only what is printed changes; whatever line is published passes the same all-or-nothing gate DEC-122 sets -- legal from the current root, ending in checkmate at exactly the claimed distance -- so a wrong line is still impossible; `MANUAL.md`, `adocs/specs.md` and `DEV_MANUAL.md`'s instrument 3 lose the bound they state today in the same commit that removes it
touches:    src/search.cpp, src/chesso.cpp, tests/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md
excludes:   changing any search behaviour, any default or any score -- the distances are right and this step must not move them; making the transposition table keep entries longer, resizing it, or changing its replacement policy, any of which alters play and owes an SPRT of its own; `go mate N`, still a separate missing feature
decisions:  DEC-122, DEC-123, DEC-124, DEC-125
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-02
done:       2026-09-02. A mate score the search did not itself prove is reported with a line that reaches it. **Three causes were measured, where the step file assumed one**, and the reproduction is tracked: `adocs/data/S170_replay.py` replays a game move by move through one engine process at fixed node budgets and `adocs/data/S170_cases.tsv` is the four games S147's run produced, each with the cheapest budget and first ply that reproduce it. A score inherited across searches (the same position on a cold table reports no mate at all); a proof this search made and then overwrote (a cold table still reports it, short); and a score printed beside a line from an aborted iteration. One fix each, all reporting-only and all under DEC-122: `proven_mate_line_t` keeps the last delivered line together with the position each of its moves is played from and is asked by key **and** by remaining distance; two plies from the mate the defender's move is looked for, and taken only when every legal reply is mated in one; and `complete_mate_pv()` is called on the line about to be printed, against the score printed beside it. **Each part isolated by measurement**: the store alone took 6 short lines to 4, the two-ply completion closed case D, and the printed-score completion closed exactly the three duplicated-depth lines of A, B and C. `tests/test_mate_carry.cpp` is the guard, observed red at 6 short lines first and green at 0, 6.6 s -- the first test here that replays whole games through one process, and it asserts a mate line was seen before asserting none is short. INV-6 discharged: 121512 / 800769 / 62907 at depth 9 and 639228 / 3430710 / 367858 at depth 12, `c3d5` / `e2a6` / `d7c8q` at both, identical to `b3f82eb`. Gate green in both builds, 24 tests, `./clang-format.sh --check` clean. One clean `fastchess.sh --fast`, 3000 games in 1 h 56 m 48 s with 0 forfeits: **12** `Incomplete mating PV` lines over 3 distinct searches from `ref-b3f82eb` and **5** over **1** from this build. **The accepts asked for 0 and was amended in place by DEC-125**: the five are one search whose mate *distance* is wrong -- `mate -9` in the game where the same position on a cold table gives `mate -7` at depth 18 and holds it to depth 24, and `stockfish` says `#-7` at depth 30 and again at 36 -- so no line of the claimed length exists and refusing to publish one is DEC-122 working. That residual is **S171**, first in the Open list under the BUGS rule. The case is kept in `S170_cases.tsv` with `guard: no`, because a test asserting a line for it would be asserting that a wrong score gets one. Deviations from `touches:`, recorded rather than silent: `src/data_structures.hpp` and `src/search.hpp` were also touched, the first for `proven_mate_line_t` and `search_state_t::proven_mate` and the second to declare `complete_mate_pv()`, which the reporting layer needs and `extend_mate_pv()` was static for; `adocs/data/` gained the reproduction. Docs: `MANUAL.md`'s `depth` row now says score and line can come from different iterations and its known-bugs list carries the residual, `adocs/specs.md` and `DEV_MANUAL.md`'s instrument 3 carry the guarantee and the standing figure, `adocs/data/README.md` indexes both new files, gate timings updated to 24 tests. `README.md` checked, human-owned, no change.

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


## What was measured, 2026-09-02

The four games that produced the ten warnings were reproduced first, and the
reproduction is kept: `adocs/data/S170_cases.tsv` is their root FEN and move
list, taken verbatim from the `Position;` and `Moves;` lines fastchess printed
beside each warning, and `adocs/data/S170_replay.py` replays a game move by
move through one engine process. The `go` budget is nodes and not movetime, so
a case is a property of the tree and not of how busy the machine is, and each
row carries the cheapest budget and first ply that still reproduces it.

**There are three causes, not one, and the step file above assumed one.**

| cause | how it was told apart | cases |
|---|---|---|
| a score inherited across searches | the same position **cold** reports no mate at any depth | A, B, C |
| a proof this search made and then overwrote | the same position **cold** still reports the mate, short | D |
| a score and a line from different iterations | the depth is printed twice, the second line from an aborted iteration | A, B, C |

Cause (a) is what DEC-123 described. Case C is its clearest instance: warm it
reports `mate 7` at depth 11 with 11 plies of 13, and cold at the same node
budget it reports no mate at all, reaching depth 13 at `cp 885`.

Cause (b) is not inheritance. Case D reports `mate -6` at depth 10 with 10
plies of 12 on a table that `ucinewgame` has just cleared, so the proof is this
search's own and the entry that carried the eleventh ply was evicted between
the iteration that wrote it and the walk that wanted it. Checked with
python-chess: after those 10 plies the position is check with one legal reply
and that reply is mated in one, so exactly two plies are missing and both are
findable.

Cause (c) is new and is not a table effect. `src/chesso.cpp` prints the last
*completed* iteration's score beside the line of the last iteration that
produced one, which may be an aborted iteration -- deliberately, so the line
shown starts with the move that will be played. When that score is a mate the
pair can claim a distance the line does not reach. Seen directly in the
verbose replay: case C at ply 74 prints `depth 11` twice, `mate 7 pv 13/13`
and then `mate 7 pv 12/13`.

## What was built

Three changes, one per cause, all reporting-only and all under DEC-122.

**The store.** `proven_mate_line_t` (`src/data_structures.hpp`) keeps the last
line the engine was shown to deliver, and `keys[i]` is the position `moves[i]`
is played from. `proven_mate_move()` (`src/search.cpp`) answers only for a
position that is on the line *at the distance still owed*: the index is
`length - remaining`, so the lookup is one comparison rather than a scan and a
line proving another distance cannot answer. The instance is the UCI layer's
because it has to outlive a `go`; `search_state_t::proven_mate` is null unless
a caller sets it, which leaves every test that drives `search()` directly on
the behaviour it had.

**The two-ply completion.** Where the walk stalls with two plies to go and
neither the table nor the store answers, the defender's move is looked for --
and taken only when *every* legal reply is mated in one, which is what the
claimed distance asserts about the position. S147 priced this and rejected it;
case D is the evidence that reopened it, and the forcedness requirement is what
stops it publishing a defender's blunder as a principal variation.

**The completion against the printed score.** `complete_mate_pv()` is no longer
static: `src/chesso.cpp` calls it on the line it is about to print whenever
that line came from an aborted iteration and the score beside it is a mate.

## Verification

- **Red observed first.** `tests/test_mate_carry.cpp` written before the fix,
  6 short lines over the four cases, 6.48 s. After: 0, 6.62 s.
- **Each part isolated.** The store alone took 6 to 4. With the store and the
  two-ply completion in and the printed-score completion out, exactly the three
  duplicated-depth lines of A, B and C come back and D does not -- which is the
  measurement behind the three-cause table above.
- **INV-6 discharged.** `tools/search_bench.py` at depth 9 gives
  121512 / 800769 / 62907 and at depth 12 gives 639228 / 3430710 / 367858, best
  moves `c3d5` / `e2a6` / `d7c8q` at both, identical to the figures
  `.moltke.local.md` records for this machine. No SPRT is owed for strength and
  none is claimed; the run below is for the mate check.
- **Both builds green**, 24 tests: `build` 51.5 s and `build-tune` 51.6 s
  re-measured after the run, 54.9 s and 55.8 s during it -- the label total
  moves a second or two between runs and is read as a size, not a stopwatch.
  `./clang-format.sh --check` clean, and the whole gate is 107.9 s end to end
  on a warm tree. The suite was 23 before; `test_mate_carry` is 6.6 s of it.

## The precondition the test asserts before the property

What a transposition table holds at a given ply is fragile, and every case here
is a reproduction of an accident. A change that moves the tree can make a case
stop reporting a mate at all, at which point "no short line" is true and means
nothing. So `test_mate_carry` asserts first that each case still reported at
least as many mate lines as it did when it was chosen -- 5, 7, 6 and 1 -- and
the failure message says the case has gone vacuous and needs re-choosing rather
than deleting.


## The run, and it does not meet the accepts

`./fastchess.sh --fast` against `b3f82eb`, which is S147's build: **3000 games
in 1 h 56 m 48 s, 0 time forfeits**, `LLR -0.73` at 3000 and no bound, which is
what two engines that play identically produce. INV-6 was already discharged on
node counts and nothing here adds to it; the run is for the mate check.

| engine | `Incomplete mating PV` lines | distinct searches |
|---|---|---|
| `ref-b3f82eb`, S147's build | **12** | 3 |
| candidate, this step | **5** | **1** |

The accepts asked for 0. **The five are one search, in one game**, reported at
depths 9, 10, 11, 12 and 13 with lines of 9, 10, 11, 12 and 13 plies where 18
are needed -- the signature of a score with no proof behind it.

**And the score is what is wrong there, not the line.** Position
`8/4ppk1/2p2np1/p7/NPP1p3/P6q/3b4/1Q3R1K w - - 0 34`, reached after 50 plies of
that game. The engine reports `mate -9`, which is 18 plies. Asked the same
position on a **cold** table, the engine itself reports no mate at all until
depth 18, where it reports **`mate -7`** -- 14 plies, with a complete line --
and holds it to depth 24 over 7.3 billion nodes. `stockfish` agrees: `#-7` at
depth 30 and again at depth 36. So the distance the game reported is not the
distance the position holds, and there is no 18-ply line anywhere to be found:
the completion refused, which is what DEC-122 asks it to do.

The case is reproducible without a match:

```
python3 adocs/data/S170_replay.py --cases adocs/data/S170_cases.tsv \
        --only E_mate_minus9 --go "nodes 1500000" --start-override 40
```

**This is a mate score that is wrong, and every mechanism this step built is a
reporting mechanism.** The step's `excludes` forbids changing any score, so the
residual is not S170's to close, exactly as S147's residual was not S147's.
