id:         S168
goal:       a second mate motif, with a mating piece that is not a queen, joins the constructed set and is proved the same way
accepts:    `adocs/data/S145_mate_set.tsv` carries a second motif -- a smothered or otherwise knight-delivered mate, or a back-rank mate, the choice recorded with the reason -- so `adocs/data/S155_motif_census.py` reports more than one mating force and more than two material signatures; every added position satisfies the hazard the set exists for, the mated side materially ahead and not in check at each guarded defender node, and each is proved twice by oracles that are not chesso, an AND/OR enumeration iterative-deepening in the distance so the answer is exact and stockfish at a node limit, 0 disagreements; the added positions are produced by an extension of `adocs/data/S145_mate_set.py generate`, so the whole file regenerates and `verify` re-proves it from scratch; the reverse-futility sweep is re-run over the enlarged set and the two floors in `tests/test_engine.cpp` -- every mate in two on time, `MATE_IN_THREE_FLOOR` -- are restated from the new numbers or kept with the measurement that says they may be; the added cost to the fast suite is measured and stated, since the suite runs at every step completion; the motif list S155 wrote into `tests/test_engine.cpp`, `adocs/specs.md` and `S145_mate_set.py` is corrected to what the set then is
touches:    adocs/data/, tests/test_engine.cpp, adocs/specs.md
excludes:   changing `RfpMinPly` or `RfpMaxDepth`, which is S148; the mined breadth set, which is S156; removing or replacing any of the 48 positions S145 landed; per-position pass/fail over mined mates, which S145's research shows is what made two surveyed projects disable their mate tests
decisions:  DEC-114, DEC-016, DEC-095
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S155 counted the constructed set and the count is the argument: 48 positions,
**two material signatures and one is the colour mirror of the other**, a **lone
queen as the mating force in 48 of 48**, `lead` 760 in 48 of 48, and eight
family labels that are one geometry under two file shifts, a mirror and a
colour swap. Broad in mate distance -- 16 / 16 / 8 / 8 over distances two to
five -- and narrow in everything else.

So a pruning rule that hides a mate delivered by a knight, or on the back rank,
or at the end of a king hunt, passes the whole gate. The owner's answer on
2026-09-01 to S155's question was that a second motif **is** worth constructing,
and that it is its own step because a new family owes its own two proofs and its
own sweep. That is DEC-114, which also records why answering *no* on S145's
distance measurement was rejected.

## What makes this harder than it looks

The hazard bounds the shape and that is why the set came out monocultural in the
first place. Reverse futility returns a static score instead of searching when
`static_score - RFP_MARGIN * depth >= beta`, at a non-PV node that is not in
check and at ply >= `RFP_MIN_PLY`. For a node to be hidden it must be a forced
loss for the side to move **while that side is materially ahead** and not in
check. A smothered mate is the promising second shape precisely because its
mechanism is the mated king's own pieces: the defender can own a large frozen
force by construction rather than by a blocked wall, and the mating piece is a
knight, which is the axis the current set has none of. A back-rank mate is the
cheaper second shape and shares less with the first: the pocket is made by the
defender's own pawns rather than by the attacker's.

Whichever is chosen, the immobility that makes exhaustive proof affordable at
mate in four and five has to be rebuilt for the new shape; S145's construction
gets it from the blocked wall and that wall is what is being replaced.
