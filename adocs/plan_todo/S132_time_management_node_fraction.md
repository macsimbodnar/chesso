id:         S132
goal:       the soft time limit scales with the share of the root's nodes the best move consumed, spending less when the choice is not in doubt
accepts:    an SPRT verdict at an increment control, recorded whatever it is, with the time-forfeit count read from the PGN (the S089 lesson); per-root-move node counting happens at the root loop only, with no per-node cost added to the tree, and a test asserts the per-move counts sum to the iteration's total; the scale formula's constants are in src/search_params.hpp with stated ranges, seeded from the published form and fitted here (DEC-084, S127); a time the GUI named with `go movetime` is still never scaled, and the S089 stability and falling-score scalers are untouched -- this multiplies them, stated in the code where the three meet
touches:    src/chesso.cpp iterative_deepening_search and the root move loop, src/search_params.hpp, tests/
excludes:   the S089 base allocation and its two scalers; the hard limit
decisions:  DEC-071, DEC-084, DEC-087
closes:
blocks:
paused_by:
done:

## Created by the second review, DEC-087

The third soft-limit scaler the surveyed set carries, and the one S089 did not
build: when one root move has eaten most of the iteration's nodes, the search
already knows the answer, and when the nodes are spread the position is
genuinely unclear and worth more clock. Ethereal measured it at **+9.9 / +9.7**
(+20.9 at a cyclic control, 60f4d5c5, crediting the idea to Koivisto), Lynx at
+3.6, and Stormphrax adopted it. Zero search risk: it touches when iterations
start, never what they search. The published shape is a linear scale on
`1 - bestmove_nodes/total_nodes` clamped to a band; the constants ship from
our own sweep and S127, per DEC-084.
