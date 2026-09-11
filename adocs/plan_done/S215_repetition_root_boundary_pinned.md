id:         S215
goal:       the one assignment the repetition rule hangs on is pinned by a test that reaches it through `search()`, and by a mutant that test kills
accepts:    a case in `tests/test_search.cpp` drives `search()` -- not `negamax()` with the field set by hand -- from a position whose draw can only come from `index > root_history_size`, so it fails if `src/search.cpp` `search()` sets `state->root_history_size` to anything but `game->history.size`; the case asserts against `DRAW_SCORE` and a non-draw score at the same board with a different pre-root history, both the S192 way against zero rather than against a golden; a mutant anchored on that assignment joins `tools/mutants/search.py` -- `history.size - 1`, which is the one that restores the pre-S207 behaviour for the root-recurrence class -- and `tools/mutation_check.py` reports it **killed** from a worktree at the completing commit; `+ 1` is either killed by the same case or is stated at the mutant as the surviving direction with what it costs; INV-6 discharged on identical `tools/search_bench.py` counts and best moves and an identical `bench` signature, because the fix is a test and not a rule
touches:    tests/test_search.cpp, tools/mutants/search.py
excludes:   any change to `classify_repetition()` or to the rule, which is S207's and is correct; `tools/datagen.cpp`'s two-fold game adjudication, which is S082/S083's question and is recorded at S207's own file
decisions:  DEC-141, DEC-173
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), 2026-09-11
done:       2026-09-11. **The line the repetition rule's boundary comes from is
            read by the suite for the first time, and both of its neighbours die
            on it.** `tests/test_search.cpp` "search() hands the rule the root's
            own index" searches **one board three times through `search()`** --
            never `negamax()` with the field set by hand -- and the three
            answers separate `game->history.size` from everything next to it.
            The board is a forced perpetual, `6k1/r4pp1/6p1/8/7Q/8/6K1/q7 w - -
            10 40`: White is a rook and three pawns down (python-chess: White
            K+Q against Black K+Q+R+3P), every check leaves Black **exactly one
            legal reply**, and **no capture exists anywhere in the cycle**, so
            the winning side never has to cooperate and nothing but the boundary
            decides what the line scores. Stockfish at depth 22 answers **0**
            with `Qd8+ Kh7 Qh4+ Kg8 Qd8+`; python-chess says `Status.VALID` for
            both FENs and counts the one legal reply at each check.

            **The three searches, and what each is for.** (1) The root reached
            from `8/r4ppk/6p1/8/7Q/8/6K1/q7 b - - 9 39` by Black's only legal
            move, so one entry sits behind the root and it is **not** the root
            position -- at **depth 4** the cycle returns to the root's *own*
            entry, which is not strictly inside the tree, so there is no draw
            and the answer is the material, **-909** against a static
            **-929**. (2) The same board and the
            same history at **depth 5**: the ply-1 position returns at ply 5,
            strictly above the root's entry, which is a draw -- **0**, and it is
            the only draw available, which is the `accepts`' "a draw that can
            only come from `index > root_history_size`". (3) The same board with
            the cycle played *before* the root instead of inside it -- the
            root's own entry plus one pre-root occurrence, two at or below the
            root, a draw wherever both lie -- **0** at the depth that answered
            the material in (1). (1) and (3) are asserted to be the same
            position, by board hash, differing in `history.size` alone, **1
            against 4**: same board, other history, other score. Scores against
            zero and a material bound, never a golden (S192).

            **Every wrong boundary observed red before the case was called
            done**, each failing the assertion written for it: `- 1` and `0`
            fail (1) at `REQUIRE_LT( 0, -300 )`; `+ 1` and the assignment
            deleted -- the `SIZE_MAX` default -- fail (2) at
            `REQUIRE_EQ( -841, 0 )`. The empty history is why (1) and (2) are
            built from a FEN one ply earlier rather than from the root's own:
            at `history.size == 0` the `- 1` underflows to `SIZE_MAX` and
            survives. Nothing else in the suite reads the line: under each
            mutant the **only** failing assertion across 33 test binaries is
            this case's.

            **M39 and M40, both killed, from a worktree at `cfe2406` -- the
            commit that carries the case and the mutants.**
            `tools/mutation_check.py tools/mutants .ref-builds/mut --only M39
            M40`, baseline green at 33 tests and `bench 30046849`:

                M39_root_boundary_one_low   killed  fast 1/33  bench moved  97.8s
                    [test_search] search() hands the rule the root's own index
                    | REQUIRE_LT( root_recurrence.score, -300 )
                M40_root_boundary_one_high  killed  fast 1/33  bench moved  97.3s
                    [test_search] search() hands the rule the root's own index
                    | REQUIRE_EQ( in_tree.score, 0 )

            mutation score **2 of 2**, wall 296s. The same pair ran once
            before, at `177171e` -- this tree before it was amended to carry
            the `Co-Authored-By` trailer every other commit here has -- and
            read the same two kills, at 99.8 s and 99.2 s.
            `+ 1` is killed rather than
            stated as a survivor, so the `accepts`' second branch does not
            apply. M39 restores the pre-S207 reading for the root-recurrence
            class, which is the class whose removal moved `bench` by 1.34 % at
            S207; the gap it closes was measured at `23f926d` by the Tier-1 fast
            check -- the whole fast suite green while `bench` read **26117924**
            against **30046849**, a tree 13 % different.

            **INV-6 discharged.** `tools/search_bench.py` depth 9 **121530 /
            801481 / 72924**, best moves **`c3d5` / `e2a6` / `d7c8q`**, and
            `bench` **30046849**, the parent's total -- identical because no
            `src/` file was touched, which is also why the commit carries
            neither a `Bench:` line nor `No functional change` (`tools/gate.sh`:
            a commit touching no `src/` file needs neither). No SPRT: a test and
            two mutants cannot alter play. **DEC-141's second tier does not
            bind** -- `make_move`, `unmake_move`, the generator and the search
            are untouched and no pruning, reduction or extension rule was added
            -- and `tools/gate_extra.sh` last ran green on 2026-09-10, inside
            its weekly cadence.

            **Gate:** `ctest -L fast` **33/33 in both builds**, format clean
            under `CLANG_FORMAT_MAJOR=22` (DEC-146).

            **Docs.** `adocs/specs.md`'s repetition paragraph names the third
            case, what each of its three searches establishes, the four
            boundaries observed red and mutants M39 and M40. `DEV_MANUAL.md`'s
            mutation cost paragraph carried **41** where the tree holds 47; the
            count now names S207's, S208's and this step's additions, the
            measured 3948 s staying attached to the 40 rows and the date it was
            taken at. `MANUAL.md` checked and unchanged: nothing on the UCI
            surface moved.

            **Excludes honoured**: `classify_repetition()` and the rule itself
            are untouched -- the new case reads them, and the two existing
            `classify_repetition()` assertions in its precondition block are
            calls, not changes -- and `tools/datagen.cpp`'s two-fold
            adjudication was not opened. Closes the Tier-1 fast check's finding
            over `23f926d`.

## Why this exists

Found by the Tier-1 fast check over S207's completing commit `23f926d`, on
2026-09-11, and **verified by measurement before it was written down**.

S207's rule has two halves. `classify_repetition()` compares the matching
history index against a boundary -- that half is pinned, by *the root's own
occurrence is the boundary* and by mutants `M35` and `M36`. The other half is
that the boundary handed in is the root's own index, one line in
`src/search.cpp` `search()`:

    state->root_history_size = game->history.size;

**Nothing in the suite reads that line.** Mutated to `game->history.size - 1`
and rebuilt, `ctest -L fast` is green on all 32 functional tests
(`test_clang_format_script` fails only for DEC-146's reason), and `bench`
reads **26117924** against the tree's **30046849** -- a tree 13 % different,
so the mutant is emphatically not equivalent and the suite is simply blind to
it.

What `- 1` does is restore the pre-S207 behaviour for one class: the root's own
entry becomes "in tree", so **the root position recurring once inside the tree
scores `DRAW_SCORE` again**. That is part of the F08 shape and it is the class
whose removal moved `bench` by 1.34 % at S207.

Why the three existing cases miss it, from their own asserted inputs:

- *the losing side takes an available repetition* reaches `search()`, but its
  draw comes from **two** matches, at history indices 0 and 4 with the boundary
  at 7. Both are below the boundary at 6, 7 and 8 alike, so `seen_one` fires
  either way and the case is boundary-invariant.
- *one occurrence before the root is not a draw* reaches `search()` with one
  match at index 0 and a boundary of 3; `0 > 2`, `0 > 3` and `0 > 4` are all
  false, so it reads `ONCE_PRE_ROOT` either way.
- *the root's own occurrence is the boundary* sets `root_history_size` by hand
  and calls `negamax()`, so it never executes the assignment.
- No fast test pins an exact node count out of `search()`, only inequalities
  and run-to-run equality, so the node shift is invisible too.

The `SIZE_MAX` default is what makes the gap structural: no `negamax()`-driven
test can reach the in-tree branch without setting the field by hand, and both
`search()`-driven cases land elsewhere.

## Shape

One case, and the position has to be built so the *only* available draw is an
in-tree one. A root that recurs inside its own tree is the direct form: a
position from which the side to move can shuffle back to it in two plies, with
no pre-root occurrence at all, where the side to move is losing by the engine's
own evaluation so the draw score is visibly preferred. Assert `DRAW_SCORE`
there; then assert a non-draw at the same board reached with one pre-root
occurrence, which is S207's own rule from the other side and is what makes the
pair sensitive to the boundary rather than to the position.

This is a test and a mutant. **It changes no rule**, so it owes node counts and
a bench signature rather than a verdict.

## Cost

Agent work, an hour. No run.
