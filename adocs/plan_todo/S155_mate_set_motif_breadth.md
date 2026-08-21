id:         S155
goal:       the constructed mate set's single motif is stated where its breadth is claimed, and what the gate therefore cannot catch is written down
accepts:    the constructed set's single motif is stated wherever its breadth is claimed -- every one of the 48 positions is a blocked pawn wall with a rook-bishop-rook-bishop battery, one queen and `lead` 760, varying only by `shift0`/`shift2`, mirror and colour -- and the reason it is close to forced by the S033 construction is stated with it, so the qualifier does not read as a defect in the set; what the gate therefore cannot catch is written down as a list rather than implied: back-rank, smothered, king-hunt and open-line mates, and any position with a realistic material balance; whether a second motif is worth constructing is answered either way and the answer is recorded; no default changes and no position in the existing set is removed
touches:    tests/test_engine.cpp, adocs/specs.md, adocs/data/
excludes:   rewriting `adocs/plan_done/S145_mate_safety_test_set.md`; removing or replacing any of the 48 positions, every one of which was independently re-proved a forced mate at its claimed distance; the mined set, which is S156
decisions:  DEC-016, DEC-095
closes:     2026-08-21_adversarial-F07
blocks:
paused_by:
done:

## What was verified, so this is not read as doubt about the set

All 48 positions were re-proved from scratch by two oracles written without
reference to the repository script: 48 of 48 by an AND/OR enumeration iterated to
distance 6, and 48 of 48 by stockfish at 20 M nodes, 0 disagreements. The
construction regenerates the tracked TSV byte-identically. The set is sound. What
is missing is the qualifier beside the breadth claim.
