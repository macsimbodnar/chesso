id:         S026
goal:       drop nodes near the horizon that cannot reach alpha
accepts:    an SPRT per technique, measured separately; mate and tactical tests in the fast suite still pass
touches:    src/search.cpp negamax
excludes:   singular extensions, which are a separate and larger change
decisions:
closes:
blocks:
paused_by:
done:

## Hazard, twice observed

Both NMP and LMR shipped with a bug that hid a mate, and both were caught by a
mate test rather than by a benchmark. Any pruning added here gets the same
treatment: a position with a forced mate inside the pruned depth, in the fast
suite, before the feature is called done.
