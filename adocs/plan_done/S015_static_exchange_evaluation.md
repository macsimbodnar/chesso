id:         S015
goal:       exact see() and fast see_ge(), and decline losing captures in quiescence
accepts:    see() agrees with an independent cross-check over a large assertion count; quiescence pruning measured in games; the result recorded whatever it is
touches:    src/bitboard.cpp see, see_ge, capture_cannot_lose, attackers_to_square, least_valuable_attacker; src/search.cpp quiescence
excludes:   ordering losing captures after the quiets, which is S025
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 566d98a, 5da613b, 63c9378. README and MANUAL checked at adoption, not when this shipped.

## Measurement

Quiescence pruning on losing captures: **0 Elo.** Kept. A later optimisation
made `see()` 12.1 % cheaper, so the same feature is now a cheaper trade than
when it measured zero -- **an SPRT rerun on that basis is still outstanding and
is noted in status.md.**

## Three bugs found inside the step

- **X-ray.** `may_uncover` excluded kings, but a king attacker is aligned with
  the target and can hide a slider behind it. Only knights are exempt. Caught by
  a determinism check: 20 nodes out of 40408 differed.
- **A wrong speculative cutoff.** On `3k4/8/1K6/8/8/8/1ppppppp/RqRRRRRR` `see()`
  returned 400 for a 500 exchange. The cutoff was removed. This one **shipped**
  and pruned quiescence on wrong values for two commits, which is the origin of
  the fix-bugs-first rule in CLAUDE.md.
- **An inverted ternary** in `see_ge`, `(result ? 0 : 1)`. Caught by the 455233
  assertion cross-check against exact `see()`.
