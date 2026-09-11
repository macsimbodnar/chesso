id:         S207
goal:       a position that repeats one occurrence from before or at the root is not scored as a draw; a repetition strictly inside the tree, or a third occurrence anywhere, still is
accepts:    `src/search.cpp` `negamax_at` scores `DRAW_SCORE` for a repetition only when the matching earlier position lies **strictly after the root** -- a history entry above the index the search started from, the entry at that index being the root position itself -- or when the position has occurred twice before, wherever those occurrences lie (the published form: once strictly after the root, or twice before or at it); the F08 reproduction (the same board searched with and without the history `g1f3 g8f6 f3g1`, through `python-chess`'s `chess.engine` and never a bare pipe) returns the same class of score with and without the history, where today it returns `cp 0` with it; a red-first case in `tests/test_search.cpp` pins that a single pre-root occurrence is not a draw, observed red on the unfixed tree; the case "the losing side takes an available repetition" is **re-stated, not deleted or relaxed** -- its precondition becomes a position that has already occurred twice before the root, so the property it asserts (a draw score beats a lost position) is unchanged and its old precondition becomes the new red-first case's input, and DEC-173 is the recorded decision that lets an agent touch it; the two `tests/test_engine.cpp` cases over `is_position_repeated` stay green or are re-stated with the reason written at the site; the rule is stated in `adocs/specs.md`'s search row and the convention it replaces is named; **one SPRT at `--nonreg`**, pre-registered in `adocs/data/S207_sprt.sh` in the `adocs/data/S165_sprt.sh` shape with its three readings and DEC-143's worst-case games before the first game, recorded whatever it returns -- a zero is kept with the reason stated, since the change removes a wrong draw score from ordinary play and the SPRT is the rule and not the motive; Debug self-play, four rounds at 4+0.04 grepped for `Assertion`, before completion (DEC-141); the commit touches `src/` and alters the tree, so its message carries `Bench: <nodes>` (DEC-140)
touches:    src/search.cpp, src/bitboard.cpp, src/bitboard.hpp, src/data_structures.hpp, src/chesso.cpp, tests/test_search.cpp, tests/test_engine.cpp, adocs/specs.md, adocs/data/
excludes:   draw detection inside quiescence, which probes with no draw checks by the published practice S106 recorded as accepted; the fifty-move and insufficient-material tests, unchanged; `tools/analyse_game.py`, which reads the score this step corrects and needs no change of its own (its silent-zero defect is S214)
decisions:  DEC-170, DEC-173
closes:     2026-09-10_adversarial-F08
blocks:
paused_by:
author:     Claude Opus 5, coordinator, 2026-09-11
done:       2026-09-11. **A repetition is scored as a draw only where the search itself walked into it, or on a third occurrence anywhere -- and the SPRT accepted H1.** `adocs/data/S207_sprt.sh`, `--nonreg` `{-5, 0}` nElo, alpha = beta = 0.05, 8+0.08, Hash 16, UHO, concurrency 12, candidate the working tree against reference `ae4eed4`, seed `20260911012459`: **LLR 2.96, H1 accepted -- Elo +3.32 +/- 5.00, nElo +4.47 +/- 6.72, LOS 90.36 %, PairsRatio 1.05, DrawRatio 41.63 %, Ptnml [434, 1025, 2135, 1079, 456] over 10258 games in 4 h 26 m 56 s**. The three readings were written before the first game and H1's is applied unchanged: **not a regression of 5 nElo or more, kept, and no magnitude claimed** -- an SPRT stops early exactly when the observed effect has run favourable, so the +3.32 is biased upward and is not the effect size (DEC-063). What the change is for is the score, and the oracles are what establish that. **The abort rule was checked independently rather than read off the harness**: `tools/forfeit_report.py` over the run's own PGN reports **0 forfeits of 10259 on each side, 0.00 %**, against a 1.0 % rule; `fastchess.sh` does not call that script, so its census and this one are two counts and not one. **The class is reachable in ordinary play and the run says how often**: 1690 of 10259 games, **16.47 %**, ended in a three-fold repetition draw, split 853 / 837 by which side was White -- even, so neither binary is systematically steering into or away from the games that end that way. That is the rule's domain and not an attribution of the Elo. **The implementation.** `src/bitboard.cpp` gains `classify_repetition()`, which compares the matching history index against `search_state_t::root_history_size` and returns `NONE`, `ONCE_PRE_ROOT` or `DRAW`; `src/search.cpp` `negamax_at` scores `DRAW_SCORE` on `DRAW` alone, keeping its position above the transposition probe because a draw is a property of the path. **The root boundary is set in `search()` and not plumbed through the UCI layer** -- that is the only way into ply 0, so `iterative_deepening_search`, `datagen`'s `run_search` and every direct-call test get it for free, an aspiration re-search and each deepening iteration re-enter with the history back at the root, and no caller can forget; the shape section above records the deviation and why. `is_position_repeated()` keeps its two-fold-anywhere contract as a two-line wrapper over the same walk, which is what leaves `tests/test_engine.cpp`'s two direct cases and `tools/datagen.cpp`'s game-level adjudication untouched. **The published rule, read as prose (DEC-016)**: a draw once earlier but strictly after the root, or twice before or at it (CPW *Repetitions*; Stockfish PR #925). **The evidence, all re-taken here rather than quoted from the audit.** Reference `ae4eed4` answered the F08 board `score 0, nodes 5387, pv f6g8` with the history `g1f3 g8f6 f3g1` and `score -1229, nodes 412680, pv e7e5` for the identical board without it -- a false draw behind a tree **77 times smaller**, for a position the engine is losing by a queen. The candidate answers **`-1229 / 412680 / e7e5` both ways**. Oracles, neither of them chesso: python-chess 1.11.2 after `f6g8` gives `is_repetition(2) True`, `is_repetition(3) False`, `can_claim_threefold_repetition() False`, `is_game_over(claim_draw=True) False`; stockfish at depth 18 with the same board and history, **-703** playing `b8c6` (the audit read -687 on 2026-09-10). **`search_bench` depth 9 is node-identical** -- `121530 / 801481 / 72924`, `c3d5` / `e2a6` / `d7c8q` -- and **`bench` moved 26851183 -> 26491479**, -1.34 %. The pair is exact rather than contradictory: from a bare FEN the only class that can move is the root position recurring once inside the tree, which the old rule called a draw and the published one does not, and none of the three `search_bench` trees returns to its own root. INV-6 was therefore not available and the games decided it, which is what the accepts asked for. **Tests, red observed first.** New: *one occurrence before the root is not a draw*, the F08 shape, red at `REQUIRE_LT( 0, -300 )` on the unfixed tree. New control: *the root's own occurrence is the boundary* -- one board, one match, two adjacent root values, which is what separates `>` from `>=`. **Three fast-suite cases pinned the old convention, not the two the accepts named**: *the losing side takes an available repetition* (red at -590) and, **discovered mid-step**, *a repetition is answered before the table is* (red at 664, its own mutation note's value). Both are **re-stated, not relaxed** under DEC-173, each now reaching its position a third time so the property asserted is unchanged, and the first's old input became the new red-first case. The third case's discovery is recorded in its own section above rather than folded in silently. **Mutants**: `M35_repetition_root_entry_counts` (`>` to `>=`) and `M36_repetition_ignores_the_root` (the pre-S207 rule) added to `tools/mutants/board.py`; applied by hand and observed before the commit -- M35 kills the boundary case at `REQUIRE_EQ( 2, 1 )`, M36 kills that case **and** the F08 case -- and re-validated by `tools/mutation_check.py` from a worktree at this commit, which is the first point their anchors exist there. M36 carries a `(void)` of the parameter it orphans or it does not compile under `-Werror=unused-parameter`, which is recorded at the mutant. **Gate**: `ctest -L fast` **33/33 in `build` and 33/33 in `build-tune`**, re-run on the idle machine after the verdict (81.31 s and 81.86 s), format clean under `CLANG_FORMAT_MAJOR=22` (DEC-146). **Debug self-play (DEC-141)**: 4 rounds at 4+0.04, concurrency 2, **8 games, 0 `Assertion`, 0 disconnect**, every engine closed gracefully. **Goldens (DEC-142)**: the `-569` beside the re-stated case re-derives -- `python3 adocs/data/S192_anchors.py`, case "black a rook down, Kh7", `-569 vs -569 OK`; the board it evaluates did not move, only the moves that reach it. **DEC-136 applied to this commit, the first time that rule has fired**: `adocs/plan.md`'s cost section gains this run's row and its four figures are re-derived -- **mean 4 h 49 m, median 5 h 27 m over nine runs, 101366 games in 43.42 hours, 2334.6 games an hour**, and the fast class widens from 1 h 49 m to **2 h 28 m** because this run was outside the interval but *close to the near bound* and took 10258 games where the other three fast runs took 2522 to 6412. The projection moves to **289 to 356 machine-hours** and `DEV_MANUAL.md` carries the same re-derivation. **S207 is also the ledger's one throughput outlier, 2305.7 games an hour against the band's 2328 to 2346, and the reason is known**: the coordinator wrote S185, S181, S182 and S183 on the same machine for the first forty minutes, which cost about 1.2 % -- three minutes on a four-and-a-half-hour run. That is the measured price of DEC-172's document lane and it is worth what it cost. **Verified, not assumed, that the match played the binary this stamp benches**: `sha256sum` of `build/src/chesso` and the run's snapshot `/tmp/chesso-candidate.enQBrQ` agree at `920c3cd4e2f4c2d9...`, and that binary re-prints **26491479**, which is the commit's `Bench:` line (DEC-140). Docs: `adocs/specs.md`'s search row carries the rule, the convention it replaces, the reproduction, the two oracles, the bench-versus-`search_bench` reading and the verdict; `DEV_MANUAL.md` carries the bench signature at S207 and the re-derived ledger; `MANUAL.md` checked and needs no change -- it documents the UCI surface and current known bugs, this fixes one that was never listed and adds no option or output; `test_uci_surface` untouched, no option or default moved. `README.md` untouched, human-owned. Closes `2026-09-10_adversarial-F08`, the one high finding of that audit that fires in ordinary play.

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

## What the implementation chose, against the shape above

**The root boundary is set in `search()`, not plumbed through
`src/chesso.cpp`.** The shape above asked for `root_history_size` in
`search_state_t` -- which is where it is -- filled in at
`iterative_deepening_search()` and defaulted for the tests that build their own
state. `src/search.cpp` `search()` sets it unconditionally instead, because it
is the only way into ply 0: `iterative_deepening_search`, `tools/datagen.cpp`
`run_search` and every direct-call test go through it, an aspiration re-search
and each iteration of iterative deepening re-enter it with the history back at
the root, and no caller can now forget. Nothing was plumbed through the UCI
layer and `uci_search_options_t` is untouched.

The default is therefore only ever read by a test driving `negamax()` or
`negamax_probed()` directly, and it is `SIZE_MAX` rather than 0: an unset
boundary then reads the whole history as pre-root, which can only miss a draw,
where 0 would score a two-fold played before the root as one -- the defect this
step removes. Stated at the field.

`is_position_repeated()` keeps its two-fold-anywhere contract as a two-line
wrapper over `classify_repetition()`, one walk between them. That is what leaves
`tests/test_engine.cpp`'s two direct cases and `tools/datagen.cpp`'s game-level
draw adjudication untouched.

## Discovered while doing it

**A third fast-suite case pinned the old convention**, not named in the accepts
and re-stated in scope under DEC-173: `tests/test_search.cpp` "a repetition is
answered before the table is" (the S191 ordering property, mutation note
`REQUIRE_EQ( 664, 0 )`). Its Ra1-a3-a1 / Ke8-d8-e8 shuffle recreates the
position the case started from **once**, before its `negamax` node, so under the
new rule it is no longer a draw and the case went red at `664`. The shuffle is
played twice now, which makes the final position a third occurrence; the
assertion, the property and the mutation that kills it are unchanged.

**`tools/datagen.cpp`'s draw adjudication is deliberately left on the two-fold
rule** (`src/search.cpp` is the caller this step changed; `tools/datagen.cpp`
line 197 calls the primitive at game level). Labelling a self-play game drawn at
a two-fold is a corpus policy and not a search rule, and it is not obviously
wrong -- both sides would likely repeat -- but it is now a *different* rule from
the one the search plays by, and the corpus inherits it. Recorded here for
S082/S083 rather than changed: out of this step's scope, and no measurement here
says which label is better.

## Evidence taken, 2026-09-11

- **Red first, observed**: the new case "one occurrence before the root is not
  a draw" failed at `REQUIRE_LT( 0, -300 )` on the unfixed tree -- `score 0`
  where the material deficit belongs. Then the two cases above at `664` and
  `-590`.
- **The F08 reproduction, both binaries, python-chess `chess.engine` at `go
  depth 10`** (never a bare pipe): reference `ae4eed4` answers `score 0, nodes
  5387, pv f6g8` with the history `g1f3 g8f6 f3g1` and `score -1229, nodes
  412680, pv e7e5` for the identical board without it -- a factor of **77** in
  the tree. The candidate answers `-1229 / 412680 / e7e5` **both ways**.
- **Oracles, re-taken rather than quoted**: python-chess 1.11.2 after `f6g8`,
  `is_repetition(2) True`, `is_repetition(3) False`,
  `can_claim_threefold_repetition() False`, `is_game_over(claim_draw=True)
  False`; stockfish depth 18 with the same board and history, **-703**, playing
  `b8c6` (the audit read -687 on 2026-09-10).
- **`tools/search_bench.py` depth 9 node-identical**: `121530 / 801481 /
  72924`, `c3d5` / `e2a6` / `d7c8q`. Not a neutrality claim -- INV-6 is not
  available -- because the three positions are bare FENs with no pre-root
  history.
- **`chesso bench` 26851183 -> 26491479**, -359704 nodes, -1.34 %. The only
  class reachable from a bare FEN is the root position recurring once inside the
  tree, which the old rule called a draw and the published one does not, so the
  boundary is reachable at depth 14 before any game history is involved.
- **Two mutants, both killed**, added as `M35_repetition_root_entry_counts`
  (`>` becomes `>=`) and `M36_repetition_ignores_the_root` (the pre-S207 rule)
  in `tools/mutants/board.py`. Applied by hand in the main tree and observed:
  M35 kills "the root's own occurrence is the boundary" at `REQUIRE_EQ( 2, 1 )`,
  M36 kills that case **and** "one occurrence before the root is not a draw".
  M36 needs a `(void)` of the orphaned parameter or it does not compile under
  `-Werror=unused-parameter`, which is recorded at the mutant.
  `tools/mutation_check.py` itself runs from a worktree at HEAD, so the two are
  validated there after the completing commit, not before it.
- **Gate**: `ctest -L fast` **33/33 in `build` (83.30 s) and 33/33 in
  `build-tune` (83.79 s)**, `./clang-format.sh --check` clean under
  `CLANG_FORMAT_MAJOR=22` (DEC-146).
- **Debug self-play (DEC-141)**, 4 rounds at 4+0.04, concurrency 2, UHO,
  Hash 16: **8 games, 0 `Assertion`, 0 disconnect**, every engine closed
  gracefully, 48 s.
- **Goldens (DEC-142)**: the `-569` beside the re-stated case re-derives --
  `python3 adocs/data/S192_anchors.py`, case "black a rook down, Kh7",
  `-569 vs -569 OK`. The board it evaluates did not move; only the moves that
  reach it did.
- **Prose and citations**: `--prose` 0 flagged over 133 ids, `--citations`
  **0 flagged over 65 files**.
- **The SPRT**: `adocs/data/S207_sprt.sh`, pre-registered with the three
  readings and DEC-143's worst-case games before the first game. Launched
  detached 2026-09-11 01:24, seed `20260911012459`, out
  `/tmp/chesso_sprt_nonreg_20260911_012459`, candidate `ae4eed4` + uncommitted
  against reference `ae4eed4`, 8+0.08, Hash 16, concurrency 12 of 12, cap 40000
  games. Watcher armed with four exits, ceiling 37 h.

## Before the completing commit, when the verdict lands

Four things, in order, and the first is a placeholder that must not ship:

1. `adocs/specs.md`'s search row carries the literal token
   **`TODO-S207-VERDICT`** where the SPRT reading belongs.
   `grep -c TODO-S207-VERDICT adocs/specs.md` must read 0 before the commit.
2. Re-read `.tuning/sprt_s207.log`'s final SPRT block and
   `tools/forfeit_report.py` over `/tmp/chesso_sprt_nonreg_20260911_012459/games.pgn`
   against the abort rule (1.0 % on a side). On H0, take the reachability count
   the pre-registration demands over that PGN **before** deciding anything.
3. `python3 tools/mutation_check.py tools/mutants .ref-builds/mut --only M35`
   and `--only M36` from a worktree at the completing commit, which is the
   first point the two anchors exist there.
4. The commit message carries **`Bench: 26491479`** (DEC-140). Verified against
   the binary the match is actually playing: `sha256sum` of
   `build/src/chesso` and `/tmp/chesso-candidate.enQBrQ` agree at
   `920c3cd4e2f4c2d9...`, and that binary re-prints 26491479.

`adocs/plan.md`'s cost section gains this run's row and re-derives its four
figures in the same commit (DEC-136, S182), and `adocs/data/S183_elo_inputs.md`
is untouched by it -- S207 carries no published Elo figure.
