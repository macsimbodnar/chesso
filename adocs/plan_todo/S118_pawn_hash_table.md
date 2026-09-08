id:         S118
goal:       the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
accepts:    an SPRT verdict if the tree changes and a node-identity check if it does not -- state which before the run; the key is a pawn-only zobrist maintained incrementally in add_piece, remove_piece and move_piece (INV-4), never recomputed in evaluate(); the hit rate is measured over a real search and recorded, not assumed; the table stores the mg/eg pawn score, the passed-pawn bitboard and **both kings' shelter and storm scores**, because the shelter is the expensive half of what this saves; a collision returns a recomputation rather than a wrong score, and a test forces one
touches:    src/evaluation.cpp, src/data_structures.hpp, src/bitboard.cpp piece primitives
excludes:   new pawn terms, which are S125; the passed pawn suite, which is S123
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
done:

## Moved out of the speed block, 2026-08-19, DEC-087

The published record has a warning this step's old position walked into: an
implementer who cached a still-cheap pawn evaluation measured a **10 %
slowdown**, and the reported ~10 % speedups come from engines whose pawn terms
were expensive first. chesso's three pawn terms share four bitboard fills and
are cheap by construction (S027). So this step now lands in the evaluation
block, after S123 and S125 have made the pawn evaluation worth caching, and
directly before S122 reads the shelter and storm slots it adds. The hit-rate
expectation stands: a few thousand entries buy >95 % on the published numbers.

## What it costs today

`evaluate_cheap()` calls `evaluate_pawns()` on **every** evaluation, and that
function does four bitboard fills plus per-pawn work. The figures this paragraph
was written on -- 83.35 ns a call, 12.0 M calls a second, against a search at
5.8 M nodes a second -- were **measured 2026-08-19 before S104** added the
architecture flag, and DEC-083's rule is that nothing taken on the unflagged
binary is comparable with anything taken after it. On the shipping `bmi2`
target `bench_eval` reads **53.90 ns a call, 18.6 M calls a second**
(2026-08-19, S104; `adocs/specs.md`). Either way the conclusion is the one the
ratio carries and not the absolute: the evaluation is a large fraction of the
clock, and the pawn structure is recomputed for a structure that changes on
perhaps one move in eight.

Reported +10.11 Elo and a 10 to 12 % speed-up, with hit rates above 95 %.

**This is not the pattern INV-4 forbids.** INV-4 is about terms rebuilt from
the bitboards at every node; this is a cache keyed on a hash that only moves
when a pawn does. It composes with accumulation instead of replacing it, and it
is what makes S125's richer pawn terms affordable.
