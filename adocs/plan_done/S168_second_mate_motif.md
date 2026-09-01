id:         S168
goal:       a second mate motif, with a mating piece that is not a queen, joins the constructed set and is proved the same way
accepts:    `adocs/data/S145_mate_set.tsv` carries a second motif -- a smothered or otherwise knight-delivered mate, or a back-rank mate, the choice recorded with the reason -- so `adocs/data/S155_motif_census.py` reports more than one mating force and more than two material signatures; every added position satisfies the hazard the set exists for, the mated side materially ahead and not in check at each guarded defender node, and each is proved twice by oracles that are not chesso, an AND/OR enumeration iterative-deepening in the distance so the answer is exact and stockfish at a node limit, 0 disagreements; the added positions are produced by an extension of `adocs/data/S145_mate_set.py generate`, so the whole file regenerates and `verify` re-proves it from scratch; the reverse-futility sweep is re-run over the enlarged set and the two floors in `tests/test_engine.cpp` -- every mate in two on time, `MATE_IN_THREE_FLOOR` -- are restated from the new numbers or kept with the measurement that says they may be; the added cost to the fast suite is measured and stated, since the suite runs at every step completion; the motif list S155 wrote into `tests/test_engine.cpp`, `adocs/specs.md` and `S145_mate_set.py` is corrected to what the set then is
touches:    adocs/data/, tests/test_engine.cpp, adocs/specs.md, DEV_MANUAL.md, MANUAL.md
excludes:   changing `RfpMinPly` or `RfpMaxDepth`, which is S148; the mined breadth set, which is S156; removing or replacing any of the 48 positions S145 landed; per-position pass/fail over mined mates, which S145's research shows is what made two surveyed projects disable their mate tests
decisions:  DEC-114, DEC-117, DEC-016, DEC-095
closes:
blocks:
paused_by:
author:     agent, 2026-09-01
done:       2026-09-01. **The set is three motifs and 82 positions**, up from
            one and 48: a lone rook in 32 rows, two knights in 2, the queen's 48
            untouched. `S155_motif_census.py` reports five material signatures,
            three mating forces and leads 760, 1160 and 1020 -- the accepts
            asked for more than one force and more than two signatures.
            `verify` re-proved all 82 from scratch, **0 checks failed**, every
            shorter distance re-refuted by the AND/OR enumeration and stockfish
            corroborating 81 of 82 at 4000000 nodes; `adocs/data/S168_verify.log`.
            **The floor is 11**, re-derived per DEC-116 because both ends moved
            -- 12 shipping, 10 with the guard removed -- and `REQUIRE( 10 >= 11 )`
            was observed red in a worktree. The mate-in-two clause got stronger:
            26 of 26 on time at the shipping guard, 15 of 26 at `RfpMinPly` 1,
            so eleven positions fail it where seven did. Cost: **0.95 s against
            0.79 s** for the gate, three runs on an idle machine, in a fast
            suite of 42.49 s that is 22 of 22 green. **Three findings the step
            was not queued for, each a measurement that refuted an argument**:
            a king and two knights does not give a knight mate by construction
            and 12 of the first 14 were pawn mates, so the mating piece is
            enforced now; "no pawn may move on the line" refuses the motif
            rather than the defect and promotions are what separate; the rook
            mate lands on the back rank in 13 of 32, so the family is named for
            its force. **Three portability bugs found and fixed in the same
            file**, none of which any test covered: a hard-coded
            `/usr/games/stockfish` in three scripts, `int.bit_count()` needing
            python 3.10 against this machine's 3.9.6, and a version-bound
            proposer that makes a full `generate` replace rows rather than add
            them -- `--only` is the fix and is how a motif is added now.
            No engine source touched, no default changed, no SPRT owed. DEC-117.

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

## The shape was chosen by measurement, 2026-09-01

Four candidate walls and forces were sampled on this machine before anything was
written into the set, one family each at the tracked seed and the tracked
6000-try budget. The wall is S145's, and the "pocket" variant adds one pawn pair
on the h-file -- a defender pawn on h7 blocked by an attacker pawn on h6, which
also covers g7 -- so the corner closes without giving either side a capture.

| candidate | past the cheap filter | proposed | accepted |
|---|---|---|---|
| `K+N+N`, S145 wall | 946 | 9 | **none** |
| `K+N+N`, pocket wall | 1013 | 11 | mate 3 x2, mate 4 x1, mate 5 x1 |
| `K+R`, pocket wall | 519 | 27 | 2 at every distance from 2 to 5 |
| `K+R`, S145 wall | 172 | 16 | 2 at every distance from 2 to 5 |

So two knights over S145's own wall build nothing, and the pocket is what makes
a knight mate constructible at all. That is the measurement the wall change
rests on; it is not a preference for a shape.

**Every accepted candidate was then run through the engine**, at the depth the
gate uses -- `2m - 1 + 8` -- on the shipping build and on the tune build with
the guard relaxed. The iteration the proved distance first appears at, `-` for
never:

| | shipping | `RfpMinPly=5` | `RfpMaxDepth=0` |
|---|---|---|---|
| the four new mates in two | 3, on time | 3 | 3 |
| the four new mates in three | – – – – | 5, 5, 5, 7 | the same |
| the five new mates in four | – | – | 4 of 5 found |
| the four new mates in five | – | – | 3 of 4 found |

Which is the reason both forces are worth having. **Every one of those four
candidate mates in three is hidden at the shipping guard and found at the first
iteration that can hold it once the ply floor rises**, and the new mates in two
are found on time today, so the gate's strongest assertion takes a second mating
piece without being weakened.

**What the shipped rows actually do to the floor is smaller than that probe
suggested, and it is measured below rather than carried over from it.** Over the
eight rook mates in three that reached the file, the counts are 3 at the
shipping guard, 3 with the guard removed and 7 at `RfpMinPly` 5 -- so they lift
both ends of the floor by a constant and do not widen the separation. The probe
drew four positions and they were not representative of the eight; recorded that
way round because the probe is what the choice was made on.

**The choice, and it is the owner's, 2026-09-01: both forces over the one new
wall.** A king and two knights, the axis DEC-114 names first and the only mating
piece in the set that does not move on a line. And a king and rook, the force
that fills every mate distance including two.

## What the construction did not give for free

The first version of the knight motif was written on the argument that a king
and two knights can only ever give a knight check, so the mating piece would be
a knight *by construction*. **That argument is wrong and the tooling is what
said so.** Walking the recorded line of all 46 new rows and asking which piece
delivers the mate:

| family | mating piece | count |
|---|---|---|
| the rook families | rook | 32 of 32 |
| the knight families | **pawn** | **12 of 14** |
| the knight families | knight | 2 of 14 |

A mobile knight standing beside a wall pawn unfreezes the capture the
non-adjacent files deny, and the freed pawn queens with check. So twelve of the
fourteen were promotion mates wearing a knight motif's name, and the wall those
positions rest on is not frozen at all.

The fix is in the generator and not in the prose: a motif may now declare
`mates_with`, and a candidate is refused unless **every** move that mates at the
end of its line is that piece. The rook families declare nothing and are
unchanged, because the same walk found their mating piece is a rook in 32 of 32.

**What the second half of that rule should be was itself a measurement, and the
first answer was wrong.** It was written as "and no pawn may move anywhere along
the line", on the reasoning that a frozen wall cannot hand the mate away. Run
that way, all four knight families accepted **nothing** out of 113 proposals.
The reason is in the fourteen rows it had already produced: **all fourteen have
pawn moves on their line**, 4 to 12 of them, the two knight mates included -- so
that clause refuses the motif rather than the defect. Counting *promotions*
instead separates perfectly:

| the fourteen, walked | mating piece | promotions available on the line |
|---|---|---|
| 12 rows | pawn | 4 each |
| 2 rows | knight | **0** |

A pawn on this wall is three ranks from queening and gets there only through the
freeing capture that unfroze it, so "a promotion is available on this line" is
the same event as "the mate can be stolen by one" and "a pawn can move" is not.
The shipped rule is the mating piece plus zero promotions on the line.

The other half of the same measurement renamed a family. Only **13 of the 32**
rook mates land on the mated side's own back rank; 19 do not. `backrank` was
therefore a claim the file could not carry, and the families are named `rook`
for their force. DEC-114 asked for "a back-rank mate" and what the construction
actually yields is a lone-rook mate that is sometimes one -- recorded here
rather than papered over with a label.
