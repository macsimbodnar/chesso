id:         S289
goal:       find by tool why S083's pasted constants hide a mate the parent finds and lose seven exact mined mates, and fix the search rule at fault as its own measured change
accepts:    the node and the rule that hide `tests/test_engine.cpp` "reverse futility at ply 2 does not hide a mate from the side to move"'s mate at S083's candidate constants are named from the engine's own output (a trace, a counter or a one-option-at-a-time bisection in the tune build), never from reading the board (CHESS); the mined mates S156's sweep reads exact on the parent and not on the paste are listed by key and each attributed the same way; if a rule is at fault, its fix ships as its own change with a guard test observed red first at the paste's constants and green at both the paste's and the shipped constants, a mutant that test kills (DEC-141), Debug self-play and gate_extra, every golden it moves re-derived by its script, and its verdict decided by SPRT against its parent if it moves play, bounds pre-registered; if no rule is at fault, a proposed decision stating what the paste trades, for the owner (DEC-304 (11)); the fast suite green in both builds
touches:    <!-- set by the implementer -->
excludes:   S083's constants (no refit, no re-anchor, no golden S083's landing owes); the values of the search's margins beyond what the fix needs (S127 owns them); any lowering of a mate floor
decisions:  DEC-305, DEC-304, DEC-303, DEC-299, DEC-142, DEC-141, DEC-171
closes:
blocks:     S083
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-22, DEC-305)

S083's landing pasted curve 0's candidate (`.tuning/s083/curve/s0/fit_6226944_s1.hpp`,
every game of S082's training side at 46 qs rows a game) onto `5a22a9c` in
`../chesso-s083l` and stopped under DEC-304 (11) -- S083's step file, "the
landing -- stopped at the floors and a hidden mate", and
`adocs/data/S083_rfp_ply2.txt`, `S083_mined_floor.log`, `S083_floor_sweep.log`:

- `EXACT_FLOOR`'s mined set reads 157 exact against the floor 163 (154 with
  the guard weakened); the parent reads 164 and 160.
- The ply-2 property case is red in both builds: through UCI the parent
  reports `mate -2` from depth 4 to 8, the paste cp 1168 to 913 at depths 1 to
  8 and never a mate. One tune option at a time: `NullMoveBase` 16 restores
  the mate at depth 6, `RfpMinPly` 63 at depth 8; `RfpMaxDepth` 0,
  `RfpMargin` 2000, `RazorDepth` 0 and `ProbCut` 0 do not.
- `MATE_IN_THREE_FLOOR` holds (12) but no longer separates (11 weakened).

The paste moves the material scale (PAWN 94 -> 84, KNIGHT 327 -> 371,
BISHOP 308 -> 352, ROOK 487 -> 519, QUEEN 716 -> 710, every table) under
search margins fitted at the old scale (S261's lane). Whether the cause is a
rule that can hide a mate whenever a mated side leads in material by enough
-- reachable at the shipped constants in other positions, which would make
it a BUGS-rule defect (DEC-171) -- or a margin the scale change crossed, is
what this step finds. Every later evaluation refit (S135 onward) meets the
same search, so the answer is wanted once.

The paste itself: `.tuning/coord/S083/s083_landing.diff` (red, not a
commit); the worktree `../chesso-s083l` holds it with both builds made.
