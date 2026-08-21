id:         S148
goal:       the reverse futility depth ceiling is re-decided against the deep mates S145 measured it losing, by SPRT and not by argument
accepts:    `RFP_MAX_DEPTH`'s default is decided by an SPRT of the shipping 15 against at least one lower value, with the mate-finding cost of each stated from `adocs/data/S145_rfp_sweep.py` in the same step -- a verdict of "keep 15" is a valid outcome and is recorded as one; `RfpMinPly` held at its shipping value throughout, because S145 measured the two bounds substituting for each other and a run that moves both attributes nothing; the mate in four and five counts in `tests/test_engine.cpp`'s `engine: mate safety` suite promoted from the recorded `MESSAGE` to an asserted floor if and only if the shipped value makes them non-zero, and left recorded if it does not; `adocs/data/S145_rfp_sweep.log` re-run at the shipped value; specs.md's reverse futility sentence and MANUAL.md's known-bug entry carry the decided number
touches:    src/search_params.hpp, tests/test_engine.cpp, adocs/data/, adocs/specs.md, MANUAL.md, adocs/decisions.md
excludes:   `RFP_MIN_PLY`, which S142 settles and which this step holds fixed; the constructed set itself, which S145 built and which this step only measures against; making the depth ceiling a function of the score the way Stockfish's `fa8b6add` does, which is a different feature and would need its own step and its own verdict; the principal-variation truncation, which is S147
decisions:  DEC-019, DEC-063, DEC-095
closes:
blocks:
paused_by:
author:     Maksym Bodnar

## What S145 measured, and why this is a trade rather than a fix

The full table is `adocs/data/S145_rfp_sweep.log`. `RfpMinPly` held at its
shipping 3, over the 48 proved mates of `adocs/data/S145_mate_set.tsv`, each
searched under the engine's own iterative deepening to `2m - 1 + 8`:

| `RfpMaxDepth` | exact | mate 2 | mate 3 | mate 4 | mate 5 |
|---|---|---|---|---|---|
| 0 | 34/48 | 16/16 | 11/16 | **4/8** | **3/8** |
| 3 | 32/48 | 16/16 | 11/16 | 3/8 | 2/8 |
| 6 | 27/48 | 16/16 | 10/16 | 1/8 | 0/8 |
| 10 | 24/48 | 16/16 | 8/16 | 0/8 | 0/8 |
| **15, shipping** | 24/48 | 16/16 | 8/16 | **0/8** | **0/8** |
| 63 | 24/48 | 16/16 | 8/16 | 0/8 | 0/8 |

Monotone, and the deep classes are the whole difference. The mate in two class
is 16 of 16 at every value, which is why nothing in the suite before S145 could
see this: all three of the old gate's cases were mates in two. S033 shipped 6 --
one mate in four and no mate in five -- and S085's SPSA run moved it to 15,
where both classes are empty. 10 is already indistinguishable from 63.

**And that move was not free in the other direction.** S085's returned vector
was SPRT-verified at **+21.02 Elo** with `RfpMaxDepth` 15 in it, so lowering the
bound gives back a share of a measured gain to buy a mate-finding property whose
Elo value is unmeasured. That is exactly the trade DEC-019 says a published
figure decides what to try and never what to conclude, so it is an SPRT.

**The published record says the trade is real and points the other way from
S085.** Stockfish removed its reverse-futility and parent-futility depth limits
in July 2021 (`09b6d283`, `dbd7f602`); both passed non-regression SPRT at STC
and LTC, and mates found on ChestUCI at 1M nodes fell from **2427 to 1246**.
Restoring either condition alone recovered only 1282 and 1630, so both were
needed, and the change was reverted at `dabaf222`. The source has carried "The
depth condition is important for mate finding" ever since. Master now makes the
bound a function of how close the score is to decisive -- 19 at small scores
decaying to 13 near the mate band, `fa8b6add` -- so the pruning depth shrinks
where mates live. That last shape is excluded here: it is a feature, not a
constant, and it deserves its own verdict.

## What is not claimed

That the engine is losing rating to this. Nothing here measures that, and the
possibility that a mate found four iterations later costs nothing at 8+0.08 is
exactly why the step exists. What is established is that the shipping value has
a cost that was invisible before S145 built a set with deep mates in it, and
that S142's "no defect was demonstrated against 15" is no longer true as a
statement about mate finding.
