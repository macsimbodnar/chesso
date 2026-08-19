id:         S130
goal:       quiescence takes the table score as its stand-pat where the stored bound allows it, instead of the static evaluation alone
accepts:    an SPRT verdict, recorded whatever it is; the table score is used only where its bound makes it valid -- a lower bound may only raise the stand-pat, an upper bound may only lower it, an exact score replaces it -- with a unit test per bound type; a score in the mate band is never used as a stand-pat, tested, because a stand-pat must not become a mate claim; the entry's eval field keeps its S094 semantics -- what is stored there is still the static score and never the improved stand-pat, for the same reason a bound is never stored (an improved stand-pat is window-relative through its bound test); the fast suite green
touches:    src/search.cpp quiescence, tests/test_search.cpp
excludes:   per-move futility, which is S112; delta pruning, which is S022; any change to what is stored
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
done:

## Created by the second review, DEC-087

S094 put the probe in quiescence and measured the probe alone at zero; S103
found the stored evaluation's consumer in the main search. This is the
consumer inside quiescence itself, and it is the one with the band's best
numbers: Weiss measured "use ttScore instead of static eval when the bound
permits" at **+10.8 / +12.1 / +21.4** across its runs (cca90ea7), and
Ethereal's equivalent probe-and-use family at +11.0/+3.1/+2.6 (25e56feb).

The probe is already paid for at the top of `quiescence()` and the bound
logic already exists in `tt_entry_answers()` -- what is missing is three
lines between them: where the entry did not answer the node outright, its
score is still a bound on this position, and a stand-pat tightened by a valid
bound prunes more and prunes honestly. `de_normalize_score()` applies on the
way out, as everywhere.
