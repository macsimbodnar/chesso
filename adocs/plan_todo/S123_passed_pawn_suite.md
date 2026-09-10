id:         S123
goal:       passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
accepts:    an SPRT verdict per group, recorded whatever it is; the table is rank crossed with can-advance and safe-advance rather than a single rank curve; the distance to **each** king is scaled by rank, because the same distance is worth more to a pawn on the seventh; candidate passers are scored; **monotonicity is not imposed** -- a fit that returns a non-monotonic middlegame curve is reporting something and is not corrected by hand; a test asserts the passer bitboard is colour-symmetric under board mirroring, written before any fit, because two published passed-pawn bugs shipped invisible to everything except an SPRT
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tests/test_evaluation.cpp
excludes:   pawn structure terms, which are S125
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## What is there

Six buckets by rank, one weight each, and the middlegame row reads

    const int passed_pawn_mg[6] = {0, -6, -4, 19, 59, -17};

Bucket 5 is a pawn one square from promotion and it is fitted **negative**,
below bucket 4 at +59. That is a fit artefact of a corpus with few such
positions, and it is the same shape of problem as S121 and S100: not enough
model and not enough data on the positions the term exists for. The endgame row
is monotonic and plausible, which is the control.

Reported for the group: the whole feature +36.1 from nothing at about 2600 --
**unverified**, no source located by the 2026-09-04 literature check -- and
**king proximity alone +22.27 +/- 9.86** at 8+0.08, which is Stash v32's
"king proximity in the passed-pawn evaluation" in `mhouppin/stash-bot`'s
`CHANGELOG.md` (https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md) and is the single largest passed-pawn entry in
the surveyed record. chesso has no king-distance term at all. The +22.3 quoted
here before 2026-09-11 was that figure rounded; the +36.1 stays quoted as
unverified and S186 owns it (DEC-137).
