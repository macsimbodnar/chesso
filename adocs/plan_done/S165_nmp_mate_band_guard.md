id:         S165
goal:       null move pruning's missing negative mate-band guard is added and measured, or recorded as deliberate with defender-side evidence
accepts:    the S145 mate sets are swept from the defender's side — engine to move as the side being mated — on the tune build with the machine idle, guard present against guard absent, asserting no mate-delay regression; then exactly one of two endings: the guard `beta > -MATE_MIN` (the mirror of reverse futility's, `src/search.cpp:661-662`) lands with a `--nonreg` verdict, or a decisions.md entry records the asymmetry as deliberate, citing the sweep and the finding; either ending closes the finding
touches:    src/search.cpp, adocs/data/S165_nmp_defender_sweep.py,
            adocs/data/S165_defender_set.tsv, adocs/data/S165_sprt.sh,
            adocs/specs.md, adocs/testing.md, DEV_MANUAL.md
            -- the sweep landed in adocs/data/ beside S145's rather than in
            tools/, because it is one step's instrument over one committed set
            and that is where every S145 sweep already lives; decisions.md is
            untouched because the guard landed, which is the ending that needs
            no decision entry
excludes:   the null-move reduction schedule and eval-scaled reduction — S114 owns those; the fail-high verification search S114 dropped
decisions:
closes:     2026-08-22_adversarial-F04
blocks:
paused_by:
done:      Null move pruning guards both edges of the mate band: beta > -MATE_MIN added, the mirror of reverse futility's. Defender-side sweep over 104 re-proved nodes takes the set from 80/104 exact to 82, short 0 and sign 0 on both, no delay regression. Reachability 0 of 301620 eligible nodes in ordinary play, 3079 of 6252 (49.2 %) inside a mate proof. H1 accepted at elo0=-5 elo1=0 in 18598 games and 7 h 58 m against e918d05, LLR 2.96, Elo 0.95 +/- 3.56, nElo 1.34 +/- 4.99, 50.14 %, 0 forfeits in 18601 -- a regression of 5 or more excluded, no gain claimed. Fast suite 20/20, clang-format clean. Closes 2026-08-22_adversarial-F04.

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
author:    Maksym Bodnar
