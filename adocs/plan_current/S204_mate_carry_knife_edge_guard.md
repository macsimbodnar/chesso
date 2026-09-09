id:         S204
goal:       `test_mate_carry`'s cases stop being pinned to isolated spikes in the node-budget grid, so the guard fires on a mate the engine has actually stopped finding and not on any change that moves the tree
accepts:    the guard's failure mode is demonstrated before it is changed, from the two sweeps this step is created with -- `adocs/data/S204_sweep_head.txt` and `adocs/data/S204_sweep_killer_iter_clear.txt`, the same grid over the same six cases at `c982f9d` and at `c982f9d` plus S159's per-iteration killer clear -- and the demonstration is a count, not an argument: how many of the nine stride-1 budgets report any mate line, per case, per side; a case whose pinned budget is its only non-zero cell in the grid is named as such; the replacement guard is shown to stay green across a tree-moving change it does not care about **and** red on a change that genuinely stops a mate being found, the second established with a mutant rather than assumed -- the recurring failure this fixture exists for is "pruning that hides a mate", and a guard that cannot distinguish the two is not guarding it; `expected_mate_lines`'s floors and `adocs/data/S170_cases.tsv`'s budgets are re-derived by `adocs/data/S203_case_sweep.sh` and never re-read from a failing run (DEC-142); DEC-156's re-sweep prescription is either upheld with its tension resolved or amended by a new decision, in writing
touches:    tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv, adocs/data/S203_case_sweep.sh, adocs/decisions.md, adocs/specs.md
excludes:   the `Incomplete mating PV` class itself, which is S202's and stays S202's -- this step is about whether the fixture can measure anything, not about the engine publishing a mate score it cannot back; the Zobrist keys and the case set's provenance, which are DEC-154's and S203's and are not reopened; any change to `src/`, which this step does not need and must not make
decisions:  DEC-122, DEC-142, DEC-154, DEC-156, DEC-161
closes:
blocks:
paused_by:
author:
done:

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
- `tests/test_mate_carry.cpp` `expected_mate_lines` and its comment block;
  `adocs/data/S203_case_sweep.sh`; `adocs/data/S170_cases.tsv`.
- `adocs/decisions.md` DEC-122, DEC-142, DEC-154, DEC-156; DEC-160 and DEC-161.
- `adocs/plan_done/S148_reverse_futility_ceiling.md`, whose stamp records the
  same test going red on its candidate and being established as budget
  calibration -- the precedent this step generalises.
- `adocs/plan_todo/S202_shallow_inherited_mate_score.md` -- the class this step excludes.
