id:         S207
goal:       a position that repeats one occurrence from before or at the root is not scored as a draw; a repetition strictly inside the tree, or a third occurrence anywhere, still is
accepts:    `src/search.cpp` `negamax_at` scores `DRAW_SCORE` for a repetition only when the matching earlier position lies **strictly after the root** -- a history entry above the index the search started from, the entry at that index being the root position itself -- or when the position has occurred twice before, wherever those occurrences lie (the published form: once strictly after the root, or twice before or at it); the F08 reproduction (the same board searched with and without the history `g1f3 g8f6 f3g1`, through `python-chess`'s `chess.engine` and never a bare pipe) returns the same class of score with and without the history, where today it returns `cp 0` with it; a red-first case in `tests/test_search.cpp` pins that a single pre-root occurrence is not a draw, observed red on the unfixed tree; the case "the losing side takes an available repetition" is **re-stated, not deleted or relaxed** -- its precondition becomes a position that has already occurred twice before the root, so the property it asserts (a draw score beats a lost position) is unchanged and its old precondition becomes the new red-first case's input, and DEC-173 is the recorded decision that lets an agent touch it; the two `tests/test_engine.cpp` cases over `is_position_repeated` stay green or are re-stated with the reason written at the site; the rule is stated in `adocs/specs.md`'s search row and the convention it replaces is named; **one SPRT at `--nonreg`**, pre-registered in `adocs/data/S207_sprt.sh` in the `adocs/data/S165_sprt.sh` shape with its three readings and DEC-143's worst-case games before the first game, recorded whatever it returns -- a zero is kept with the reason stated, since the change removes a wrong draw score from ordinary play and the SPRT is the rule and not the motive; Debug self-play, four rounds at 4+0.04 grepped for `Assertion`, before completion (DEC-141); the commit touches `src/` and alters the tree, so its message carries `Bench: <nodes>` (DEC-140)
touches:    src/search.cpp, src/bitboard.cpp, src/bitboard.hpp, src/data_structures.hpp, src/chesso.cpp, tests/test_search.cpp, tests/test_engine.cpp, adocs/specs.md, adocs/data/
excludes:   draw detection inside quiescence, which probes with no draw checks by the published practice S106 recorded as accepted; the fifty-move and insufficient-material tests, unchanged; `tools/analyse_game.py`, which reads the score this step corrects and needs no change of its own (its silent-zero defect is S214)
decisions:  DEC-170, DEC-173
closes:     2026-09-10_adversarial-F08
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-10_adversarial-F08`, high. `src/bitboard.cpp` `is_position_repeated`
scans the whole history window and returns true on the **first** hash match,
and its one caller in `src/search.cpp` `negamax_at` scores `DRAW_SCORE` on it
at every ply above the root. Nothing distinguishes a match inside the tree
from one before the root. Driven through `python-chess`'s `chess.engine` at
`go depth 10`, the same board reads `score 0, nodes 4249, pv f6g8` with the
history `g1f3 g8f6 f3g1` and `score -900, nodes 325965` without it; python-chess
on the position after `f6g8` says `is_repetition(3) False`,
`can_claim_threefold_repetition() False`, and stockfish at depth 18 with the
identical board and history scores it `-687`. No draw exists. The tree behind
the false draw collapses by a factor of 77.

It is the one high finding that fires in **ordinary play**: fastchess sends
every game as `position startpos moves ...`, so the pre-root history is the
game, and a shuffle that returns to an earlier position is scored `cp 0` by an
engine that is losing by several pawns -- and the engine chooses its moves on
that score. `tools/analyse_game.py`, the tool `CLAUDE.md` mandates because agent
chess judgement is banned, reads those scores.

**The suite pins the wrong convention.** `tests/test_search.cpp` "the losing
side takes an available repetition" plays `Ra2b2 Kh8h7 Rb2a2` from
`7k/8/8/8/8/8/R7/K7 w - - 0 1` and requires a draw score for `Kh7h8`, which
recreates the position the game started from -- **one** earlier occurrence,
before the root. python-chess on that position after `Kh7h8`:
`is_repetition(3) False`, `outcome(claim_draw=True) None`. The comment at the
case states the mechanism as intended. No decision recorded the convention.

## The rule, as published

CPW *Repetitions*: a repetition detected inside the search tree is scored as
a draw at its first recurrence, because the side that repeated could repeat
again and the search cannot see past the horizon to a third; a position from
the game history before the root is a different matter, because the side to
move at the root gets to choose again and the opponent cannot force the third
occurrence alone. Stockfish PR #925 (2017) is the measured form and the one
whose own example is a losing move preferred over a drawing one: **a draw is
returned if the position repeats once earlier but strictly after the root, or
repeats twice before or at the root.** Read as prose from the pull request and
the wiki; no source was opened (DEC-016).

Two things follow for the implementer. The old convention is not "wrong chess"
-- older engines used it -- it is a **weaker** convention with a measured Elo
cost in the one engine that A/B'd it, and it is the one this engine's own
oracle contradicts on the reproduction above. And the draw the new rule still
scores on a **third** occurrence is exactly what FIDE 9.2 lets a player claim,
so the re-stated test's precondition is the rule's own.

## Shape for chesso

- `history_t` (`src/data_structures.hpp` `history_t`) keeps one entry per
  ply played, the key of the position the move was played from in
  `history_entry_t`. The root's history size at the moment `go` is received is
  the boundary: the entry at index `root_size` is the root position itself,
  written when the search made its first move, and an entry above it was
  pushed by the search; so a match strictly inside the tree is a match at an
  index above `root_size`, and a match at `root_size` or below is one
  occurrence from before or at the root.
- Plumb `size_t root_history_size` into `search_state_t`
  (`src/data_structures.hpp` `search_state_t`), set where the state is built
  for a `go` in `src/chesso.cpp` `iterative_deepening_search` and, for every
  test that builds its own state and calls `src/search.cpp` `search` directly,
  defaulted so that a zero-initialised state means "the whole history is
  pre-root" -- which is the conservative reading and keeps the direct-call
  tests on the new rule without editing each one. State which default was
  chosen and why at the field.
- `src/bitboard.cpp` `is_position_repeated` either grows a parameter for the
  root boundary and returns the class (none / one pre-root / in-tree or
  third), or a second function does and the old one keeps its two-fold contract
  for `tests/test_engine.cpp`'s two direct cases. Either way one function
  holds the walk: the loop steps by two inside `min(halfmove_clock,
  history.size)` as today, and stops at the second match or the first in-tree
  one.
- `src/search.cpp` `negamax_at` keeps its order: the repetition test stays
  above the transposition-table probe, because a draw is a property of the
  path and the key does not carry it (the comment there says so).

## Tests, red first

1. **New**: from the F08 shape -- a knight retreat that recreates a position
   occurring once before the root -- `search()` at a fixed depth returns a
   score that is not `DRAW_SCORE` and a best move that is not the repeating
   one where the position is lost for the side to move by the engine's own
   evaluation (the precondition asserted the S192 way, against zero and not
   against a golden). Observed red on the unfixed tree.
2. **Re-stated**: "the losing side takes an available repetition" plays the
   shuffle twice -- `Ra2b2 Kh8h7 Rb2a2 Kh7h8 Ra2b2 Kh8h7 Rb2a2` -- so `Kh7h8`
   is the **third** occurrence, and the assertion (`score == 0`, best move
   `h7h8`) is unchanged. The `GOLDEN (DEC-142)` note beside its `-569`
   precondition stays and is re-derived by `adocs/data/S192_anchors.py` if the
   evaluation changes, which this step does not touch.
3. **In-tree**: a repetition whose first occurrence is inside the tree -- the
   S170-style fixed-depth search from a position where a perpetual is
   available -- still scores the draw; observed on the unfixed tree (green
   before and after) so it is the control and not a red-first case.
4. `tests/test_engine.cpp` "a shuffled knight repeats" and "an irreversible
   move clears the window" drive the primitive, not the search; they stay as
   they are if the primitive keeps its contract.

## Measurement

Play-altering by construction: node counts move wherever a pre-root two-fold
was reached. `tools/search_bench.py` is read before and after and its counts
recorded, but INV-6's neutral half is not available and the SPRT decides.
`--nonreg`, `elo0=-5 elo1=0`, `alpha=beta=0.05`, at the S105 regime (8+0.08,
Hash 16, UHO, all 12 threads), against HEAD. Worst-case games from the nElo
formula: 41861 at the midpoint, 18.4 h at 2277 games an hour; 25591 on a bound,
11.2 h; if the record's small positive holds, far less. Abort rule: forfeit
rate over 1.0 % on a side (`tools/forfeit_report.py`). Readings:

- **H1**: not a regression of 5 nElo. Kept; the magnitude is not established
  and no gain is claimed.
- **H0**: it costs 5 nElo or more. Before believing it, count how often the
  changed class fires in the run's own PGN -- a pre-root two-fold reached
  inside a search -- because a rule that fires rarely cannot cost that much,
  and a wrong count is likelier than a wrong oracle. Then a decision.
- **No verdict at the cap**: recorded as zero and **kept**, with the reason
  stated: the oracle says the old score is wrong on a position in ordinary
  play, and S005, S006, S015 and DEC-103 are the precedent for keeping a
  measured zero.

The night rule (DEC-155): the estimate is 4 to 18 hours, so the run is a night
run, and it is the run the coordinator launches on 2026-09-11's night
(DEC-172). Launch detached with the WATCHERS loop through `Monitor`, ceiling
2x the worst case.

## Sources read

- https://www.chessprogramming.org/Repetitions -- the tree-versus-history
  distinction, and the two-fold-in-tree convention.
- https://github.com/official-stockfish/Stockfish/pull/925 -- the measured
  refinement and its example; commit prose only.
- `adocs/audit/2026-09-10_adversarial.md` F08 and its Part F note 3 -- the
  reproduction, and the bare-pipe trap the coordinator fell into re-taking it.
- `src/bitboard.cpp` `is_position_repeated`, `src/search.cpp` `negamax_at`,
  `tests/test_search.cpp` "the losing side takes an available repetition",
  `tests/test_engine.cpp` "a shuffled knight repeats".
