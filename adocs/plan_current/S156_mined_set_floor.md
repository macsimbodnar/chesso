id:         S156
goal:       the mined breadth set is either asserted as the count with a floor its accepts asked for, or the accepts is discharged in the stamp with the reason
accepts:    `adocs/data/S145_mined_set.tsv` is either asserted in the suite as the count with a floor its `accepts` asked for -- with the floor placed from the measured 146 at `RfpMinPly` 2 and 3 against 139 at 1 and 0, and observed red below it -- or the clause is discharged in writing with the reason the measurement in the step body satisfies it; whichever way it goes, the file stops being data that nothing reads; if it is asserted, the cost it adds to the fast suite is measured and stated, since the suite runs at every step completion
touches:    tests/test_mate_breadth.cpp, tests/CMakeLists.txt, adocs/data/, DEV_MANUAL.md, adocs/specs.md
            # amended when the plan met the code, 2026-09-01. It said
            # tests/test_engine.cpp, and the assertion did not go there. 318
            # positions at depth 10 cost 18.28 s in Release and 697 s in Debug;
            # folded into test_engine that is its line going from 3.19 s to
            # 21.5 s with nothing in the ctest output to say where the time
            # went, and a Debug run that the shared 60 s timeout would report
            # as a hang. So it is its own binary with its own measured timeout,
            # which is tests/CMakeLists.txt as well. DEV_MANUAL.md and
            # specs.md are the DOCS completion check landing: both carried the
            # old 146 / 139 reading, which no longer reproduces.
excludes:   the constructed set and its assertions, which S145 landed; per-position pass/fail over mined mates, which S145's research shows is what made two surveyed projects disable their tests; re-mining the set from a different game source
decisions:  DEC-019
closes:     2026-08-21_adversarial-F08
blocks:
paused_by:
author:     agent, 2026-09-01
done:

## The gap

`grep -rn "mined_set|S145_mined" tests/ src/ CMakeLists.txt` returns one
comment at `tests/test_engine.cpp:1694` and nothing else. The 318 positions and
their script exist; no assertion, no floor, no ctest registration, and the
`done:` stamp does not mention them.

## What was measured, 2026-09-01

Every number here was read on the machine `.moltke.local.md` describes, at
`120497e`, `Hash` at its default 16, one engine process per sweep row with
`ucinewgame` before each position.

### 1. The accepts' own numbers no longer reproduce, and one of them was
### unreachable

S145 placed the floor at 143 from **146 exact at `RfpMinPly` 2 and 3 against
139 at 1 and 0**, at depth 10. Re-measured here:

| `RfpMinPly` | exact | right sign | wrong sign |
|---|---|---|---|
| 3, ships | 145 | 147 | 0 |
| 2 | 145 | 147 | 0 |
| 1 | 141 | 143 | 0 |
| 0 | 141 | 143 | 0 |

The shipping reading is one lower and the weakened reading two higher, so the
gap has closed from 7 to 4. The tree has moved since S145 -- S142, S149 and
S165 all alter play -- and the machine changed too, so neither cause is
separable from the other and neither needs to be: 143 still sits strictly
between the two, which is the only property the floor has to have.

**The first sweep taken for this step read a perfect null and was wrong.**
`setoption name RfpMinPly value 1` returned identical counts to the default at
every depth tried. The cause is S142: it narrowed the parameter's declared
minimum to 2 on S145's own evidence, so the engine refuses the value --
`info string refused [RfpMinPly] value 1, outside [2, 63]` -- and stays at 3.
A sweep that does not notice is measuring one engine against itself.
`DEV_MANUAL.md` already recorded the refusal for `S145_rfp_sweep.py` and said
what to do about it; this step is that instruction automated.

### 2. Depth is the cost, and the cheap depth separates better

| depth | exact, ships | exact, `RfpMinPly` 1 | gap | wall |
|---|---|---|---|---|
| 8 | 113 | 101 | **12** | 3.0 s |
| 9 | 143 | 135 | 8 | 7.4 s |
| 10 | 145 | 141 | 4 | 17.6 s |

Depth 8 is six times cheaper and discriminates three times wider. **The owner
chose depth 10 and floor 143 (2026-09-01)**, which is the accepts' own depth
and the accepts' own floor; the table is recorded because the next time this
floor goes red the cheaper reading is the alternative to lowering it, and it
should not have to be re-derived.

### 3. What the gate cost the suite

Release, this machine: the fast label goes from **28.50 s over 21 tests to
45.92 s over 22**, and `test_mate_breadth` is **18.28 s** of it -- the largest
single line in the gate. Debug: **697 s**, 38 times the Release figure, which
is why the binary carries its own timeout of 1500 s rather than
`add_doctest_target`'s shared 60. `ctest --test-dir build-debug -L fast` is
now an eleven-minute-longer command, and that is stated rather than discovered.

### 4. Two drivers, one count

The gate scores in-process through the UCI layer in C++; `S145_mined_set.py`
scores through python-chess in a subprocess, and python-chess is not installed
on this machine. So the C++ count was cross-checked against an independent
standard-library subprocess driver over the same TSV at the same depth: both
read **145 exact, 147 right sign, 0 wrong sign**. The sweep script uses that
same standard-library driver, which is why it runs here at all.

## What this does not do

The mined set is breadth and not the guard. Nothing in it was built for the
reverse futility hazard, which S145 measured at roughly never in play -- of 191
sampled positions where the side to move is mated within six, one has a
non-negative score for the mated side. The constructed set in
`tests/test_engine.cpp` remains the instrument for the hazard, and this one
covers the other direction: a change that costs mate finding generally, on
positions nobody chose.

It also cannot tell `RfpMinPly` 2 from 3. That was already S145's finding and
it is unchanged; the floor separates 1 from 2 and nothing finer.
