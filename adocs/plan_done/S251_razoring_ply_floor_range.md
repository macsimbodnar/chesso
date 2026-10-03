id:         S251
goal:       `RfpMinPly`'s declared range holds a mate-safe value at every setting now that razoring reads it too, or razoring takes a floor of its own
accepts:    one of two forms, chosen with the reason and decided before the change: (a) `RfpMinPly`'s minimum moves 2 -> 3, the S145 mate-in-two case re-run at 3 green and the range's comment and `MANUAL.md` row saying why; or (b) razoring reads a separate `RazorMinPly` (seed 3, range with a mate-safe minimum shown by the same case) and `RfpMinPly` keeps 2 for reverse futility alone; either way the tune build at the range's minimum finds all 26 S145 mates in two at iteration 3, observed, and both fast suites are green; the shipped engine is node-identical (INV-6) unless the step says otherwise and measures it
touches:    src/search_params.hpp, src/search.cpp (only under form b), MANUAL.md, tests/test_search_params.cpp (golden_defaults); as built also DEV_MANUAL.md, tests/test_engine.cpp and tests/test_mate_breadth.cpp (comments only), adocs/data/S154_floor_margin_sweep.py, adocs/data/S156_mined_floor_sweep.py
excludes:   the shipped value 3; any change to the rule's verdict, which is S116's SPRT
decisions:  DEC-095, DEC-171, DEC-215, DEC-248
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S251_brief.md), 2026-10-03
done:       2026-10-03 -- form (a): `RfpMinPly`'s declared minimum 2 -> 3, the shipped 3 unchanged, because razoring reads the same floor (DEC-248) and at 2 the tune build finds 17 of the 26 S145 mates in two on time; S127, the only tuner, needs no 2. INV-6 against a Release build of `711f788`: `bench` 4081329 with the whole stream and all 8 bestmove replies, `bench 12` 1860699, `search_bench` 9 and 12 identical. The tune build at 3 finds 26 of 26 on time (S154 `here`); asked for 2 it refuses ("outside [3, 63]") and keeps 3. `golden_defaults`' row 2 -> 3, count 73; the range comment, `MANUAL.md`'s row, three `DEV_MANUAL.md` statements and two test comments follow; S154's and S156's sweep scripts accept the new X-macro line beside S142's. M06a unaffected (S252's). `specs.md`'s S168 reading gains the S251 sentence. Both fast suites 41 of 41; clang-format clean; plan prose checks clean; cold fast check FIX-FIRST: the S154 sweep script's patch made both accepted X-macro lines required, aborting slack, floor and red at every ref -- fixed and proved by real `floor` runs at the new tree and `711f788` (identical tables), `red` red, and S156's sweep run by the coordinator on the new tree (149 against 143 at 145, gate red); one stale comment figure dated. No other project's code was opened; `README.md` human-owned, no change.

## Why this exists (2026-10-02, the coordinator)

S116's build (finding 1 in its "As built" section) found that razoring, which
reads `RfpMinPly` as its ply floor by DEC-248, reaches the ply-2 node holding a
mate in two's quiet mating move at `RfpMinPly` 2: the tune build there finds 17
of the 26 S145 mates in two at iteration 3. The range's minimum 2 was measured
for reverse futility alone (S145, DEC-095). Unreachable in ordinary play (the
Release build compiles the shipped 3; only the tune build sets it), so a filler
behind S116 (DEC-171), named in `adocs/data/S116_sprt.sh`'s open findings as
item 20.

## As built (2026-10-03, the Opus subagent)

### Deviations and findings, first

1. **`touches:` widened, stated on the line.** Beyond the four named files:
   `DEV_MANUAL.md` (three statements of the range), comment-only edits in
   `tests/test_engine.cpp` (the mate-in-two clause said "at RfpMinPly 2 and
   above it is 26 of 26 ... on time", false since S116) and
   `tests/test_mate_breadth.cpp` (the minimum named), and the two sweep scripts
   below. No assertion changed anywhere.
2. **S154's and S156's sweep scripts would have stopped.** Both match
   `RfpMinPly`'s X-macro line whole (`3, 2, 63`) and abort when it is absent,
   so after this change `floor`, `slack` and `red` (S154) and the whole of
   S156 would refuse to run. Each now knows both lines: the shipping
   `3, 3, 63` and S142's `3, 2, 63`, either rewritten to the relaxed or
   weakened line, so a `--ref` before S251 still works. In S154 both tuples
   are optional (the fourth field `False`) and `verify` is the guard that
   refuses a tree carrying neither; S156's `patch()` takes the first line
   present and stops when neither is. **The first version marked both S154
   tuples required, which aborted every mode at every ref** (the coordinator's
   cold fast check, FIX-FIRST); `script_patch_check.log` had tested only the
   find and the verify, never `worktree()`'s loop. Fixed, then run for real
   (logs under `.tuning/coord/S251/`): `floor --ref a3c126b` (a `git stash
   create` object carrying this tree, `stash_sha.txt`) and `floor --ref
   711f788` both complete and print the same table, node for node
   (`S154_floor_stash_a3c126b.log`, `S154_floor_711f788_fixed.log`), and `red
   --ref a3c126b` builds the gate at default 1 and reads **RED, as it must
   be**, `REQUIRE( 7 >= 11 )`, 28 of 3496 assertions failed
   (`S154_red_stash_a3c126b.log`). S156 was not run (its sweep is the mined
   set's, minutes long); its `patch()` change is the find over a tuple
   checked in `script_patch_check.log`. Neither script's sweep values change.
3. **Stale figure, dated:** `tests/test_engine.cpp`'s same paragraph said 1
   and 0 give "13 of 26 on time"; S154's `floor` at `711f788` reads **12**
   (20 found). The comment now dates 13 to S168 and gives 12 at `711f788`
   (fourteen positions red, not thirteen). Comment only, no assertion reads
   it. `specs.md`'s search row has the S168-dated sentence
   "`RfpMinPly` 2 and 3 are identical ... with every mate in two immediate",
   now false at 2 -- edit proposed below, not made.
4. **`MANUAL.md`'s row also carries the S250-dated mined reading "146 ... at
   2 and 3"**; S116 re-read 149 at 3 and 145 at 2. Dated, left alone.

### The form, and why: (a), decided before any change

Form (a), `RfpMinPly`'s minimum 2 -> 3. The brief prefers it unless reverse
futility needs its 2 for tuning, and nothing does: S127 (`plan_todo/`) is the
only tuner of these and tunes "the full set in `src/search_params.hpp`" with
no word on `RfpMinPly` or its minimum; its config is not written yet and will
read the declared range. No current `tools/spsa*.json` lane carries
`RfpMinPly` (`spsa_s085.json` is S085's finished run, `min 0`, below even the
old minimum and refused by the engine since S142 either way). One floor for
both rules is then the stricter rule's, 3, and form (b) would add a parameter,
a `search.cpp` change and a golden count move to keep a value (2) no step asks
for.

### What changed

- `src/search_params.hpp`: `X(RFP_MIN_PLY, "RfpMinPly", 3, 3, 63)`; the range
  comment says the minimum is 3 since S251 and why (razoring, 17 of 26 on time
  at 2), the 2 paragraph is dated "from S142 to S251", and the file header's
  "measured" example reads 3.
- `tests/test_search_params.cpp`: `golden_defaults`' `RfpMinPly` row min
  2 -> 3; the count stays 73 (form (a) adds no parameter).
- `MANUAL.md`: the `RfpMinPly` row's range `3 to 63`, why, and the refusal
  line the tune build prints for 2.
- `DEV_MANUAL.md`: the S156 reproduction note, the S145 floor-sweep note (now
  stops at 3; the refusal line re-taken) and the mate-gate paragraph ("below
  3").

### Measurements (logs under `.tuning/coord/S251/`)

- **INV-6, Release against a Release build of `711f788`**
  (`ref/chesso_711f788`, built in this worktree before the edit): `bench`
  **4081329** both, the whole stream identical with `time`/`nps` stripped and
  all 8 `bestmove` replies identical (`ref_bench.txt`, `new_bench.txt`);
  `bench 12` **1860699** both, stream identical (`ref_bench12.txt`,
  `new_bench12.txt`); `search_bench` at 9 (32932 / 70095 / 25178, c3d5 /
  e2a6 / d7c8q) and 12 (67792 / 280873 / 137893, c3d5 / d5e6 / d7c8q)
  identical in nodes and best moves (`ref_sb*.txt`, `new_sb*.txt`,
  `inv6_summary.txt`). No SPRT owed.
- **The tune build at the new minimum**, `python3
  adocs/data/S154_floor_margin_sweep.py here --engine build-tune/src/chesso`:
  **m2 26/26, on-time 26, d0**; m3 12/24, 9782127 nodes
  (`S154_here_tune_min3.log`) -- the same row and node total as `711f788`'s
  `RfpMinPly=3` row.
- **The parent's table**, `S154_floor_margin_sweep.py floor --ref 711f788`
  (pre-edit script, `S154_floor_711f788.log`, 15.1 s): `RfpMinPly` 3 m2 26/26
  on-time 26; **2 m2 26/26 on-time 17, delay 1** (S116's reading,
  reproduced); 1 and 0 m2 20/26 on-time 12.
- **Asked for 2, the tune build refuses and keeps 3**: `info string refused
  [RfpMinPly] value 2, outside [3, 63]`; `uci` prints `option name RfpMinPly
  type spin default 3 min 3 max 63` (`tune_refusal.txt`). S154's `measure`
  stops on that refusal rather than measuring the default
  (`S154_refused_2.log`).

### M06a and S156

- **M06a** (`tools/mutants/search.py`, `ply >= RFP_MIN_PLY - 1` in reverse
  futility's guard) mutates the source expression, not the declared range,
  and the binaries it builds compile the constant 3, so it still puts reverse
  futility's floor at ply 2 and razoring's at 3. Nothing changes for it; its
  kill is S252's. M06b (`- 2`) likewise.
- **S156's sweep** relaxes the bound to `3, 0, 63` in a throwaway worktree and
  then weakens the default to 1; it now finds either shipping line (finding 2).
  Its sweep values and its gate reading are unchanged by the minimum.

### Suites

Both fast suites green, 41 of 41 each (Release 100.7 s, tune 101.4 s);
`./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22`
(`suites.log`). `tools/plan_prose_check.py` `--citations`, `--touches`,
`--params`, one mode per call, each exit 0, 0 flagged
(`prose_*.log`). No match, no `gate_extra.sh` (the brief's; no search,
`make_move` or generator change, so DEC-141's second tier is not owed).
`DEV_MANUAL.md` and `MANUAL.md` checked and edited; `README.md` human-owned,
no change.

### Proposed `specs.md` edit (the coordinator's)

After "... eleven positions failing the clause that fences the tuner where
seven failed it over the 48." in the search row, add: "Since S116 razoring
reads the same floor, and at `RfpMinPly` 2 the mates in two read 26 found and
17 on time, so S251 raised the declared minimum to 3 (S154's `floor` at
`711f788`); the shipped value was already 3 and the Release build is node for
node the same."

### Proposed `done:` stamp

`2026-10-03 -- form (a): RfpMinPly's declared minimum 2 -> 3, the shipped 3
unchanged, because razoring reads the same floor (DEC-248) and at 2 the tune
build finds 17 of the 26 S145 mates in two on time; S127, the only tuner, needs
no 2. INV-6 against a Release build of 711f788: bench 4081329 with the whole
stream and all 8 bestmove replies, bench 12 1860699, search_bench 9 and 12
identical. The tune build at 3 finds 26 of 26 on time (S154 here); asked for 2
it refuses ("outside [3, 63]") and keeps 3. golden_defaults' row 2 -> 3, count
73; the range comment, MANUAL.md's row and three DEV_MANUAL.md statements
follow; S154's and S156's sweep scripts take the new line beside S142's, S154's floor and red run at a stash object of this tree and floor at 711f788. M06a
unaffected (S252's). Both fast suites green; clang-format clean; plan prose
checks clean. No other project's code was opened.`

### Proposed commit

```
Raise RfpMinPly's minimum to 3 now razoring reads it

S251, a filler behind S116 (DEC-171). Razoring reads RfpMinPly as its
ply floor (DEC-248), and at 2 the tune build finds only 17 of the 26
S145 mates in two at the first iteration that can hold them; the
measured 2 was reverse futility's alone (DEC-095). Form (a): the
declared minimum moves 2 -> 3, the shipped 3 unchanged, so the Release
build is node for node the same (INV-6: bench, bench 12, search_bench 9
and 12 identical against 711f788). The tune build at 3 finds all 26 on
time and refuses 2. golden_defaults, MANUAL.md, DEV_MANUAL.md and the
S154/S156 sweep scripts follow.

No functional change
```
