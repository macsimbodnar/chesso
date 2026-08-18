id:         S100
goal:       the five evaluation terms shipped at zero weight are re-examined against the current fit and the current search
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
