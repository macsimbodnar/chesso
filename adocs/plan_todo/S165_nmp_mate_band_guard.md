id:         S165
goal:       null move pruning's missing negative mate-band guard is added and measured, or recorded as deliberate with defender-side evidence
accepts:    the S145 mate sets are swept from the defender's side — engine to move as the side being mated — on the tune build with the machine idle, guard present against guard absent, asserting no mate-delay regression; then exactly one of two endings: the guard `beta > -MATE_MIN` (the mirror of reverse futility's, `src/search.cpp:661-662`) lands with a `--nonreg` verdict, or a decisions.md entry records the asymmetry as deliberate, citing the sweep and the finding; either ending closes the finding
touches:    src/search.cpp, tools/, adocs/decisions.md
excludes:   the null-move reduction schedule and eval-scaled reduction — S114 owns those; the fail-high verification search S114 dropped
decisions:
closes:     2026-08-22_adversarial-F04
blocks:
paused_by:
done:

## Evidence

2026-08-22_adversarial-F04. Reverse futility guards both sides of the mate
band (`beta < MATE_MIN && beta > -MATE_MIN`, `src/search.cpp:661-662`); null
move pruning guards only the positive side (`src/search.cpp:719-721`), and
its fail-high return handles only the positive band. With `beta <=
-MATE_MIN` — reachable at defender nodes inside a mate proof — any null-move
result clears beta, so the node fails high on a reduced search's word. The
reduced search is exactly the instrument that misses mates, the project's
recurring bug class (NMP hid a mate in 2 once already). No comment, decision
or step records why the two rules differ. Exposure is mate-distance
correctness and conversion speed rather than false mates: S145 measured 0
false mates over twelve settings, but that sweep varied RFP, not NMP, and
asserts from the attacker's side.

## Cost

A sweep in minutes on an idle machine, then at most one `--nonreg` verdict —
last in the audit batch because it is the only entry that owes machine time.
