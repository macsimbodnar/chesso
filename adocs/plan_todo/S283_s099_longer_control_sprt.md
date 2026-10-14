id:         S283
goal:       S099's pair, the pawn key and the pawn-keyed correction against the tree before them, decided by an SPRT at 32+0.32, Hash 64, {0, 10} nElo (DEC-286)
accepts:    (1) a pre-registration `adocs/data/S283_sprt.sh` written before the first game: CAND `8758622`, REF `bacead1`, 32+0.32, Hash 64, `noob_3moves.epd`, `{0, 10}` nElo, DEC-143's worst-case and expected games for that pair of bounds at this control, the abort rule (forfeits over 1.0 % on either side, a crash or disconnect voids), the open DEC-171 findings, and the three outcomes as DEC-286 states them; (2) the run detached with a terminal marker and a watcher with four exits; (3) the verdict recorded with DEC-220's block and the ledger regenerated; (4) on H1, S099 relanded onto the then-tree by its own step, every golden re-derived by its script, S110 and S111 back in the Open list conditional; on H0 or no verdict, nothing ships
touches:    adocs/data/S283_sprt.sh, adocs/data/S283_sprt.log, adocs/data/README.md
excludes:   any change to the candidate; a re-tune of S099's two parameters; any other verdict at a longer control
decisions:  DEC-286, DEC-270, DEC-202, DEC-143, DEC-220
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S099 read H0 at the harness setting (nElo -4.71 +/- 6.84, `{0, 5}` at
8+0.08, Hash 16) and its DEC-202 estimate at 32+0.32, Hash 64 read nElo
+12.46 +/- 15.23 (`adocs/data/S099_ltc.log`). The owner ruled one SPRT on the
same pair at the longer control (DEC-286), after S023 verdict 1's SPRT. The
pair is fixed and independent of the tree, so the run's attribution does not
depend on what lands before it.
