id:         S157
goal:       every recorded conclusion that states an SPRT bound in plain Elo says nElo instead, and DEV_MANUAL.md says which scale the bounds are in
accepts:    `DEV_MANUAL.md` and `fastchess.sh` state that the bounds are normalized Elo, beside the CPW table they are taken from, so the scale is connected to the numbers rather than left to be re-derived; every recorded conclusion that states a bound in plain Elo is restated -- the eight sentences of the form "not a regression of 5 Elo or more" in `adocs/specs.md` and the pending-side documents -- with the logistic equivalent given from the run's own printed `Elo` and `nElo` pair rather than from a general constant, since the ratio is a property of the draw rate; `model=normalized` is **not** changed and the reason is stated, because the CPW table's own Stockfish rows are fishtest's nElo bounds, so the current setting matches the cited source
touches:    DEV_MANUAL.md, fastchess.sh, adocs/specs.md
excludes:   changing the SPRT model or any bound pair, which would alter every verdict's cost and is not what this is about; rewriting `adocs/plan_done/`, which is history -- the restatement lands in `specs.md`'s current-state wording and in `DEV_MANUAL.md`; re-running any earlier verdict
decisions:  DEC-063
closes:     2026-08-21_adversarial-F09
blocks:
paused_by:
done:

## The arithmetic

`fastchess.sh:226` passes `model=normalized`; `fastchess --help` says that model
"Uses nElo". S085 printed `Elo 21.02` and `nElo 26.81`, a ratio of 1.2755, so at
that draw rate a bound of 5 nElo is 3.92 logistic Elo. `DEV_MANUAL.md:1274`
already records that verdicts *report* nElo; what is missing is that the bounds
are in it too.
