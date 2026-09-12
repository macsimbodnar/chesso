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

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki does not describe the technique.** CPW *Piece-Square Tables*
(https://www.chessprogramming.org/Piece-Square_Tables) carries one 2013 forum
title and no description of a king-relative or king-bucketed form; the
2026-09-04 literature check established that and it is unchanged. So there is
no wiki definition to depart from, and the definitions below come from engine
release notes and pull-request bodies, read for form only.

**The two deltas this file already corrects are release bundles and stay
that way.** Berserk 4.3.0's "about 65 Elo stronger than Berserk 4.2.0"
(https://github.com/jhonnold/berserk/releases/tag/4.3.0) is a ten-item release;
Leorik 2.5's 2829-to-2917 CCRL Blitz delta of +88
(https://github.com/lithander/Leorik/releases/tag/2.5) is a four-change
release. Both are already written that way above and neither is used to price
the tables.

**This pass located the first per-feature figures for king bucketing, and they
are small.** Lynx pull requests, bodies only
(https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+bucket+in:title+type:pr,
fetched 2026-09-13):

| what | figure | state |
|---|---|---|
| #2001, index the king open/semi-open bonus and penalty by file **and king bucket** | **+2.86 +/- 2.33**, LOS 99.21 %, 32638 games | merged |
| #2002, index the rook open/semi-open bonus **by king bucket** | **+1.69 +/- 1.36** at 8.0+0.08s Hash 32MB, 107210 games | merged |
| #2003, index king virtual mobility **by bucket** | **-0.98 +/- 2.55** at 8.0+0.08s Hash 32MB, 30246 games | closed |
| #2657, custom piece-square-table buckets, 24 symmetrical | **-2.79 +/- 3.30** at 8.0+0.08s Hash 32MB, 14430 games | closed |

Four readings, and they change what this step should expect.

1. **King bucketing is a single-digit idea per feature where it is measured at
   all**, +1.69 and +2.86, and it needed 32 to 107 thousand games to resolve
   those. Nothing located prices bucketing the *whole* piece-square table on
   its own.
2. **It measures negative as often as positive.** #2003 and #2657 both failed.
   A bucket scheme is not free strength; it is a scheme that must earn its
   rebuilds.
3. **#2657 is the closest located analogue to this step and it failed**: more
   piece-square-table buckets, at an engine well above chesso's band,
   **-2.79 +/- 3.30**. That is a record about *how many* buckets, and it is
   direct support for this file's own "Start at the two-bucket end".
4. **All four are at 8.0+0.08s**, the same control this project measures at,
   which is rare in this plan's sources and makes them unusually comparable
   (DEC-019 still applies: they decide what to try, never what to conclude).

**No figure in this section is unverified.** The step's two release deltas
carry their URLs and their bundle warnings; the four Lynx rows carry theirs.
The step's argument remains the **coverage** one -- Berserk, Leorik and Lynx
all carry some king-relative form and chesso carries none -- which needs no
number, plus the cheap-end-of-a-network argument this file already makes.

### 2. Shape for chesso

Two bucket schemes are on the table and the accepts chooses between them by
held-out fit error before any match:

- **Enemy-king-side mirroring, two buckets.** Berserk 4.3.0's form as its
  release note describes it: tables indexed by the side of the board the enemy
  king stands on. The rebuild is bounded to enemy-king side changes.
- **Own-king file buckets, up to four.** Not a published form at any surveyed
  engine; Leorik's is a linear function of both kings' squares (18 parameters),
  which is a different shape and needs SIMD.

**Departures, stated:** Leorik's linear-in-king-squares form is not what this
step builds and the file already says so. Lynx's bucketing is applied to
individual features rather than to the whole table; this step buckets the
table, which no located source prices alone. Both departures are deliberate
and both are why the step owns its own verdict.

### 3. Implementation sketch

- The accumulators in `src/bitboard.cpp` `add_piece`, `src/bitboard.cpp`
  `remove_piece` and `src/bitboard.cpp` `move_piece` are what a bucket crossing
  invalidates. A king move that changes its bucket is a full rebuild on that
  side, and the accepts requires the **bucket-crossing frequency counted over a
  real search** -- the rebuild cost is frequency times rebuild, and neither
  half is guessable.
- The existing accumulator-equals-recompute test gains exactly the
  bucket-crossing case, which is the one INV-4 could otherwise lose silently.
- `tools/eval_model.hpp` carries the same buckets and `tools/tuner_groups.hpp`
  the groups, partition properties green.
- Every entry is fitted on the S082 corpus with the S077 provenance stamp.

### 4. Constants and seeds

**No seeds.** Every entry is fitted (DEC-084, and the accepts). No engine's
king-relative table seeds anything here wherever it is republished; a bucketed
table published on the wiki would still be that engine's table (DEC-105,
DEC-134), and `CLAUDE.md` records that chesso's own tables were hand-written
for exactly this reason and have since been fitted by this project's own tuner
over its own self-play (S028, DEC-105).

Two numbers are declared rather than fitted, both **(b) derived**:

- **The bucket count**, from the held-out fit error across the two schemes
  named in the accepts -- a derivation this step runs at its start, on its own
  corpus, before any match. Lynx #2657's -2.79 is a record that more buckets
  can be worse and is direction only.
- **The bucket boundary**, from the board: the enemy king's file against the
  centre line for the two-bucket scheme, which follows from the geometry.

### 5. Pitfalls

- **The rebuild is the cost and it is measured, not argued.** nps beside the
  verdict and the crossing frequency counted, per the accepts.
- **INV-4 across a bucket crossing is the invariant most likely to break
  silently.** The accumulators are correct everywhere except at the one node
  where the bucket changes, and nothing in the fast suite sees that today.
- **A wider table is a worse-conditioned fit.** Doubling the table halves the
  rows per entry, and S100's lesson is that a sparsely-represented column
  reports the corpus rather than the feature. Held-out error is the check and
  it is taken before the match.
- **The degeneracy S134 removed can come back.** A per-square table already
  expresses any feature defined on a single rank; a bucketed per-square table
  expresses more. `tools/feature_audit.cpp`'s identity report is re-run over
  the new parameterisation, which is S134's rule applied forward.
- **`PARAM_COUNT` grows by a large factor.** The tuner's cost per epoch and the
  corpus size S083 chose are both read against the new count.

### 6. Measurement

Held-out fit error decides the scheme, recorded with its figures, **before any
match**. Then one SPRT at the S105 regime, bounds stated in advance with the
nElo worst case and the abort rule (DEC-143) -- and sized on the located
record, which is single digits per bucketed feature and two failures out of
four attempts, not on either release bundle. nps and crossing frequency beside
the verdict.

### 7. Interactions

- **S134 (before)**: the degeneracy fold, and the identity check this step
  re-runs.
- **S117 (packed score) and S104 (AVX2)**: what would make a wider or linear
  form affordable if the sweep points there.
- **S126 (immediately after)**: the full refit covers whatever this ships,
  which is why the two are adjacent.
- **S029 (parked, DEC-054)**: this is a table indexed by a bucket and not a
  network; the accumulator discipline is the same discipline an NNUE would
  need.

### 8. References

- - https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+bucket+in:title+type:pr
  -- #2001 (+2.86 +/- 2.33, merged), #2002 (+1.69 +/- 1.36, merged), #2003
  (-0.98 +/- 2.55, closed), #2657 (24 symmetrical PSQT buckets,
  -2.79 +/- 3.30, closed). Pull-request bodies only. Fetched 2026-09-13.
- - https://github.com/jhonnold/berserk/releases/tag/4.3.0 -- "PSQTs indexed
  based on same side as enemy king" in a ten-item release estimated at "about
  65 Elo". Release note only.
- - https://github.com/lithander/Leorik/releases/tag/2.5 -- piece-square values
  as linear functions of both kings' positions and phase, 18 parameters, AVX2;
  a four-change release, CCRL delta +88. Release note only.
- - https://www.chessprogramming.org/Piece-Square_Tables -- **does not describe
  the king-relative form**; one 2013 forum title.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` rows A18 and A19 --
  where the two release deltas were separated from the tables.
