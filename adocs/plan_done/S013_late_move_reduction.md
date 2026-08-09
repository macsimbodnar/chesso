id:         S013
goal:       search late quiet moves at reduced depth and re-search when they beat alpha
accepts:    an SPRT reaches its bound or a clear trend; the root move is never reduced
touches:    src/search.cpp negamax, lmr_table
excludes:   reducing captures, and reducing while in check
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 4633857. README and MANUAL checked at adoption, not when this shipped.

## Measurement

4.1x at fixed depth. SPRT **+129.2 +/- 33.8 over 183 games**, killed at 96 %
LLR rather than run to the bound -- the trend was unambiguous and the machine
was needed. Recorded as never formally concluded.

## Bug found and fixed inside the step

LMR reduced the mating move at the root. Fixed with a `ply > 0` guard. Same
class of defect as the S012 one and caught the same way.

This is also the change that should make S006 start paying, since staged
generation only profits when the quiet moves above it are being pruned.
