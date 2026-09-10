id:         S133
goal:       the piece-square tables become king-relative -- indexed by a king bucket as well as piece and square -- and every entry is fitted
accepts:    an SPRT verdict, recorded whatever it is; the bucket scheme is stated and deliberately small -- enemy-king-side mirroring (two buckets) or own-king file buckets (up to four), chosen by held-out fit error before any match is played, and the choice recorded with the error figures; INV-4 holds across a king move that crosses a bucket boundary -- the accumulators are rebuilt there, and the existing accumulator-equals-recompute test gains exactly that case; the nps cost of the rebuild is measured and recorded next to the verdict, with the bucket-crossing frequency counted over a real search; tools/eval_model.hpp carries the same buckets, tools/tuner_groups.hpp gains the groups, and the partition properties hold; every entry is fitted on the S082 corpus with the S077 provenance stamp
touches:    src/eval_tables.hpp, src/evaluation.cpp, src/bitboard.cpp add_piece/remove_piece/move_piece, tools/eval_model.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp
excludes:   NNUE, parked at DEC-054 -- this is a table indexed by a bucket, not a network; any change to the terms around the tables
decisions:  DEC-071, DEC-084, DEC-087
closes:
blocks:
paused_by:
done:

## Created by the second review, DEC-087, on the owner's explicit approval

The largest documented evaluation item this plan had no step for. **Neither
of the two figures below prices the tables**, which DEC-133 already amended
DEC-087 (d) for and S185 now writes at the site (2026-09-04 literature check,
rows A18 and A19). Leorik 2.5 replaced its piece-square tables with linear
functions of both king positions and phase -- 18 parameters, AVX2 -- and the
release is **2917 against 2.4's 2829 on CCRL Blitz, a delta of +88**, its
author claiming about +100
(https://github.com/lithander/Leorik/releases/tag/2.5); that release also
carries the MIT relicence, .NET 8, PEXT move generation and threads, so +88 is
a **four-change release delta**, not the tables. Berserk 4.3.0 mirrored its
tables to the enemy king's side and its author estimates the *release* at
"about 65 Elo stronger than Berserk 4.2.0"
(https://github.com/jhonnold/berserk/releases/tag/4.3.0) -- self-play
+157.76 +/- 3.86 at 8+0.08 and +130.11 +/- 4.79 at 32+0.32, halved -- bundled
with space, imbalance tables, an expanded king area, phased move generation,
history pruning, a null threat in LMR, ordering, TT bucket size 4 and bug
fixes. So **~+65** and **~+88** are the two bundles, and the tables' own share
is unmeasured in the record. CPW's *Piece-Square Tables* page does not describe
the king-relative form at all
(https://www.chessprogramming.org/Piece-Square_Tables, one 2013 forum title).
The step is kept on the coverage argument below, not on the two deltas. The idea is the cheap end of what a
network buys: the value of a square depends on where the kings stand, and a
bucket index captures the largest slice of that for the price of a wider
table.

The cost is real and is the reason this needed an owner decision: `make_move`
today updates the PSQT accumulators incrementally through the S014 primitives,
and a king move that changes its bucket invalidates every entry on that
side -- a full accumulator rebuild at that node. Berserk's two-bucket mirror
bounds the rebuild to enemy-king side changes; Leorik's full linear form needs
SIMD, which S104's AVX2 target and S117's packed score make affordable if the
sweep points there. Start at the two-bucket end; the fit decides if more
buckets earn their rebuilds.

It sits directly ahead of S126 so the full refit covers whatever it ships.
