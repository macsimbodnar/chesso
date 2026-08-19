id:         S110
goal:       a second correction table keyed on the non-pawn structure, split by colour
accepts:    an SPRT verdict, recorded whatever it is; the key is maintained incrementally in add_piece, remove_piece and move_piece and is never recomputed in evaluate() -- INV-4, and the 25 % of nps S014 removed is what a recomputation puts back; the corrected score can never cross into the mate or decisive band, and a test asserts it; the table is a constant-sized array with its dimensions and its clamp stated in src/search_params.hpp
touches:    src/search.cpp, src/bitboard.cpp piece primitives, src/data_structures.hpp, src/search_params.hpp
excludes:   the pawn table, which is S099 and lands first; continuation correction, which is S111
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## Reserve, 2026-08-19, DEC-087

Demoted behind the 3000 push by the second review: the non-pawn and
continuation tables measure +3 to +8 and only above ~3100 in the surveyed
record, where the pawn table (S099, which stays in the main order) has
sub-3000 evidence at +11.4. The long-control doubling noted below is one more
reason this family reads better after S128 moves the measurement nearer the
list's control.

## Order, and why it is three steps

The published record measures these separately and they are **not** inert
apart, so DEC-082 does not apply and they do not become one step. Reported, all
short then long time control: pawn +11.29 / +12.40 and +4.87 / +11.70,
non-pawn +6.98 / +12.28 and +2.80 / +6.84, continuation +2.58 / +5.46 and
+2.75 / +5.46.

Two things follow. **The long-control figure is consistently about twice the
short one**, so `--fast` at 8+0.08 will under-measure this family by half --
say so in the verdict rather than reading the point estimate as the effect.
And the whole family is reported to be worth *more* for a hand-crafted
evaluation than for a network, because the residual there is to correct is
larger. That is the one place on this plan where DEC-054's no-NNUE constraint
is an advantage rather than a cost.
