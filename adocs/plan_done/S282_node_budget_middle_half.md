id:         S282
goal:       `adocs/data/S192_node_budget.py`'s "middle half" verdict can read inside on the band it derives itself
accepts:    (1) the script's middle-half bounds are stated in terms of the count the band is derived from (the band is `[count / FLOOR_DIVISOR, BUDGET_RATIO x count]`), and a count equal to the derivation count reads inside, shown by running the script on a build whose shipping band was derived from that build's own count; (2) the rule DEV_MANUAL.md's goldens row and `tests/test_search.cpp`'s comment at "ordering keeps the tree small" state for when the band is re-derived is the rule the script prints, word for word in substance, and the row's history sentence that relied on the old reading ("inside the pair's middle half") is checked against the new bounds and corrected if it no longer holds; (3) no golden number moves in this step: the band 65024 / 3251 stays until a step that moves the count re-derives it by the script
touches:    adocs/data/S192_node_budget.py, DEV_MANUAL.md (the goldens row), tests/test_search.cpp (comment only)
excludes:   re-deriving the band itself; any change to the case's assertion or to search
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent, phase 1 blind beside S023 verdict 1's run (DEC-260), phase 2 on fd33954
done:       2026-10-15 07:55 -- adocs/data/S192_node_budget.py reads the middle half around the count the shipping pair was derived from, [c / sqrt 5, c x sqrt 4], with c recovered from the pair (budget / 4, the floor checked as c // 5): [7270, 32512] for 65024 / 3251 from 16256. A pair not [c // 5, 4c] of one count reads OUTSIDE, re-derive by the two ratios; shown on a scratch copy with the budget hand-widened to 80000. (1) On 326f274's tree, where the pair was derived, the script reads 16256, 1.00 of c, inside, where the old quartiles [18694, 49581] read it OUTSIDE. On the current tree, S261's engine 7e36271, it reads 13446, 0.83 of c, inside, in both builds; S023's candidate read the same count. (2) DEV_MANUAL.md's goldens row and the case's comment state the script's rule. Their history is corrected: every count read against the S091 pair -- 17451, 17332, 17321, 16256, 16246 -- sat below its quartiles [20068, 53226] from S091 on and inside its middle half [7805, 34902], the script reading 16256 on 3a649c0's tree at 0.93 of c. So the 2026-09-20 re-derivation fired on the defect, though it was done correctly from 16256 by the two ratios. 20427 holds inside both readings. (3) 65024 / 3251 unmoved, no src/. Found on the way and fixed: a relative binary path failed (abspath). Fast check: 3 findings, fixed. TESTS chain green in both builds, 46 of 46 each, format clean at CLANG_FORMAT_MAJOR=22; plan_prose_check flags nothing in any of its five modes. MANUAL.md checked, no change. Implemented by an Opus subagent in two phases, phase 1 blind under DEC-260; stamped by the coordinator.

## Why this exists

Found by S023's phase 2 (2026-10-14, Opus subagent), checked by the
coordinator by reading the script. The band is `[c / 5, 4c]` for a derivation
count `c` (`FLOOR_DIVISOR` 5, `BUDGET_RATIO` 4). The script's middle half is
`low = floor + (budget - floor) // 4`, which is `0.2c + 0.95c = 1.15c`, and
`high = 4c - 0.95c = 3.05c`. The count the band was derived from, `c`, is
below `low`, so a freshly derived band always prints "OUTSIDE" for its own
count, and the reading DEC-142's re-derivation rule leans on says nothing.

On the shipping band 65024 / 3251 (from 16256): the middle half is
[18694, 49581]; 16256 itself, the count the band was taken from, is outside.

Tool and documentation only: no play, no reported score, no UCI surface, so
filler under DEC-171, scheduled behind the next strength step and named in
the pre-registration of any run taken while it is open.

## As built, phase 1 -- 2026-10-15, Opus subagent, written blind (DEC-260)

S023 verdict 1's SPRT held the machine: nothing was compiled, tested or run.
`python3 -m py_compile` on the script is the only execution; every number
below is arithmetic from the script's formula, checked with shell integer
arithmetic, never read off a run.

### Surprises, first

1. **The defect was found once before and not opened.** S231's revert
   (`326f274`, 2026-09-20) wrote it into its step file -- the drift line
   cannot read inside for a freshly derived band, and three step files in a
   row had read it as a verdict -- and proposed it as filler; no step followed
   until S023's phase 2 found it again.
2. **The shipping pair is itself a product of the defect.** The 2026-09-20
   re-derivation, 69804 / 3490 to 65024 / 3251, fired on the old reading. Around
   17451, the count the S091 pair was derived from, the new middle half is
   [7805, 34902], and every count on record from S222 to that revert -- 17332
   (S222), 16246 (S231's candidate), 16256 (the reverted tree) -- is inside it.
   The pair was still derived correctly by the two ratios, and accepts (3)
   keeps it.
3. **Accepts (1) cannot be shown on the current tree.** Its count is 13446
   (S023 verdict 1, `adb5d78`, is in the base), not the 16256 the shipping pair
   was derived from. Phase 2 has to build a tree whose count is 16256: below.
4. **A pair not derived by the ratios has no `c`.** 440000 / 20000, the band
   of 2026-08-14 and of `adocs/data/S192_node_budget.log`, is not
   `[c // 5, 4c]` of one count (440000 / 4 = 110000, 110000 // 5 = 22000), and
   the script now prints "middle half none" for such a pair instead of a
   verdict. History only: 65024 / 3251 passes the check.

### Design

The middle half is stated around `c`, the count the shipping pair was derived
from: **`[c / sqrt 5, c x sqrt 4]`, half of the factor from `c` to either end**
of the band `[c / 5, 4c]`. A count inside has moved less than half of the way,
in factors, toward the floor or toward the budget; a count outside has used
more than half of its margin on that side, the band is no longer centred on
the search, and DEC-142 re-derives it. `c` reads inside by construction.

- `c` comes from the band, not a separate input: `c = budget // 4`, accepted
  only when `4c == budget` and `c // 5 == floor`. 65024 // 4 = 16256;
  16256 x 4 = 65024; 16256 // 5 = 3251. Both agree.
- Exact integers: `low` is the least `n` with `5 n^2 >= c^2`,
  `isqrt((c^2 - 1) // 5) + 1`; `high` is the greatest `n` with `n^2 <= 4 c^2`,
  `isqrt(4 c^2)`, which is `2c`.
- No new constant: both halves come from `FLOOR_DIVISOR` and `BUDGET_RATIO`.
  Rejected: `[c / 2, 2c]`, symmetric and simpler to say, but a third constant
  the comment would have to defend beside "neither is a new constant"; and the
  band's log-space middle half, which contains `c` but is centred on the band's
  geometric centre, `0.894c`, not on `c`. On every count on record the three
  give the same verdicts.

### The arithmetic on the shipping band 65024 / 3251

    c       = 65024 // 4 = 16256          (16256 // 5 = 3251, the floor agrees)
    c^2     = 264257536
    low     = isqrt(264257535 // 5) + 1 = isqrt(52851507) + 1 = 7269 + 1 = 7270
              (7269^2 = 52838361 <= 52851507 < 52852900 = 7270^2;
               c / sqrt 5 = 7269.83)
    high    = isqrt(4 x 264257536) = isqrt(1057030144) = 32512 = 2c
    old     = [3251 + 61773 // 4, 65024 - 61773 // 4] = [18694, 49581]

**16256 reads inside [7270, 32512]**, at 1.00 of `c`. Outside: a count of 7269
or less, or 32513 or more -- within the case's own band, 3252 to 7269 below
and 32513 up to the 65024 budget above.

The readings on record against this pair:

| count | where | old [18694, 49581] | new [7270, 32512] |
|---|---|---|---|
| 16256 | derived from, 2026-09-20 | OUTSIDE | inside, 1.00 |
| 22217 | S095 | inside | inside, 1.37 |
| 20350 | S120 | inside | inside, 1.25 |
| 20427 | `cbaa699`, S022's parent and after its H0 | inside | inside, 1.26 |
| 127342 | S022 verdict 1's candidate, gate deleted | OUTSIDE | OUTSIDE, past the budget |
| 13446 | S023 verdict 1's candidate, the base of this step | OUTSIDE | inside, 0.83 |

The S091 pair 69804 / 3490: `c` = 17451, 17451^2 = 304537401, `low` =
isqrt(60907480) + 1 = 7804 + 1 = 7805 (7804^2 = 60902416, 7805^2 = 60918025),
`high` = 34902; the old quartiles were [3490 + 66314 // 4, 69804 - 66314 // 4]
= [20068, 53226].

### What changed, wording by wording

- `adocs/data/S192_node_budget.py` `main`: the quartile bounds go; `c` is
  recovered from the shipping pair with the two checks above, and a pair that
  fails them prints "middle half none" and stops. Three lines are new in the
  output -- `derived from  c = <c>`, `the count is  <x> of c`, and the rule
  under the middle-half line -- and the comment says why the old reading could
  never read inside. The docstring names the new reading; "it asserts nothing
  and changes nothing" stays true and stays.
- `tests/test_search.cpp` "ordering keeps the tree small", comment only: the
  S023 paragraph (13446 inside [7270, 32512], so the pair stays; S023 kept it
  against the old quartiles); the `Re-derive:` line names what the script now
  prints; the 2026-09-20 history sentence keeps "below the S091 pair's
  quartiles, [20068, 53226]" as what happened and adds that around 17451 the
  middle half is [7805, 34902] with 17332, 16246 and 16256 inside, so by this
  reading the trigger had not fired; and the rule, "re-derive when the count
  leaves the middle half of the band, [c / sqrt 5, c x sqrt 4] around the count
  c the pair was derived from, half of the factor from c to either end,
  [7270, 32512] for this pair".
- `DEV_MANUAL.md` "ordering keeps the tree small" (the goldens row): the pair
  cell names `c` and S023's 13446 inside [7270, 32512]; the re-derive cell
  carries the same rule sentence; the S222 history sentence is corrected as in
  the comment; "the count is 20427 again, inside the pair's middle half" was
  checked against the new bounds and holds -- 20427 is inside [7270, 32512] as
  it was inside [18694, 49581] -- so it now says "was" and names both readings.
- The rule the script prints is the rule both documents state:
  `middle half [7270, 32512]` over `[c / sqrt 5, c x sqrt 4], half of the
  factor from c to either end`.

Checked, unchanged: the case's assertion, its failure message, 65024 and
3251; `MANUAL.md` (no UCI surface moves); `adocs/data/README.md`'s rows for the
script and its log -- the script row's "asserts nothing ... the step's
decision" stays true, and the log row's "inside the middle half" is the
2026-09-10 reading under the old formula, of evidence that is not rewritten;
`adocs/specs.md` names neither the case nor the script. No `src/`, so no
`Bench:` line is owed.

### Phase 2, to run when the machine is free

1. `cmake --build build -j8`, then `python3 adocs/data/S192_node_budget.py` on
   the current tree. Expected, if S023 verdict 1 stands (count 13446):

        count         13446 nodes, depth 5 on KIWIPETE_POS, cold table
        budget        53784  (4x the count)
        floor         2689  (the count over 5)

        shipping      budget 65024, floor 3251
        the count is  4.14x the shipping floor and 0.21 of the shipping budget
        derived from  c = 16256  (the shipping budget over 4; the floor agrees)
        the count is  0.83 of c
        middle half   [7270, 32512]: the count is inside it
                      [c / sqrt 5, c x sqrt 4], half of the factor from c to either end

   If verdict 1 is H0 and reverted, the count is the parent `ade38e1`'s,
   which no record names (20427 was `cbaa699`'s, and ten `src/` commits sit
   between them): the bounds are the same, and the verdict is whatever that
   count gives against [7270, 32512].
2. Accepts (1), on a build whose shipping pair was derived from its own count:
   a scratch worktree at `326f274` (the 2026-09-20 re-derivation, count 16256,
   read there twice and once more from the tune build), this step's script
   copied over its `adocs/data/S192_node_budget.py`, a Release build of the
   `test_search` target, and the script given that binary. Expected:
   `count 16256`, `budget 65024`, `floor 3251`, `5.00x the shipping floor and
   0.25 of the shipping budget`, `derived from  c = 16256`, `1.00 of c`,
   `middle half   [7270, 32512]: the count is inside it`. The script reads the
   source beside itself, so the copy reads `326f274`'s case.
3. The TESTS chain in both builds and `./clang-format.sh --check`: the only
   C++ change is comment text, every line within 80 columns.
4. `tools/plan_prose_check.py` in all five modes: `--prose`, `--citations`,
   `--touches`, `--params`, `--gate`. Expected green; this file cites by path
   and doctest title or symbol only, and no parameter number moved.

### Proposed `done:` stamp

`2026-10-<dd>. adocs/data/S192_node_budget.py reads the middle half around the
count the shipping pair was derived from, [c / sqrt 5, c x sqrt 4], with c
recovered from the pair (budget / 4, the floor checked as c // 5): on 65024 /
3251, c = 16256 and [7270, 32512]. (1) On 326f274's tree, the 2026-09-20
re-derivation, with this script: count 16256, 1.00 of c, inside <observed>; on
the current tree <count>, inside. (2) DEV_MANUAL.md's goldens row and the
case's comment state the script's rule; the 2026-09-20 history is corrected
-- around 17451 the middle half is [7805, 34902] and 17332, 16246 and 16256
sit inside it, so that re-derivation fired on the defect -- and 20427 holds
inside both readings. (3) 65024 / 3251 unmoved, no src/. TESTS chain green in
both builds, format clean, plan_prose_check green in all five modes
<observed>. MANUAL.md checked, no change. Implemented by an Opus subagent in
two phases, phase 1 blind under DEC-260; run and stamped by the coordinator.`

### Commit text

    Read the node band's middle half around its own count (S282)

    S192_node_budget.py drew the middle half as the band's own quartiles,
    which start at 1.15c for a band [c / 5, 4c], so every freshly derived
    band read its own count as outside and DEC-142's re-derivation trigger
    said nothing. The middle half is now [c / sqrt 5, c x sqrt 4] around
    the count c the pair was derived from, recovered from the pair itself:
    [7270, 32512] for 65024 / 3251 from 16256, which reads 16256 inside.

    The case's comment and DEV_MANUAL's goldens row state the same rule,
    and their history is corrected: the 2026-09-20 re-derivation fired on
    the old reading, every count on record then being inside the new
    middle half of the S091 pair. No golden number moves. Filler under
    DEC-171; DEC-142.

## Phase 2 -- 2026-10-15, Opus subagent, on `fd33954`

The machine was this step's, shared with a read-only reviewer; no timing was
taken and no match started. The worktree was fast-forwarded from `fb53847` to
`fd33954` (S023 verdict 1 read H0, reverted as `0cdd39a`) under a stash. Two
conflicts, both sides kept: `DEV_MANUAL.md` takes this step's row for "ordering
keeps the tree small" and the revert's row for `short_line_ceiling`;
`tests/test_search.cpp` "ordering keeps the tree small" lost S023's separate
paragraph to the revert, and its reading moved into the History block,
rewritten for the reverted tree. The phase 1 stamp and commit text above are
superseded by the two below.

### Stops and surprises, first

No stop. Nothing red for an unpredicted reason.

1. **The count on the current tree is 13446, not 16256.** The reverted `src/`
   is `7e36271`'s byte for byte (`git diff 7e36271 HEAD -- src/` is empty),
   and the script reads 13446 on it in both builds. So 13446 was never S023's
   own count: its candidate read the same number as its parent. Phase 1
   attributed it to S023's tree, and that attribution is gone from the
   comment. No record says where between `326f274` (16256) and `7e36271` the
   count reached 13446: `cbaa699` read 20427, and 53 `src/` commits sit in that
   range.
2. **Accepts (1) is therefore not shown on the current tree**, whose pair was
   derived from 16256 and not from its own count. It is shown on `326f274`'s
   tree as phase 1 planned: `git archive 326f274` into the session scratchpad
   (no worktree, no ref), the submodule's doctest copied in (same commit,
   `ae7a135`), this step's script copied over, a Release build of
   `test_search`.
3. **Found and fixed, trivial and in scope:** given a relative binary path the
   script failed. `read_count` runs the case with its working directory set to
   the binary's own, where a relative path no longer names it:
   `python3 adocs/data/S192_node_budget.py build-tune/tests/test_search` raised
   `FileNotFoundError`, observed. `main` now takes `os.path.abspath` of the
   argument, and the same command reads 13446.
4. The first chain run had `test_clang_format_script` red only because this
   shell had not exported `CLANG_FORMAT_MAJOR=22` (DEC-146; `.moltke.local.md`
   exports it). Re-run with it exported: green. No code involved.

### Readings, every number from the script

Saved under `.tuning/coord/S282/` as `p2_node_budget_*.txt`.

| tree | pair | count | of c | middle half | verdict | old quartiles |
|---|---|---|---|---|---|---|
| current, `build` and `build-tune` | 65024 / 3251, c = 16256 | 13446 | 0.83 | [7270, 32512] | inside | [18694, 49581], OUTSIDE |
| `326f274`, where the pair was derived | 65024 / 3251, c = 16256 | 16256 | 1.00 | [7270, 32512] | inside | [18694, 49581], OUTSIDE |
| `3a649c0`, `src/` equal to `326f274`'s, the S091 pair | 69804 / 3490, c = 17451 | 16256 | 0.93 | [7805, 34902] | inside | [20068, 53226], OUTSIDE |

The old quartiles are the script as it stood at `fd33954`, run on the same
binaries. Every phase 1 figure is reproduced: [7270, 32512], [7805, 34902],
[18694, 49581], [20068, 53226], and the current tree's 53784, 2689, 4.14x and
0.21.

### Wording changed in phase 2

- `tests/test_search.cpp` "ordering keeps the tree small": the History block
  ends "**13446 on S261's engine**, `7e36271`, read by the script at S282: 0.83
  of the 16256 the pair was derived from and inside its middle half, so the
  pair stays. S023 verdict 1's candidate read the same 13446 and kept the pair
  against the band's own quartiles, [18694, 49581], which read it as outside;
  it read H0 and was reverted." The rule sentence and the S222 history
  correction are unchanged from phase 1.
- `DEV_MANUAL.md` "ordering keeps the tree small": the pair cell says the same
  in the row's terms. The rest of the row is unchanged from phase 1.
- `adocs/data/S192_node_budget.py` `main`: the `abspath` above.

### Checks

- TESTS chain with `CLANG_FORMAT_MAJOR=22` exported: `build` 46 of 46, then
  `build-tune` 46 of 46, then `./clang-format.sh --check` clean, exit 0
  (`p2_tests_chain.txt`; re-run on the final tree, `p2_tests_chain_final.txt`).
- `tools/plan_prose_check.py`: `--prose`, `--citations`, `--touches`,
  `--params` and `--gate` each exit 0 and flag nothing.
- No `src/`, so no bench is owed and the second tier does not apply.
  `MANUAL.md` was checked and needs no change. `adocs/specs.md` names neither
  the case nor the script. Both of `adocs/data/README.md`'s rows stay true.

### Fast check: 3 findings, fixed

1. **A pair not derived by the script gave no verdict.** `main` printed
   "middle half none" and exited 0 for a pair that is not `[c // 5, 4c]` of
   one count, so a hand-widened budget passed silently where the old code
   gave a verdict. That branch now prints `derived from  none: <budget> /
   <floor> is not [c // 5, 4c] of one count c` and `middle half   none: the
   count is OUTSIDE it, the pair not derived by the script: re-derive the pair
   by the two ratios`. The docstring, the case's comment and DEV_MANUAL.md's
   row carry the clause: a pair not `[c // 5, 4c]` of one count was not
   derived by the script and reads OUTSIDE, so it is re-derived by the two
   ratios. Shown on the scratch copy of `326f274` with its source's budget
   hand-widened to 80000 against the floor 3251 (the binary untouched, the
   tracked test untouched): `derived from  none: 80000 / 3251 is not
   [c // 5, 4c] of one count c` and the OUTSIDE line, exit 0
   (`p2_node_budget_widened.txt`). Put back to 65024, the same copy reads
   16256 inside [7270, 32512] again.
2. **"Below the S091 pair's quartiles since S222" was false**, and the list of
   counts left one out. 17451 is S091's own count and S222's parent reading,
   and it is already below 20068, so the count sat below the quartiles from
   S091 on. Every count read against that pair was checked by grep in the
   step files: 17451 (S091; S222 line 214), 17332 (S222 phase two), 17321
   (S222 phase three, `d0a6667`), 16256 (S231 phase one; the revert), 16246
   (S231 phase two). All sit below [20068, 53226] and inside [7805, 34902],
   and both the comment and the row now list them.
3. **Two triggers read as if they disagreed**, and "that count" had lost its
   antecedent. Both texts now say it in order. The 2026-09-20 re-derivation
   fired on the old quartiles. It was done correctly, 65024 and 3251 from
   16256 by the two ratios, which is what DEC-142 asks. The middle half is the
   trigger that governs since S282, and by it that re-derivation would not
   have fired.

Re-run after the fixes on the final tree: the TESTS chain with
`CLANG_FORMAT_MAJOR=22`, 46 of 46 in `build` and in `build-tune`, format
clean, exit 0 (`p2_tests_chain_fastcheck.txt`). The script on the current
tree reads 13446, 0.83 of c, inside [7270, 32512]. `plan_prose_check.py`
flags nothing in any of its five modes.

### Proposed `done:` stamp

`2026-10-15 HH:MM -- adocs/data/S192_node_budget.py reads the middle half
around the count the shipping pair was derived from, [c / sqrt 5, c x sqrt 4],
with c recovered from the pair (budget / 4, the floor checked as c // 5):
[7270, 32512] for 65024 / 3251 from 16256. A pair not [c // 5, 4c] of one
count reads OUTSIDE, re-derive by the two ratios; shown on a scratch copy
with the budget hand-widened to 80000. (1) On 326f274's tree, where the pair
was derived, the script reads 16256, 1.00 of c, inside, where the old
quartiles [18694, 49581] read it OUTSIDE. On the current tree, S261's engine
7e36271, it reads 13446, 0.83 of c, inside, in both builds; S023's candidate
read the same count. (2) DEV_MANUAL.md's goldens row and the case's comment
state the script's rule. Their history is corrected: every count read
against the S091 pair -- 17451, 17332, 17321, 16256, 16246 -- sat below its
quartiles [20068, 53226] from S091 on and inside its middle half
[7805, 34902], the script reading 16256 on 3a649c0's tree at 0.93 of c. So
the 2026-09-20 re-derivation fired on the defect, though it was done
correctly from 16256 by the two ratios. 20427 holds inside both readings.
(3) 65024 / 3251 unmoved, no src/. Found on the way and fixed: a relative
binary path failed (abspath). Fast check: 3 findings, fixed. TESTS chain
green in both builds, 46 of 46 each, format clean at CLANG_FORMAT_MAJOR=22;
plan_prose_check flags nothing in any of its five modes. MANUAL.md checked,
no change. Implemented by an Opus subagent in two phases, phase 1 blind under
DEC-260; stamped by the coordinator.`

### Commit text

    Read the node band's middle half around its own count (S282)

    S192_node_budget.py drew the middle half as the band's own quartiles,
    which start at 1.15c for a band [c / 5, 4c], so every freshly derived
    band read its own count as outside and DEC-142's re-derivation trigger
    said nothing. The middle half is now [c / sqrt 5, c x sqrt 4] around
    the count c the pair was derived from, recovered from the pair itself:
    [7270, 32512] for 65024 / 3251. On 326f274's tree, where that pair was
    derived, 16256 reads inside at 1.00 of c; the current tree reads 13446,
    inside at 0.83. A pair not [c // 5, 4c] of one count, such as a
    hand-widened budget, reads OUTSIDE: re-derive it by the two ratios.

    The case's comment and DEV_MANUAL's goldens row state the same rule,
    and their history is corrected: every count read against the S091 pair
    sat below its quartiles from S091 on and inside its new middle half,
    so the 2026-09-20 re-derivation fired on the old reading, though it
    was done correctly from 16256. A relative binary path now resolves. No
    golden number moves. Filler under DEC-171; DEC-142.
