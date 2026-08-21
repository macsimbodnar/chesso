id:         S149
goal:       the second killer slot holds a move distinct from the first, so a repeated fail-high stops destroying it, and the test asserts distinctness rather than non-zeroness
accepts:    the store at `src/search.cpp:761-762` does not copy slot 0 into slot 1 when the move being stored already equals slot 0, so the two slots always hold distinct moves -- the published rule, CPW Killer Heuristic: "The replacement scheme ought to ensure that all the available slots contain different moves"; the duplicate rate is re-counted after the change on the same instrumentation that measured 66 % of stores and 44 % of nodes before it, and reported; `tests/test_search.cpp:477-499` is re-targeted from counting non-zero slots to counting **distinct** slots and is observed red at the parent commit, because the existing assertion passes on a duplicated slot and is the reason this went unseen; play-altering, so INV-6 is not available and it is decided by SPRT -- a verdict of zero is recorded as zero and the guard may still be kept with the reason stated; `adocs/specs.md`'s ordering row states the eligibility and the distinctness rule together
touches:    src/search.cpp, tests/test_search.cpp, adocs/specs.md
excludes:   the killer slot count, which stays at two; the ordering band constants, which CLAUDE.md and `src/search_params.hpp:29` keep out of the tuned set; history malus, gravity and butterfly indexing, which are S093 and which this step must land before rather than merge with
decisions:  DEC-019, INV-6
closes:     2026-08-21_adversarial-F01
blocks:
paused_by:
done:

## Why this is first

AGENTS.md par.0: a bug that has been found gets fixed before anything else
starts. This is a found bug in shipped code, measured at 44.4 % of negamax
nodes, so it precedes S142 and every other pending step.

**It must land before S093.** S093 rewrites this same block. A verdict taken on
S093's history rewrite while the second killer slot is dead attributes to history
whatever the dead slot was costing, and neither number then means anything.
