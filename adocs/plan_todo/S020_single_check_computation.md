id:         S020
goal:       compute the in-check state once per node instead of once per call site
accepts:    identical search_bench node counts and best moves, since this is behaviour-neutral; the cost measured with hyperfine over interleaved fixed-depth runs, its noise floor recorded, and the keep-or-revert call made from that number — a zero is recorded as zero and does not block completion
touches:    src/search.cpp negamax and quiescence
excludes:   changing when the search decides it is in check
decisions:
closes:
blocks:
paused_by:
done:

## Why

`is_check` was about 12 % of the profile **on the Apple machine under Apple
clang** -- a pre-DEC-049 figure that keeps its conditions attached and has not
been re-taken here. Re-profile on this machine before starting: the ranking
that placed this step was made from that number. The structural claim still
holds at HEAD -- `is_check` is recomputed at every node in both `negamax` and
`quiescence`, and the value is a property of the node, not of the call site.

Behaviour-neutral by construction, so this needs a determinism check and not an
SPRT -- see INV-6.
