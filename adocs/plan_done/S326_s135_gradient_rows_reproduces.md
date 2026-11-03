id:         S326
goal:       `adocs/data/S135_gradient_rows.cpp` re-derives the fixture pair it picked, and says so, at the tree it runs on
accepts:    the program takes as its base the fixture as it stood before S135's pair (a stated row count, checked against the pair's own marker in `tests/test_eval_positions.hpp`), applies DEC-316's rule unchanged, and prints whether its pick is the landed pair, exiting non-zero when it is not, in `S101_gradient_rows.cpp`'s shape; run at `HEAD` and at S135's landing commit, each reading stated; if the rule at `HEAD`'s constants picks another pair, that is reported as a finding for the owner (DEC-142: re-derived, never re-read), not fixed by editing the fixture; both fast suites and `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` green; extended by DEC-347: at a layout without the bishop's single middlegame mobility weight, detected from the layout itself, the program picks nothing, says why and exits non-zero, its output at `18c2d9a` byte-equal to the yes-reading; the pair's site comment in `tests/test_eval_positions.hpp` says the pair is a coverage row since S121, the derivative case's comment in `tests/test_tuner_gradient.cpp` places it at rows 41 and 42, and `DEV_MANUAL.md`'s S324 paragraph says it re-derives at its landing tree, the test sources changed in comments only and proved so in S321's form
touches:    adocs/data/S135_gradient_rows.cpp, adocs/data/README.md, adocs/data/S326_identity.txt, tests/test_eval_positions.hpp, tests/test_tuner_gradient.cpp, DEV_MANUAL.md
excludes:   any change to the fixture's rows, a case's code, a bound, `h` or DEC-316's rule; the other five re-derivers
decisions:  DEC-142, DEC-171, DEC-316, DEC-347
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- `accepts` by clause, as extended by DEC-347. The base is the fixture's first `BASE_ROWS` = 40 rows, the count checked against the pair's own marker (`added at S135 (DEC-316)`, once, 40 rows before it) and the header's row count against the compiled list. DEC-316's rule is unchanged, and the program prints `reproduces the landed pair: yes|NO`, `CHECK` making it exit 1 on NO, `S101_gradient_rows.cpp`'s shape. At S135's landing commit `18c2d9a`: candidate 47, `|g774|` 1.868e-05 over 5.704e-06, **yes**, rc 0, S135's recorded lines byte-equal, and byte-equal with and without the layout check by `cmp`. The layout check is `tuner_layout::MOBILITY_COUNT == 4`, one mobility weight a piece type, read from the layout and not from a sha. At `HEAD` (`1bb3080`'s constants, 66 a half) the program checks the fixture, then picks nothing and prints why (DEC-316's target gone since S121, `12cab3c`, the bishop's middlegame mobility 14 cells with no single weight; the pair, rows 41 and 42 as the check found them, re-derived at `18c2d9a`, a coverage row since S121, DEC-347), exit 1, the same from three directories; after the fast check's three fixes, the output at `18c2d9a` is still byte-equal by `cmp`. Run there without the check it picked candidate 1, the finding DEC-347 rules on. The pair's site comment, the derivative case's comment (rows 41 and 42, not "last") and `DEV_MANUAL.md`'s S324 paragraph say so. The two test sources are comments only, `S326_identity.txt`: preprocessed sources SAME, 340 of 340 objects byte-equal after a ccache-off rebuild, the three dependents in each build seen recompiled, bench 5482277 in both. Before and after diffed at both trees against the before program on the same 40 rows: only the new check's lines and doctest's counts differ. Fast suites 53 of 54 in both builds, the one red `plants_threats`'s 60 s clock under the screen, passing alone, unclocked, in 57 s and 41 s; format rc 0; prose checks rc 0. `MANUAL.md` checked, no change. Filler (DEC-171), Tier 2 does not bind.

## Why

Found 2026-11-03 by S324's implementer. The program takes the whole landed fixture as its base, so S135's own pair
(rows 41 and 42 of 106) counts as known. It then picks the first candidate, a row the fixture does not hold, and exits 0,
because it has no check that its pick is the landed pair. That holds before S324 and after it. So the golden pair
`test_tuner_gradient`'s derivative case reads is, today, a golden its script does not re-derive, which DEC-142 makes a
finding. Filler by DEC-171: the case is green and no reported score, move or line moves.

## Done, 2026-11-03

In the worktree `chesso-s326`, on the stacked base (tree `f27a03c6`: `1bb3080` with S324 and S325 staged, neither under
`src/`, so `1bb3080`'s constants), Release and tune builds, with S122's screen on the machine. Every build and run under
`nice -n 10`. Outputs, the before program, the two 40-row headers, the script that cut them and the identity scripts are
in the main tree's `.tuning/coord/S326/`, gitignored. Delivered once with the finding below open; the coordinator ruled
it as DEC-347 (option (a) of that delivery), and this step was extended to match and delivered again.

### What changed

- `adocs/data/S135_gradient_rows.cpp`, in `S101_gradient_rows.cpp`'s shape.
  - The base is the fixture's first `BASE_ROWS` = 40 rows, the list as the rule ran on it; until now it was the whole
    landed list. Before any gradient, the program reads `tests/test_eval_positions.hpp` as text under
    `CHESSO_SOURCE_DIR`. It counts the row lines and requires them equal to the compiled list. It finds the marker
    `added at S135 (DEC-316)`, the pair's own comment, once, and requires exactly 40 rows before it; the marker is
    required whenever the list is longer than the base. It prints `tests/test_eval_positions.hpp: 106 rows, marker
    found, 40 rows before it`.
  - After the pick it prints `reproduces the landed pair: yes|NO`: yes when rows 41 and 42 are the pick and its mirror,
    in that order. `CHECK(same)` makes doctest exit 1 on NO.
  - **The layout check (DEC-347).** `HAS_TARGET` is `tuner_layout::MOBILITY_COUNT == 4`: the mobility block holds one
    weight a piece type a half, knight, bishop, rook, queen, so `MOB_MG_BASE + 1` is the bishop's middlegame weight.
    That is the layout `tuner_layout.hpp` held until S121 (its own comment: "Four weights a half, one per type times the
    count, until S121"). `MOBILITY_COUNT` is the one name both layouts define, 4 at `18c2d9a` and 66 since `12cab3c`.
    The fixture check above runs first, at every layout. Then, where `MOBILITY_COUNT` is not 4, the program requires
    the marker and prints the refusal. It says DEC-316's rule targets `mobility_mg[1]`, and this layout has
    `MOBILITY_COUNT` cells a half, a table since S121 (`12cab3c`). The bishop's middlegame mobility is 14 cells there,
    with no single weight to target. The pair, at the rows the check found (`before_marker + 1` and `+ 2`, 41 and 42),
    is re-derived at `18c2d9a` and is a coverage row since S121 (DEC-347). Nothing is picked. Then it `FAIL`s, exit 1;
    no gradient is taken. From the fast check: the fixture check moved above the refusal, so the rows it names are
    checked, not a fixed string; "no weight that is the bishop's alone", false since the bishop owns 14 cells
    (`MOBILITY_OFFSET[1]` 9, `MOBILITY_WIDTH[1]` 14, the same at `12cab3c`), corrected in the refusal, the header and
    the site comment.
  - The header: a paragraph on the base and the check; WHERE IT REPRODUCES, AND WHERE IT REFUSES, with the two readings,
    why they differ, and DEC-347; how to run the program at an older tree, the `tests/doctest` submodule supplied by
    hand since `git archive` leaves it out. The print paragraph names the new lines, the
    `CHESSO_SOURCE_DIR` paragraph names the header it reads, and `TARGET`'s comment says when it is the bishop's.
  - Unchanged: `TARGET`, `threshold_of()`, eligibility, candidate order, labels, scores, every existing print. DEC-316's
    rule is as S135 stated it.
- `tests/test_eval_positions.hpp`, the pair's site comment, one sentence in S083's form: "**Since S121 this pair is a
  coverage row, no longer the rule's** (DEC-347)", why, and that `adocs/data/S135_gradient_rows.cpp` re-derives it at
  `18c2d9a` and picks nothing at a later layout, the bishop's middlegame mobility being 14 cells with no single weight.
  Comment only, five lines.
- `tests/test_tuner_gradient.cpp`, the comment over "gradient() is the derivative of the error it descends": "the
  fixture's last pair" was stale, since S121 and later blocks appended 64 rows after it. It now reads "the pair at rows
  41 and 42 of tests/test_eval_positions.hpp". The five lines are re-wrapped in place, so the file's line count, 2053,
  and every doctest `__LINE__` stay put.
- `DEV_MANUAL.md`, the S324 paragraph: one sentence saying `S135_gradient_rows` re-derives its pair at `18c2d9a`, that
  the pair is a coverage row since S121, and that at a later layout the program picks nothing, says why and exits 1
  (DEC-347). "Their headers'" becomes "The six headers'", since the sentence before now names one program. `MANUAL.md`
  checked, no change: 0 hits for `gradient_rows`.
- `adocs/data/README.md`: `S135_gradient_rows.cpp`'s row gains the check, the reading at `18c2d9a`, and the refusal;
  a row for the new `S326_identity.txt`.
- `adocs/data/S326_identity.txt`: the identity evidence, below.

Nothing under `src/` or `tools/`. No fixture row, case code, bound or `h` touched.

### The readings

- **At S135's landing commit, `18c2d9a`** (the commit that pasted the fit and added the pair and the program): built
  from `git archive 18c2d9a` in the scratchpad, its own `chesso_engine_trace` target only, this file compiled against
  that tree by the header's line. Run from that tree's `tests/`, since load_json reads `CHESSO_SOURCE_DIR` only from S310
  on. `42 rows, marker found, 40 rows before it`; base `param 774 analytic -1.342036e-06`, `target 774 at 1.342e-06 of
  the threshold 5.990e-06`. Candidates 1 to 46 short, then `candidate 47 |g774| 1.868e-05 threshold 5.704e-06 PICK
  1r1Nk2r/ppp1n1pp/8/2b5/8/2P5/PP2BPPP/R3K2R w KQk - 2 9`, the mirror as landed. **`reproduces the landed pair: yes`**,
  rc 0, 68 lines, sha256 `c9785fcec22b...`. The base-to-pick lines equal what S135 recorded
  (`adocs/data/S135_goldens.txt` section 10), by `diff`. With the layout check added, the output is byte-equal to the
  output without it, by `cmp`.
- **At `1bb3080`, the base, with the layout check**: the refusal, rc 1, the same from the repository root, from
  `build/tests` (no `tests/assets` there) and from an empty third directory, `cmp` equal, stderr empty. The marker
  line, the refusal's seven lines, then doctest's `FATAL ERROR` block; 8 assertions, 7 passed, 1 failed (the `FAIL`):

  ```
  tests/test_eval_positions.hpp: 106 rows, marker found, 40 rows before it
  DEC-316's rule targets mobility_mg[1], the bishop's middlegame
  mobility weight, one of four a half at S135's layout. This layout
  has 66 a half, a table since S121 (12cab3c): the bishop's
  middlegame mobility is 14 cells there, with no single weight to
  target. The pair, rows 41 and 42 of tests/test_eval_positions.hpp,
  is re-derived at 18c2d9a, S135's landing tree, and is a coverage
  row since S121 (DEC-347). Nothing is picked.
  ```

- **At `1bb3080`, the rule run anyway** (the first delivery, before the layout check): `106 rows, marker found, 40
  rows before it`; base `param 774 analytic -5.852483e-07`, **`param 780 analytic +0.000000e+00`**, `target 774 at
  5.852e-07 of the threshold 0.000e+00`. Then `candidate 1 |g774| 5.574e-07 threshold 0.000e+00 PICK
  1BQKRBRN/PPPPPPPP/1N6/8/8/8/3k4/8 b - - 1 1`, a row the fixture does not hold. `reproduces the landed pair: NO`, rc 1.

**Why `1bb3080` has no reading of its own.** The rule reads layout offsets: `TARGET` = `MOB_MG_BASE + 1` and
`threshold_of()`'s window `[MOB_MG_BASE, MOB_MG_BASE + 8)`. At `18c2d9a` `MOBILITY_COUNT` is 4, so they name
`mobility_mg[1]`, the bishop's, among the four middlegame and four endgame mobility weights: the rule's text. S121
(`12cab3c`, 2026-10-29) made mobility a table, `MOBILITY_COUNT` = `MOBILITY_CELLS` = 66 a half, knight at offset 0 and
width 9 (`src/evaluation.hpp`). So the same offsets now name `mobility_mg[knight][1]` and the knight's middlegame cells
0 to 7. Cell 7, parameter 780, has a gradient of exactly 0 over the 40 rows. The smallest of the other seven is then 0,
the threshold is 0, every eligible candidate qualifies, and the first is the pick. Candidate 1 is the same position at
both trees. What moved is the question the offsets ask, not the candidate order. DEC-347 settles it: a coverage row
since S121, re-derived where the rule's question exists.

### Before and after

The before program is the base's file (`f7032a5a...`, S324's), saved before any edit.

- At the base, as it stood (whole list as base): 20 lines, sha256 `6ad1772bca04...`, the before output S324 recorded.
  Its pick is candidate 1, `|g774| 3.413e-06` over `1.943e-07`, rc 0.
- Like for like: the before program compiled against a copy of the header cut at the marker, so the list is the 40
  rows, against the after program at the full header.
  - At `18c2d9a`, with S135's own program as it was there as the before, the final program's output differs by the
    marker line, the `yes` line and the assertion count, 66 to 73. The seven new assertions are the new check's, each
    run once; the layout check adds none where the layout has the target. Every other line is byte-equal, the
    gradients and every candidate included.
  - At the base, the program without the layout check (the first delivery) differs from the before by the marker line,
    the `NO` line, and doctest's failure block for `CHECK(same)`, 17 assertions for 10. Every gradient and candidate
    line is byte-equal. With the layout check, the base computes nothing, so it prints the refusal instead.
  - stderr is empty in every run.

### Identity of the two test sources (`adocs/data/S326_identity.txt`)

S321's form, by `.tuning/coord/S326/scripts/`, adapted from S325's.
- `g++ -fpreprocessed -dD -E -P` of each source, old (`f27a03c6`) against new: `tests/test_eval_positions.hpp` and
  `tests/test_tuner_gradient.cpp` SAME, line counts 400 to 405 and 2053 to 2053. The control is a changed statement,
  which differs, and an added comment, which does not.
- Every other changed path is classed: three documents, one data file, and the program, which no build compiles.
- Objects: every `.o` and `.a` of `build/` and `build-tune/`, and their generated `chesso_build_info.hpp`,
  `CTestTestfile.cmake`, `flags.make`, `link.txt` and `compile_commands.json`, were snapshotted on the base before any
  edit to a compiled file. Against the edited tree rebuilt with `CCACHE_DISABLE=1`: **340 of 340 byte-equal, none
  differ**.
- The three objects per build whose gcc dependency files name an edited source, `test_eval_model`, `test_eval_trace` and
  `test_tuner_gradient`, were each really recompiled (newer than the snapshot).
- The stamp was already `1bb3080-dirty` on the staged base, so nothing stamped moves. There is no doctest `__LINE__`
  shift, since `test_tuner_gradient.cpp` keeps its line count and the header holds no doctest macro.
- Bench 5482277 in `build` and in `build-tune`, the parent's. `S326-IDENTITY HOLDS`.

### The suites

On the final tree, after the fast check's fixes: serial, `CLANG_FORMAT_MAJOR=22`, `nice -n 10`, with S122's screen on
the machine (load average 10 to 18 on 12 cores across the step). Both builds read **53 of 54**, in 358 s and 372 s.
The one red in each is a clock: `test_tuner_gradient_plants_threats` timed out at 60.15 s, while `plants_pawns` ran
49 s against its own 60 under the same load. Rerun alone with no clock (`--timeout 1200`), it passed in 57.02 s
(`build`) and 40.84 s (`build-tune`), 1 of 1 each. The round before, on text that differs only in comments, it timed
out once in `build`, passed alone in 33.01 s, and passed at 36.20 s in `build-tune`, which read 54 of 54. Its object
is byte-equal to the base's (above), so this is load, not the change. The brief names this entry as one that times
out under the screen. In the first delivery `build` read 53 of 54 at first: `test_plan_citation_freshness` was red on
this file, whose first draft cited a test comment by line number, which DEC-135 forbids. It now names the case, and
that suite rerun whole read 54 of 54.

`CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` rc 0, the two test sources included. It does not format `adocs/`
(S075), so a hand check: the lines this step wrote in the program are at most 84 columns; the file's widest, 85, is
S324's. `plan_prose_check.py` `--prose`, `--touches` and `--citations` on this file, and the suite's
`test_plan_*` entries in both builds after the last edit, all green.

Tier 2 (DEC-141) does not bind: no `make_move`, generator or search change, no pruning rule. Filler (DEC-171). Nothing
under `src/`, so the bench is the parent's, 5482277 in both builds.

### Found, not fixed

- Whether the derivative case still needs the pair at today's constants was not measured. DEC-347 leaves that to the
  case itself, which reads red if the fixture again leaves a weight stationary.
