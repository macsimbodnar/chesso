id:         S204
goal:       `test_mate_carry`'s cases stop being pinned to isolated spikes in the node-budget grid, so the guard fires on a mate the engine has actually stopped finding and not on any change that moves the tree
accepts:    the guard's failure mode is demonstrated before it is changed, from the two sweeps this step is created with -- `adocs/data/S204_sweep_head.txt` and `adocs/data/S204_sweep_killer_iter_clear.txt`, the same grid over the same six cases at `c982f9d` and at `c982f9d` plus S159's per-iteration killer clear -- and the demonstration is a count, not an argument: how many of the nine stride-1 budgets report any mate line, per case, per side; a case whose pinned budget is its only non-zero cell in the grid is named as such; the replacement guard is shown to stay green across a tree-moving change it does not care about **and** red on a change that genuinely stops a mate being found, the second established with a mutant rather than assumed -- the recurring failure this fixture exists for is "pruning that hides a mate", and a guard that cannot distinguish the two is not guarding it; `expected_mate_lines`'s floors and `adocs/data/S170_cases.tsv`'s budgets are re-derived by `adocs/data/S203_case_sweep.sh` and never re-read from a failing run (DEC-142); DEC-156's re-sweep prescription is either upheld with its tension resolved or amended by a new decision, in writing
touches:    tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv, adocs/data/S203_case_sweep.sh, adocs/decisions.md, adocs/specs.md; amended on completion with the four the work reached and the list did not name -- `adocs/data/S204_class_census.py` and its two outputs, which are the class split the shape rests on and had to be reproducible (S159's lesson); `DEV_MANUAL.md`, whose paragraph on this fixture ended "Whether re-pinning to a fresh spike at every step is a guard at all is DEC-161 and S204" and now carries the answer, which the DOCS rule requires; `adocs/plan_todo/S192_golden_hygiene.md`, whose row 6 cited `expected_mate_lines` and turned the citation gate red the moment the symbol went, so it names the replacement instead; and `adocs/data/S170_cases.tsv` in the end was *not* touched, its budgets deliberately unmoved
excludes:   the `Incomplete mating PV` class itself, which is S202's and stays S202's -- this step is about whether the fixture can measure anything, not about the engine publishing a mate score it cannot back; the Zobrist keys and the case set's provenance, which are DEC-154's and S203's and are not reopened; any change to `src/`, which this step does not need and must not make
decisions:  DEC-122, DEC-142, DEC-154, DEC-156, DEC-161, DEC-162
closes:
blocks:
paused_by:
author:     Claude Opus 5, 2026-09-09
done:       2026-09-09. **The guard no longer fires on a change that moves the tree, and it fires on three separate things that matter, each shown red under a mutant.** DEC-162. The demonstration `accepts:` asks for is above as a count: over the nine stride-1 budgets `C_mate7_depth11` reports a mate line in **one cell** at `c982f9d` and its configured budget is that cell, `B_mate6_shallow`'s is 100000, the lowest cell and the edge it switches on, and every case's union over the nine is non-zero on **both** sides -- 13 against 48 for C -- so no case lost its mate and only the cell holding it moved. The replacement is three assertions and the budgets did not move: a line as long as the distance it claims **ends in checkmate**, at zero and pinned to nothing; a shorter line is S202's residue against a **per-case ceiling** (A 5, B 11, C 0, D 1, E 8) derived by `adocs/data/S203_case_sweep.sh --ceilings` from the two recorded grids; and a **majority of the guarded cases, 3 of 5**, reports a mate line at all, over the set instead of per case. The split rests on a measurement neither DEC-156 nor DEC-161 had: `adocs/data/S204_class_census.py` over both sides, **881 mate lines, 64 short, 0 that run their claimed distance and fail to be checkmate**, reproducing the two recorded grids in all 90 cells. **Green on S159's candidate A**, the change that produced four reds on the old guard -- 61 assertions, and the reconstruction verified identical to the recorded grid at every configured cell before it was trusted. **Red on a mutant, once per assertion**: no checkmate detection anywhere takes the majority to `0 >= 3`; DEC-122's own rejected option -- extend to the claimed length rather than to the mate -- takes the mate-reaching assertion red at a cell where the walk stalls, with the same budget green unmutated; the walk publishing nothing takes four ceilings red. **Three plausible search mutants stayed green and that is the guard working**: the table refusing mate scores, null move reducing to depth 0 inside the mate window, and quiescence losing checkmate detection each move the tree and each leaves the mates being found, two of them in larger numbers than HEAD. **DEC-156 is amended, not upheld** -- its re-sweep prescription stands for the budgets and no longer applies to floors, because there are none. No `src/` change: `git diff -- src/` is empty against `6324b22`, so no `Bench:` line and no self-play tier (DEC-141 applies to `make_move`, the generator and the search, none of which this step touches; `tools/gate_extra.sh` is S197 and does not exist yet). The mutants were applied by hand and reverted. Gate green in both builds, 31 of 31 each, plus `./clang-format.sh --check` with this machine's documented `CLANG_FORMAT_MAJOR=22` (DEC-146); the fixture costs 23.5 s against 26.3 s before. DOCS: `specs.md`'s wording for this fixture rewritten in the same commit, `DEV_MANUAL.md`'s open question answered, `MANUAL.md` checked and needs no change -- no UCI surface moved -- and `README.md` is human-owned and untouched

## Where this comes from

Found on 2026-09-09 while doing S159, and it is why S159 did not touch `src/`
in the end. S159 built one candidate -- the killer table cleared once per
iteration -- ran the fast suite, and got `test_mate_carry` red against a green
HEAD, on four assertions across four cases. Two were incomplete mating PVs,
which is S202's class. Two were cases reporting **no mate line at all**, which
`expected_mate_lines`'s message calls "gone vacuous and needs re-choosing, not
deleting".

The BUGS rule says a found bug is fixed before anything else starts, so the
question "is this the engine or the fixture?" was answered with the script
DEC-142 puts beside the golden -- `adocs/data/S203_case_sweep.sh`, the whole
grid, both sides. That is the evidence this step is created with.

## What the sweep says

Stride 1, nine budgets from 100000 to 4000000 nodes, cells reporting at least
one mate line:

| case | at `c982f9d` | with S159's clear | pinned budget |
|---|---|---|---|
| `C_mate7_depth11` | **1500000 only** | 1000000, 2000000, 4000000 | 1500000 |
| `B_mate6_shallow` | all nine | eight; **zero at 100000** | 100000 |
| `A_mate8_shallow` | eight | eight | 1000000 |
| `D_mate_minus6_depth10` | 1200000, 1500000, 3000000, 4000000 | 1500000, 4000000 | 4000000 |

`C_mate7_depth11` reports mate lines in **one cell of nine** at HEAD, and its
configured budget is that cell. `B_mate6_shallow`'s configured budget is
100000, the lowest in the grid and the edge at which the case switches on.
Neither is a case that a change broke; both are cases sitting on a knife edge
whose position is a property of the tree.

The short-line half is the same story from the other side. Incomplete mating
PVs are scattered across the grid **at HEAD** -- `A_mate8_shallow` 2 at 300000
and 2 at 500000, `B_mate6_shallow` 5, 6, 5 and 7 at four budgets,
`E_mate_minus9` 8 at 1000000, `F_mate6_inherited_no_line` 2 at 2000000 -- and
simply do not happen to fall on the pinned budgets. They are not new under a
change; they move.

## Why this is a defect and not a re-sweep

DEC-156 already recorded the knife edge -- it is where "C reports 13 mate lines
at 1500000 nodes and 0 at both 1000000 and 2000000" is written down -- and
prescribed the response: "The sweep is to be re-run after anything that moves
the tree, not only after a key change, and a step that moves the tree and
leaves this test green by luck has learned nothing."

Applied to a grid this sparse, that prescription re-pins each golden to a fresh
spike after every tree-moving change. DEC-156's own `Rejected:` refuses exactly
that move -- "Choosing each budget because it happened to be green. Refused
because that is fitting the fixture to the test" -- and answers it with one
rule stated before it is applied. The rule constrains *which* green cell is
chosen; it does not stop the choosing from happening again at every step, and a
golden re-fitted at every step measures nothing across the change it was
re-fitted for. That tension is not resolvable inside DEC-156 and it is what
this step is for.

The measurement above is the part DEC-156 did not have: not that the grid is a
knife edge, which it says, but that at HEAD one case's pinned budget is its
**only** non-zero cell, so the guard's green is a coincidence of one cell in
nine rather than a property the engine has.

## What is not being claimed

That S159's clear is harmless. It was reverted and never measured by SPRT
(DEC-160), so nothing here says what it does to strength. What the sweep
establishes is narrower and is all this step needs: the four reds it produced
are cells moving in a grid whose cells move on both sides, and the mate guards
written for the recurring failure -- `tests/test_engine.cpp` `"engine: mate
safety"`, S145's set, and `tests/test_search.cpp`'s mate cases -- were **green**
on that same candidate, 30 of 31 fast tests passing.

## The count, from the two recorded sweeps

Reproduced from `adocs/data/S204_sweep_head.txt` and
`adocs/data/S204_sweep_killer_iter_clear.txt` -- read, not re-run. Each case at
**its own stride** (rows A to E are stride 1, F is stride 2), over the nine
budgets of the grid. `cells` is how many of the nine report at least one mate
line; `mates` is the sum over the nine; `@budget` is what the configured cell
reports, as `(mate lines, short lines)`.

| case | guard | budget | HEAD cells | HEAD mates | HEAD @budget | clear cells | clear mates | clear @budget |
|---|---|---|---|---|---|---|---|---|
| `A_mate8_shallow` | yes | 1000000 | 8/9 | 66 | (12, 0) | 8/9 | 75 | (15, **1**) |
| `B_mate6_shallow` | yes | 100000 | 9/9 | 223 | (8, 0) | 8/9 | 247 | (**0**, 0) |
| `C_mate7_depth11` | yes | 1500000 | **1/9** | 13 | (13, 0) | 3/9 | 48 | (**0**, 0) |
| `D_mate_minus6_depth10` | yes | 4000000 | 4/9 | 15 | (6, 0) | 2/9 | 7 | (4, **1**) |
| `E_mate_minus9` | yes | 1500000 | 7/9 | 111 | (23, 0) | 6/9 | 76 | (13, 0) |
| `F_mate6_inherited_no_line` | no | 300000 | 8/9 | 124 | (4, 0) | 8/9 | 118 | (4, 0) |

The four bold cells in the `clear` columns are S159's four reds, and the table
names which class each belongs to: **B and C reported no mate line at all**
(the vacuity assertion), **A and D reported one short line each** (the
completeness assertion). Nothing else moved.

**`C_mate7_depth11`'s pinned budget is its only non-zero cell at HEAD**, which
is the demonstration `accepts:` asks for: 1500000 reports 13 mate lines and the
other eight budgets report none, so the guard's green is one cell in nine.
Under the clear the same case is non-zero at 1000000, 2000000 and 4000000 --
three different cells, none of them the pinned one. `B_mate6_shallow`'s budget
is 100000, the lowest cell in the grid and the edge at which it switches on:
9/9 at HEAD, and under the clear that single cell is the one of nine that
turned off.

The cells are not a knife edge only for the sparse cases. Per case, the union
over the nine budgets is non-zero on **both** sides for every one of the six --
C is 13 against 48, the smallest pair in the table -- so no case became
genuinely unable to find its mate. What moved is which cell holds it.

Short lines say the same from the other side. Across the stride-1 grid at HEAD
they appear at `A` 300000 and 500000, `B` 1000000, 2000000, 3000000 and
4000000, `D` 1200000, 1500000 and 3000000, `E` 1000000 and 1200000, and under
the clear at `A` 1000000 and 3000000, `B` 1000000 and 1500000, `D` 1500000 and
4000000, `E` 1000000 and 4000000. They are present on both sides at about the
same rate and merely miss the pinned budgets at HEAD. `C` is the exception: 0
short lines in all nine cells on both sides.

Totals at the configured budgets, over the five guarded cases:

| | cases reporting a mate | mate lines | short lines |
|---|---|---|---|
| HEAD | **5 of 5** | 62 | 0 |
| with S159's clear | **3 of 5** | 32 | 2 |

## What the fixture costs today

`build/tests/test_mate_carry` is **26.30 s** wall, measured 2026-09-09 on the
i7-8700K with the machine otherwise loaded, over 188 M nodes of budget: A 53
searches x 1000000, B 22 x 100000, C 35 x 1500000, D 16 x 4000000, E 11 x
1500000. That prices the shapes: running each case over the union of all nine
budgets is 13.5x the node budget per case, **about 4.3 minutes**, which is not
a fast-suite test.

## What the replacement is, and what it was shown to do

DEC-162. Three assertions where there was one failure list and one per-case
floor. The budgets in `adocs/data/S170_cases.tsv` did not move.

1. A mate line at least as long as the distance it claims ends in checkmate at
   exactly that distance. Zero tolerance, at any budget.
2. A shorter line is S202's residue and is counted against a per-case ceiling:
   A 5, B 11, C 0, D 1, E 8, derived by
   `adocs/data/S203_case_sweep.sh --ceilings` over the two recorded grids.
3. A majority of the guarded cases -- 3 of 5 -- reports a mate line at all.

**The class split is the part DEC-156 and DEC-161 did not have.**
`adocs/data/S204_class_census.py` replays the five guarded cases over all nine
budgets and separates the two, on both sides: **428 mate lines, 39 short, 0
class ii at `c982f9d`** and **453, 25, 0 with S159's clear**. Its mate and short
counts reproduce `adocs/data/S204_sweep_head.txt` and
`adocs/data/S204_sweep_killer_iter_clear.txt` in **all 90 cells**, which is what
says the driver agrees with `adocs/data/S203_case_sweep.sh` rather than
measuring something else. So one of the two things the old failure list merged
is budget-independent over 881 lines and the other is not, and only the second
needed a budget chosen for it.

**Green across the tree-moving change it does not care about.** S159's candidate
A -- `killers_clear` in `src/search.cpp`, called first in the depth loop of
`src/chesso.cpp` `iterative_deepening_search` -- rebuilt and run: **1 test case
passed, 61 assertions**, against four failed assertions over four cases on the
old guard. The reconstruction is the same code that produced the recorded grid,
checked rather than assumed: `adocs/data/S203_case_sweep.sh --at` under it
prints A (15, 1), B (0, 0), C (0, 0), D (4, 1), E (13, 0), F (4, 0), every cell
identical to `adocs/data/S204_sweep_killer_iter_clear.txt`.

**Red on a mutant, once per assertion, every failure observed and not assumed.**
`tools/mutation_check.py` is S196 and does not exist yet, so these were applied
by hand to `src/` and reverted; `git diff -- src/` is empty.

| mutant | what it removes | verdict |
|---|---|---|
| the table never answers with a mate score | the inherited score, this file's subject | **green.** 62 mate lines to 37, all five cases still report |
| null move reduces to depth 0 and runs inside the mate window | the three guards CLAUDE.md's recurring bug names | **green.** 121 assertions, more mate lines than HEAD |
| quiescence stops recognising checkmate | S147's premise, that these mates are found there | **green.** A 22, B 8, C 16, D 4, E 25 -- more than HEAD |
| Zobrist redraw, five seeds | the key set the budgets were chosen under | 3 to 5 of 5 cases live. Seed 1 **red on E's ceiling**, 10 short against 8 |
| no checkmate detection anywhere -- main search and quiescence both score a mated node as a draw | the mates themselves | **red on the majority**, `0 >= 3` is false, all five named silent |
| the walk extends to the claimed *length* instead of to the mate: a stall takes the first legal move, and the line is published whether or not it ends in checkmate | DEC-122's all-or-nothing rule, which its `Rejected:` names in exactly these words | **red on the mate-reaching assertion**, at a cell where the walk stalls -- A at 300000, 2 of 2 lines, `the position after 17 reported plies has legal replies`. At the pinned budgets it is a no-op, because HEAD stalls nowhere there |
| the walk publishes nothing | S147 and S170 entirely | **red on four ceilings**: A 12 > 5, C 11 > 0, D 4 > 1, E 22 > 8 |

The first three greens are the guard working rather than the mutants being
weak, and the counts say which: each one moves the tree and **mates are still
being found**, two of them in larger numbers than HEAD. A guard that went red on
those is the guard this step removed.

The sixth needed a cell where the walk stalls, and that is worth recording
rather than hiding: at the pinned budgets HEAD publishes no short line at all,
so a mutant that only changes what happens on a stall cannot be seen there. Run
at A's 300000 -- 2 short lines at HEAD -- it turns both into full-length lines
that do not mate. The same modified budget on unmutated HEAD is **green, 81
assertions**, with A's 2 short lines inside its ceiling of 5, so the red is the
mutant and not the budget. On the old guard that same cell was red.

**Where the majority sits, measured.** Cases reporting a mate line, over every
variant built here: HEAD 5, S159's clear 3, Zobrist seeds 20260905 5, 1 4, 7 4,
42 3, 20260401 4. The majority of 3 is both the stated rule and the tightest
threshold that survives all of them -- 4 would have been red on S159's clear,
which is the false red this step exists to remove. There is no headroom above
it and the file says so rather than implying it: a change that silences a third
case is red, and that is the intended line.

## Order of work

1. Reproduce both sweeps from the recorded files; do not re-run to start.
2. Count the cells per case per side and write the table into this file as the
   demonstration `accepts:` asks for.
3. Decide the guard's replacement shape, and record it as a decision before
   implementing it. The shapes worth pricing, none of them chosen here:
   a floor over the union of several budgets rather than one; a case whose
   budget sits in a *contiguous* green window with the window recorded and
   asserted; asserting only the completeness of whatever mate lines appear and
   dropping the count floor to a separate, differently-sourced guard.
4. Show the replacement green across a tree-moving change and red on a mutant
   that stops a mate being found. The mutant is the load-bearing half.
5. Re-derive budgets and floors with `adocs/data/S203_case_sweep.sh` only after
   the shape is settled, so nothing is fitted to a run.

## Sources read

- `adocs/data/S204_sweep_head.txt`, `adocs/data/S204_sweep_killer_iter_clear.txt`
  -- the two grids, 2026-09-09.
- `tests/test_mate_carry.cpp` and its comment block -- the per-case floor read
  there is gone, replaced by `short_line_ceiling`, so the symbol this line used
  to cite no longer exists to be cited; `adocs/data/S203_case_sweep.sh`;
  `adocs/data/S170_cases.tsv`.
- `adocs/decisions.md` DEC-122, DEC-142, DEC-154, DEC-156; DEC-160 and DEC-161.
- `adocs/plan_done/S148_reverse_futility_ceiling.md`, whose stamp records the
  same test going red on its candidate and being established as budget
  calibration -- the precedent this step generalises.
- `adocs/plan_todo/S202_shallow_inherited_mate_score.md` -- the class this step excludes.
