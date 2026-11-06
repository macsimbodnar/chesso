id:         S102
goal:       outpost and space terms in the evaluation, fitted like every other constant
accepts:    one SPRT verdict for the family -- outposts and space, each screened first (DEC-259), and a family H0 bisected into the two; each fitted with every other constant frozen, held-out error reported before and after; each reads a pawn-derived fill from `pawn_entry_t` or builds it itself at every evaluation, and the step states which and prices it, adding no second pawn pass (amended 2026-11-01 on S118's landing, DEC-332 (3)); INV-5 holds, asserted by the mirror case in tests/test_evaluation.cpp; the taper keeps the divisions HEAD has (S055 read H0) and the model guard's bound still holds; the trace (tools/tuner_trace.hpp, tools/tuner_layout.hpp) gains each feature, tools/eval_model.hpp having gone at S276 (amended 2026-11-04, DEC-351), and tools/tuner_groups.hpp a group per term with the partition properties intact; the fast suite green
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/tuner_trace.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, src/data_structures.hpp, src/eval_trace.hpp, tools/tuner_layout.hpp, tools/feature_audit.cpp, tools/truncation_scan.cpp, tools/eval_spread.cpp, tests/test_eval_trace.cpp, tests/test_eval_model.cpp, tests/test_walked_counts.hpp, tests/test_tuner_groups.cpp, tests/test_tuner_gradient.cpp, tests/test_eval_positions.hpp (widened 2026-11-04, DEC-351 (12))
excludes:   threat terms, which are S101; mobility, which exists; any term that needs a pass over the board that is not already being made -- a fixed sequence of shifts and masks over sets the term already builds, as P6's knight moves of the outpost squares, is not such a pass (amended 2026-11-04, DEC-351)
decisions:  DEC-071, DEC-259, DEC-332, DEC-351
closes:
blocks:
paused_by:
author:     the coordinator; phase 0 (research pass) by an Opus 5.5 subagent, briefed by the coordinator, 2026-11-04
done:

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

Each term -- and each part this file used to give a verdict of its own --
is first fitted with every other constant frozen and kept only if it lowers
held-out loss on S082's fixed validation set by more than the margin the
pre-registration states; a term that fails is recorded with its figures and
does not enter the match. The survivors ship under **one** `{0, 5}` nElo SPRT
for the family, with DEC-143's worst case and abort rule pre-registered. On
H0 or no verdict the family is bisected along a partition written in the
pre-registration, each part its own SPRT. Where this file names
`tools/eval_model.hpp` or `test_eval_model` as the tuner's mirror of a term,
S262's trace and its exact-reconstruction test discharge that clause once
S262 has landed. S263 built no per-side attack set, since no term read one;
a term here that needs one builds it once per evaluation. S039 read H0 (record `ca578de`,
revert `74d0adf`): the lazy shortcut and the clamp are kept. S122's design
waits on the owner's amendment before S121 starts (DEC-258), and that
amendment binds here.

## Why these two and not a longer list

They are the two terms on the standard hand-crafted list that reuse work
already being done: an outpost is a square no enemy pawn can attack -- which is
the pawn-attack span the passed pawn and pawn structure terms already fill --
and space is a count over safe squares behind one's own pawns in the centre,
from the same fills. Anything needing its own pass over the board is a
different trade and is not in this step.

## The measurement risk this step carries

Both terms are known to fit well and to be worth little on their own. **No
located source prices either term alone** -- the claim that the literature's
figures for them are small was itself unsourced and is deleted (DEC-203); the
honest statement is that the literature gives no figure at all. The
expectation of a small verdict rests on this engine's own record instead: it
has measured 0, 0 and *slower* on terms adopted from published reports
(DEC-019). A verdict of zero is the expected outcome for at least one of the
two and is recorded as zero.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**Both terms are on the wiki, and one of them carries a published magnitude.**

- CPW *Outposts* (https://www.chessprogramming.org/Outposts, fetched
  2026-09-13) defines an outpost as "a chess term most often related to knights
  in the center or on the opponent's half of the board, defended by an own
  pawn, and either no longer attackable by opponent pawns at all" -- so the
  published definition has **three** clauses (a knight, defended by an own
  pawn, not attackable by an enemy pawn), where this step's goal states only
  the third. The page's one magnitude is **an engine's, and it is refused as a
  seed**: "Toga log user manual advocates a bonus for a knight outpost on a
  central square as 10 centipawns, but it is possible to see bonuses as large
  as 16 centipawns" -- Toga's documented constant, republished on the wiki, so
  DEC-105 as amended by DEC-134 bars it from seeding anything here. It is read
  for form -- the term is worth a small fraction of a pawn to somebody who
  shipped it -- and for nothing else, the same treatment S122 gives CPW's
  2 / 3 / 5 / 6 attack units and the Glaurung table. **No Elo figure.**
  Reachable outposts are **not** described on the page.
- CPW *Space* (https://www.chessprogramming.org/Space, fetched 2026-09-13)
  defines space as "a loosely defined evaluation feature related to square
  control, in particular center control considered by piece placement dependent
  on pawn structure", and gives the one concrete specification on the page:
  "Stockfish defines a space area bonus by the number of safe squares for minor
  pieces on the central four files on ranks 2 to 4, counting twice if on a
  rearspan of an own pawn". **No centipawn value and no Elo figure.**

**No located source prices either term alone.** The 2026-09-04 literature check
recorded both as present in Ethereal, Berserk and Stash and found no figure
for either; this pass fetched the two wiki pages and confirms neither carries
one. Berserk's 4.3.0 release note names "space" among a ten-item bundle
estimated at "about 65 Elo stronger than Berserk 4.2.0"
(https://github.com/jhonnold/berserk/releases/tag/4.3.0), which prices the
release and not the term -- the same bundle S133 reads and the same caution
applies. **So this step's old claim that "the literature's figures for them are
small" had no source and is deleted** (DEC-203): the honest statement is that
the literature gives no figure at all, and the expectation of a small verdict
rests on DEC-019's three local cases (staged generation 0, SEE pruning in
quiescence 0, capture ordering slower) and not on a published number. "The
measurement risk this step carries" above now says that.

### 2. Shape for chesso

**Departures from the wiki's definitions, stated:**

- **Outposts.** The page's form is knight-specific, requires the square to be
  defended by an own pawn, and requires it to be unattackable by an enemy
  pawn. This step's goal names only the pawn-attack-span clause. The step
  states which clauses it ships: shipping fewer than the page's three is a
  departure and changes what the term measures, and the pawn-defended clause
  is free because the same fill gives it. ~~Bishop outposts and *reachable*
  outposts are outside the page entirely and outside this step.~~ **Amended
  2026-11-04, DEC-351:** bishops on an outpost (the page's "diagonal
  outpost", named and not defined) and knights one move from an outpost (the
  page's "about to be occupied by a knight") are screened as P5 and P6, each a
  part of its own after the page's own form, their definitions chesso's own
  and said to be.
- **Space.** The page's one concrete form is Stockfish's -- central four
  files, ranks 2 to 4, safe squares for minor pieces, doubled on an own pawn's
  rearspan. That is a **description of another engine's term in a publication
  about the technique**, so its *shape* may be used (DEC-014); its weights are
  not published and would not be seeds if they were (DEC-105). The step states
  which square set it counts and why, and any departure from the page's set.

Both terms reuse the pawn-derived fills `src/evaluation.cpp` `pawn_pass` already builds -- the accepts requires the step to name which fill each one reuses rather than adding a pass, and after S121 the enemy pawn-attack span is the mobility area's input as well. **Since S118 those fills are that function's locals, over the two pawn bitboards alone, on a table miss only**: a term that reads pieces reads what `pawn_entry_t` stores, or builds the fill itself at every evaluation, and the entry is 32 bytes with two spare (DEC-332 (3)). The step says which and prices it.

### 3. Implementation sketch

- Two terms, two features, two groups in `tools/tuner_groups.hpp`, partition
  properties green in `tests/test_tuner_groups.cpp`.
- `tools/eval_model.hpp` gains each feature in the same commit.
- INV-5 is asserted by the mirror case in `tests/test_evaluation.cpp`.
- The taper is a single division if S055 has landed, and the model guard's
  bound is re-read at this step's own HEAD rather than off any sentence in a
  plan file -- the same rule S136's accepts states.

### 4. Constants and seeds

Both terms are fitted (DEC-084), both start from zero like every other new
term, and **neither takes a published magnitude**: the only one either wiki
page carries is Toga's (section 1) and DEC-105 as amended by DEC-134 refuses
an engine's constant wherever it is republished. Where a starting value is
wanted anyway -- for a sweep, or for an SPSA range -- both are **(c) the
midpoint of a range this file declares by purpose**, and both purposes are
chesso's own:

- **Outpost bonus.** A placement bonus must never be worth trading material
  for: a knight that is one square from an outpost may spend a tempo to reach
  it and must not be told to give up a pawn for it. So the declared range is
  **0 to half a pawn** on chesso's material scale (`src/eval_tables.hpp`
  `piece_value`, `PAWN` 94) -- 0 to 47, midpoint **24**, stated as a midpoint
  and never as a target. The band is wide, and that is the honest consequence
  of refusing the narrow one somebody else tuned: what ships is the fit's
  output and the band does not move it.
- **Space bonus.** Same form: the term counts squares and its per-square
  weight is bounded above by the smallest weight that would let a full board
  of counted squares outweigh a pawn, which the step writes out, with the
  midpoint stated as such.

No engine's outpost table or space weight seeds anything here wherever it is
republished (DEC-105, DEC-134).

### 5. Pitfalls

- **Zero is the expected verdict for at least one of the two** and is recorded
  as zero. That is this file's own sentence and DEC-019 is why; what changes
  above is only that no published figure supports the expectation either way.
- **Two terms, two verdicts.** The accepts says "measured separately"; a
  bundle at this expected size prices neither.
- **No new pass over the board.** A term that needs its own scan is a
  different trade and is excluded. The step names the fill each term reuses.
- **The stage decision.** Both terms are cheap-stage by construction if they
  ride the pawn fills; if either lands in the expensive stage it enters the sum
  the clamp bounds, kept by S039's H0 (record `ca578de`), and the step says
  which.
- **The outpost definition is a choice, not a detail.** Dropping the
  pawn-defended clause makes the term fire on far more squares and the fitted
  weight is then not comparable with any published one.

### 6. Measurement

One SPRT per term at the S105 regime, bounds stated in advance with the nElo
worst case and the abort rule (DEC-143), sized for a term that may measure
zero. Held-out error before and after each fit, reported. nps recorded.

### 7. Interactions

- **S121 (before)**: builds the enemy pawn-attack span both terms read, and the
  mobility area's safe-square notion is the same one space counts.
- **S125 (before)**: the pawn fills.
- **S101 (separate)**: threats; different terms, same reuse argument.
- **S055 (before, by plan order)**: the taper division count this step's model
  guard is read against.
- **S126 (block end)**: refits both.

### 8. References

- - https://www.chessprogramming.org/Outposts -- the three-clause definition
  quoted above; the Toga log user manual's 10 centipawns for a knight outpost
  on a central square, "as large as 16" -- an engine's documented constant,
  read for form and **refused as a seed** (DEC-105, DEC-134); reachable
  outposts **not** described; **no Elo figure**. Fetched 2026-09-13, page and
  footnote re-read 2026-09-13 after the fast check.
- - https://www.chessprogramming.org/Space -- the definition quoted above and
  Stockfish's square set as the page states it; **no centipawn value and no Elo
  figure**. Fetched 2026-09-13.
- - https://github.com/jhonnold/berserk/releases/tag/4.3.0 -- "space" named in a
  ten-item release bundle estimated at "about 65 Elo" for the whole release.
  Release note only.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` part B3 -- outposts
  and space present in Ethereal, Berserk and Stash; no figure located for
  either.

## 2026-11-04: the research pass

Read at `2bf3ad4`, bench 5482277 reproduced in the pass's own build: counts, label readings and instruction counts only,
no fit, no match, no timing, nothing under `src/` or `tools/` changed. Revised the same day after the coordinator's fast
check (below, "What the fast check moved"). Every figure's source is adocs/data/S102_census.txt, by
adocs/data/S102_census.sh (adocs/data/S102_census.cpp), and adocs/data/S102_cost.txt, by adocs/data/S102_cost.sh
(S102_cost_sink.patch).

**What the census reads.** It reads the FEN and game columns of every corpus. It reads the result column of one: S122's
regenerated prepared training corpus (`.tuning/s122/prepared/corpus.tsv`, sha256 `04c596de...`, 6307294 rows, 137991
games, written by S331's `datagen`, DEC-349), at its own K 1.2302. Its three held-out views (`pool.tsv.{qs,root,pv}.validation`,
12000 games) and S083's corpus (the old two-fold rule's, DEC-349) are read for counts, and their labels are unread.

**How it is checked.**
- Every count is held to a square-by-square walk of its definition on every row of all five corpora: 0 differ.
- A planted span without its neighbour files turns the walk red: 48535 side-counts on 20000 rows.
- Every traced row's integer score equals `evaluate()`.

CPW *Outposts*, *Space* and *Center* were re-read on 2026-11-04 for their descriptions (DEC-221). No engine's source was
read. No position was judged: a feature is a count of men and squares picked by pawn attacks, files and ranks
(DEC-023). The proposal is `.tuning/coord/S102/decision_draft.md`.

**What the fast check moved.**
- **Space's byte.** The count is stored as its complement, so a zeroed slot reads true (below, "Where each term lives").
- **Reach.** It is defined set-wise and shown equal to the per-knight form on every row.
- **The region.** It is derived from the page, so P1's cells changed, and a flank part was added.
- **The cost.** The sink was rebuilt one change at a time.
- **Wording.** Novelty is stated per feature, and every increment carries a paired error. Finding (e) is corrected.

**The pages, as they read today** (each sentence below is the page's own).
- *Outposts* opens: "a chess term most often related to knights in the center or on the opponent's half of the
  board, defended by an own pawn, and either no longer attackable by opponent pawns at all, or otherwise, provoking to
  weaken an opponent pawn".
- It adds: "Some programmers apply a more common meaning of the word outpost, using this term in regard to a strong
  square in the center or opponent half of the board, defended by own pawn and no longer attackable by the opponent's
  pawn, occupied or about to be occupied by a knight."
- It also says: "One game commentary mentions also 'a diagonal outpost', though this subject is not expanded."
- Its "Factors influencing score" list includes "if opponent has no minors that he can exchange for an outpost piece"
  and "outpost value can be increased if it is defended by two pawns".
- *Center*: "The Center of the chessboard are the four inner squares {d4, e4, d5, e5}." The extended center "incorporates
  the next concentric ring around the center, including C- and F-file as well as third and sixth rank".
- *Space*: Stockfish's term counts "the number of safe squares for minor pieces on the central four files on ranks 2 to
  4, counting twice if on a rearspan of an own pawn". Also: "The space area bonus is multiplied by a weight, determined
  by the number of own pieces minus number of open files."

Section 1 quoted *Space*'s first sentence alone and none of *Outposts*' refinements. No page gives a number this pass
could seed from, and Toga's constant stays refused (DEC-105, DEC-134).

**The region, derived from the page (DEC-221).** "In the center or on the opponent's half of the board": the center
is CPW's four inner squares, and the opponent's half is own ranks 5 to 8. So the region is:
- own rank 4 on files d and e, the center's own-half squares;
- every square of own ranks 5 to 8, which hold the other two center squares.

The other ranks:
- **Own rank 3 is out.** It is in the extended center, which the page does not name, and in neither of the page's two
  areas.
- **Own rank 4 off d and e is out** by the page. It is measured as a departure of its own (P4).
- **Own ranks 7 and 8 are in.** Rank 8 can fire: no enemy pawn can ever attack it, so every square there is a hole, and
  an own pawn on its seventh defends it. It fires on 0.012 % of rows in 339 games, under DEC-326 (5)'s 1000, so its
  cell is rank 7's, and the 7-and-8 cell stands on 3646 games.

No engine's mask was read. The first reading used own ranks 4 to 6 on every file without deriving it. It is kept as
a comparison (the O_ variants) and proposes nothing.

**What exists.**
- **Stage one** is `src/evaluation.cpp` `cheap_score`. It sums the entry's score and `outside_pawn_terms` (S135's
  placement, S123's A and B) into the packed positional pair, takes one `taper`, adds the material untapered and
  tempo apart.
  - `pawn_pass` takes the two pawn bitboards and nothing else, and runs on a table miss only. It builds every fill
    these terms need, all as its locals, for both colours: the forward and backward fills (`white_ahead`,
    `white_behind`), the front spans, `white_beside`, the pawn attacks `white_attacks`, the attack span
    `white_attack_span` (S263) and the whole-file sets.
  - `src/data_structures.hpp` `pawn_entry_t` keeps only what outside terms read: the two pawn bitboards (the
    verifier), `passers`, the packed `score`, a byte of each side's file set, and `spare`, two bytes nothing writes.
  - So the enemy attack span (an outpost's "no longer attackable"), the own pawn attacks ("defended by own pawn", and
    space's "safe") and the rear span ("rearspan") exist only inside the pass. An outside term reads the pawn
    bitboards and rebuilds what it needs: two fills and a `neighbour_files` a side.
- **Stage two**, `evaluate_mobility_and_king_safety`, rebuilds `pawn_attacks` (two shifts a side), looks each piece's
  attack set up, and holds S101's unions `attacks_by`. S121's mobility area is the squares no own man holds and no
  enemy pawn attacks. It sits behind the lazy shortcut and under the clamp, `LAZY_EVAL_MARGIN`.
- **Nothing reads an outpost, a hole or a space count today.** The nearest terms are S135's placement counts. The rook
  file counts read the same file sets. The trapped minors read enemy pawns around the a and h files' corners. None
  reads the span.

**Where each term lives, and what it reads** (the accepts' "states which and prices it").
- **Outposts read knights and bishops**, so they cannot sit in the entry (DEC-332 (3)). They go in `outside_pawn_terms`
  and **build the span and the pawn attacks themselves at every evaluation**, from the pawn bitboards, behind a gate.
  - Storing the outpost squares instead needs 24 a side, 48 bits against the entry's 16 spare. The entry would
    outgrow DEC-332 (3)'s 32 bytes, two to a line, and S118's size would owe its DEC-266 credit again. Not proposed.
- **Space reads pawns alone** while "safe" does. `pawn_pass` computes it on a miss from fills it already makes:
  `white_behind`, the pawn attacks, the file sets.
  - The count and its open-files product go into the entry's score, free on a hit.
  - **The count is a byte a side in the entry's spare bytes, stored as 12 less the count** (a signed byte, the count
    running 0 to 24), and `outside_pawn_terms` decodes it for the one product that reads pieces. That is a
    pawn-derived value read from `pawn_entry_t`.
  - Neither term adds a second pawn pass.
- **Why the complement.** The table is zeroed every `go`, and its verifier is the two pawn bitboards alone. So an
  untouched zero slot is a hit for every pawnless board, and S118's "a hit is exact" holds only if zeros are the
  pawnless board's true entry. With no pawns the space count is 12 a side: the area's 12 squares, none on a rear span.
  - **A plain stored count would read 0 there.** The pieces product would then read 12 times the difference in
    knights to queens wrong on any pawnless board with unequal pieces, KRvK or KQvKR, in Release.
  - No guard would see it: the trace reads no table, and seeds at zero hide it from INV-6.
  - The corpus holds 155890 pawnless rows in 18063 games, 82150 of them with unequal knights to queens, and the qs view
    71517 rows, 41295 of them unequal.
  - The complement reads 12 from a zero byte. The entry's score reads 0 there, and so do the count's and its
    open-files product's parts in it, which is their pawnless value, 12 less 12.
- **The rule, for this and every later field:** every field the entry stores must read correctly from a zero slot under
  a pawnless key.
- **INV-4:** neither term is a sum over pieces that `make_move` could keep. Each is a product or a conjunction of two
  piece sets, one of them pawns, so a move by either side changes it with no O(1) delta. S135's argument for
  placement holds word for word.

**The cost** (adocs/data/S102_cost.txt). The counts sit where the proposal puts them, in the page's region, at forced
non-zero weights in an asm barrier, the score the landing's. Node counts read 5482277 and 14931387 on all twelve builds
before any count. Five interleaved `tools/perf_counters` runs, instructions only, under S122's screen's load. Each row
changes one thing over the row above it, unless it names its base:

| sink build, `S102_SINK`, instructions | `bench` | `bench 16` |
|---|---|---|
| `=1`: P1's cells and P3's hole, every call, no gate (over base) | +2.938 % | +2.971 % |
| `=3`: the narrow gate, a knight in either side's region (over base) | +1.662 % | +1.781 % |
| `=259`: the wide gate instead, a knight anywhere or a bishop in the region; the gate change alone | +3.089 % | +3.174 % |
| `=291`: + P2, the no-exchange cell | +3.416 % | +3.518 % |
| `=307`: + P5, bishops | +3.768 % | +3.880 % |
| `=371`: + P6, reach as a loop over the knights | +4.996 % | +5.133 % |
| `=435`: P6 set-wise instead of the loop (over `=307`) | +5.502 % | +5.676 % |
| `=947`: + P4, the flank cell: every outpost part, reach set-wise | +5.934 % | +6.125 % |
| `=696`: every outpost part in stage two, reach set-wise, no gate (over base) | +3.468 % | +3.670 % |
| `=4`: space, P7 and P8 on a miss, P9 decoded at every call (over base) | +1.192 % | +1.216 % |
| `=951`: every outpost part and space (over base) | +6.737 % | +6.955 % |

Spreads run 68 to 371 instructions over five runs. Read as steps, `bench` first:
- **The old `=19` row was confounded.** The gate's widening alone costs +1.43 points (+1.39 on `bench 16`). It is paid
  for bishops (a bishop in the region) and reach (a knight anywhere) together.
- **The cells' own increments:** the no-exchange cell +0.33, bishops +0.35, reach +1.23 as a loop or +1.73 set-wise,
  and the flank +0.43.
- **The proposal's outposts with reach as a loop** would read about +5.43 % (inferred, not built).

No cycle, wall or nps was read. The space row is the proposal's shape: the count and its open-files product on a
miss, the pieces product decoded at every call. Frames: S124's factor and mop-up +1.9 %, S122's parts +6.2 % to
+6.5 %, S123 +4.96 %, S125 +3.50 %.

**What the corpus holds.** Every definition is chesso's own and is said to be (DEC-221). They are in
adocs/data/S102_census.cpp's header and summarised here. White's squares are given, Black's are their mirror, and
"own rank" is the side's own. The span is every square an enemy pawn can ever attack as it advances, `pawn_pass`'s
`attack_span` of the other colour.
- A **hole** is a square of the region outside the enemy span. An **outpost** is a hole an own pawn attacks.
- **Space's area** is files c to f on own ranks 2 to 4. **Safe** means no own pawn stands there and no enemy pawn
  attacks it. **Rear** means behind an own pawn on its file, strictly.
- A term is White's count less Black's.
- **"Rows" and "rows by phase" both count rows where the two counts differ**, the rows a term reads. That is
  `differs` in the census file, beside `fires` (either count non-zero).
- **Games** are games with such a row, per cell.

| feature | rows % | games | rows by phase 0-4, 5-8, 9-12, 13-16, 17-20, 21-24, % | qs view %, games | root % | pv % | S083 % |
|---|---|---|---|---|---|---|---|
| Q_N4: knights on an outpost, own rank 4, files d and e | 1.470 | 17502 | 0.6, 1.7, 2.2, 2.1, 2.0, 1.0 | 1.393, 1812 | 1.463 | 1.440 | 1.421 |
| Q_N5: own rank 5 | 4.699 | 48972 | 2.0, 4.7, 6.0, 5.8, 6.8, 4.3 | 4.366, 4922 | 4.748 | 4.521 | 4.456 |
| Q_N6: own rank 6 | 1.431 | 18774 | 0.8, 1.9, 2.4, 2.0, 1.9, 0.6 | 1.324, 1981 | 1.470 | 1.463 | 1.217 |
| Q_N78: own ranks 7 and 8 | 0.200 | 3646 | 0.2, 0.3, 0.3, 0.2, 0.2, 0.1 | 0.181, 424 | 0.202 | 0.221 | 0.149 |
| of it own rank 8 | 0.012 | 339 | 0.0 in every band | 0.012, 50 | 0.014 | 0.023 | 0.011 |
| Q_NX: Q_N with no enemy knight and no enemy bishop of the square's colour | 1.527 | 18534 | 1.5, 3.2, 2.6, 1.8, 1.2, 0.0 | 1.512, 1884 | 1.550 | 1.849 | 1.280 |
| Q_NH: knights on a hole no own pawn attacks | 14.246 | 100165 | 15.4, 22.2, 19.4, 15.8, 12.1, 6.4 | 14.326, 9358 | 14.266 | 17.048 | 13.645 |
| O_N4W: knights on an outpost on own rank 4 off d and e (the flank) | 1.935 | 21448 | 0.7, 1.9, 2.7, 2.7, 2.9, 1.7 | 1.821, 2149 | 1.915 | 1.684 | 1.784 |
| Q_B: bishops on an outpost | 5.161 | 49211 | 5.0, 8.6, 7.1, 5.5, 5.0, 1.9 | 5.652, 4719 | 5.864 | 6.092 | 4.117 |
| Q_RCHS: own knights in the knight moves of the outpost squares no own man holds | 13.462 | 107341 | 6.9, 13.3, 16.1, 15.9, 16.1, 14.1 | 12.308, 9948 | 12.769 | 11.883 | 13.610 |
| Q_N2: Q_N with two own pawns attacking it | 0.922 | 11262 | 0.2, 0.6, 1.1, 1.1, 1.7, 1.1 | 0.872, 1130 | 0.974 | 0.876 | 0.814 |
| S_PUB: safe area squares, rear ones twice (S_P + 2 S_R) | 84.162 | 137986 | 71.9, 79.6, 87.3, 88.5, 89.5, 88.9 | 80.546, 11999 | 81.000 | 79.315 | 86.582 |
| S_XO: S_PUB times open files | 60.568 | 137022 | 71.7, 78.4, 79.0, 77.7, 64.0, 26.9 | 60.028, 11920 | 58.617 | 66.620 | 57.541 |
| S_XP: S_PUB times own knights, bishops, rooks, queens | 88.104 | 137989 | 78.3, 88.0, 90.7, 91.0, 91.0, 90.3 | 86.167, 12000 | 86.771 | 85.651 | 89.883 |

The census also reads:
- S_P and S_R apart;
- S_W1 and S_W2, the page's single weight with own pieces read as knights to queens or as every man but the king,
  below zero on 32.6 % and 17.2 % of side-rows;
- S_XM, the count times every man but the king;
- S_A, "safe" refusing any square an enemy man attacks;
- the first reading's ranks 4 to 6 variants.

**Each proposed cell stands on 3646 games or more in the corpus** (DEC-326 (5)'s 1000): the 7-and-8 cell, rank 8's
339 being tied into it. **In the qs view, the screen's instrument, two cells are thin:** the 7-and-8 cell (424 games)
and Q_N2 (1130). Q_N2 is not proposed. The 7-and-8 cell stays, being the page's region, and reads nothing (below).
DEC-259's amendment screens it like any cell: fitted with every other constant frozen, it survives only as part of a
P1 above the margin.

**What the labels read beside the shipped score.** S122's corpus at K 1.2302. Each set has its two halves free from
zero and every other weight at the shipped constants. One Gauss-Newton step on the squared error, read three ways:
- in sample;
- at the step rounded to the integers the engine ships;
- across halves: the step taken on the even-numbered games and read on the odd, the odd's read on the even, the two
  summed over all rows.

The increment of a part is the paired difference of the across-halves per-game dMSE, against the set before it, with
the whole-game error of that difference. dMSE is in 1e-6. These are readings and never fits, and the held-out views'
labels are unread.

| part, each added to the row above | in sample | rounded | across halves | its increment, paired |
|---|---|---|---|---|
| P1, Q_N4, Q_N5, Q_N6, Q_N78 | -38.77 +/- 5.02 | -38.75 | -37.02 +/- 5.05 | |
| P2, + Q_NX | -43.72 | -43.76 | -41.44 | -4.43 +/- 1.70 |
| P3, + Q_NH | -57.26 | -57.21 | -54.69 | -13.25 +/- 2.36 |
| P4, + O_N4W | -74.65 | -74.65 | -71.47 | -16.78 +/- 3.75 |
| P5, + Q_B | -87.88 | -87.90 | -84.44 | -12.97 +/- 3.06 |
| P6, + Q_RCHS | -137.43 | -137.37 | -133.89 | -49.45 +/- 5.33 |
| P7, + S_PUB | -151.67 | -149.01 | -145.34 | -11.44 +/- 3.84 |
| P8, + S_XO | -279.35 | -280.37 | -271.70 | -126.36 +/- 8.89 |
| P9, + S_XP | -346.73 | -326.36 | -338.49 +/- 14.63 | -66.79 +/- 6.26 |
| P1 to P7 and the page's single weight S_W1, the tie of P8 to P9 | -216.91 | -192.14 | -209.76 | -64.42 +/- 7.00 over P7 |

The across-halves column sits within about one error of the in-sample one in every set, so these signals are carried
by the games and not by one half's rows.

**What reads nothing**, across halves and paired:
- **The 7-and-8 cell** adds +0.32 +/- 0.27 over P1's three other cells. Split into 7 and 8 it adds +0.60 +/- 0.24
  more, worse.
- **Q_N6 alone reads -0.31 +/- 0.72**: nothing alone, while inside P1 its endgame half's step is 15.8.
- **Two pawns defending** add +1.14 +/- 0.80 over P1, and +1.47 +/- 0.67 over the first reading's O_N.
- **The first reading's rank 7** adds +0.29 +/- 0.29.

So two pawns and the ranks past 6 read worse than nothing, by up to two and a half errors, and are not cells of
their own.

**By signal instead** (Q4): reach -50.52 +/- 5.48 over P1, then the flank -15.30 +/- 3.56, the bishop -14.29 +/-
3.20, the hole -12.94 +/- 2.20, and the exchange -3.83 +/- 1.59. Each part's increment moves by under one error with
its place in the order.

**Space:**
- The count, rear squares twice, reads -13.58 alone across halves. S_P and S_R apart read -10.82: the page's doubling
  holds.
- The two weight factors apart over the tie read -128.73 +/- 8.63.
- In sample at integer weights, space with the factors apart keeps 94 % of its gain (-192.5 of -204.0), the every-man
  single weight 63 % (-80.1 of -126.2), and every man with open files apart 47 % (-104.4 of -224.3).
- The attack-aware S_A with its weight reads -21.34, against -75.03 for the pawn-only count with the same weight.

**The first reading's region.** It reads -21.49 +/- 3.90 better than P1 to P6 here, across halves. Its holes,
bishops and reach targets also cover own rank 4's flank, which only P4's knights do here (Q7).

**What these readings do not give:** the held-out loss the screen reads, any Elo, and the search's distribution of
these features. The latter is unmeasured; no `bench` census was run.

**The rank against what exists** (157683 rows, every 40th). Two forms of R^2 for each feature column:
- `feature_audit`'s form: the untapered difference regressed on the piece-square columns it overlaps.
- The tuner's view: each tapered half regressed on every existing parameter's column. 1101 of 1172 columns are live
  once the dependent ones drop, the intercept included.

"New" is 1 less R^2.

| feature | on | R^2 psqt | R^2 all, mg | R^2 all, eg | new, mg / eg |
|---|---|---|---|---|---|
| Q_N (P1's cells together) | knight+pawn | 0.219 | 0.290 | 0.255 | 71 % / 74 % |
| Q_N4, Q_N5, Q_N6, Q_N78 | knight+pawn | 0.079 to 0.272 | 0.120 to 0.375 | 0.081 to 0.297 | 63 to 88 % / 70 to 92 % |
| Q_NX | knight+pawn | 0.077 | 0.095 | 0.142 | 91 % / 86 % |
| Q_NH | knight+pawn | 0.546 | 0.505 | 0.712 | 49 % / 29 % |
| O_N4W | knight+pawn | 0.173 | 0.200 | 0.209 | 80 % / 79 % |
| Q_B | bishop+pawn | 0.200 | 0.244 | 0.311 | 76 % / 69 % |
| Q_RCHS | knight+pawn | 0.120 | 0.159 | 0.192 | 84 % / 81 % |
| S_PUB | pawn | 0.960 | 0.980 | 0.985 | 2 % / 1.5 % |
| S_XO | pawn | 0.553 | 0.611 | 0.827 | 39 % / 17 % |
| S_XP | pawn | 0.741 | 0.962 | 0.911 | 4 % / 9 % |

The outpost cells are mostly new except the hole: its endgame half is 71 % spanned by the knight and pawn tables,
mobility, threats and every other column together.

**Space's count is 96 % the pawn tables' and 98 % the whole design's.** With one pawn a file and no enemy pawn
attacking the area, a pawn's contribution to the count is a function of its square alone: a piece-square column. Its
residual is the enemy pawns attacking the area and the doubled pawns.

None reads 1, so no exact dependency is expected. `feature_audit`'s identities report decides it, and at the
landing it is expected to read the rank up by one per new column (inferred).

**The form, as this pass proposes it.** Two terms, both in stage one's positional pair before its one taper. They add
no division, so the truncation bound stays 2.875, 3.833 with tempo fitted, or S124's figures once its cells leave F.
They enter no clamp. Twelve cells, twenty-four parameters, every one linear. Each part's ground in the published
description, DEC-014's phase one, is quoted with it.

*Outposts*: the region and the definitions above.
- **P1** `outpost.knight`: knights on an outpost by cell, four cells: own rank 4 on d and e; own rank 5; own rank 6;
  own ranks 7 and 8. The page's form: "knights in the center or on the opponent's half of the board, defended by an
  own pawn, and either no longer attackable by opponent pawns at all".
- **P2** `outpost.exchange`: knights on an outpost where the enemy has no knight and no bishop on that square's
  colour. The page's factor: "if opponent has no minors that he can exchange for an outpost piece".
- **P3** `outpost.hole`: knights on a hole no own pawn attacks. A departure: the "defended by an own pawn" clause left
  out, measured as a cell of its own beside P1 and not dropped from it, so section 2's "fewer clauses changes what it
  measures" is answered by keeping both. The page's section heading is "Occupying Holes".
- **P4** `outpost.flank`: knights on an outpost on own rank 4 off files d and e. A departure: the page's region
  widened by the rest of own rank 4 (Q7).
- **P5** `outpost.bishop`: bishops on an outpost. The page's "One game commentary mentions also 'a diagonal outpost',
  though this subject is not expanded": the subject is named and not defined, so the definition, the page's clauses
  with a bishop, is chesso's own (Q1).
- **P6** `outpost.reach`: own knights in the knight moves of the outpost squares no own man holds. The page's "a
  strong square ... occupied or about to be occupied by a knight"; the rule, one move away, is chesso's own (Q2).

Not proposed: two pawns defending ("outpost value can be increased if it is defended by two pawns"), which reads
worse than nothing above, and the first reading's ranks 4 to 6.

*Space*: the page's area and doubling.
- **P7** `space.count`: S_P + 2 S_R, one cell. The page's "the number of safe squares for minor pieces on the central
  four files on ranks 2 to 4, counting twice if on a rearspan of an own pawn". "Safe" is chesso's reading, pawns
  alone.
- **P8** `space.open`: the count times open files, pawn-only, so in the entry's score.
- **P9** `space.pieces`: the count times own knights, bishops, rooks and queens.

P8 and P9 are the page's "multiplied by a weight, determined by the number of own pieces minus number of open files",
its two factors apart; the page's single multiplier is the tie w8 = -w9 (Q3). The attack-aware S_A reads less and
would sit behind the shortcut and under the clamp, so it is not proposed.

**P6, set-wise, and the census's per-knight form are one feature.** A knight is in the knight moves of a set exactly
when one of its jumps lands in the set, since knight moves are symmetric. The census computes both on every row: **0
side-counts differ of 12614588 on the corpus**, and 0 on the qs, root and pv views (2768364, 2770986, 2632332) and on
S083's corpus (12453888). The walk holds both.

**So the feature is fixed before the screen, and what is screened is what ships.** The definition is set-wise: own
knights intersected with the knight moves of the outpost squares no own man holds, eight shifts and eight masks
(adocs/data/S102_census.cpp `knight_moves`). The per-knight loop computes the same number exactly, so the code form
is phase 1's pick on cost and changes no count. The sink prices the loop 0.51 points cheaper on `bench` (0.54 on
`bench 16`). The set-wise form stays the definition and the walk's check.

**Is it "a pass over the board not already being made"?** No, in either form.
- **The set-wise form** is a fixed sequence of shifts and masks over a set the term already builds, with no loop over
  squares or men: the class of `pawn_pass`'s own fills, reading the board's kept occupancy.
- **The loop** visits the side's knights, a table lookup each, as S135's trapped minors and S123's passers visit
  their men.

**The scope amendment, drafted for the ruling.** Section 2 and `excludes:` both bar P5 and P6 as written.
- **Section 2's sentence** "Bishop outposts and *reachable* outposts are outside the page entirely and outside this
  step." becomes: "Bishops on an outpost (the page's 'diagonal outpost', named and not defined) and knights one move
  from an outpost (the page's 'about to be occupied by a knight') are screened as P5 and P6, each a part of its own
  after the page's own form, their definitions chesso's own and said to be (DEC-351)."
- **The `excludes:` line** becomes: "threat terms, which are S101; mobility, which exists; any term that needs a pass
  over the board that is not already being made -- a fixed sequence of shifts and masks over sets the term already
  builds, as P6's knight moves of the outpost squares, is not such a pass (DEC-351)".

**Seeds** are all at zero, the identity, DEC-134 form (b) as S121's and S124's: the landing is `No functional change`,
with liveness flags compiling a zero group out (S101's form). With the old constants frozen each cell is linear, so
the seed does not decide the optimum. Section 4's bands do not hold what the readings ask (Q6):
- Its outpost band is 0 to 47. The steps sit inside it but for P3's endgame half, -12 to -17, the 7-and-8 cell's,
  -9 to -17, and a few halves within 7 of zero: rank 4's endgame, rank 6's middlegame, and the exchange's and the
  bishop's endgame.
- The space count's steps run -8.0 to 0.2 in the middlegame and -1.6 to 14.9 in the endgame.
- The products' unit is a count times a count, up to 360 a side, so any non-zero integer is past a band under 94/360.

What ships is the fit's output (DEC-084). Section 4's bands stay sweep ranges.

**What S262's machinery carries and what it lacks.**
- **Carried, everything a linear positional term needs.** Its `EVAL_TRACE` call; a `SITE_POSITIONAL` row in
  `tools/tuner_trace.hpp` `term_layout`, where `exact_parts`, `smooth` and `gradient` read it with no new code; a
  block appended to `tools/tuner_layout.hpp`; groups and named parts in `tools/tuner_groups.hpp` `free_mask`, as
  `PLACEMENT_TERMS` and `CURVE_TERMS` are, held by the partition tests; ties for DEC-326 (5).
- **S124's product** (DEC-350 (5)) reaches these terms with no new code. A positional endgame weight's taper weight is
  w_eg f_c / F there.
- **Lacking: nothing in the machinery.** By hand there are three things:
  - `tools/feature_audit.cpp`'s collinearity list is written out, so the new columns join it with the tables they
    overlap;
  - `src/evaluation.cpp` `stage_one_half` gains each term's reach for the 16-bit assert;
  - `tests/test_walked_counts.hpp` gains a walk per term.
- **One blind spot, stated:** the trace build reads no table (DEC-332 (4)), so the spare bytes' hit path is invisible
  to every trace tool. The table's own tests carry it.

**The identity plan.**
- **`tools/truncation_scan` is the identity.** It holds `exact_score` to `evaluate()` on every row. Three plants each
  turn it red:
  - an outpost cell traced one off;
  - a term put on `SITE_MOBILITY` in `term_layout`;
  - the space product traced without its multiplier.
- **`tools/eval_spread` reads stage two alone.** It is blind to both terms and stays green, and its header says so.
- **A wrong count in the engine passes both**, the trace reading the engine's own variable. So the walk comes first,
  `tests/test_walked_counts.hpp` gaining `outpost_features` and `space_features` from the definitions. It is held by
  `tests/test_eval_model.cpp` "the walk counts what the engine counts", and goes red on three plants:
  - a span without its neighbour files (this pass's plant);
  - a rear span shifted the wrong way;
  - knight moves with one file mask dropped.
- **The table path, which no trace sees** (phase 1's plan):
  - **a pawnless test with unequal pieces through the table path**: KRvK and KQvKR, each mirrored, read through a fresh
    table and its zero slot, must equal `evaluate()`'s pure form. It is red on a planted plain count in place of the
    complement.
  - `src/evaluation.cpp` `pawn_entry_is_fresh` compares `spare`;
  - `spare` is initialised in `evaluate_cheap`'s and `evaluate_pawns`' local entries, and `pawn_pass` writes it on
    every fill;
  - `tests/test_evaluation.cpp` "a hit answers from the entry" gains a planted spare byte that must move the score;
  - "the same pawns with other pieces read their own terms" gains pairs that differ in an outpost cell and in the
    pieces product;
  - "a mirrored position reads the same through one table" covers the bytes' colours.
- **INV-5:** "the outpost family's counts mirror exactly" and "the space family's counts mirror exactly", beside the
  passer, structure and threat cases.
- **INV-6 at the landing:** `tools/search_bench.py` identical, and bench unchanged at the seed.
- **The trace's own tests:** "the positions move every term both ways" gains positions for each new cell, a pawnless
  pair among them, and "every coefficient is the count its index names" gains the cells.

**The screen, as proposed: a pre-registration draft** (S122's and S124's form). Nothing here is fixed until the
coordinator writes the screen's script and this becomes its header.
- **The corpus.** S122's regenerated prepared corpus, at the landed tree's own K, re-read after S122 and S124 land.
  The instrument is its own held-out qs view, DEC-350 (7)'s.
- **The identity rule before any fit.** `feature_audit --free outposts,space`: every new column non-zero, and the
  rank up by 24 on the landed tree's, expected and decided by the run. On S124's phase 1 tree that is 1118 + 24 of
  1214.
- **Parts P1 to P9**, in the order above, each by `--only` with the earlier survivors live and the old constants
  frozen. Each is paired against its predecessor: the landed tree, plus the survivors so far.
  - **Kept iff d > m, where m = max(1.645 sd(d_s), 2e-6)** over five quarter samples. That is DEC-331 (6)'s rule as
    DEC-344 (3) and DEC-350 apply it: the margin is the five samples' own spread, with a floor at 2e-6.
  - Every fit runs `--epochs 20000 --report 100 --patience 20` (DEC-345 (1)).
  - Each d is read at the fit's weights and again at the integers the engine ships. A part kept at real weights alone
    stops for the coordinator.
- **The cost.** `S102_cost.sh`'s sink at the survivors' bits. It decides nothing; the verdict carries it.
- **The SPRT.** One `{0, 5}` nElo of the survivors, at 8+0.08, Hash 16, `books/noob_3moves.epd`, concurrency 12, against
  the landed tree (DEC-259 (b)).
  - DEC-143's worst case is 41861 games, 19.84 h at 2110 games an hour, at the midpoint; 25591 games, 12.13 h, on a
    bound; the cap is 20950 rounds, 41900 games, 19.86 h.
  - The abort rule is S101's: a time-forfeit rate past 1.0 % on either side, a crash or a disconnect, mains power lost,
    another load on the machine, a banner not the pinned one, or 40 h with neither bound.
  - An A/A of 1000 games comes first if the harness moved since the last.
  - The open DEC-171 findings at the run's HEAD are named by id.
- **On H0 or no verdict:** the accepts' bisection into the two terms, in order, each `{0, 5}` against the landed tree,
  survivors only. A is outposts, at the screen's fit of its last survivor. B is space, refit with the outpost
  weights at zero.
- **Machine hours, inferred and not timed.** S122's screen's own margin fit wrote epoch 1700 at 22:28:20 and epoch 4300
  at 22:41:23, so 0.30 s an epoch on 6307294 rows of 38.34 entries, 50 free, the machine shared with this pass's niced
  work.
  - A part's six fits are about 2.25 full-corpus fits: about 28 min at a linear part's patience (about 2500 epochs,
    S122's figure), 3 h 45 m on the cap.
  - Nine parts: about 4 h 15 m typical, 34 h worst. The identity rule and K are minutes, and the cost about 1 h.
  - The SPRT is 12.1 to 19.9 h, so about 17 to 25 h in all, and up to 55 h.
  - H0 adds up to two verdicts of 19.9 h.

**Interactions.**
- **S122 lands first.** Its terms are stage two's, behind the shortcut and under the clamp. These are stage one's, so
  its curve, its margin re-derivation (DEC-324) and `eval_spread`'s reading are untouched, and this family's fits
  freeze S122's fitted constants. S122's `ks_defenders` reads own minors' attacks in the zone, and the outposts read
  own minors in the region. The two meet only where the king stands that far up; the walk and the screen hold them
  apart.
- **S124's factor on E1.** E1 is the material plus `eg_value` of the positional pair, and both terms land in that pair.
  So in a scaled cell their endgame halves are scaled by f_c / F (reading A), and the tuner's product (DEC-350 (5))
  covers their endgame weights as written. Their endgame weights move E1's sign, which picks the cell; both candidate
  cells are traced, so nothing goes stale. OCB_PURE has no knight, so only P5 fires there. Space fires in every cell.
  - Putting these terms outside E1 would need a site and a division of their own, against the accepts' "keeps the
    divisions HEAD has", so they stay inside it.
  - Phase 1 rebases on S124's landing: `PARAM_COUNT` 1190 to 1214, and S124's 16-bit and int32 headroom asserts gain
    these terms' reach.
- **Mobility and threats.** S121's knight cells count squares no enemy pawn attacks. S101 owns attacks on enemy men,
  and an outpost knight cannot be S101's hanging man. With every mobility and threat column in the regression, the
  outpost cells stay 63 % to 92 % new, the hole aside (49 % and 29 %), per the table above. No column is shared,
  so the fit prices the overlap and nothing is double-counted by construction.
- **The clamp and the lazy margin.** Stage one's terms are outside the clamped sum, so |`evaluate` - cheap| <= the
  margin holds by construction. The cheap score moves, which changes the nodes the shortcut takes: a play change the
  SPRT reads. Were outposts ruled into stage two (Q5), they would enter the clamped sum, and DEC-324's re-derivation
  would bind here too.
- **S133's king-relative tables**, later. Pawn tables indexed by a king bucket would span more of space's count, which
  the flat tables already span to 98 %. Knight tables by bucket may take some of P1, and of the hole, which the
  tables already span most. S133's research pass should re-read these readings and R^2 at its tables, and S126 refits
  both.
- **S135's placement.** The open-files factor reads the file sets the rook counts read. The trapped minors stand on the
  a and h files at own ranks 6 to 8, so only a trapped minor there can also count here, and the fit prices that
  overlap.

**Questions**, each in DEC-265's form; the draft carries them as proposals. **None is the owner's.** Each amends this
step's text or a coordinator's ruling, and no owner's decision. Reserved to the owner at the landing, as DEC-344 (9)
lists them for every evaluation family: `EXACT_FLOOR` under 157, `MATE_IN_THREE_FLOOR` under 11, or a mate-carry
ceiling rising by more than 2 (DEC-299, DEC-303, DEC-330, DEC-343). These are read only if the candidate moves them.

1. **Bishops on an outpost (P5). The coordinator's.** *Why:* section 2 bars it (the amendment above). The page names "a
   diagonal outpost" and does not define it. Across halves it adds -12.97 +/- 3.06 over P1 to P4, on 49211 games.
   - *(a)* Screen it as P5, chesso's definition, the page's clauses with a bishop. It costs +0.35 % on `bench` over
     the no-exchange row, beside its share of the wider gate's +1.43, and the screen decides.
   - *(b)* Leave it out and record the reading as a lead: 0 h, and whatever it carries is unmeasured.

   **Recommended (a).**
2. **Reach (P6). The coordinator's.** *Why:* it is the largest outpost signal, -49.45 +/- 5.33 over P1 to P5. The page
   says "about to be occupied by a knight" without a rule. Section 2 and the `excludes:` line bar it as written.
   - *(a)* Screen it as P6, defined set-wise, the form shown equal to the census's on every row. It costs +1.23 % on
     `bench` as a loop over the knights and +1.73 % set-wise, each over the bishops row (+1.25 and +1.80 on `bench
     16`), beside its share of the wider gate's +1.43. The two forms read the same feature on every row, so the code
     form is phase 1's pick on cost. Neither is a pass over the board (above).
   - *(b)* Leave it out, as a step of its own.

   **Recommended (a):** what is screened is what ships, and the stamp says the rule is chesso's own.
3. **Space's weight. The coordinator's.** *Why:* the page gives one multiplier, own pieces less open files, and does not
   say what "own pieces" counts. Across halves, the two factors apart read -128.73 +/- 8.63 over the tie. At integer
   weights the factors apart keep 94 % of their in-sample gain, the every-man single weight 63 %.
   - *(a)* The page's single multiplier, with knights to queens.
   - *(b)* The same, with every man but the king.
   - *(c)* The factors apart, P8 and P9, with the page's form as the case w8 = -w9.
   - *(d)* (c) with P9 tied to -P8: the page's form exactly.

   **Recommended (c):** a superset whose special case is the page's, each factor screened alone, robust to rounding.
   It departs from the page's coefficients, and the stamp says so.
4. **The order of the parts. The coordinator's.** *Why:* a later part gets credit only for its increment (DEC-344 (3)'s
   reasoning). *(a)* Described first: the page's definition and its factor, then the departures (hole, flank, bishop,
   reach); for space the count, then the weight's factors. *(b)* By signal: reach, the flank, the bishop, the hole, the
   exchange. Every increment moves by under one error between the two orders (above). **Recommended (a):** the
   described form claims what it carries, and each departure earns over it.
5. **The outposts' stage. The coordinator's.** *Why:* every outpost part costs +5.93 % in stage one (reach set-wise,
   the wide gate) and +3.47 % in stage two, where the sink recomputes what the mobility loop holds (+6.13 % and +3.67 %
   on `bench 16`). Stage two would read reach off that loop nearly free (inferred).
   - *(a)* Stage one, the step's "cheap-stage by construction": no clamp, every node sees the term.
   - *(b)* Stage two: under the clamp and skipped by the shortcut on about half of lazy calls, and DEC-324's margin
     re-derivation binds on this family too.

   **Recommended (a).**
6. **Section 4's bands and the integers. The coordinator's.** *Why:* the bands exclude the signed steps the readings
   ask for, and no non-zero integer weight on a product fits them.
   - *(a)* The bands stay sweep ranges and the fit ships signed integers (DEC-084). The screen reads each d again at the
     integers. The whole family's step keeps 94 % of its in-sample gain there (-326.4 of -346.7).
   - *(b)* A fixed-point divisor for the products: one more truncating division and the bound moves, against the
     accepts' "keeps the divisions HEAD has".

   **Recommended (a).**
7. **The region's departure (P4). The coordinator's**, new with the derived region. *Why:* the page's region leaves out
   own rank 4 off d and e. The flank's knights add -16.78 +/- 3.75 over P1 to P3, and the first reading's region, whose
   holes, bishops and reach targets also cover the flank, reads -21.49 +/- 3.90 better than P1 to P6.
   - *(a)* P4 as proposed: the flank's knights, one cell, a departure.
   - *(b)* No flank: the page's region exactly, and the flank's reading unmeasured by the screen.
   - *(c)* The flank for every outpost feature: three more cells (holes, bishops, reach targets on the flank), not read
     by this pass.

   **Recommended (a):** one measured cell, and (c) waits on a reading it does not have.

**Findings beyond the step, not fixed here.**
- (a) Section 1 quotes half of *Space*'s concrete form and none of *Outposts*' refinements (above).
- (b) The accepts still names `tools/eval_model.hpp`, deleted at S276; the 2026-10-08 amendment discharges it. The
  step's `touches:` lacks what this design needs: `src/eval_trace.hpp`, `tools/tuner_layout.hpp`,
  `tools/feature_audit.cpp`, `tools/truncation_scan.cpp` and `tools/eval_spread.cpp` (headers),
  `tests/test_eval_trace.cpp`, `tests/test_eval_model.cpp`, `tests/test_walked_counts.hpp`,
  `tests/test_tuner_groups.cpp`, `tests/test_tuner_gradient.cpp`, `tests/test_eval_positions.hpp`.
- (c) Section 7's "S121 (before): builds the enemy pawn-attack span both terms read" is stale. S121's mobility reads the
  enemy pawn attacks, and the span is `pawn_pass`'s since S263.
- (d) `pawn_entry_t`'s `spare` is never written today. `evaluate_cheap`'s local entry leaves it indeterminate, and
  `pawn_entry_is_fresh` does not compare it. This is harmless while nothing reads it. Phase 1's plan above closes all
  three before anything stores there.
- (e) S122's phase 1 estimates "about 0.87 s a part-epoch", a part being six fits, about 2.25 full-corpus fits. Like for
  like, its screen's own log reads 0.30 s a full-corpus epoch, so 0.87 / (0.30 x 2.25) is about 1.3: its machine-hour
  figures, and S124's taken from them, may run about 1.3 times long (inferred, not timed).

**Unmeasured or inferred, flagged.**
- No fit ran: every "the fit would" is inferred.
- The label readings are a single Gauss-Newton step, in sample and across halves, at the corpus's own K; held-out
  loss is the screen's.
- The search's distribution of these features is unmeasured.
- The cost rows are a sink's instructions.
- No part's Elo, no nps.

## 2026-11-04: the ruling, DEC-351

The research pass, revised after the coordinator's fast check, is ruled in DEC-351. Every item of the decision draft
is taken as written, (1) to (13), and every question takes the pass's recommendation:
- Q1 (a), bishops on an outpost screened as P5;
- Q2 (a), reach screened as P6, defined set-wise; the code form, set-wise or the per-knight loop, is phase 1's pick on
  cost, the two reading the same count on every row of five corpora;
- Q3 (c), the page's weight with its factors apart, P8 and P9; the page's own form is the tie w8 = -w9;
- Q4 (a), the page's definition first, each departure after it;
- Q5 (a), outposts in stage one, outside the clamp, at about 2.5 points of instructions over stage two;
- Q6 (a), seeds at zero, the fit ships signed integers (DEC-084), and each part's gain is read again at them;
- Q7 (a), the flank's knights alone as P4, a departure measured as one.

**Changed in the step's own text, history kept (DEC-348):** section 2's sentence on bishop and reachable outposts,
the `excludes:` line, the accepts' `tools/eval_model.hpp` clause (the file went at S276; the trace takes each
feature), `touches:` (draft item (12)) and `decisions:`. Section 7's sentence that the enemy pawn-attack span is
S121's input is stale: S121 reads the enemy pawn attacks, and the span is `pawn_pass`'s since S263 (the pass's
finding (c)).

**The pawn entry's rule binds every later field:** a field `pawn_entry_t` stores must read correctly from a zero slot
under a pawnless key, because the table is zeroed every `go` and verified by the pawn bitboards alone. Phase 1 builds
the pawnless table test, red on a planted plain count, before anything stores in `spare`.

**Order.** Phase 1 is built on S124's landed tree and the screen runs after S124's fits. Every reading here is
re-read on the landed tree, at its K and through its trace.

**Unmeasured at this ruling:**
- every fit and every held-out loss: the readings are one Gauss-Newton step per set, in sample and across halves, on
  the training corpus at K 1.2302;
- the search's distribution of the features;
- cycles and nps: the cost rows are a sink's instructions;
- every part's strength.

## 2026-11-06: phase 1 starts, the coordinator

Worktree `../chesso-s102p1`, branch `s102p1`, at `ef866de`. That tree has S124 phase 1 landed at its identity, and
S124's screen is still running. One Opus 5.5 subagent builds the family at its identity under DEC-351:
- seeds at zero, in no expression, INV-6 against the parent;
- the trace and the tuner groups;
- the pawnless-table test, red first;
- the census, and each guard seen red on its plant.

It runs no fit or match. If S124's candidate lands first, phase 1 rebases onto it, re-deriving its goldens, as S124 did
onto S122.
