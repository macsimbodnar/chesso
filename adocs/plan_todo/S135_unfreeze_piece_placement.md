id:         S135
goal:       unfreeze the piece placement group and refit it, one bundled SPRT over the three remaining features, by the owner's decision of 2026-08-20
accepts:    S134 has landed first, so the group is three identified features and carries no unidentified column -- a bundled verdict over this bundle is at least attributable, which S027's was not; the fit is `--only piece_placement` on the corpus S082 and S083 produce, so the SPRT measures the term and not a joint refit of the other 820 constants; the SPRT bounds are chosen for the effect size and stated in advance, not `--fast` -- DEC-063 is the measurement that the pair and not the hardware sets the cost, and the published parts are +8.2 and +9.86 (DEC-084, order of magnitude only); the term's own speed cost is inside its verdict, measured with the weights forced non-zero because at zero the compiler deletes it (DEC-047); a verdict of zero is recorded as zero, and **if the bundle fails it is bisected** rather than zeroed by hand, which is the S027 failure this step exists not to repeat (DEC-082's bisect rule); the fast suite green
touches:    src/evaluation.cpp piece_placement_mg/eg, the tuner's freeze list in whatever script runs the fit, adocs/testing.md
excludes:   tempo, which is S136; the seventh-rank feature, deleted at S134; any change to how the three features are computed
decisions:  DEC-091, DEC-057, DEC-063, DEC-084
closes:
blocks:
paused_by:
done:

## What S100 established, and what it did not

Refuted for all three of these features: a wrong count (10795695 rows
re-extracted, 0 disagreements), a wrong gradient (worst 4.9e-8 over all 827
parameters), a pipeline that cannot recover a planted vector, a coverage desert
(bishop pair 15.81 %, rook open 31.38 %, rook half-open 31.26 % of rows), and
redundancy with the tables (R^2 0.487, 0.265, 0.168 -- against 0.99 for
redundancy, on the widest right-hand side the model offers).

Not refuted, and the operative cause: **these three have never been separately
measured.** They shared one SPRT with the seventh-rank feature at `--fast`
bounds `elo0=0 elo1=10`, returned -5.48 +/- 11.46 over 2284 games, and were then
zeroed **by hand**; every fit since S065 has held them there with
`--freeze piece_placement` (DEC-057). The freeze's stated reason was that S027
verdict, and S100 calls it procedural.

**Still unresolved and not refuted:** the correlation form of the corpus
hypothesis -- chesso's self-play cannot show the value of a feature chesso
weights at zero. That is why this step's fit is on S082's corpus and not on the
outgoing one.

## The owner's decision, and the concern it overrides

The agent's recommendation was to split the group and measure the bishop pair
alone first: it is the free feature -- the compiler rewrites
`count_bits(x) >= 2` into `x & (x - 1)` -- it carried the largest fitted weight
of the four, and the three rook features cost 3.1 to 4.0 % of a search between
them, so a bundle prices a cheap term that may work together with expensive ones
that may not.

**The owner chose one bundled fit and one SPRT over all of them** (DEC-091). The
concern is recorded, not re-argued. Two things make this bundle materially better
than S027's, and they are why the choice is defensible: S134 removes the one
member that was not an identified quantity, and the bisect-on-failure rule means
a negative verdict is followed by attribution rather than by a hand revert.

## Measurement

One SPRT, bounds stated before the run. DEC-063 is the constraint that decides
whether the night buys a verdict at all: the same constant took 6 h 36 m over
9036 games for nothing at `elo0=0 elo1=5` and returned H1 in 1 h 41 m at
`elo0=-5 elo1=5`. At S105's measured 23.1 to 38.7 games a minute that is roughly
1.5 to 3 hours for a resolving pair.
