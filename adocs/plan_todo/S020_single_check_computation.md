id:         S020
goal:       compute the in-check state once per node instead of once per call site
accepts:    identical search_bench node counts and best moves, since this is behaviour-neutral; measured gain larger than the benchmark's own reported resolution
touches:    src/search.cpp negamax and quiescence
excludes:   changing when the search decides it is in check
decisions:
closes:
blocks:
paused_by:
done:

## Why

`is_check` is about 12 % of the profile and is recomputed at every node in both
`negamax` and `quiescence`. The value is a property of the node, not of the
call site.

Behaviour-neutral by construction, so this needs a determinism check and not an
SPRT -- see INV-6.
