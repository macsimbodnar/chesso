id:         S330
goal:       the recipe that rebuilds `selfplay_v2_dedup.tsv` from the archived corpus names the tree that rebuilds it byte for byte
accepts:    both digests re-derived on `.tuning/selfplay_v2.tsv` read in place (sha256 `ad8c5dfe...`): `corpus_dedupe` built from a tree before S042 (`a3e84e1`, by `git archive`) writes the `0a6b59b6...` DEC-183 names, and one built at `HEAD` writes `cdb43ba2...` (10795203 rows, S327's run), each run stated; the repository's statement of the recipe (where `DEV_MANUAL.md` and the dedupe step's records give it) names the tree and both digests, history kept (DEC-348); the archive in the owner's Synckeeper folder (`.moltke.local.md` names it) is not rewritten -- a corrected `CORPUS_README.md` goes beside it as its own file with a `.sha256` sidecar, built under `.tuning` and moved in (the Synckeeper rule), or the step says why not; a decision drafted for the coordinator if the recipe's rule changes, for example which digest later fits name
touches:    DEV_MANUAL.md, adocs/data/S330_*, adocs/data/README.md
excludes:   regenerating or replacing `.tuning/selfplay_v2_dedup.tsv`; any refit; `src/` and `tools/`
decisions:  DEC-183, DEC-171, DEC-348
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-03 by S327's implementer. DEC-183's archive records the `corpus_dedupe` command that rebuilds the derived
dedup file `0a6b59b6...`, which twelve `plan_done/` records name. Since S042 the key drops an en passant square no pawn
can take, so a current binary writes `cdb43ba2...` with 10795203 rows from the same input. The local file still hashes
`0a6b59b6...`, but a machine restoring from the archive with a current binary gets a different corpus from the one the
records name. Filler by DEC-171: no fit moves until someone refits on a regenerated file.
