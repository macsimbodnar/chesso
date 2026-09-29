id:         S243
goal:       a direct guard test for the multicut's mate band -- the verification search's returned value is never handed back as the node's when it is in the mate range -- with a mutant only it kills, so E21's coverage no longer depends on a mined row
accepts:    a test drives a node where the multicut's verification search returns a mate-range value and asserts the node does not return it (the guard's own statement), observed red with the gate dropped (E21) and green shipped; `tools/mutation_check.py` over S097 verdict 2's list on a fresh fixture kills E21 by that test; the mined multicut row stays as it is (a second witness, re-derived by its script when the tree moves); the fast suite green in both builds
touches:    tests/test_search.cpp, tools/mutants/ (S097's list)
excludes:   the multicut's rule and its constants; any other guard's test; the mining script
decisions:  DEC-141, DEC-142, DEC-238
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-29 00:35 CEST
done:       2026-09-29 -- "the multicut never ends a node on a mate from its verification" (`tests/test_search.cpp`, `se_mate_drive_t`) drives a non-PV node at `SeMinDepth` whose verification search comes back at mate in one -- Ra8 in `6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1`, python-chess checkmate, Stockfish depth 20 `#+1` with no other mate over multipv -- reads every other condition of the multicut off the probe and the value as `MATE_MAX - (ply + 1)`, and asserts the guard's statement twice: the node's returned score is outside the mate band (added at the cold fast check) and the rule does not fire. Observed red with E21 applied by hand, `CHECK( 48998 < 48000 )` then `REQUIRE( !record.se_multicut )`, green reverted; `tools/mutation_check.py --only E21` on a clean fixture of the final tree (`28723ae`) kills E21 by it, fast 1/41, bench same, with the mined row as the second killer; the whole S097 list 22 of 22 on the first round's fixture. The mined row stays; `DEV_MANUAL.md`'s DEC-142 row says its going stale now shows only as E21's kill list shrinking to one case, and that it is re-derived then. E21's description names the new killer. No golden added, `src/` untouched, so no `Bench:` line; no new rule, so no second tier. Both fast suites, the format check and the prose checks green. `MANUAL.md` and `specs.md` checked, no change. Test-side filler under DEC-171, named in the pre-registrations taken while it was open (S131's). Written by an Opus subagent briefed by the coordinator.

## Why this exists

S097 verdict 2's mate-band guard was covered by a mined mate row alone. The
row moved at S113's rebase and stopped separating E21 at S131's -- the third
time in three rebases that a mined mate row moved with the tree -- and
DEC-238 re-derived it under a guard mode of the mining script. A direct test
is what DEC-141 asks of a rule's guard, and it ends the corpus dependence for
this one: it drives the node, plants a mate-range return in the verification
search's path (a drive fixture in the shape of S113's `probcut_drive_t`, or a
position whose verification search returns a mate at a small depth), and
asserts the guard's statement directly. Filler under DEC-171: agent-only
work, closed by the block boundary, named in every pre-registration taken
while it is open.

## What was found, 2026-09-29

E21 (`tools/mutants/S097_singular_extension.py`
`E21_multicut_mate_band_gate_dropped`) drops the `vscore < MATE_MIN && vscore
> -MATE_MIN` term from the multicut's condition in `src/search.cpp`
`negamax_at`, and leaves S165's `beta > -MATE_MIN` beside it. On this tree it
is killed by two cases once the new one lands, and by one before: the mined
row in `tests/test_search.cpp` "pruning does not hide a forced mate" is red
under E21 on S131's tree, as DEC-238 re-derived it. `se_drive_t`'s fortress
cannot deny the guard, because every score there sits outside the band -- the
multicut case says so in its own comment.

## What changed

`tests/test_search.cpp`, in the singular-extension block beside the multicut's
own case:

- `SE_MATE_DRIVE_POS`, `6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1`. From tools, never
  from the board: python-chess reports `is_valid()` True, `is_check()` False,
  17 legal moves, no capture, no promotion, and a1a8 checkmate; stockfish at
  depth 20 through python-chess reports `#+1`, pv a1a8, and over all 17 root
  moves (multipv) no other mate; g1f1 is legal and neither check nor mate
  (`.tuning/coord/S243_logs/oracle.py`, `oracle_candidate1.txt`).
- `SE_MATE_WINDOW_SCORE` 3000, the planted entry's score: a window only the
  mate reaches, so the verification's one fail-high is Ra8. Not a golden --
  the case asserts the value that comes back is the mate itself, so an
  alternative reaching the window would fail the precondition loudly.
- `se_mate_drive_t`, a drive in the shape of `probcut_drive_t` with
  `se_drive_t`'s plant: the node at `SeMinDepth`, the entry at the shallowest
  depth the margin accepts, both derived from the parameters, g1f1 planted as
  a `TT_BETA_NODE` move; the mate re-checked by the engine's own rules (after
  Ra8 the side to move is in check with no legal reply).
- "the multicut never ends a node on a mate from its verification": every
  condition of the rule but the band is read off the probe -- verified, failed
  high, at or above the node's beta (`SE_MULTICUT_BETA`), not PV, beta not a
  defender's -- and the value is named: `MATE_MAX - (ply + 1)`, mate in one
  one ply below the node, derived from `negamax_at`'s own mated-node score and
  read back from the drive. Then the guard: `!se_multicut` and a searched move.
  The branch is asserted, not the returned number, because the node's own
  search may reach the same mate, and that one is proved.
- The multicut case's comment now names this case as what holds the guard,
  with the mined row as a second witness (comment only).

`tools/mutants/S097_singular_extension.py`: E21's description names the new
case as its killer and the mined row as the second witness; the mutation
itself is unchanged.

`DEV_MANUAL.md`: the DEC-142 row for `mate_the_multicut_hides` says the
property it stands in for has its own case since S243; the row and its
derivation are unchanged.

## Evidence

- Green shipped in both builds, 92 assertions; the drive costs 60 nodes and
  0.05 s (a temporary MESSAGE, removed: vscore 48998, `singular_beta` 2910,
  one move searched).
- **Red with E21, the S033 way**, in the fixture below with E21 applied by
  hand: `REQUIRE( !record.se_multicut ) is NOT correct!` after 90 passing
  assertions, every precondition held; the mined row red as well at
  `REQUIRE( result.mate_found )`; reverted, both green
  (`.tuning/coord/S243_logs/red_e21_case.log`, `red_e21_mined_row.log`,
  `green_fixture_case.log`, `green_fixture_mined_row.log`).
- **`tools/mutation_check.py --only E21`** on a clean detached fixture,
  `.ref-builds/mut` at `c3f641a` (`13caf43` plus this worktree's whole diff,
  both steps' tests), niced at `--jobs 4`:

      worktree /home/max/ws/chesso-fill/.ref-builds/mut at c3f641a clean
      list     /home/max/ws/chesso-fill/.ref-builds/mut/tools/mutants/S097_singular_extension.py   clean
      baseline green, 41 tests, bench 3429473 nodes via engine
        E21_multicut_mate_band_gate_dropped killed      fast 1/41  bench same  137.2s
         [test_search] pruning does not hide a forced mate  |  REQUIRE( result.mate_found )
         [test_search] the multicut never ends a node on a mate from its verification  |  REQUIRE( !record.se_multicut )

  The bench does not move under E21, so the suite is the only oracle that
  sees it (`mutation_e21.log`, `mutation_e21/results.tsv`). The fixture
  predates the DEV_MANUAL sentence and this step file's sections, documents
  only.
- **The whole of S097's list** on the same fixture, niced at `--jobs 4`,
  3223 s: **22 of 22 killed** -- baseline green, 41 tests, bench 3429473 --
  and E21 again by this case and the mined row. This case also reddens under
  E08, E13, E14 and E18 at its preconditions (the verification answered by
  the planted entry, run at full depth, under a window above the entry, or
  not at all), which is what asserting the drive's premises rather than
  assuming them buys (`mutation_full.log`, `mutation_full/results.tsv`).
