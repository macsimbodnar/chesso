id:         S127
goal:       an SPSA run over the whole search parameter set as it stands after the search block, and an independent SPRT of what it returns
accepts:    the run is over the full set in src/search_params.hpp, which by then includes every margin, threshold and blend weight the search block added; the objective is games and the verdict is an **independent** SPRT against the incumbent, not the SPSA's own score; the run is detached with a terminal marker and a watcher that exits on it (AGENTS.md section 12); the shipped values are what the run returned, and any that agree with a published seed are noted as confirmations (DEC-084)
touches:    tools/, src/search_params.hpp
excludes:   the driver itself, which is S084; the first run over the pre-block set, which is S085
decisions:  DEC-084, DEC-041
closes:
blocks:
paused_by:
done:

## Two runs, not one

The old plan put SPSA last on the argument that it "tunes the constants every
step above it adds, so it cannot precede them". That is right about this run
and wrong about the first one, which is why S084 and S085 moved to the front.

The evidence that the *existing* set is mis-set, measured 2026-08-19 on the
tune build at depth 12: `MaxQsearchDepth` is 8 and **the bound binds**. Raised
to 16 the Ruy Lopez position after `e4 e5 Nf3 Nc6 Bb5 a6` drops from 1038972 to
940880 nodes -- 9.4 % fewer -- and its score moves from 20 to 33 with a
different line; kiwipete goes the other way, 5167100 to 6061763; the endgame
position is unchanged. 16 against 32 is identical, so the natural depth is
under 16. The parameter is load-bearing, position-dependent and has never been
fitted. It is one of twenty in that file in the same condition.
