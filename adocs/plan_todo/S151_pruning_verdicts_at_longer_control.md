id:         S151
goal:       a change that moves a pruning or reduction parameter has its verdict re-taken at a control at least four times longer before the number is banked, starting with S085's shipped vector
accepts:    S085's shipped vector is re-tested against `3488506` at a control at least four times longer than `8+0.08`, with the control and its cost stated before the run is committed to, and whatever it returns is recorded -- including a regression, which is the outcome the published record says to expect if it exists; the rule is written where the bounds rule already lives in `fastchess.sh` and `DEV_MANUAL.md`: a change that moves a pruning or reduction parameter has its verdict re-taken at a longer control before the magnitude is banked; the rule is scoped so it does **not** apply to all 45 to 55 pending verdicts, because that roughly doubles the plan's machine budget, and the scoping reason is stated
touches:    fastchess.sh, DEV_MANUAL.md, adocs/data/, adocs/decisions.md, adocs/specs.md
excludes:   a second SPSA run at the longer control, which is a tuning step and not a verification one; re-testing the earlier verdicts S021, S068, S076, S089 or S107, which is a separate decision about history; changing any default, which only the verdict may do and which would be its own step
decisions:  DEC-019, DEC-063, DEC-094
closes:     2026-08-21_adversarial-F03
blocks:
paused_by:
done:

## The evidence, and the one instance that matters most

vondele, `nevergrad4sf`: "the optimal parameters are often time sensitive, i.e.
can be verified to be a gain at the VSTC used for tuning, but regress at STC or
LTC". Stockfish issue #2600: of the last 40 LTC tests with gaining bounds, "23
reds, 16 yellows, 1 green". The fishtest wiki: a long TC "is best to get better
scaling values".

S085's vector moves every affected axis toward more pruning and more reduction --
`RfpMaxDepth` 6 to 15, `LmrDivisor` 225 to 182, `MaxQsearchDepth` 8 to 19,
`AspirationMinDepth` 5 to 2 -- tuned at `2+0.02`. DEC-094 already excluded the
nine `Tm*` axes from S085 on exactly this reasoning, so the failure mode is
known; that it applies to the axes that were tuned is not recorded.
