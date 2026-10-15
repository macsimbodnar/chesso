id:         S286
goal:       S099's pawn key and pawn-keyed correction, measured H1 at 32+0.32 by S283, reland onto S039's outcome with every golden re-derived by its script
accepts:    (1) `8758622`'s change -- the branchless pawn key on board_t, the pawn-keyed correction table (4096 slots a side, owned by the UCI layer, one per datagen worker, cleared per game), its read at negamax_at's compute-or-read site with both TT stores keeping the raw evaluation, its update beside the node's store, `CorrHistMax` and `CorrHistWeightCap` -- re-applied onto S039's outcome (S039 kept, or reverted on its H0), not re-designed and not re-tuned; (2) the tune build at `CorrHistMax` 0 prints the parent's bench stream (DEC-215); bench and `tools/search_bench.py` recorded; (3) every golden the change moves re-derived by its own script, never read off a run (DEC-142): carry ceilings and S170 budgets, `EXACT_FLOOR`, `MATE_IN_THREE_FLOOR`, first-mate, the mined mate rows (re-mined by their scripts if they stop separating, DEC-288 and DEC-293), the ordering band by S282's rule; **no relaxation allowance**: DEC-272 and DEC-284 lapsed with S099's H0, so a ceiling rise, a loosened budget or floor, or a dropped mate floor stops for the owner's ruling; (4) DEC-141's second tier: S099's mutants and any a re-mine touches killed, Debug self-play four rounds at 4+0.04 with no `Assertion`, `tools/gate_extra.sh`; (5) no SPRT: it ships on S283's H1 (DEC-286), and its commit cites S283's record; INV-4 holds (the key is maintained in add_piece, remove_piece and move_piece, never recomputed)
touches:    src/data_structures.hpp, src/search.cpp, src/search.hpp, src/search_params.hpp, src/bitboard.cpp (the piece primitives), src/uci.cpp or wherever the table is owned, tools/datagen.cpp, tests/, tools/mutants/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md, adocs/data/S170_cases.tsv
excludes:   any change to the correction's design or its two parameters' defaults; S110 and S111, each its own step and verdict; the two consumers DEC-222 named
decisions:  DEC-286, DEC-271, DEC-222, DEC-142, DEC-215, DEC-141, DEC-294
closes:
blocks:     S110, S111
paused_by:
author:
done:

## Why this exists

S099 read H0 at the harness setting (`74bb9ad`, nElo -4.71 +/- 6.84 at
8+0.08) and was reverted (`5a04ffa`); its 32+0.32 estimate read +12.46 +/-
15.23, and the owner booked one SPRT at that control (DEC-286). S283 read H1
there: nElo +12.56 +/- 9.89 over 4738 games (`adocs/data/S283_sprt.log`). By
DEC-286's H1 row S099 relands, rebased onto the then-tree, which is S039's
outcome (DEC-290); S099's own step file (`adocs/plan_done/S099_correction_history.md`)
holds the design, its readings and its mutants. Phase 1 is written blind while
S039's SPRT holds the machine (DEC-260); phase 2 runs on S039's outcome.
