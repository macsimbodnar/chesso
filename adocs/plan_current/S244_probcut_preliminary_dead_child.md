id:         S244
goal:       ProbCut's preliminary answers a dead-board child as the draw it is instead of entering quiescence on it, so S210's rule holds for every caller of quiescence
accepts:    a capture in ProbCut's loop that leaves insufficient material is answered `DRAW_SCORE` before `quiescence` is entered, so no material score of a dead board is computed or stored at `TT_DEPTH_QS`; the S210 comment in `quiescence` reads true again without its S113 exception; a test drives the case and is observed red without the screen; node counts move only where a dead child was entered, and the change is decided the way its reach says -- INV-6 identity if the bench stream and `search_bench` do not move, otherwise one `{-5, 0}` non-regression SPRT priced per DEC-143 -- with the reach counted on the bench positions first (DEC-107's census discharge if it applies); the fast suite green in both builds
touches:    src/search.cpp negamax_at (the ProbCut loop), tests/test_search.cpp
excludes:   every other ProbCut behaviour; the quiescence side of S210
decisions:  DEC-171, DEC-215, DEC-233
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-29 09:31 CEST
done:

## Why this exists

S113's cold fast check (2026-09-28): the preliminary of ProbCut is the first
caller that enters `quiescence` at a child `negamax_at` never screened, so a
capture leaving insufficient material is scored on the material left and
stored at `TT_DEPTH_QS`, which S210's comment in `quiescence` said could not
happen. No cut is wrong -- the shallow `negamax_at` that follows returns
`DRAW_SCORE` before the cut test, and `negamax_at` answers a dead node before
it probes the table -- so the cost is one wasted preliminary and a stale
entry no main-search node reads. Recorded as open finding 6 of
`adocs/data/S113_sprt.sh`, carried by S131's block, and scheduled here under
DEC-171 as a filler behind the strength steps. It is cheap to write and dear
to measure: a screen in the loop moves node counts wherever a dead child was
entered, so the accepts asks for the reach first -- over the bench positions
at depth 12 and 14 the census says how often the case occurs -- and for the
cheapest proof the reach allows. If a later ProbCut change owes an SPRT
anyway, this folds into it and the step closes on that verdict.

## What the tree did, 2026-09-29 (checked at `0c0db1b`)

The finding reads true on the tree, and it is one clause short. ProbCut's loop
in `src/search.cpp` `negamax_at` made each filtered capture and called
`quiescence` on the child with no test; `negamax_at` answers a dead node at
its top, before the table probe, so the shallow search after a held
preliminary returned `DRAW_SCORE` and no cut was ever wrong. **What the
finding did not say: a cut could be missing.** The draw met the bar only
where the preliminary's material score had cleared it first. Where the side
making the capture is the one short of material -- a king taking the last
pawn of an opponent left a knight -- the material score fails the bar, the
shallow search is never asked, and the capture's value, the draw, never
meets the bar at all, though it clears it. The drive below shows both
halves on the parent: a dead child stored at `TT_DEPTH_QS` with a material
score (113), and no cut with the bar on the draw. So the screen changes a
decision in the class, not only its node count, and the reach below is what
bounds that.

## Reach, counted before the screen (DEC-239: reach, not a forecast)

An instrumented copy of `0c0db1b` (`.ref-builds/reach`, a plain copy with
write-only counters, `.tuning/coord/S244_reach/instrument.py`) benches the
parent's own totals -- 3656950 at 14 and 1702684 at 12 with the parent's
eight replies, `search_bench` at 12 128099 / 258594 / 83277 with its three
best moves -- so its counters are write-only.

| run | ProbCut nodes entered | captures made | leaving a dead board |
|---|---|---|---|
| `bench` (14) | 2914 | 3040 | **0** |
| `bench 12` | 1232 | 1372 | **0** |
| `search_bench` 12 | 713 | 957 | **0** |

With none dead, every counter downstream is 0 as well: preliminaries entered,
preliminaries held, cuts on the draw, cuts the screen adds, `TT_DEPTH_QS`
entries stored at a dead board, entries they evicted, and reads of such an
entry by `negamax_at` or `quiescence` (`.tuning/coord/S244_reach/*.tsv`).
**No bench position puts a dead board inside ProbCut's loop at any depth
taken**, which is what makes the INV-6 identity below vacuous for the class:
it says the bench cannot see the change, not that the change is inert.

## What changed

- `src/search.cpp` `negamax_at`, ProbCut's loop: after `make_move`,
  `pc_dead = is_insufficient_material(...)`, S210's test on the board the
  capture leaves. A dead child's value is `-DRAW_SCORE`, in the child's frame
  and negated as S210's is; neither `quiescence` nor the shallow
  `negamax_at` is called for it; `probcut_tried` is not counted, so the probe
  field keeps its meaning, "the moves that paid the preliminary". The draw
  then meets the bar like any other value: a bar at or under the draw ends
  the node on it, stored as the block stores every cut.
- `src/search.cpp` `quiescence`: S210's comment reads true again without its
  S113 exception -- the node quiescence is entered at cannot be dead already,
  because `negamax_at` tested it first, at the top of every node and in
  ProbCut's loop before the preliminary.
- `tests/test_search.cpp` `probcut_drive_t`, one new case, "probcut answers a
  capture that leaves a dead board as a draw", on
  `n6k/8/8/8/3pK3/8/8/8 w - - 0 1`, White to move, whose one capture Kxd4
  leaves king against king and knight. From a tool: python-chess reports
  `is_valid()` True, `is_check()` False, 7 legal moves, one capture, no
  promotion, and `is_insufficient_material()` True after Kxd4
  (`.tuning/coord/S244_reach/verify_fen.py`); the engine's own test agrees
  and the case asserts it. Two legs, betas derived from
  `ProbCutMargin`: the bar a point above the draw (no cut, the move loop runs,
  nothing paid) and the bar on the draw (the node ends on the draw, Kxd4 its
  move, before its move loop). In both: no preliminary paid and no entry at
  the dead child -- CHECKs, so a red run shows both.
- `tools/mutants/S244_probcut_dead_child.py`, new: PD01 the screen dropped,
  PD02 the dead child skipped instead of answered (its draw never meets the
  bar). Every single-letter prefix has been used on some branch, so the file
  opens a two-letter one and says why.
- `MANUAL.md`, the `ProbCut` row: one clause, a capture that leaves neither
  side the material to mate is answered as the draw with neither search.

Nothing else. Beyond `touches:` are the mutant file (DEC-141 clause 2) and
`MANUAL.md`'s row (DOCS).

## Red, then green

The case was written first and run on the parent's `src/`
(`.tuning/coord/S244_red_parent.log`), four failures: the bar-above leg
`probcut_tried` 1 against 0 and "the dead child has an entry, depth -1 score
113"; the bar-on-the-draw leg `probcut_tried` 1 and `REQUIRE(
record.probcut_cutoff )` false. With the screen: the ProbCut cases 9 of 9 in
Release (297 assertions) and 11 of 11 in the tune build (327).

## INV-6

The candidate against the parent, both Release builds of this worktree: the
whole `bench` stream at 14, 12 and 9 -- every `info` line's depth, score,
nodes and PV, every `bestmove`, every total -- and `tools/search_bench.py` at
9 and 12, compared with `time`, `nps` and `knps` stripped: **identical**
(`.tuning/coord/S244_counts_parent.log`, `S244_counts_candidate.log`).
`bench` 3656950, `bench 12` 1702684, `bench 9` 520727; replies c3d5 d5e6
d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 at 14 and 12, c3d5 e2a6 d7c8q g7h8q f8e8 a1b2
e5e6 e5e6 at 9; `search_bench` 9 70913 / 71220 / 24293 (c3d5 e2a6 d7c8q), 12
128099 / 258594 / 83277 (c3d5 d5e6 d7c8q). The reach above is why, and it is
also why this identity says nothing about games.

## Ordinary play, the S210 census's instrument (reach, DEC-239)

Because the bench cannot see the class, the class was counted where it
lives, with the instrument and sample S210 used for its own dead-board rule:
`adocs/data/S210_f22_census.py positions` over the S219 A/A PGN, every tenth
ply, **11503 positions**, each answered at `go depth 10` from `ucinewgame` by
the instrumented parent copy and by the candidate, both Release, niced beside
the SPRT (`.tuning/coord/S244_reach/games.py`, `games_*_d10.*`).

| counter, over the 11503 searches | count |
|---|---|
| ProbCut nodes entered / captures made | 518943 / 393040 |
| captures leaving a dead board, every one entering the preliminary | **175** (0.045 %) |
| ... the preliminary held, the shallow search paid | 149 |
| ... the node cut on the draw, after a held preliminary | 95 |
| ... **no cut, though the draw clears the bar** (what the screen adds) | **8** |
| ... the draw under the bar, no cut either way | 72 |
| nodes the two searches spent on dead children | 324 |
| `TT_DEPTH_QS` entries stored at a dead board / another position's entry evicted | 175 / 3 |
| reads of such an entry by `negamax_at` / by `quiescence` | 0 / 60 |

And the answers, parent against candidate: **0 of 11503 best moves and 0 of
11503 scores moved**; 65 node counts moved, all 65 on boards of three to five
men, each by 1 to 28 nodes and every one down, 760090008 -> 760089679 in all
(`games_compare_d10.txt`). So the class is reached in ordinary play, rarely,
and here it changed the tree in 0.57 % of positions and no answer. That is
reach and not a value (DEC-239); what contrasts it with S210's own census,
which owed that step a run, is the answers -- 194 of 11503 moved there, none
here. The 60 quiescence reads are the stale entries being used: a second
preliminary at the same dead board answered from the first one's material
score.

## Decided by reach

The accepts' first branch: the bench stream and `search_bench` do not move,
so the change is decided by INV-6 identity and **no run is owed**. The census
above is recorded beside it because DEC-107 refuses to call a change
behaviour-neutral on bench counts alone where the class executes in games,
and this one does: it is DEC-107's own discharge that holds here as well --
a correctness fix on a rare boundary, whose census over the games that would
carry the effect moved no answer -- not the bench identity by itself.

## Mutation (DEC-141 clause 2)

`tools/mutants/S244_probcut_dead_child.py` on a clean detached fixture,
`.ref-builds/mut` at `1a8b7c5` -- a throwaway commit of this working tree on
no branch, cut through a temporary index so the worktree's own index was
never touched, its `src/`, `tests/` and `tools/` this tree's byte for byte
(it predates only the `MANUAL.md` clause and this file's sections) -- with
doctest cloned into it from the worktree's copy, run from the fixture so the
list read is its own: `tools/mutation_check.py tools/mutants . --only PD01
PD02 --jobs 1`, niced, which validates all 164 anchors of the directory
first. Header `worktree .../.ref-builds/mut at 1a8b7c5 clean`, `list
.../.ref-builds/mut/tools/mutants clean`, `mutants 2 of 164`, `baseline
green, 41 tests, bench 3656950 nodes via engine`; **mutation score 2 of 2
(100%), killed 2**, wall 1017 s, `MUTATION-RUN-DONE`, the fixture clean after
(`.tuning/coord/S244_mutation.log`, `S244_mutation/results.tsv`).

| mutant | bench | killed by |
|---|---|---|
| PD01 screen dropped | same | the new case, `CHECK_EQ( record.probcut_tried, 0 )` and three more |
| PD02 dead child never cuts | same | the new case, `REQUIRE( record.probcut_cutoff )` in the bar-on-the-draw leg |

Both are bench-blind, as the reach says they must be: only the case sees
them. The first attempt, at `--jobs 4`, was refused at its baseline --
`test_mate_carry` Timeout at 120.23 s under the S022 verdict 2 SPRT's load
beside a gate of the coordinator's (`.tuning/coord/S244_mutation_try1.log`);
the serial run passed it.

## Suites and checks

Both fast suites green, niced beside the SPRT: Release 41 of 41, tune 41 of
41 (`test_mate_carry` 117.21 s against its 120 s ceiling, a load reading);
`./clang-format.sh --check` exit 0 with `CLANG_FORMAT_MAJOR=22`;
`tools/plan_prose_check.py` `--citations` 0 flagged over 39 files,
`--touches` 0 flagged, `--params` and `--gate` clean
(`.tuning/coord/S244_gate.log`).

## Documents

- `MANUAL.md`: the `ProbCut` row gains the clause above; no number in the
  table moves, so `test_uci_surface` needs no refresh.
- `DEV_MANUAL.md`: checked, no change -- the bench total does not move, so
  the ledger takes no row, and no golden of its DEC-142 list is touched.
- `adocs/specs.md` is the coordinator's; the proposed sentence is below.
- `README.md` is the owner's, untouched.

## Not run here, by the brief

DEC-141's second tier -- the Debug self-play of four rounds at 4+0.04 grepped
for `Assertion`, and `tools/gate_extra.sh` -- is owed before this completes,
because the step touches the search. The machine is the S022 verdict 2
SPRT's, and a match is the coordinator's to start.

## Proposed `specs.md` sentence (the coordinator edits specs.md)

In the ProbCut sentence, after "a zero-window search of the capture's reply
at `depth - ProbCutDepthOffset` (5, so the node-level pair is the paper's (4,
8))": "; a capture that leaves insufficient material is answered `DRAW_SCORE`
before either, so no dead board is scored on its material or stored, and the
draw meets the bar like any other value (S244)". The S210 sentence's "the
entered node was answered a ply higher" then holds for every caller of
quiescence, ProbCut's preliminary included.

## Proposed commit text

`.tuning/coord/S244_commit_msg.txt`: subject "Answer a dead-board ProbCut
capture as the draw it is (S244)", the body's why, the reach, INV-6 and the
census, the case and the mutation score, and `Bench: 3656950` -- the
parent's total, since `src/` moves and no node does.

## Proposed `done:` stamp (written by the coordinator after the second tier)

2026-09-29 -- ProbCut's loop in `negamax_at` makes S210's test on the board
each capture leaves and answers a dead child `DRAW_SCORE` with neither the
preliminary quiescence nor the shallow search, so no material score of a
dead board is computed or stored at `TT_DEPTH_QS`; the draw meets the bar
like any other value, which also ends the nodes the parent never cut, where
the capturing side was short of material and the material score failed the
bar first -- the clause open finding 6 lacked, observed on the drive.
S210's comment in `quiescence` reads true again without its S113 exception.
**Reach first**, on a write-only instrumented copy benching the parent's own
totals: 0 of 3040 ProbCut captures dead at `bench`, 0 of 1372 at `bench 12`,
0 of 957 over `search_bench` at 12. **Decided by INV-6 identity, no run
owed**: the bench stream at 14, 12 and 9 and `search_bench` at 9 and 12
node-identical to `0c0db1b`, every best move the same, `bench` 3656950.
Because that identity cannot see the class, the class was counted in games
with S210's instrument, the 11503 S219 A/A positions at depth 10: 175 dead
captures of 393040, 8 cuts the screen adds, 65 node counts down by 1 to 28,
**0 best moves and 0 scores moved** -- reach, not a value (DEC-239), and
DEC-107's discharge. "probcut answers a capture that leaves a dead board as
a draw" observed red on the parent (preliminary paid; dead child stored at
depth -1, score 113; no cut with the bar on the draw) and green with the
screen; `tools/mutants/S244_probcut_dead_child.py` 2 of 2 killed on a clean
fixture (`1a8b7c5`), both bench-blind. Both fast suites 41 of 41, format and
prose checks green. DEC-141's second tier: <the coordinator's result>.
`MANUAL.md`'s ProbCut row gains one clause; `DEV_MANUAL.md` checked, no
change; `specs.md` sentence proposed. Filler under DEC-171, named in the
pre-registrations taken while it was open. Written by an Opus subagent
briefed by the coordinator.
