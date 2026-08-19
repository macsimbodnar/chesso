id:         S106
goal:       the transposition bound signs and the mate-score round trip are checked against a red test in both searches, not assumed
accepts:    a test that fails when the alpha and beta bound conditions in `tt_entry_answers` are swapped, and a second that fails when either arm of the normalize/de_normalize pair drops its ply term, both observed red and the printout recorded; the tests cover **quiescence as well as the main search**, since quiescence is the only writer that reaches +/-MATE_MAX; a test that a mate found at ply p and read back at ply q reports the same distance to mate; whatever the sweep finds is fixed in this step under the house rule that a found bug is fixed before anything else starts, and each fix gets its own verdict; if nothing is found, "nothing found" is the recorded outcome and the tests stay
touches:    tests/test_search.cpp, src/search.cpp, src/transposition_table.cpp
excludes:   the entry layout, which is S119; any new pruning rule
decisions:  DEC-025
closes:
blocks:
paused_by:
done:

## Why this is cheap insurance and goes early

Two published failures in this exact area are worth more than most features on
this plan:

- One engine had the upper and lower bound inverted in its quiescence store.
  Fixing one line moved it from 44.5 % to 53.7 % over 2000 games -- about
  **64 Elo** from a defect no test caught.
- Mate scores not adjusted by ply on **both** store and probe produce "mate in
  N" announced forever while the engine shuffles. Reported at **100+ Elo** in
  the affected endgame class.

This is not hypothetical here. S094 found exactly one of these: `de_normalize_score()`
excluded +/-MATE_MAX, quiescence is the only writer that reaches it, and a mate
with no legal reply read back **three plies short** until the bound was made
inclusive. One defect of this shape has already shipped in this file. The prior
that there is another is not low, and every measurement taken above a live one
is contaminated.
