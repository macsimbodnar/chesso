id:         S250
goal:       `test_mate_breadth`'s exact-count floor separates the shipping guard from the weakened one again, placed by its own script on today's tree
accepts:    `adocs/data/S156_mined_floor_sweep.py` re-run on the tree of the day prints the per-`RfpMinPly` table and the gate built at the weakened default goes **red**; `EXACT_FLOOR` is re-placed strictly above the weakened guard's count and at or below the shipping count, by the rule S156 placed it with (`adocs/plan_done/S156_mined_set_floor.md`), the rule and the reading quoted at the site with the command; the site's table and margin prose read that run; the stale figures in the comment around `MATE_IN_THREE_FLOOR` in `tests/test_engine.cpp` ("10 at RfpMinPly 1", "21 at RfpMinPly 4", mates in four 1/16) re-derived by `adocs/data/S154_floor_margin_sweep.py` and corrected, that floor itself unmoved unless its own sweep says it no longer separates; both fast suites green; if no floor can be placed (the gap closes to zero), stop and report -- that is a decision, not a placement
touches:    tests/test_mate_breadth.cpp (the floor and its comment), tests/test_engine.cpp (the comment beside `MATE_IN_THREE_FLOOR`), DEV_MANUAL.md if its golden rows quote either, MANUAL.md, adocs/data/S156_mined_floor_sweep.py (its `--floor` default and docstring)
            # amended when the plan met the docs, 2026-10-01: MANUAL.md's
            # RfpMinPly row quoted S156's 145 / 141 against "a floor of 143",
            # and its reverse-futility paragraph the mates in four as 1 of 16;
            # the DOCS completion check found both stale.
excludes:   the mined set, its labels, the depth, any engine change; lowering any floor
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
author:     agent (Opus subagent briefed by the coordinator), 2026-10-01
done:       2026-10-01 -- `EXACT_FLOOR` 143 -> 145 by S156's rule (`weak < floor <= ships`) from `adocs/data/S156_mined_floor_sweep.py` at `3f7acd3`: 146 exact shipping (`RfpMinPly` 3 and 2) against 143 weakened (1 and 0), gap 3; 145 takes the spare point on the weakened side, the end that has moved (139, 141, 143), margin 1 below shipping and 2 above weakened; the gate built at `RfpMinPly` 1 observed **red** at 145 (143 exact, "under the 145"), read from a `git stash create` tree since the sweep compiles the committed floor (`.tuning/coord/S250/S156_mined_145.log`). `MATE_IN_THREE_FLOOR` (11) still separates by `adocs/data/S154_floor_margin_sweep.py floor`, 12 of 24 against 7, and its `red` mode reads `REQUIRE( 7 >= 11 )`; unmoved. The stale figures beside it re-read from those runs (7 weakened, 20 at `RfpMinPly` 4, mates in four 2 of 16, mate in two 20 found and 13 on time weakened, the motif split, churn 8), and `DEV_MANUAL.md` and `MANUAL.md` (its `RfpMinPly` row and reverse-futility paragraph) re-read to the same runs; the sweep script's `--floor` default and docstring set to 145 at hand-back. No floor lowered, no assertion changed but the raised floor; `src/` untouched. Both fast suites 41 of 41, clang-format clean, `plan_prose_check.py` `--citations`, `--touches`, `--params` clean; cold fast check LAND, its one trivial (S148 ceiling figures in present tense beside the new 2 of 16) attributed and dated. `README.md` human-owned, no change.

## Why this exists (2026-10-01, the coordinator)

S249's item 1 re-read the mined set's count before editing its comment and
found more than a stale number. `adocs/data/S156_mined_floor_sweep.py` on
`0cf3ec8` reads 146 exact with the guard shipping (RfpMinPly 3 and 2) and
143 with it weakened (RfpMinPly 1 and 0); `EXACT_FLOOR` is 143, so the gate
built at the weakened default goes **green** and the script prints "THE FLOOR
NO LONGER SEPARATES" (`.tuning/coord/S249/S156_mined.log`, gitignored, quoted
in S249's stamp). The last tracked reading,
`adocs/data/S156_mined_floor_sweep.log`, was 145 against 141 with the gate
red. Nothing is red today: the shipping tree passes its floor; what is lost is
the test's power to catch the weakening it exists for. S249's excludes forbid
moving a floor, so the item moves here; raising a floor to restore its
separation tightens a test and is DEC-142's re-derivation, not a relaxation.
S249's item 2 sweep (`S154_floor_margin_sweep.py floor`) found
`MATE_IN_THREE_FLOOR`'s own comment true at 12 of 24 and its floor (11)
separating (7 of 24 weakened), but three figures in the surrounding prose
stale; those join here.

Ordered ahead of S116 by the coordinator: a test-only defect (DEC-171 scope),
but S116 is a pruning change and this is a mate guard over pruning, so the
guard gets its teeth back before the next pruning lands.

## As built (2026-10-01, on `3f7acd3`, the worktree `chesso-s250`)

**Deviations first.**

1. **The red at the new floor was read from a `git stash create` object, not
   from a commit.** The sweep checks out `--ref` in a throwaway worktree and
   compiles the gate from that commit's `EXACT_FLOOR`; its `--floor` argument
   only labels the separation line. With nothing committed, `--ref HEAD` would
   build the old 143 and read green. `git stash create` made
   `0a695ffb466a650be248160078c8aaa3cd804092` (the working tree with the floor
   at 145, no ref moved, no branch touched) and the sweep ran at that sha.
2. **More stale figures than the three named were corrected**, every one from
   the same `floor` run: in `tests/test_engine.cpp` the distance table (mates
   in three "delay up to 8" -> 7, mates in four 1 -> 2 of 16, in three places),
   the mate-in-two clause at `RfpMinPly` 1 and 0 (21 -> 20 of 26 found, 15 ->
   13 on time, eleven -> thirteen positions, and "seven rather than eleven" ->
   "thirteen"), the motif split (queen 10, 4, 14 and rook 2, 3, 6 now, the
   S168 reading kept as history: the rook rows now run one the wrong way), the
   churn 4 -> 3 (9 -> 8), and the GOLDEN block's weakened end and margin.
3. **`MANUAL.md` was touched**, outside the brief's file list: its `RfpMinPly`
   row quoted 145 / 141 against "a floor of 143", and its reverse-futility
   paragraph the mates in four at 1 of 16. `touches:` amended above.
   `DEV_MANUAL.md` took more than its golden row: the mined-set section's table
   and prose, the GOLDENS example "a floor of 143 mates", and the
   mate-in-three section's distance table, weakened end (10 -> 7) and observed
   red (`REQUIRE( 10 >= 11 )` -> `REQUIRE( 7 >= 11 )`, re-observed by `red`
   mode for this).
4. **Not edited, flagged:** `adocs/data/S156_mined_floor_sweep.py` still
   defaults `--floor` to 143 and its docstring says "at least 143"; outside
   `touches:`. Run without `--floor 145` it prints "THE FLOOR NO LONGER
   SEPARATES" on a tree that does separate. The site and `DEV_MANUAL.md` name
   the command with `--floor 145`. A one-line follow-up. The ceiling figures in
   `tests/test_engine.cpp` ("13 of 16 and 11 of 16 at RfpMaxDepth 0 against 1
   and 0 from 10 up") are S148's ceiling sweep, which no run here re-took;
   left. `DEV_MANUAL.md`'s "one ply of the guard itself moves five" is S154's
   reading over the 48, dated; left.

**Placement.** The rule, S156's as its step file states it: the floor sits
strictly above the weakened guard's count and at or below the shipping count
(the script's own test, `weak < floor <= ships`). On 146 against 143 that
admits 144, 145 and 146. 146 is a zero-margin floor (DEC-116 rejected one
before) and S156's 143 sat at the even split, 2 and 2; a gap of 3 has none, so the choice was
144 or 145. **145**: the end that has moved is the weakened one, 139 (S145) ->
141 (S156) -> 143 (S249), and its rise is what closed the separation; the
shipping end has sat at 145-146 throughout. Margin 1 below shipping, 2 above
weakened.

**Commands, figures, logs** (all under `.tuning/coord/S250/`):

- `python3 adocs/data/S156_mined_floor_sweep.py` (ref `HEAD` = `3f7acd3`,
  depth 10, floor 143): `RfpMinPly` 3 146 / 149 / 0, 2 146 / 150 / 0, 1 143 /
  145 / 0, 0 143 / 145 / 0; gap 3; "THE FLOOR NO LONGER SEPARATES"; gate at
  `RfpMinPly` 1 **GREEN, WHICH IS THE FAILURE**, 143 exact. `S156_mined_143.log`.
  Byte for byte S249's table.
- `python3 adocs/data/S156_mined_floor_sweep.py --floor 145 --ref 0a695ff...`:
  the same table; no separation warning; gate built at `RfpMinPly` 1 **RED, as
  it must be**: "mined set at depth 10: 143 exact, 145 right sign, 0 wrong
  sign", "143 exact at depth 10, under the 145". rc 0. `S156_mined_145.log`.
- `python3 adocs/data/S154_floor_margin_sweep.py floor`: m3 12/24 at 3 and 2,
  **7** at 1 and 0, 20 at 4 and 5; m2 26/26 on time at 2 and up, 20/26 found
  and 13 on time at 1 and 0; m4 2/16 at 2 to 5; churn 4 -> 3 is 8.
  `MATE_IN_THREE_FLOOR` (11) separates, 7 < 11 <= 12: **unmoved**.
  `S154_floor.log`; identical to S249's but for wall time.
- `python3 adocs/data/S154_floor_margin_sweep.py red`: gate at `RfpMinPly` 1
  red, six mate-in-two `CHECK( false )`, `REQUIRE( 7 >= 11 )`, 27 failed
  assertions. `S154_red.log`.
- Motif split from the `floor` masks against `adocs/data/S145_mate_set.tsv`'s
  family column (16 `shift*` queen rows, 8 `rook*` rows): queen 10 / 10 / 4 /
  4 at 3 / 2 / 1 / 0 and 14 at 4 and 5; rook 2 / 2 / 3 / 3 and 6. Read by a
  one-off Python snippet over the log, not saved as a script.

**Old and new text.** `EXACT_FLOOR` 143 -> **145**. Its GOLDEN block: ends
"145 ... 141" -> "146 ... 143"; Re-derive gains `--floor 145` and "S250 read
the gate built at RfpMinPly 1 red at 145"; Margin "2 below the shipping count,
2 above the weakened one. The gap was 7 ... is 4 now" -> "1 below the shipping
count, 2 above the weakened one", with the uneven split's reason. The header
table re-read to the run above, dated `3f7acd3`, and its prose carries the
history 146/139 -> 145/141 -> 146/143 and the gap 7 -> 4 -> 3.
`MATE_IN_THREE_FLOOR` 11 unchanged; its comment's figures as in deviation 2.

**Suites.** `CLANG_FORMAT_MAJOR=22` exported. Release `build`: 41 of 41,
`test_mate_breadth` 18.61 s, reading 146 exact / 149 / 0 against 145
(`fast_release.log`, `mate_breadth_shipping.log`). Tune `build-tune`: 41 of 41
(`fast_tune.log`). `./clang-format.sh --check` clean (`clang_format.log`).
`tools/plan_prose_check.py` `--citations` (0 flagged over 35 files),
`--touches` (0 flagged), `--params` (rc 0, silent): `prose--*.log`. `git diff 3f7acd3 -- src` empty: no engine change, no
`Bench:` line owed. No match, no `gate_extra.sh`. `DEV_MANUAL.md` and
`MANUAL.md` checked and changed as in deviation 3; `README.md` human-owned, not
touched.

**Proposed `done:` stamp.** 2026-10-01 -- `EXACT_FLOOR` 143 -> 145 by S156's
rule from `adocs/data/S156_mined_floor_sweep.py` at `3f7acd3`: 146 shipping
against 143 weakened, gap 3, the spare point above the weakened end, which is
the end that has moved; the gate built at `RfpMinPly` 1 observed **red** at
145 (143 exact, "under the 145"), from a `git stash create` tree since the
sweep compiles the committed floor. `MATE_IN_THREE_FLOOR` (11) still separates
by `S154_floor_margin_sweep.py floor`, 12 against 7, and `red` reads
`REQUIRE( 7 >= 11 )`; unmoved; its comment's stale figures corrected (7 and
20 at RfpMinPly 1 and 4, mates in four 2 of 16, mate in two 20 found / 13 on
time, motif split, churn 8). `DEV_MANUAL.md` and `MANUAL.md` re-read to the
same runs. Both fast suites 41 of 41, clang-format clean, prose checks clean,
`src/` untouched. Open follow-up: the sweep script's `--floor` default (143).

**Proposed commit text.**

```
Raise test_mate_breadth's floor to 145 so it separates again (S250)

The weakened guard (RfpMinPly 1 and 0) had risen to 143 exact on the
mined set, the floor itself, so the gate built at the weakened default
went green and the test could no longer catch the weakening it exists
for (DEC-247). S156's sweep on 3f7acd3 reads 146 shipping against 143
weakened; 145 is strictly between, the spare point of the gap of 3 on
the weakened side, the end that has been moving. Raising a floor is
DEC-142's re-derivation, not a relaxation. The weakened gate was
observed red at 145.

MATE_IN_THREE_FLOOR still separates (12 against 7) and stays at 11;
the stale figures in its comment, DEV_MANUAL.md and MANUAL.md are
re-read from S154's floor and red runs. No functional change: src/
is untouched.
```

**The coordinator, at hand-back (2026-10-01):** deviation 4's follow-up is done here rather than left: `adocs/data/S156_mined_floor_sweep.py`'s `--floor` default is 145 and its docstring says "at least 145 (143 until S250)", so the script run bare reads the floor the test asserts. `touches:` amended.
