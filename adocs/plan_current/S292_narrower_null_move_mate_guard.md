id:         S292
goal:       close the defect S289's guard closed -- six roots proved mated in two misreported because a null move fails high against the table's mated bound -- with a narrower form that does not cost what S289's run measured, or show by measurement that there is none
accepts:    over positions from S289's run's own games (`.tuning/sprt_s289_20261022_155029/games.pgn`, archived as `chesso_S289_sprt_2026-10-22.tar.xz`), from the engine's own output (a counter or trace in a scratch build, never shipped), at S289's guard: how often it fires per node searched, how many of the bounds it fires on were stored with moves unsearched (a shallow-depth rule skipped a move) against bounds from a search of every legal move, and the node cost of each kind; if a narrower condition, taken from a published description (DEC-221, source named and quoted), reports `mate -2` at all 31 of `tests/test_engine.cpp`'s roots mated in two that S289's case drives, it ships as its own change on the reverted tree: guard tests observed red first, a mutant they kill (DEC-141), Debug self-play command written for the coordinator, gate_extra, every moved golden by its script inside DEC-299's guard, `tools/search_bench.py` d9/d12, and a `{-5, 0}` SPRT pre-registered in `adocs/data/S292_sprt.sh` in `S289_sprt.sh`'s shape; if none does, the figures and a proposed decision in DEC-158's form for the owner (DEC-309 (3)); the fast suite green in both builds
touches:    adocs/data/S292_*; this file; src/search.cpp, tests/ and tools/mutants/ only if a narrower form ships
excludes:   the full verified null-move search (DEC-087 (e), the owner's); late move reduction; S083's constants; any lowering of a mate floor; another run of S289's guard as it was
decisions:  DEC-309, DEC-306, DEC-171, DEC-158, DEC-087, DEC-221, DEC-142, DEC-141, DEC-299, DEC-063
closes:
blocks:     S289
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-22, DEC-309)

S289's guard -- no null move at a node whose entry is an upper bound or an
exact score at or below `-MATE_MIN` -- read H0 at `{-5, 0}`: Elo -7.42 +/-
5.07 over 8716 games against a predicted zero (`adocs/data/S289_sprt.log`).
The run stands, and its pre-registered row reverted the guard whole, so the
defect it closed is open again: six of the 31 roots
`adocs/data/S165_defender_set.tsv` proves mated in two report `cp 0` or about
-670 through 12 iterations at the shipped weights (`adocs/data/S289_reach.txt`).
That moves a reported score, so the BUGS rule binds (DEC-171 (c)).

The guard's own comment named a way it could cost more than predicted: at a
non-PV node with alpha outside the mated band the shallow-depth rules skip
moves without touching `best_so_far`, so a mated-band upper bound can be stored
with moves unsearched, and a guard trusting such a bound removes passes where
nothing was proved. That is a hypothesis, not a reading. This step measures
where the cost came from before anything is changed, and tries the narrower
form the pre-registration offered before the owner rules between the BUGS
rule and the measured cost.
