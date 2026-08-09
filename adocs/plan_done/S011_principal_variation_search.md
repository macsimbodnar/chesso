id:         S011
goal:       search the first move with a full window and the rest with a null window
accepts:    fixed-depth time falls; an SPRT returns a verdict
touches:    src/search.cpp negamax
excludes:
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 79dbb50, measured jointly with S012. README and MANUAL checked at adoption, not when this shipped.

## Measurement

-21 % at fixed depth on its own. The SPRT was run over PVS and NMP together
and returned **+132.4 Elo**; see S012 for the run.

PVS is also the precondition for reductions being safe, which is why S013
follows it rather than the other way round.
