id:         S089
goal:       a time budget that scales with best-move stability and with a falling score, instead of remaining over a fixed movestogo plus half the increment
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); a soft limit that decides whether to start another iteration and a hard limit that stops the search inside one, with the hard limit never exceeding the time the clock actually has and a test that asserts it on a short clock; the budget scales by how long the best move has been stable and by a score that has fallen since the previous iteration, each factor a constant in src/search_params.hpp with a stated range (S073); no fixed movestogo assumption at a sudden-death control; tests/test_search.cpp keeps a case that a 1 ms clock returns a legal move, which is the S036 defect; the fast suite green
touches:    src/chesso.cpp compute_search_time_ms and the iterative deepening loop, src/uci.hpp DEFAULT_MOVES_TO_GO, src/search_params.hpp, tests/test_search.cpp
excludes:   pondering; multi-threading; any change to what the search itself computes
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## What is there now

`src/chesso.cpp:405-410`:

    budget = (remaining_ms / movestogo) + (increment_ms / 2)

with `DEFAULT_MOVES_TO_GO 20` (`src/uci.hpp:13`) used when the GUI sends no
`movestogo`, and a soft-limit percentage at `chesso.cpp:589-592` deciding at
`:761` whether to begin another iteration. Nothing looks at the search at all:
a position whose best move has been the same since depth 6 gets the same slice
as one where it changed at every iteration, and a score that just fell 200 cp
buys no extra time.

The whole match record of this project, including the 2570, was played on that
allocation.

## Why it is first of the search block

It is the only item on the DEC-071 list that costs no search correctness risk:
it prunes nothing, reduces nothing and cannot hide a mate. It also interacts
with nothing else queued, so its verdict is clean whenever it runs.

## Hazard

The failure mode is losing on time, and a forfeit is a whole point rather than
noise. The hard limit is asserted by a test, not by argument, and the SPRT log
is checked for time losses before the verdict is read.
