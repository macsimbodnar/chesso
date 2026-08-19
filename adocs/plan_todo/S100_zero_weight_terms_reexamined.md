id:         S100
goal:       find out why five evaluation terms fit to exactly zero -- feature extraction, corpus composition or a real result -- before any further weight is fitted beside them
accepts:    the bishop pair is fitted and measured on its own, separately from the three rook features, which is the split S027's own comment says it never made; every fit runs with the other constants frozen, on the corpus that exists at the time, and reports held-out error before and after; a pre-registered hypothesis pair per SPRT, chosen for a small term rather than left at the default, with DEC-063's evidence that elo0=-5 elo1=5 resolves what elo0=0 elo1=5 does not; a verdict of unresolved is recorded as unresolved and not as zero, which is the distinction S027's tempo comment draws and the ledger row must keep; any term that stays at zero keeps its weights at zero so the compiler still deletes it, and the reason is updated rather than replaced; the fast suite green
touches:    src/evaluation.cpp piece_placement_mg/eg and tempo_mg/eg, tools/ for the fit, adocs/testing.md
excludes:   new terms, which are S101 and S102; any change to how the terms are computed, only to their weights
decisions:  DEC-071, DEC-063
closes:
blocks:
paused_by:
done:

## Why they are worth re-asking

Bishop pair, rook on an open file, rook on a half-open file, rook on the
seventh and tempo are in every hand-crafted engine in the 3000-plus band on the
CCRL list. Here all five ship at zero because they measured zero at S027, and
three things about that measurement have since changed:

1. **The corpus is not the same corpus.** `src/evaluation.cpp:194-198` says so
   itself -- the fit was on self-play by an engine predating all of S027. The
   corpus has since been regenerated (S065), deduplicated by zobrist key
   (S076), and S082 and S083 will move the label to the quiescence leaf and take
   it past 50 M positions.
2. **The search around them is about to change eleven times** (DEC-071). Eval
   parameters are only optimal relative to the search that uses them.
3. **Four of the five were measured as one block.** The comment at
   `evaluation.cpp:186-191` states the case for splitting the pair -- which is
   free and carried the largest fitted weight -- from three rook features that
   cost 3.1 to 4.0 % of a search between them, and then does not split it.

Tempo is the clearest: its SPRT reached neither bound, `-0.69 +/- 9.64` over the
full 3000 games. That is unresolved, not zero, and the code says so.


## Re-targeted 2026-08-19, and moved to the front

This was "re-examine the five terms against the current fit". It is now a
**diagnostic**, and it goes early rather than at position 84, because five
terms fitting to exactly zero is not a plausible outcome of a correct fit and
whatever causes it is also acting on the other 817 constants.

What ships at zero: bishop pair, rook on an open file, rook on a half-open
file, rook on the seventh, and tempo. Published fits of the same terms measure
bishop pair **+16.7**, rook and queen on the seventh **+9.2**, rook on an open
file **+4.2**, tempo **+12.99**. Zero for all five at once is a pattern, not
five results.

Three candidate causes, and the step's job is to tell them apart rather than to
pick one:

1. **Feature extraction.** The counts are wrong, so the fit is regressing on a
   column that does not mean what its name says. `piece_placement_counts()` is
   exposed for the test and the test can only check counts while the weights
   are zero -- which is a circular guard.
2. **Corpus composition.** The corpus is chesso's own self-play, and chesso
   values none of these terms at zero weight. An engine that never keeps the
   bishop pair on purpose generates few positions where keeping it
   discriminates. This is the documented failure mode for king safety and the
   same mechanism applies here.
3. **A real result** -- the tapered piece-square tables already absorb them.
   Possible, and it is the answer only after the first two are excluded.

`passed_pawn_mg` is the fourth data point: `{0, -6, -4, 19, 59, -17}`, with the
seventh-rank bucket fitted **below** the sixth and negative. Whatever explains
that probably explains the five.
