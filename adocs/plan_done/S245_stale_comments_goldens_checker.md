id:         S245
goal:       five stale or wrong things the fillers of 2026-09-28 and 2026-09-29 noted and did not touch -- two comments, one timing golden, one checker defect, one budget table -- each verified against the tree and corrected or re-derived by its own script
accepts:    (1) the comment in `iterative_deepening_search` quoting 42371 nodes for the depth-1 iteration of the eight-queens board reads what the engine reports on that board, re-taken with `go depth 1`, and says which case, if any, still uses it (since S022 verdict 2 the stop half of "a stop inside the first iteration cuts it and the hard timer ends the search within its bound" runs on a heavier board, and the early-out cuts the eight-queens depth 1 to about 10187 nodes); (2) moot since S022 verdict 2, which moved the stop half off the eight-queens board and recorded the old golden at the site: verify the site's record and close the item without a re-derivation; (3) `tools/plan_prose_check.py`'s `holds_phrase` finds a quoted title that wraps across two string literals in the source (a test in `tests/` that plants such a citation and is red before the fix, green after); (4) the "needs fractional reductions" comments naming S237 and S238 in `src/search.cpp`, `src/search_params.hpp`, `tests/test_search.cpp` and `DEV_MANUAL.md` say what is true after both left; (5) `S170_cases.tsv`'s budgets are re-derived by `adocs/data/S203_case_sweep.sh` under the DEC-156 rule with DEC-162's stride, the old rows quoted; the fast suite green in both builds, and `bench` unchanged (comments and tests only) or the change's reach counted and decided the way its reach says
touches:    src/search.cpp (comments), src/search_params.hpp (comments), src/chesso.cpp (a comment), tests/test_engine.cpp, tests/test_search.cpp, tools/plan_prose_check.py, tests/ (a checker test), adocs/data/S170_cases.tsv, DEV_MANUAL.md
excludes:   any change to the engine's behaviour; any ceiling; the mined mate rows
decisions:  DEC-171, DEC-142, DEC-156, DEC-162
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-29 10:45 CEST
done:       2026-09-29 -- the five items, each verified against the tree first. (1) The comment in `iterative_deepening_search` names the eight-queens board by FEN, keeps its 13.5 ms and 42371 nodes as what it cost when the defect was found, and says `go depth 1` there reports 10187 nodes since S022's second verdict and that no case uses it, naming the case's two boards (the rook-and-knight board of 146994 nodes and the timer's own). (2) Moot as the accepts says: the first-iteration case's GOLDEN comment records the board's move and both numbers, which the re-take reads again; closed without a re-derivation. (3) `holds_phrase` flattens the phrase before the title map as well as on the fallback, so a title clang-format split across literals resolves when the prose wraps its quote -- S242's citation, which refused S243's first fixture, reproduced on that fixture and resolved after; the planted test in `tests/test_plan_citations.py` red before, green after. (4) The five comments naming S237 and S238 as the accumulator's consumers say both left on their H0s without carrying a fraction (S237 whole plies, DEC-232; S238 measured at 1024 ticks, one ply), the unit kept for `LmrRoundBias` alone. (5) The budgets re-derived by `adocs/data/S203_case_sweep.sh` under DEC-156 as DEC-162 left it, every row: A 500000, C 1000000, E 500000, F 100000, B and D unchanged, the old rows quoted in the TSV, the grid `adocs/data/S245_sweep.txt`, `--at` reproducing the six cells, no ceiling moved; the rule's text brought to DEC-162's form in the script, the test and the TSV, and the comments the new budgets made false corrected from the grid (the cold fast check's list). Both fast suites 41 of 41, format and prose checks green; `bench` 3656950, the Release binary S244's byte for byte. `DEV_MANUAL.md` two edits, `MANUAL.md` checked, no change. Filler under DEC-171, named in the pre-registrations taken while it was open. Written by an Opus subagent briefed by the coordinator.

Five things noted in passing by the agents of S242, S243 and the 2026-09-28
handover, none of them a defect reachable in ordinary play, on the UCI
surface or in a reported score (DEC-171): a source comment quoting a node
count the engine no longer reports; a timing golden recorded at 13.8 ms that
reads 6 to 7 ms today (its floor holds); `tools/plan_prose_check.py`
reporting a citation MISSING when its quoted title wraps across two string
literals in the source, which refused S243's first mutation fixture; the
"needs fractional reductions" comments that still name S237 and S238 after
both left; and `S170_cases.tsv`'s budgets, which S238's pre-registration
already recorded as not the DEC-156 rule's answer. Bundled as one filler
because each is an afternoon and none needs the machine; scheduled behind
the strength steps and named by id in every pre-registration taken while it
is open. Comments and tests only, so no verdict is owed unless (5) or (2)
moves a number the search reads, which they do not.

## The five items, each verified against the tree first (2026-09-29, on S244's tree)

S245 lands after S244 and was done on the same working tree, on top of it:
`0c0db1b` plus S244's screen. S244 moves no node on the bench positions
(INV-6; its basis is DEC-242, its census having moved 65 node counts in
games), and nothing below depends on it.

### 1. The comment in `iterative_deepening_search` quoting 42371 nodes

**Before**: "the board this repository's case uses spends 13.5 ms and 42371
nodes finishing a depth-1 iteration that was told to stop before it began" --
the eight-queens board, which no case uses since S022's second verdict.
**Re-taken**: `position fen q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - -
0 1`, `go depth 1`, waited out to its `bestmove` on the Release build: depth
1, **10187 nodes** (`.tuning/coord/S245_depth1.py`,
`S245_item1_depth1.log`); the case's current stop-half board,
`r1r1r1r1/1r1r1r1k/8/2n1n3/2N1N3/8/1R1R1R1K/R1R1R1R1 w - - 0 1`, **146994
nodes**, its GOLDEN's own count. `q1q1q1q1` occurs in `tests/` and `src/` only
in that case's history comment. **After**: the comment names the board by FEN,
keeps 13.5 ms and 42371 nodes as what it spent when the defect was found, says
`go depth 1` there reports 10187 nodes since S022's second verdict and that no
case uses it, and names the case's two boards -- the rook-and-knight board of
146994 nodes for the stop half and a board of its own for the hard timer. No
time was re-taken: the machine is the SPRT's, and a node count does not
depend on load.

### 2. The eight-queens depth-1 golden (13.8 ms recorded)

**Moot, verified and closed without a re-derivation**, as the accepts says.
The first-iteration case's GOLDEN comment in `tests/test_engine.cpp` records
the move: the stop half left the eight-queens board at S022's second verdict,
"36165 nodes and 6 to 7 ms at depth 1 before that verdict, recorded here as
13.8 ms when the case was written", and the early-out "cuts the iteration to
10187 nodes". Item 1's re-take reads 10187 on that board and 146994 on the
board that replaced it, the site's two numbers. No assertion reads the
eight-queens board any more, so nothing is owed; the floor is untouched. (The
brief asked for a re-derivation after the match; the accepts supersedes it and
is followed.)

### 3. `tools/plan_prose_check.py` `holds_phrase` and a split title

**The defect, found on the tree before the fix**: `titles_of` stitches a title
clang-format split across adjacent literals (S221) and flattens it, but
`holds_phrase` looked the phrase up in that map **as quoted** -- and a phrase
quoted across a line break of the hard-wrapped prose carries the break. The
raw-text fallback flattens, and by S221's design does not bridge the `" "`
between two literals. So a split title quoted across a prose wrap missed both:
exactly S242's step file citing its renamed case, which refused S243's first
mutation fixture (`.tuning/coord/S243_logs/fx_mutation_e21_refused_b9f17ee/`).
Reproduced on that fixture's own tree (`b9f17ee`, in a scratch copy): MISSING.
**The fix**: the phrase is whitespace-flattened before the title map as well,
one line, with the comment beside it; the docstring's "both sides are
whitespace-flattened" is now true of both branches. **The test**:
`tests/test_plan_citations.py`
`test_split_title_quoted_across_a_prose_wrap_passes`, S221's case with the
prose wrap added, the wrap planted at the literals' join and inside a literal,
against `tests/S221_split_title_fixture.cpp`. **Red before the fix**, both
sub-cases MISSING (`.tuning/coord/S245_item3_red.log`); **green after**, 20 of
20 (`S245_item3_green.log`); and the S243 fixture's citation resolves with the
fixed checker (0 flagged).

### 4. The "needs fractional reductions" comments naming S237 and S238

What they were, from `git log` and their step files: S237's hindsight
reductions moved a child's depth by **whole plies** (DEC-232 read its accepts
as a fixed one-ply correction) and left on an H0 (`d9bac46`, removed in
`1680439`); S238's cutoff count was written in the accumulator's ticks with a
range of 0 to 2048 but **seeded and measured at 1024, one ply**, and left on
an H0 (`95ea28d`, removed in `086320c`). So neither carried a fraction, and
no term in the tree carries one. What stays true: the unit is kept because it
is behaviour-neutral at `LmrRoundBias` 0, and a fraction is left for that bias
alone, rounding the table's own (S127 may sweep it). Rewritten to say so:

- `src/search.cpp`, "THE ACCUMULATOR'S UNIT": "because S237 and S238 both want
  a reduction that can carry a fraction" -> both left on their H0s, neither
  carrying one (S237 whole plies, DEC-232; S238 measured at 1024 ticks, one
  ply); no term carries a fraction; what one is left for is `LmrRoundBias`.
- `src/search.cpp` `lmr_adjusted_reduction`'s comment: "What it buys is that
  S237 and S238 can express a fraction" -> "a later term can", and the two it
  was kept for left having moved whole plies.
- `src/search_params.hpp`, the `LmrRoundBias` block: "the scaffolding S237 and
  S238 need to express a fraction of a ply" -> they left without carrying one;
  a fraction is left for this bias alone.
- `tests/test_search.cpp`, the S236 accumulator section: "is what S237 and S238
  will express a fraction in" -> they left having moved whole plies.
- `DEV_MANUAL.md`, the ledger entry for S236's removal: the second of its two
  reasons for keeping the unit is marked as not having held, the first as the
  one that keeps it.

### 5. `S170_cases.tsv`'s budgets, re-derived

`adocs/data/S203_case_sweep.sh` with no argument, niced beside the S022
verdict 2 SPRT, on a copy of this tree's Release build: the full grid, 108
cells, now tracked as `adocs/data/S245_sweep.txt`. **The rule, stated before
it was read** (DEC-156 as DEC-162 left it, S238's stated reading): each row
takes the cheapest cell at its own stride that reports a mate line -- DEC-162
deleted the per-case floors and left the choice on the mate count alone --
applied to every row, not only to the ones that moved.

| case | stride | old budget | old cell now | new budget | its cell (mates / short) |
|---|---|---|---|---|---|
| A_mate8_shallow | 1 | 1000000 | 4 / 0 | **500000** | 2 / 0 |
| B_mate6_shallow | 1 | 100000 | 30 / 0 | 100000 | 30 / 0 |
| C_mate7_depth11 | 1 | 1500000 | **0 / 0** | **1000000** | 16 / 0 |
| D_mate_minus6_depth10 | 1 | 4000000 | 4 / 0 | 4000000 | 4 / 0 |
| E_mate_minus9 | 1 | 1500000 | 10 / 0 | **500000** | 2 / 1 |
| F_mate6_inherited_no_line | 2 | 300000 | 15 / 0 | **100000** | 7 / 0 |

The old rows are quoted in the TSV's new S245 paragraph. C's own cell had gone
silent: it reports 16 at 1000000 and 0 in every other stride-1 cell, the knife
edge the TSV already describes, one cell over. `--at` on the new TSV
reproduces the six cells exactly (`.tuning/coord/S245_sweep/at.txt`), five of
five guarded cases reporting. **No ceiling moved and none was asked to**:
`--ceilings` over the four recorded grids answers 5, 15, 0, 2, 11, 5, with
`S245_sweep.txt` added the same six, and the new grid alone 0, 4, 0, 0, 5, 2
-- every cell inside the shipped ceilings. The grid is not added to
`test_mate_carry.cpp`'s command, which is the ceilings' and outside this step.
Both suites green at the new budgets (below); `test_mate_carry` ran 81.8 s and
87.1 s under the match's load, against 117 s at the old budgets an hour
earlier -- a load reading, not a timing.

## Suites and checks

Both fast suites green, niced, serial: Release 41 of 41, tune 41 of 41
(`.tuning/coord/S245_gate.log`); `./clang-format.sh --check` exit 0 with
`CLANG_FORMAT_MAJOR=22`; `tools/plan_prose_check.py` `--citations` 0 flagged
over 39 files, `--touches` 0 flagged, `--params` and `--gate` clean. `bench`
**3656950** in both builds, the parent's (S244's) total, and the Release
binary is byte-identical to S244's build: comments and tests only, so no
reach is owed.

## Documents

- `DEV_MANUAL.md`: item 4's ledger clause, and one sentence where the budgets
  are discussed saying how a re-sweep reads them off the grid (the no-argument
  run, the cheapest cell at the row's stride reporting a mate line, `--at` to
  confirm) and that S245 moved four.
- `adocs/data/README.md`: a row for `S245_sweep.txt`.
- `MANUAL.md`: checked, no change -- nothing here is on the UCI surface.
- `adocs/specs.md` is the coordinator's, and **it carries item 4's stale claim
  a sixth time**: the S236 sentence in the search row ends "and stays as the
  precondition every fractional reduction term needs (S237, S238)". Proposed
  wording: "and stays as the precondition a fractional reduction term would
  need; S237 and S238, the two steps it was kept for, left on their H0s
  without carrying a fraction -- S237 moved whole plies and S238 was measured
  at 1024 ticks, one ply (S245)". No sentence there names the budgets or the
  checker's matching.
- `README.md`: the owner's, untouched.

Beyond `touches:`: `tests/test_mate_carry.cpp` and
`adocs/data/S203_case_sweep.sh`, comments only (the fast check's fix-ups
below), and `adocs/data/S245_sweep.txt` and its README row, the
budgets' evidence. `tests/test_engine.cpp` is in `touches:` and was not
edited, item 2 being moot.

## The cold fast check's fix-ups (2026-09-29, the coordinator's list)

The check found no code defect; three document items, all comments and
documents, each corrected from `adocs/data/S245_sweep.txt` or DEC-162's own
text:

1. **The budget rule was the pre-DEC-162 text in three places**, while the
   budgets follow DEC-162's Consequences -- E at 500000 is 2 lines with 1
   short, which "all of them complete" would refuse. Now each says the rule
   as DEC-162 left it, the cheapest budget at the row's own stride whose cell
   reports a mate line, on the mate count alone, with DEC-156's first form
   named as history, citing DEC-162 and S245: `adocs/data/S203_case_sweep.sh`'s
   header (the rule, and "a row is usable when mate lines clears its floor
   and short is 0"; its first line also named the deleted
   `expected_mate_lines()` floors as something it re-derives, now the
   ceilings), `tests/test_mate_carry.cpp`'s header, and the TSV's S203
   paragraph (which also said the script re-derives "the floors in
   tests/test_mate_carry.cpp", now the ceilings).
2. **Comments the new budgets made false.** `tests/test_mate_carry.cpp`'s
   ceiling block said E's "own cell -- stride 1, 1500000 nodes, which is the
   cell this case drives": it names 1500000 as the cell E drove until S245,
   attributes the 22 lines with 11 short to S095's tree, and gives S245's
   reading there (10, 0 short) and E's cell now (500000, 2 with 1 short). The
   header's and `DEV_MANUAL.md`'s "C reports 13 mate lines at 1500000 nodes and
   0 at both 1000000 and 2000000" become S203's reading, with S245's beside it
   (16 at 1000000, 0 in every other stride-1 cell). The same sentence in the
   TSV's knife-edge paragraph is put in the past tense, the S245 paragraph
   below it carrying the current cells. One more of the same kind, found
   while reading: the header's "D ... Between 1200000 and 3000000 nodes it
   reproduces a short line" is S203's reading too, and now says so, with
   S245's (no mate line in that range, 4 with none short at 4000000).
3. **"Node-identical" said too much**: the TSV's S245 paragraph and the
   `S245_sweep.txt` row in `adocs/data/README.md` now say S244 is
   node-identical to `0c0db1b` **on the bench positions** -- its census moved
   65 node counts in games. So does this file's opening paragraph above.

Both fast suites re-run once after these edits, niced, serial: **Release 41
of 41, tune 41 of 41** (`test_mate_carry` 83.2 s and 85.7 s under the
match's load), `./clang-format.sh --check` exit 0, the four prose modes
clean, the Release binary still S244's byte for byte and the tune build's
`bench` 3656950 (`.tuning/coord/S245_gate_fixups.log`).

## Proposed commit text

`.tuning/coord/S245_commit_msg.txt`: subject "Correct five stale things the
fillers noted (S245)", a paragraph per item, and `No functional change` --
`src/` moves by comments only and `bench` prints the parent's 3656950.

## Proposed `done:` stamp

2026-09-29 -- the five items, each verified against the tree first. (1) The
comment in `iterative_deepening_search` names the eight-queens board by FEN,
keeps its 13.5 ms and 42371 nodes as what it cost when the defect was found,
and says `go depth 1` there reports 10187 nodes since S022's second verdict
and that no case uses it, naming the case's two boards (the rook-and-knight
board of 146994 nodes and the timer's own). (2) Moot as the accepts says: the
first-iteration case's GOLDEN comment records the board's move and both
numbers, which the re-take reads again; closed without a re-derivation. (3)
`holds_phrase` flattens the phrase before the title map as well as on the
fallback, so a title clang-format split across literals resolves when the
prose wraps its quote -- S242's citation, which refused S243's first
fixture, reproduced on that fixture and resolved after; the planted test in
`tests/test_plan_citations.py` red before, green after. (4) The five comments
naming S237 and S238 as the accumulator's consumers say both left on their
H0s without carrying a fraction (S237 whole plies, DEC-232; S238 measured at
1024 ticks, one ply), the unit kept for `LmrRoundBias` alone. (5) The budgets
re-derived by `adocs/data/S203_case_sweep.sh` under DEC-156 as DEC-162 left
it, every row: A 500000, C 1000000, E 500000, F 100000, B and D unchanged,
the old rows quoted in the TSV, the grid `adocs/data/S245_sweep.txt`, `--at`
reproducing the six cells, no ceiling moved; the rule's text brought to
DEC-162's form in the script, the test and the TSV, and the comments the new
budgets made false corrected from the grid (the cold fast check's list).
Both fast suites 41 of 41,
format and prose checks green; `bench` 3656950, the Release binary S244's
byte for byte. `DEV_MANUAL.md` two edits, `MANUAL.md` checked, no change.
Filler under DEC-171, named in the pre-registrations taken while it was open.
Written by an Opus subagent briefed by the coordinator.
