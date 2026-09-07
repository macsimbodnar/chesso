id:         S202
goal:       a mate score inherited from the table at a depth too shallow to back it is either given a line that reaches it or not published as a mate -- the class S171's census measured and its own excludes forbade it to touch (DEC-150)
accepts:    the mechanism is reproduced from `adocs/data/S170_cases.tsv`'s `F_mate6_inherited_no_line` before anything is changed, and the reading below is confirmed or corrected from the code rather than argued; whatever is chosen is decided by SPRT if it alters play at all -- it very likely does, since withholding or re-deriving a score changes what the table answers to later nodes -- and by identical `tools/search_bench.py` node counts and best moves if it does not (INV-6); the guard row's `guard` flips to `yes` and `tests/test_mate_carry.cpp` is red before the change and green after; a `fastchess.sh --fast` census reports a candidate count **at or below 8 lines from 1 search in 3000 games**, against the reference's own count in the same run, which is the ceiling DEC-150 set and not a zero; `adocs/specs.md`, `DEV_MANUAL.md`'s instrument 3 and `MANUAL.md` take whatever the verdict is, in the same commit
touches:    src/search.cpp, tests/test_mate_carry.cpp, adocs/data/, adocs/specs.md, DEV_MANUAL.md, MANUAL.md
excludes:   the completion walk itself -- S147, S170 and S171 are that ground and DEC-150 measured that no walk can close this case, because the line the score names is not in the table to be found; improving mate *finding*, which is S148's and S154's ground
decisions:  DEC-122, DEC-127, DEC-150, DEC-151
closes:
blocks:
paused_by:
author:

## What this is

S171's census, 2026-09-07 on the workstation, left **8** `Incomplete mating PV`
lines from **1** search in 3000 games. DEC-150 is the reading. The short version:

| asked | answer |
|---|---|
| is the score right | **yes** -- `stockfish` gives `#+6` at depth 20 and 30, and `#+5` after the move chesso plays |
| where does it come from | the table, at **depth 3 on 1224 nodes** -- too shallow to build the 11 plies a `mate 6` owes |
| why can the walk not finish it | the line that iteration built continues, in the table, into a chain proving **`mate 8`**; both lookups in `complete_mate_pv()` are keyed on the distance still owed, so both refuse |
| is it a regression | **no** -- three byte-identical short lines on `457e355`, the commit before `certified_mate_move()` |
| does it cost a game | unmeasured; the two builds are INV-6 identical, so nothing in S171's run says either way |

By depth 7 the same search reports `mate 8` with a complete 15-ply line, and by
depth 11 a complete 11-ply `mate 6`. Only the shallow iterations are affected,
and they are the ones a fast time control publishes most of.

## The reproduction, which exists and is cheap

```bash
~/.venv/chess/bin/python adocs/data/S170_replay.py --engine build/src/chesso \
    --cases adocs/data/S170_cases.tsv --only F_mate6_inherited_no_line
```

`nodes 300000`, from ply 64, **stride 2**. Deterministic: 6 mate lines, 3 short,
three runs each on both binaries. The stride is not optional and DEC-151 is why
-- a game gives one engine only its own turns, and this case does not reproduce
at stride 1 at all. `build/tools/mate_trace --stride 2` reads the entries behind
it; `DEV_MANUAL.md` instrument 3 has the invocation.

## Where to look, and what is already ruled out

Ruled out by measurement, not by argument:

- **the score being wrong.** It is not; Stockfish confirms the distance.
- **a lost slot the walk could route around.** That is S171's
  `certified_mate_move()` and it does not apply: the 11-ply line is not in the
  table at that moment under any route, so nothing to find one ply down.
- **a forced move refused for carrying a bound.** Implemented during S171 and
  reverted -- the guard case stayed red under it. Recorded in DEC-150 so it is
  not tried a second time.

What is left, and the order they are worth separating in:

- **The score and the line come from different places.** The score is the
  table's, for the root; the line is `pv_table`'s, built by this shallow
  iteration. S170's third cause is the same shape one level up -- a score and a
  line from different *iterations* -- and its answer was to complete the line
  against the score it is printed beside. Here that completion cannot succeed,
  so the question is what to print instead.
- **Not publishing what cannot be backed.** The obvious form: where
  `complete_mate_pv()` refuses, report the bound rather than the mate. This is
  where it stops being reporting-only -- what the search *stores* is a separate
  question from what it prints, and the two must not be conflated.
- **Whether a shallow iteration should trust a mate distance from the table at
  all.** The most invasive reading and the one that plainly alters play.

## Why it is not urgent, and where it sits

It changes no game that has been measured: S171's run put the two builds at
+3.24 +/- 8.91 Elo over 3000 games with the residual present in one of them and
absent from the other only by which side met the position. It is a reporting
defect with an unmeasured search-side question behind it. It sits after the
search block's own steps rather than before them, and DEC-150's ceiling of 8
is what a later census is read against.
