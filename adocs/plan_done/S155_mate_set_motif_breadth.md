id:         S155
goal:       the constructed mate set's single motif is stated where its breadth is claimed, and what the gate therefore cannot catch is written down
accepts:    the constructed set's single motif is stated wherever its breadth is claimed -- every one of the 48 positions is a blocked pawn wall with a rook-bishop-rook-bishop battery, one queen and `lead` 760, varying only by `shift0`/`shift2`, mirror and colour -- and the reason it is close to forced by the S033 construction is stated with it, so the qualifier does not read as a defect in the set; what the gate therefore cannot catch is written down as a list rather than implied: back-rank, smothered, king-hunt and open-line mates, and any position with a realistic material balance; whether a second motif is worth constructing is answered either way and the answer is recorded; no default changes and no position in the existing set is removed
touches:    tests/test_engine.cpp, adocs/specs.md, adocs/data/
excludes:   rewriting `adocs/plan_done/S145_mate_safety_test_set.md`; removing or replacing any of the 48 positions, every one of which was independently re-proved a forced mate at its claimed distance; the mined set, which is S156
decisions:  DEC-016, DEC-095, DEC-114
closes:     2026-08-21_adversarial-F07
blocks:
paused_by:
author:     claude-opus-5, coordinator
done:       2026-09-01. The motif is counted, not conceded: `adocs/data/S155_motif_census.py` reads the tracked TSV and reports **two material signatures over the 48 and one is the colour mirror of the other** (`K Q PPP` against `K RR BB ppp`), a **lone queen as the mating force in 48 of 48**, `lead` **760 in 48 of 48**, a pawn wall on three non-adjacent files throughout, and eight family labels that are one geometry under two file shifts, a mirror and a colour swap -- against real breadth in distance, 16 / 16 / 8 / 8 at two to five. The qualifier now stands in five places, each beside the breadth claim it qualifies: `tests/test_engine.cpp`'s suite comment, `adocs/specs.md`'s reverse-futility paragraph, `adocs/data/S145_mate_set.py`'s docstring, `DEV_MANUAL.md`'s mate-testing section (with the census command), and `MANUAL.md`'s limits entry -- the last two outside `touches:`, written because the DOCS rule checks both manuals and both carried the claim. Each states the reason the narrowness is close to forced by the hazard, so it does not read as carelessness, and each carries the same explicit list of what the gate cannot catch: back-rank, smothered and any knight mate, king hunt, open-line mate or line-opening sacrifice, promotion mate, and any position with a realistic material balance. **The second-motif question is answered yes, by the owner, 2026-09-01, recorded as DEC-114**, and is `plan_todo/S168_second_mate_motif.md` rather than work folded in here, because a new family owes its own two proofs, its own regeneration and its own reverse-futility sweep before the two floors can be restated. No default changed, no position removed, no assertion added or weakened: the 48 cases, `MATE_IN_THREE_FLOOR` 7 and `MATE_DEPTH_SLACK` 8 are byte-identical, so this step alters no behaviour and owes no SPRT. Green: `cmake --build build -j8`, `ctest -L fast` 21 of 21 in 30.56 s, `./clang-format.sh --check` exit 0. `2026-08-21_adversarial-F07` closed in the report.

## What was verified, so this is not read as doubt about the set

All 48 positions were re-proved from scratch by two oracles written without
reference to the repository script: 48 of 48 by an AND/OR enumeration iterated to
distance 6, and 48 of 48 by stockfish at 20 M nodes, 0 disagreements. The
construction regenerates the tracked TSV byte-identically. The set is sound. What
is missing is the qualifier beside the breadth claim.

## The census, which is what makes the motif a measurement

`adocs/data/S155_motif_census.py` reads the tracked TSV and prints what varies
and what does not. It reads that file only -- no engine, no oracle, no
python-chess -- because re-proving the set is `S145_mate_set.py verify`'s job
and this is a census, not a third proof. Over the 48 positions:

| what | reading |
|---|---|
| material signature | **2**, and one is the colour mirror of the other: `K Q PPP` against `K RR BB ppp` |
| mating force, side to move | a **lone queen, 48 of 48** |
| `lead`, the mated side's material lead | **760, 48 of 48** |
| pawn files | three non-adjacent files throughout: `ace`, `bdf`, `ceg`, `dfh` |
| family | 8 labels = one geometry x `shift0`/`shift2` x mirror x colour |
| proved distance | 16 / 16 / 8 / 8 at two, three, four and five |

Breadth in mate distance is real and breadth in motif is absent. That is the
audit's finding and it reproduces.

## Why the narrowness is close to forced, stated so it does not read as a defect

Reverse futility returns a static score instead of searching when
`static_score - RFP_MARGIN * depth >= beta`, at a non-PV node that is not in
check and at ply >= `RFP_MIN_PLY`. A static score is never a mate score, so the
node that can be hidden is one that is **lost by force while the side to move is
materially ahead**. Nothing in ordinary play offers that -- S145 measured it: of
191 positions in a 6347-position sample where the side to move is mated within
six, one has a non-negative score for the mated side and the median is -1093 --
so the positions are built, and a frozen clump behind a blocked pawn wall is
close to the only way to build the property. Every row reading `lead` 760 is
that constraint showing through, not carelessness. S033 built its one position
by hand the same way; S145's construction generalises the shape rather than
choosing it.

## What the gate therefore cannot catch, as a list

A pruning rule that hides any of these passes the whole suite:

- a back-rank mate;
- a smothered mate, or any mate delivered by a knight -- the mating piece here
  is a queen in 48 of 48;
- a king hunt, where the king is driven across the board instead of held in a
  pocket;
- an open-line mate, or the sacrifice that opens the line;
- a promotion mate -- no pawn in this set can promote;
- any mate in a position with a realistic material balance.

## The second-motif question, answered

**Answered yes, by the owner, 2026-09-01: a second motif is worth constructing,
as its own step. DEC-114.** The census is the reason -- one mating piece across the whole
set means a defect that depends on the mating piece is invisible here -- and the
step is **S168**, in `plan_todo/`, because a new family owes its own two proofs,
its own regeneration of the TSV and its own reverse-futility sweep before the
two floors in `tests/test_engine.cpp` can be restated. Folding that into this
step would have made a documentary step one that cannot close.

Recorded against the alternative that was not taken: answering *no* on S145's
own measurement -- that the reading is almost entirely a function of mate
distance, 16/16 at two against 0/8 at five -- is defensible and was rejected,
because that measurement is over one motif and therefore cannot say whether
shape matters.

## Where the qualifier now stands

- `tests/test_engine.cpp`, in the suite comment that introduces the set, next to
  the breadth claim it qualifies.
- `adocs/specs.md`, in the reverse-futility paragraph, immediately after the
  "48 forced mates spanning mate distances two to five" sentence.
- `adocs/data/S145_mate_set.py`'s module docstring, where the construction is
  described, so anyone regenerating the file reads it before they do.
- `DEV_MANUAL.md`, in the mate-testing section, beside the distance table that
  is the breadth claim, with the census command added to the block above it.
- `MANUAL.md`, in the reverse-futility limits entry, as one clause naming the
  motif and pointing at `DEV_MANUAL.md` for the list.

The last two are outside this step's `touches:` and were written because the
DOCS rule checks both manuals at every step completion and both claimed the
breadth. Recorded as a deviation rather than an amended field: the step's scope
did not change, its `accepts` said *wherever* the claim is made, and these are
two more wheres.

`adocs/plan_done/S145_mate_safety_test_set.md` is untouched: `plan_done/` is
history and a done step that got something wrong gets a new step, which is this
one.
