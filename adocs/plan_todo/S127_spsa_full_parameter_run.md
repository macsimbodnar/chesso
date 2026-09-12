id:         S127
goal:       an SPSA run over the whole search parameter set as it stands after the search block, and an independent SPRT of what it returns
accepts:    the run is over the full set in src/search_params.hpp, which by then includes every margin, threshold and blend weight the search block added; the objective is games and the verdict is an **independent** SPRT against the incumbent, not the SPSA's own score; the run is detached with a terminal marker and a watcher that exits on it (AGENTS.md section 12); the shipped values are what the run returned, and any that agree with a published seed are noted as confirmations (DEC-084)
touches:    tools/, src/search_params.hpp
excludes:   the driver itself, which is S084; the first run over the pre-block set, which is S085
decisions:  DEC-084, DEC-041, DEC-139
closes:
blocks:
paused_by:
done:

## Two runs, not one

The old plan put SPSA last on the argument that it "tunes the constants every
step above it adds, so it cannot precede them". That is right about this run
and wrong about the first one, which is why S084 and S085 moved to the front.

The evidence that the *existing* set is mis-set, measured 2026-08-19 on the
tune build at depth 12: `MaxQsearchDepth` **was** 8 when this was written and
**the bound binds**. S085 shipped 19 on 2026-08-21, so the sweep below is the
argument that moved it and not a claim about what compiles today. Raised
to 16 the Ruy Lopez position after `e4 e5 Nf3 Nc6 Bb5 a6` drops from 1038972 to
940880 nodes -- 9.4 % fewer -- and its score moves from 20 to 33 with a
different line; kiwipete goes the other way, 5167100 to 6061763; the endgame
position is unchanged. 16 against 32 is identical, so the natural depth is
under 16. The parameter is load-bearing, position-dependent and has never been
fitted. It is one of twenty in that file in the same condition.

## Deferred here 2026-09-05: a `NodesTime` clock for the sweep

R12 of `adocs/testing_strategy.md` (DEC-139): a `NodesTime` option in the tune
build, Stockfish's `nodestime` shape -- the clock, the increment and the move
overhead converted to nodes, so the soft and hard limits and the S089/S132
scaling still run on a clock hardware cannot disturb. fishtest uses it for
SPSA "removing noise introduced by inconsistent hardware speed", only "if the
value you're tuning is not susceptible to change significantly the nps". The
owner deferred the decision to this step's design: when the run is sized,
state whether it plays wall time or node time, and if node time, that the
verdict SPRT is wall time regardless and that the bias -- a change that spends
time to save nodes looks better than it plays -- is accepted for the sweep
only. Half a day to build, behaviour-neutral in the shipping build, proved on
the bench signature.

## Amended 2026-09-12: `TmHardPercent` is out of the set, DEC-200

S214's `spsa_driver.py check` probes every axis for reachability and fails by
name when both bounds search identically. Over the full 28-axis set it reached
27 and named **`TmHardPercent`** as the one no node-count probe reaches: at the
shipped `TmSoftPercent` the scaled soft limit never exceeds the hard one, so
the hard timer fires only on an iteration overrunning its start by more than
11 %, and depths 9 and 13, three positions and five clock forms all tied.
"The full set in `src/search_params.hpp`" in the goal line therefore reads as
**27 axes**: this run's config excludes `TmHardPercent` with DEC-200 in its
comment, and `check` is the gate the config passes before the run starts. Its
value is decided by a direct SPRT, the way S089 decided the time manager, or by
a step that first makes it bind at the probe's control.
