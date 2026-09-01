id:         S082
goal:       the corpus labels a resolved position rather than the root -- the quiescence leaf, or the leaf reached by playing out a deep search's whole principal variation -- and samples few positions per game rather than many
accepts:    `tools/datagen` records the position at the leaf quiescence resolves to, with the game result unchanged as the label, and states in its own output how often the leaf differs from the root; the tactical-move filter clause is retired rather than flagged, since its whole justification was that the root might not be quiet; a corpus is regenerated, refitted, and **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is; the leaf is verified to be quiet by construction -- a test asserts that the recorded position has no capture the quiescence search would still make, and fails if the walk is truncated by the depth bound instead
touches:    tools/datagen.cpp, src/search.cpp or src/search.hpp for the leaf walk, tests/, .tuning/
excludes:   the corpus size and the node budget, which are S083's; dedupe, which is S076's; the label blend, which is S075's; any change to what quiescence itself does
decisions:  DEC-055, DEC-041
closes:
blocks:
paused_by:
done:

## Why this exists

`adocs/eval_tuning_strategy.md` calls this "the critical design decision" and
answers it in section 2.3: label the **quiescence search score at a quiet leaf**,
because "the eval function is only well-defined on quiet positions, so the target
must be anchored at a quiescent leaf". Section 2.6 says the same as a filter:
"Drop positions where the qsearch leaf differs from the root (or rather: label
the leaf, not the root)."

chesso labels the root and works around the consequence. `tools/datagen.cpp`
records `generate_FEN(&game.board)` at the position played, and its fourth filter
clause drops any position whose best move is a capture or a promotion -- an
approximation of quietness that DEC-055 already found wanting for the opposite
reason:

> That fourth clause was justified here by the claim that evaluate() is only ever
> asked about a position quiescence has already resolved. It is not:
> src/search.cpp:349 calls evaluate_lazy() at the top of every quiescence node,
> before a single capture is generated.

S065 loosened the clause behind `--allow-tactical` and regenerated with it on, so
today's corpus contains tactical roots labelled as if they were quiet. Anchoring
at the leaf answers the question the flag was a compromise on: the leaf is quiet
by construction, so nothing has to be dropped and nothing has to be admitted.

## What it changes about the fit

The tuner's float model evaluates the row it is given. Move the row to the leaf
and the model is fitted on the positions `evaluate()` is actually asked about,
which is the whole argument. It also removes a bias nobody has measured: a filter
that drops every position whose best move is a capture removes sharp positions
from the fit and keeps their outcomes, while `--allow-tactical` keeps them and
mislabels them.

## The trap

`MAX_QSEARCH_DEPTH` is 8 (`src/search.cpp:27`). A leaf reached by exhausting that
bound is not quiet, it is truncated, and recording it puts back exactly the noise
this step removes. The gate asks for a test that separates the two, and datagen
should count the truncated ones rather than silently keep them.

Second trap: the leaf of a search is not the leaf of a plain quiescence call from
the root. Which one is recorded has to be stated -- the principal variation's
quiescent end, or a quiescence run from the played position -- because the two
differ and only one of them is reproducible from the FEN alone.

## Cost

One night of generation, on the S065 precedent: 120000 games at 100000 nodes took
about eight hours for 11.0 M rows. One fit, minutes. One SPRT, three to four and
a half hours.


## Widened 2026-08-19

Two additions from the published method, both cheap once datagen is being
changed anyway.

**Resolve by playing out the principal variation.** Search the sampled position
deeply, play the **whole** PV, and store the leaf. This makes the position
quiet by construction rather than by filter, which is what the quiescence-leaf
label is reaching for by a shorter route. It costs some label precision -- the
game result is now attached to a position several plies from where it was
sampled -- and the published assessment is that the diversity and the
resolution are worth more than the precision.

**Sample few positions per game.** The same source states plainly that many
positions from one game **lowers** dataset quality. The corpus this engine
fits on is about 92 rows per game by construction -- S066 fixed the splitter
that this broke, and the underlying density was never revisited. Two to four
rows a game is the published practice. That interacts with S083's fifty
million: fifty million rows at four a game is twelve and a half million games,
which is a datagen cost this step has to price before S083 commits to it.
