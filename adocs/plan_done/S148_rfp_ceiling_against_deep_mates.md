id:         S148
goal:       the reverse futility depth ceiling is re-decided against the deep mates S145 measured it losing, by SPRT and not by argument
accepts:    `RFP_MAX_DEPTH`'s default is decided by an SPRT of the shipping 15 against at least one lower value, with the mate-finding cost of each stated from `adocs/data/S145_rfp_sweep.py` in the same step -- a verdict of "keep 15" is a valid outcome and is recorded as one; `RfpMinPly` held at its shipping value throughout, because S145 measured the two bounds substituting for each other and a run that moves both attributes nothing; the mate in four and five counts in `tests/test_engine.cpp`'s `engine: mate safety` suite promoted from the recorded `MESSAGE` to an asserted floor if and only if the shipped value makes them non-zero, and left recorded if it does not; `adocs/data/S145_rfp_sweep.log` re-run at the shipped value; specs.md's reverse futility sentence and MANUAL.md's known-bug entry carry the decided number
touches:    src/search_params.hpp, tests/test_engine.cpp, adocs/data/, adocs/specs.md, MANUAL.md, adocs/decisions.md
excludes:   `RFP_MIN_PLY`, which S142 settles and which this step holds fixed; the constructed set itself, which S145 built and which this step only measures against; making the depth ceiling a function of the score the way Stockfish's `fa8b6add` does, which is a different feature and would need its own step and its own verdict; the principal-variation truncation, which is S147
decisions:  DEC-019, DEC-063, DEC-095
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-08
done:       2026-09-09. **The reverse futility depth ceiling was re-decided by SPRT and stays at 15, and the deep mates it loses are its measured price.** The challenger came from a rule fixed before the grid was run (DEC-105 form (b)): the largest ceiling at which both the mate in four and the mate in five exact counts over the 82-row set are non-zero. `adocs/data/S148_rfp_ceiling_sweep.py --mined` swept **every value from 0 to 15** at `192a5a3` against the 82 constructed rows at depth `2m - 1 + 8` and the 318 mined rows at depth 10, `RfpMinPly` **held at 3 throughout and read from the binary rather than written down** -- a grid no earlier reading had, S145 and S154 both having run six values over the 48 rows the set held before S168. Per value, exact of 82 / m2 / m3 / m4 / m5 / mined exact: **0** 70 / 26 of 26 / 20 of 24 / 13 of 16 / 11 of 16 / 177; **1** 70 / 26 / 21 / 14 / 9 / 175; **2** 62 / 26 / 20 / 10 / 6 / 175; **3** 60 / 26 / 20 / 10 / 4 / 166; **4** 52 / 26 / 18 / 7 / 1 / 159; **5** 47 / 26 / 17 / 4 / 0 / 149; **6** 46 / 26 / 15 / 5 / 0 / 149; **7** 44 / 26 / 15 / 3 / 0 / 145; **8 and 9** 41 / 26 / 13 / 2 / 0 / 145; **10 through 15** 39 / 26 / 12 / 1 / 0 / 145. `short` **0** and `sign` **0** at all sixteen settings, which is the defect column and is what makes the rest a strength reading. So **C1 = 4**: the mate in five class is the binding one and it is a cliff, 11 / 9 / 6 / 4 / 1 and then nothing, while the mate in four class survives to the shipping value at 1 of 16. **The verdict: H0 accepted.** `adocs/data/S148_sprt.sh`, `--nonreg` `{-5, 0}` nElo, alpha = beta = 0.05, 8+0.08, Hash 16, UHO, `-repeat`, `-check-mate-pvs`, candidate the working tree with the one integer at 4 against `a6b74be`: **LLR -2.95 (-100.2%), Elo -5.66 +/- 4.33, nElo -7.31 +/- 5.60, LOS 0.52 %, PairsRatio 0.95, 14808 games, Ptnml [770, 1594, 2798, 1591, 651], 6 h 19 m 35 s, 0 time forfeits of 14809** (2342 games an hour against S198's measured 2277); `adocs/data/S148_sprt.log`, the 73 MB PGN not committed. The three readings were written before the first game and H0's was applied unchanged: 4 costs five nElo or more, so it is rejected, 15 is kept, and the deep-mate loss is recorded as the measured price of S085's ceiling. **No C2 = 6 fallback was bought** -- the owner decided C1 alone on 2026-09-08, with all five deferred questions answered as the file recommended -- so the rejection ends the step. **The candidate was not behaviour-neutral and INV-6 could not discharge it, which is why the games decided it:** `search_bench.py` depth 9 121530 / 801481 / 72924 -> **124511 / 805638 / 111393**, depth 12 636677 / 3520847 / 494098 -> **716171 / 3707680 / 560079**, `bench` 26851183 -> **28339749**, best moves `c3d5` / `e2a6` / `d7c8q` unchanged throughout. Debug self-play at 4+0.04, 4 rounds: **0 `Assertion` over 8 Debug games**, 0 disconnect, crash or illegal (DEC-141). **No golden moved and none was re-derived, because the tree went back:** after the revert `src/` differs from the parent only in one comment, both builds reproduce **121530 / 801481 / 72924** and `bench` **26851183**, and `MATE_IN_THREE_FLOOR` (11), `EXACT_FLOOR` (143), `expected_mate_lines` and the `S170_cases.tsv` budgets are untouched. **Neither deep class was promoted to an asserted floor**, and the accepts' condition is subtler than it reads: mate in four ships at **1 of 16**, which is non-zero, but a floor of 1 has no margin between its ends and that is what DEC-116 rejected for the mate in three -- the owner decided on 2026-09-08 that a class at exactly 1 stays recorded. Both stay in the `MESSAGE`, whose text and the comment above `MATE_IN_THREE_FLOOR` now say which, why, and what the refused ceiling would have made them. **One finding, raised by the fast suite and closed before the machine was committed.** The candidate turned `test_mate_carry` red -- `A_mate8_shallow` publishing 2 of 9 mate lines whose PV does not reach the mate, `B_mate6_shallow` vacuous at 0 lines against a floor of 7. `adocs/data/S203_case_sweep.sh` over the candidate says what that is: the `short` column moves with the **node budget** and not with the ceiling, A short 2 at 1000000 nodes and **0** at 1200000, 1500000, 2000000, 3000000 and 4000000, and B short 4 at stride 2 / 2000000 alone. That is DEC-122's eviction class, open under S202, and S203's own warning that the grid is a knife edge and the budgets are re-swept after anything that moves the tree -- so the engine was not newly broken, the BUGS rule was not triggered, and the run was launched. **The run then read the same class from both sides: 7 `Incomplete mating PV` lines from the candidate against 13 from the reference.** The candidate is *below* the reference, so there was no BUGS-rule finding to read before the verdict, and the direction is the sweep's -- the lower ceiling loses fewer mate lines than the shipping one. S202 inherits that as a reading about its own class. **S151:** this is a pruning-parameter verdict and falls inside its re-take at a control at least four times 8+0.08, though the magnitude decides nothing here, the incumbent having been kept; its list gains this run beside S085's vector. Gate: 28/28 in both builds, format clean under `CLANG_FORMAT_MAJOR=22` (DEC-146), `bench` **26851183** equal to the parent's, so the commit signs `No functional change` (DEC-140). The fast suite's own line reads `mate in 4: 1 of 16 exact, mate in 5: 0 of 16`. Docs: `adocs/specs.md`'s reverse futility sentence, `MANUAL.md`'s `RfpMaxDepth` option row **and** its known-bug entry, `DEV_MANUAL.md`'s mate safety table **re-taken over the 82 rows** (it still read 48 positions and S154's counts), the `RFP_MARGIN` comment block in `src/search_params.hpp` and the comment above `MATE_IN_THREE_FLOOR` -- the five places 15 is written, each now carrying the price beside it. `test_uci_surface` builds its option line from the table and no default moved, so no golden refresh was owed. `README.md` untouched, human-owned. **`--citations` was checked and put back where it was found:** the comment in `src/search_params.hpp` grew by eight lines, which shifted every line-number citation below it and took the checker from 52 flags to 59. The seven are restored mechanically -- S098:120, S109:193, S114:87 and :117, S120:89 and :192, S132:87, each +8 -- and the check reads **52 over 66 files** again, the same 52 it read at the parent over 67. It is red on both sides and has been since before this step: that is F07, the class **S187** converts to symbol citations next, and one of those seven (S109's, which names `LMR_DIVISOR` and points at the reverse futility comment) is a worked example of why line numbers are the wrong unit. `--prose` and `--params` green, Debug fast suite 28/28, `ctest -L slow` 1/1. DEC-158 records the trade.

## What S145 measured, and why this is a trade rather than a fix

The full table is `adocs/data/S145_rfp_sweep.log`. `RfpMinPly` held at its
shipping 3, over the 48 proved mates of `adocs/data/S145_mate_set.tsv`, each
searched under the engine's own iterative deepening to `2m - 1 + 8`:

| `RfpMaxDepth` | exact | mate 2 | mate 3 | mate 4 | mate 5 |
|---|---|---|---|---|---|
| 0 | 34/48 | 16/16 | 11/16 | **4/8** | **3/8** |
| 3 | 32/48 | 16/16 | 11/16 | 3/8 | 2/8 |
| 6 | 27/48 | 16/16 | 10/16 | 1/8 | 0/8 |
| 10 | 24/48 | 16/16 | 8/16 | 0/8 | 0/8 |
| **15, shipping** | 24/48 | 16/16 | 8/16 | **0/8** | **0/8** |
| 63 | 24/48 | 16/16 | 8/16 | 0/8 | 0/8 |

Monotone, and the deep classes are the whole difference. The mate in two class
is 16 of 16 at every value, which is why nothing in the suite before S145 could
see this: all three of the old gate's cases were mates in two. S033 shipped 6 --
one mate in four and no mate in five -- and S085's SPSA run moved it to 15,
where both classes are empty. 10 is already indistinguishable from 63.

**And that move was not free in the other direction.** S085's returned vector
was SPRT-verified at **+21.02 Elo** with `RfpMaxDepth` 15 in it, so lowering the
bound gives back a share of a measured gain to buy a mate-finding property whose
Elo value is unmeasured. That is exactly the trade DEC-019 says a published
figure decides what to try and never what to conclude, so it is an SPRT.

**The published record says the trade is real and points the other way from
S085.** Stockfish removed its reverse-futility and parent-futility depth limits
in July 2021 (`09b6d283`, `dbd7f602`); both passed non-regression SPRT at STC
and LTC, and mates found on ChestUCI at 1M nodes fell from **2427 to 1246**.
Restoring either condition alone recovered only 1282 and 1630, so both were
needed, and the change was reverted at `dabaf222`. The source has carried "The
depth condition is important for mate finding" ever since. Master now makes the
bound a function of how close the score is to decisive -- a table that shrinks
the ceiling as scores approach the decisive band, `fa8b6add` -- so the pruning
depth shrinks where mates live. That last shape is excluded here: it is a feature, not a
constant, and it deserves its own verdict.

## What is not claimed

That the engine is losing rating to this. Nothing here measures that, and the
possibility that a mate found four iterations later costs nothing at 8+0.08 is
exactly why the step exists. What is established is that the shipping value has
a cost that was invisible before S145 built a set with deep mates in it, and
that S142's "no defect was demonstrated against 15" is no longer true as a
statement about mate finding.

## The HEAD sweep, 2026-09-08, and the challenger it names

`adocs/data/S148_rfp_ceiling_sweep.py --mined` at `192a5a3`, the tune build,
`RfpMinPly` read from the binary and held at 3, the 82 constructed rows at
depth `2m - 1 + 8` and the 318 mined rows at depth 10. The full transcript is
`adocs/data/S148_rfp_ceiling_sweep.log`; every value from 0 to 15, which is
the grid the challenger rule needs and which no earlier sweep had:

| `RfpMaxDepth` | exact / 82 | mate 2 | mate 3 | mate 4 | mate 5 | mined exact |
|---|---|---|---|---|---|---|
| 0 | 70 | 26/26 | 20/24 | 13/16 | **11/16** | 177 |
| 1 | 70 | 26/26 | 21/24 | 14/16 | **9/16** | 175 |
| 2 | 62 | 26/26 | 20/24 | 10/16 | **6/16** | 175 |
| 3 | 60 | 26/26 | 20/24 | 10/16 | **4/16** | 166 |
| **4, C1** | **52** | 26/26 | 18/24 | **7/16** | **1/16** | 159 |
| 5 | 47 | 26/26 | 17/24 | 4/16 | 0/16 | 149 |
| 6 | 46 | 26/26 | 15/24 | 5/16 | 0/16 | 149 |
| 7 | 44 | 26/26 | 15/24 | 3/16 | 0/16 | 145 |
| 8 | 41 | 26/26 | 13/24 | 2/16 | 0/16 | 145 |
| 9 | 41 | 26/26 | 13/24 | 2/16 | 0/16 | 145 |
| 10 | 39 | 26/26 | 12/24 | 1/16 | 0/16 | 145 |
| 11 to 14 | 39 | 26/26 | 12/24 | 1/16 | 0/16 | 145 |
| **15, shipping** | **39** | 26/26 | 12/24 | **1/16** | **0/16** | 145 |

`short` 0 and `sign` 0 at all sixteen settings -- the two defect columns -- so
the rest is a strength reading and not a correctness one.

**C1 = 4**, by the rule this file pre-registered: the largest ceiling at which
both deep classes are non-zero. The mate in five class is the binding one and
it is a cliff, 11 / 9 / 6 / 4 / 1 at ceilings 0 to 4 and 0 from 5 up; the mate
in four class survives to 15 at 1 of 16, which is where S145's "0 of 8" now
reads on the larger set. S180's estimate of "between 3 and 5 on the finer
grid" was right.

Two readings the coarse grids could not give. The plateau starts at **10**,
not at 15 -- every count from 10 up is identical, so the shipping value
confines nothing that three lower values do not also fail to confine. And the
step's own header table is superseded on every row: it is the `14748c9`
reading over 48 positions, the set is 82 now, and the counts here are the ones
this step decides on.

What it costs is not in this table. A ceiling of 4 switches reverse futility
off for every node with more than 4 plies left, so the engine searches more
nodes per iteration and reaches fewer plies at 8+0.08. That is what the SPRT
prices, and nothing here anticipates it.

## The run, launched 2026-09-08 19:47

Candidate: the working tree with `RFP_MAX_DEPTH`'s default at 4 and the
`golden_defaults` transcription in `tests/test_search_params.cpp` moved with
it. Nothing else in `src/`. Left uncommitted so `./fastchess.sh --nonreg`
measures the tree against `a6b74be`, which is `192a5a3` plus this step's own
document commit and carries no `src/` change; the banner printed both shas
with their dates before the first game. The candidate binary is snapshotted to
a temp file before a game is played (DEC-020), so the working tree is free to
move while the run plays -- but `src/` does not, until the verdict lands
(MEASUREMENT, DEC-144).

Pre-registration, written before the first game: `adocs/data/S148_sprt.sh`.
`{-5, 0}` nElo, `--nonreg`, alpha = beta = 0.05, 8+0.08, Hash 16, UHO,
`-repeat`, `-check-mate-pvs`. 18.4 h at the midpoint and 11.2 h on a bound, at
the 2277 games an hour S198 measured here on 2026-09-08; the 20000-round cap
is 40000 games and about 17.6 h, below the midpoint case, so reaching it is
"no verdict" by construction. Watcher: the WATCHERS poll loop, all four exits,
36 h ceiling.

**The candidate is not behaviour-neutral, and INV-6 is recorded rather than
claimed** (DEC-140, and DEV_MANUAL "Measure" -- no saving is read off three
positions):

| | HEAD `a6b74be` | candidate |
|---|---|---|
| `search_bench.py` depth 9 | 121530 / 801481 / 72924 | **124511 / 805638 / 111393** |
| `search_bench.py` depth 12 | 636677 / 3520847 / 494098 | **716171 / 3707680 / 560079** |
| `chesso bench` | 26851183 | **28339749** |
| best moves | `c3d5` / `e2a6` / `d7c8q` | unchanged, both depths |

Debug self-play, DEC-141 clause 1: `fastchess` 4 rounds at 4+0.04 between two
`build-debug` binaries, **0 `Assertion` over 8 games** in both the trace log
and the console, 0 disconnect, crash or illegal. The gate was green at HEAD in
both builds, 28/28, before the integer moved.

### The finding the fast suite raised, and why it did not stop the run

The candidate turns two fast tests red, and only one of them is a lag.

`test_plan_params` is red because the four documents and the 68 pending step
files still write 15, which is exactly what `--params` is for: the documents
carry the decided number, and the decision is what this run takes. It goes
green when the verdict is applied, in either direction.

`test_mate_carry` is the one that had to be established before 18 hours of
machine were committed. It reads **`A_mate8_shallow`: 2 of 9 mate lines do not
reach their mate** -- `mate -8` published at depths 4 and 5 with a PV of 4 and
5 plies where 16 are needed -- and **`B_mate6_shallow` reported 0 mate lines**
against a floor of 7, the vacuity guard. `adocs/data/S203_case_sweep.sh` over
the candidate says what that is, and the answer is the node budget rather than
the ceiling:

| case | stride | 1000000 | 1200000 | 1500000 | 2000000 | 3000000 | 4000000 |
|---|---|---|---|---|---|---|---|
| A, mates / short | 1 | 9 / **2** | 6 / 0 | 16 / 0 | 17 / 0 | 10 / 0 | 10 / 0 |
| A, mates / short | 2 | 2 / 0 | 4 / 0 | 4 / **4** | 4 / **4** | 4 / 0 | 4 / 0 |
| B, mates / short | 1 | 16 / 0 | 30 / 0 | 15 / 0 | 29 / 0 | 27 / 0 | 54 / 0 |
| B, mates / short | 2 | 9 / 0 | 15 / 0 | 17 / 0 | 14 / **4** | 17 / 0 | 27 / 0 |

Short lines appear at one budget and vanish at the next, in both directions,
which is DEC-122's eviction class and not a property of the ceiling. It is
open under S202, and S203 said this would happen in so many words: the grid is
a knife edge, and the budgets are re-swept after anything that moves the tree,
not only after a key change. So the engine is not newly broken, the BUGS rule
is not triggered, and the run was launched. The budgets and floors are
re-derived by `adocs/data/S203_case_sweep.sh` under its own stated rule -- the
cheapest budget at which the case reports at least its floor of mate lines
with all of them complete -- **after** the verdict, because a rejection puts
the tree back and retires the table above with it.

The same table is the reason the run's `Incomplete mating PV` count is read
before the verdict and not after: a candidate excess over the reference is
recorded as a finding against S202, and this evidence is what it is read
against rather than being attributed to the ceiling by default.

## The verdict, 2026-09-09: H0 accepted, 15 stays

```
SPRT ([-5.00, 0.00]) completed - H0 was accepted
Elo: -5.66 +/- 4.33, nElo: -7.31 +/- 5.60
LOS: 0.52 %, DrawRatio: 37.79 %, PairsRatio: 0.95
Games: 14808, Wins: 4892, Losses: 5133, Draws: 4783, Points: 7283.5 (49.19 %)
Ptnml(0-2): [770, 1594, 2798, 1591, 651], WL/DD Ratio: 2.50
LLR: -2.95 (-100.2%) (-2.94, 2.94) [-5.00, 0.00]
Total Time: 06:19:35        forfeits 0 of 14809, 0.00 %
```

`adocs/data/S148_sprt.log` is the run, 14809 games written to
`/tmp/chesso_sprt_nonreg_20260908_193719/games.pgn` (73 MB, not committed;
the log carries every game's result and every periodic SPRT block). 2342 games
an hour against S198's measured 2277.

**Read as the script pre-registered it, and the reading was written before the
first game.** H0 accepted means C1 = 4 costs 5 nElo or more: reject it, keep
15, and record the deep-mate loss as the measured price of S085's ceiling. No
C2 = 6 fallback was bought -- the owner decided C1 alone on 2026-09-08 -- so
the rejection ends the step. The integer is reverted; `git diff` against
`a6b74be` over `src/` is empty but for one comment.

The bound cleared decisively rather than by a hair: LLR -2.95 against -2.94 at
14808 games, LOS 0.52 %, and the point estimate -7.31 nElo sits below the -5
bound rather than between the bounds. The run never approached its 40000-game
cap.

**Both abort checks were clean, and one of them is a result in its own right.**
0 time forfeits on either side. `Incomplete mating PV`: **7 lines from the
candidate against 13 from the reference**. The candidate is *below* the
reference, so there is no BUGS-rule finding to read before the verdict -- and
the direction is the sweep's direction, the lower ceiling losing fewer mate
lines than the shipping one. S202 inherits that as a reading about its own
class: this ceiling is not what produces it.

### What this costs, stated plainly

The engine does not find the deep mates in the constructed set, and it will not
until something other than this bound finds them. At the shipping 15 the set
reads 39 of 82 exact, 26 of 26 mates in two, 12 of 24 in three, **1 of 16 in
four and 0 of 16 in five**. At 4 those become 52, 26, 18, **7 and 1** -- and 4
is 7.31 nElo worse over 14808 games. That is the whole trade, measured on both
sides, and the answer is that the mates are not worth the rating at 8+0.08.

S142's "no defect was demonstrated against 15" was retired by S145 as a
statement about mate finding; what replaces it is not a defect either. It is a
price, and it is now written in five places rather than inferred.

### The accepts, clause by clause

- **`RFP_MAX_DEPTH`'s default decided by SPRT of 15 against at least one lower
  value, with each value's mate-finding cost from the sweep in the same step.**
  Done: 4 against 15, the full 0-to-15 grid in
  `adocs/data/S148_rfp_ceiling_sweep.log` and tabled above. "Keep 15" is the
  outcome and it is recorded as one, not as a non-event.
- **`RfpMinPly` held at its shipping value throughout.** 3, read from the
  binary by the sweep script rather than written down, and never set by the
  SPRT candidate.
- **The mate in four and five counts promoted to an asserted floor if and only
  if the shipped value makes them non-zero.** Not promoted, and the condition
  is subtler than it reads: at 15 the mate in four count is 1 of 16, which is
  non-zero, but a floor of 1 has no margin between its ends and that is what
  DEC-116 rejected for the mate in three. The owner decided on 2026-09-08 that
  a class at exactly 1 stays recorded. Both classes stay in the `MESSAGE` and
  the comment above `MATE_IN_THREE_FLOOR` now says which, why, and what the
  refused ceiling would have made them.
- **`adocs/data/S145_rfp_sweep.log` re-run at the shipped value.** Read as
  question 3 proposed and the owner confirmed: S145's log is evidence and is
  never rewritten, so the re-run is `adocs/data/S148_rfp_ceiling_sweep.log`,
  which contains the shipped value's row and fifteen others.
- **`specs.md`'s reverse futility sentence and `MANUAL.md`'s known-bug entry
  carry the decided number.** Both, plus `DEV_MANUAL.md`'s mate safety table
  re-taken over the 82 rows (it still read 48 positions and S154's counts),
  the `RFP_MARGIN` comment block in `src/search_params.hpp`, and
  `MANUAL.md`'s `RfpMaxDepth` option row.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

Reverse futility pruning (RFP) sits at the top of `src/search.cpp` `negamax`:
at a quiet, non-PV node with `depth <= RFP_MAX_DEPTH` plies left, if the static
evaluation minus `RFP_MARGIN * depth` is at or above beta, the node returns
that bound without searching a move. A static evaluation is never a mate score,
so a node whose true value is "mated by force" can fail high on material and
take the mating line with it. S033 shipped the rule with a ceiling of 6 plies;
S085's SPSA run moved the ceiling to 15 and verified the whole tuned vector at
+21.02 Elo. At 15 the ceiling confines nothing this engine reaches (median
depth 11 at the tuning control), and S145 then built a set of proved mates and
measured that the mates in four and five are found only when the ceiling is
low. This step adds no rule and touches no code path: it moves one integer
default in `src/search_params.hpp` -- or leaves it -- on the word of one SPRT
per challenger value against HEAD, after a fresh sweep at HEAD says what each
value buys in mates. "Keep 15" is a real, recorded outcome.

### 2. The technique as published

The Chess Programming Wiki (https://www.chessprogramming.org/Reverse_Futility_Pruning)
defines RFP as the "reversed" or negamaxed form of extended futility pruning:
the whole node fails high when a reliable static score "minus safety margin
greater or equal than beta"; it "relies on the null move observation" and is
"a special case of null move pruning without explicitly making one". Its
pseudo-code is chesso's shape -- `if (eval - margin >= beta) return eval`,
fail soft, skipped in check and at PV nodes -- and "The base RFP margin is
usually a constant multiple of depth". No Elo figure is given
(`adocs/data/2026-09-04_plan_review_literature_check.md`, row "Reverse
futility pruning (S148)").

The depth ceiling is the technique's inheritance, not a refinement. Futility
pruning was "Historically at frontier nodes (depth == 1)"; Ernst Heinz's
extended form reached pre-frontier nodes "only with the greater margin"; and
"Modern engines also perform futility pruning at non-leaf nodes, and scales
margin by depth" (https://www.chessprogramming.org/Futility_Pruning). Once the
margin grows with depth the cap is redundant for the Elo question -- Stockfish
removed its child-node cap as "double capped by depth and by futility margin,
which is also a function of depth" (`09b6d283`, 2021-07-06, "passed STC and
LTC tests") and its parent-node cap at `<-2.50,0.50>` (`dbd7f602`,
2021-07-10) -- and not redundant for mate finding: the revert (`dabaf222`,
2021-08-05) reports "sf14 2427, master 1246, patched 2467" mates over
ChestUCI_23102018.epd at 1M nodes, and vondele's PR #3641 comment adds "both
conditions are needed... patch 2467, patchPart1 1282, patchPart2 1630". That
is the controlled experiment this step re-runs at chesso's scale: an
Elo-neutral cap can still be what finds mates. Stockfish later made the cap a
function of the evaluation and beta (`fa8b6add`, PR #7040, non-regression at
STC and LTC); that is a feature with its own verdict, excluded here, and its
constants are that engine's and are not quoted (DEC-134).

Chesso's form: a fixed integer ceiling on *remaining* depth plus a ply floor
(`RFP_MIN_PLY`) that no surveyed engine has (S145). This step re-decides the
integer only, with the floor held.

### 3. What chesso has today, and where the change plugs in

`src/search_params.hpp` `CHESSO_SEARCH_PARAMS(X)` declares the three rows
`X(RFP_MARGIN, "RfpMargin", 63, 0, 2000)`, `X(RFP_MAX_DEPTH, "RfpMaxDepth", 15,
0, 63)` and `X(RFP_MIN_PLY, "RfpMinPly", 3, 2, 63)`. The release build folds
each as `inline constexpr int`; the tune build makes each a UCI option; both
are described by `search_param_info()`, which `tests/test_search_params.cpp`
`golden_defaults` holds a transcription of and `tests/test_uci_surface.cpp`
builds its expected `option name ... type spin default` lines from.

`src/search.cpp` `negamax`: the static evaluation is computed once at the top
of the node since S108 -- `TT_EVAL_NONE` in check, the table's stored `eval`
where the probe found one, `evaluate()` otherwise -- and written to
`state->static_evals[ply]`. The RFP guard reads it: `!is_pv && !is_in_check &&
static_cast<int>(ply) >= RFP_MIN_PLY && depth <= RFP_MAX_DEPTH && beta <
MATE_MIN && beta > -MATE_MIN`, margin `RFP_MARGIN * depth`, returning
`static_eval - margin` (fail soft). `MATE_MIN` is a `#define` in
`src/search.cpp`. `improving_at` (`src/search.hpp`) exists with no in-search
caller and RFP does not read it; do not wire it in here, that is S109's.

RFP fires only at nodes with at most `RFP_MAX_DEPTH` plies left, the nodes
nearest the leaves; a lower ceiling switches it off for the upper tree and
keeps the near-leaf pruning, so it searches more nodes per iteration and
reaches fewer plies at 8+0.08. That trade is what the SPRT prices.

The measurement chain: `adocs/data/S145_rfp_sweep.py` drives the tune build
over UCI with python-chess (`~/.venv/chess/bin/python`; check the path exists
on the workstation), one fresh engine process per setting, and reports per
mate distance `exact`, `delay`, `short` and `sign`; its `sweep()` takes the
axis, the value list, the held options and the rows, and `CEILING_VALUES` is
`[0, 3, 6, 10, 15, 63]`. `tests/test_engine.cpp` `engine: mate safety` asserts
the constructed set (`adocs/data/S145_mate_set.tsv`, 82 rows since S168) under
iterative deepening in "a proved mate is never mis-scored, and every mate in
two is found on time": every mate in two on time, `exact_by_distance[3] >=
MATE_IN_THREE_FLOOR` (11), the mate in four and five counts in a `MESSAGE`.

Order of edits:

1. Sweep at HEAD, no source edit: a wrapper `adocs/data/S148_rfp_ceiling_sweep.py`
   that puts its own directory on `sys.path`, imports `S145_rfp_sweep` and
   calls its `sweep()` for `RfpMaxDepth` over every value 0 to 15, `RfpMinPly`
   held at the binary's default, over the 82 rows and the mined set
   (`--mined`). The data directory is append-only in practice
   (`adocs/data/README.md`): import the S145 script, do not edit it, and write
   `adocs/data/S148_rfp_ceiling_sweep.log`, not over `S145_rfp_sweep.log`
   (question 3).
2. Choose the challenger from that table by the rule in section 4 and write
   `adocs/data/S148_sprt.sh` with its pre-registration (section 7).
3. Change the one integer in `src/search_params.hpp`, rebuild `build` and
   `build-tune`, run the fast suite in both (a lower ceiling can only gain
   mates; a red anywhere else is a finding), then launch the SPRT detached.
   Leave the change uncommitted so `./fastchess.sh --nonreg` measures the
   working tree against HEAD, or commit and pass `REF=HEAD~1` as S085 did;
   the banner prints both shas either way.
4. Read the verdict; keep or revert the integer.
5. If kept: re-derive every golden the tree change moves (section 6), promote
   the mate in four and five counts per the accepts, update the `RFP_MARGIN`
   and `RFP_MIN_PLY` comments in `src/search_params.hpp` ("6 -> 15", "held at
   15"), the `golden_defaults` row, the numbers in the `MATE_IN_THREE_FLOOR`
   comment, and the three documents (section 8).
6. If not kept: the documents carry 15 with the measured cost beside it, the
   `MESSAGE` stays, and a decision entry records the trade (through the
   coordinator).

### 4. Constants and seeds

Exactly one constant can move, `RFP_MAX_DEPTH`'s default. Its declared range
stays `0` to `63` (DEC-095; a tuner keeps every value). The challenger is DEC-105
form (b), a derivation over chesso's own data run at the step's start: the
HEAD sweep of step 1, and the rule

> C1 = the largest ceiling at which both the mate in four and the mate in five
> exact counts over the 82 rows are non-zero.

That is the elbow of the curve and the smallest change that gives the accepts'
promotion clause content for both classes. S154's re-take over the 48 at
`fc5526e` (`adocs/data/S154_floor_margin_sweep.log`, block `ceiling`) reads
exact 40 / 36 / 30 / 25 / 25 / 25 at ceilings 0 / 3 / 6 / 10 / 15 / 63, mates
in four 6 / 5 / 2 / 0 / 0 / 0 of 8 and mates in five 5 / 1 / 0 / 0 / 0 / 0 of
8, so C1 is expected between 3 and 5 on the finer grid. A pre-registered
fallback C2 = 6 (form b too: S033's shipped value, where the +59.98 verdict
was taken, and the largest value with a non-zero mate in four class in every
sweep so far) runs only if C1 is rejected and the owner buys a second verdict
(question 2). Not 10: identical to 15 on every count. Not 0: that switches the
rule off, a different question from "where is the ceiling".

Bounds `{-5, 0}` nElo, reasoning in section 7; watcher ceiling 36 h, twice the
17.9 h worst case (WATCHERS rule).

### 5. Interactions and traps

- **Floor and ceiling are one guard read two ways** (S145, DEC-095 as
  amended). `RfpMinPly` stays 3 throughout; a run that moved both attributes
  nothing. The sweep script reads the held value from the binary, so a tune
  build with the new ceiling default still holds the floor at 3.
- **The mate-band guard on beta is inert** here: `evaluate_expensive()` clamps
  to `+/-LAZY_EVAL_MARGIN`, so `static_eval - margin >= beta` cannot hold with
  beta in the band. Do not argue mate safety from it (S145, `src/search.cpp`
  `negamax`'s own comment).
- **The bench moves, so INV-6 is the SPRT.** `python3 tools/search_bench.py
  ./build/src/chesso 9` and `12` will differ from HEAD's; take HEAD's first
  (last recorded in `.moltke.local.md` as 121512 / 800769 / 62907 and 639228 /
  3430710 / 367858), quote both sets in the stamp (DEC-140) and read no saving
  off three positions (DEV_MANUAL "Measure").
- **Every mate golden is a floor placed between two measured ends, and a tree
  change moves both ends.** `MATE_IN_THREE_FLOOR` (11, ends 12 shipping and 10
  with the floor guard removed), `tests/test_mate_breadth.cpp` `EXACT_FLOOR`
  (143, ends 145 and 141), and `tests/test_mate_carry.cpp`
  `expected_mate_lines` are re-derived by their scripts whenever either end
  moves, never re-read from a green run (DEC-116, DEC-142). A lower ceiling
  is expected to raise the shipping ends, which is exactly the case where a
  floor stops separating without going red.
- **The number 15 is quoted in five places** (section 8 lists them), and
  `tools/plan_prose_check.py --params`, which compares document numbers
  against `search_param_info()`, is not in the fast suite: run it by hand.
- **`adocs/data/S145_rfp_sweep.log` is evidence**, "left as it was taken" per
  its header; S154 re-took the ceiling into its own log for that reason, and
  this step's re-run lands in `S148_rfp_ceiling_sweep.log` (question 3).
- **S198 first.** The workstation's fixed-rounds A/A is DEC-143's condition
  for the first verdict there, and it re-measures the 2337 games an hour this
  guide converts at; if S198 reads a different throughput, convert at that.
- **S151.** Its rule -- a pruning-parameter verdict is re-taken at a control
  at least four times longer before the magnitude is banked -- applies to this
  verdict whichever lands first: the default moves on the 8+0.08 verdict, as
  S085's did, the number is provisional until the longer control has spoken,
  and if this step lands first its stamp says so and S151 adds the verdict to
  its list beside S085's vector.
- **The harness.** `REF` defaults to HEAD and a clean tree is refused; do not
  edit `fastchess.sh` while a run plays; the candidate is snapshotted
  (DEC-020) but no other `src/` change is made meanwhile (MEASUREMENT,
  DEC-144); `ps aux | sort -rnk3 | head` before launch; the POWER rule's
  `pmset` does not exist on the Linux workstation, which has no battery;
  builds use `-j12` there.
- **DEC-141 applies in one clause and not the other.** No new rule, so no new
  mutant; `M05_rfp_margin_flat`, `M06a` and `M06b` in
  `adocs/data/2026-09-04_test_review/mutants.py` cover the margin and the
  floor, and no ceiling mutant exists until S191 lands, after this step. The
  search's behaviour changes, so the Debug self-play habit applies (section 6).

### 6. Tests

No guard test is owed: nothing is added to the condition. What is asserted
changes in one place, and the change is observed red first.

The accepts' promotion. If the shipped ceiling makes the mate in four (and
five) count non-zero, the `MESSAGE` in "a proved mate is never mis-scored, and
every mate in two is found on time" becomes an assertion, in the
`MATE_IN_THREE_FLOOR` pattern:

    // Golden: re-derived by adocs/data/S148_rfp_ceiling_sweep.py at the
    // shipping ceiling and at 15, the removed end (0 of 16 there since S145).
    // Placed one below the shipping count, DEC-116's margin.
    static constexpr int MATE_IN_FOUR_FLOOR = <shipping count - 1, at least 1>;
    ...
    REQUIRE(exact_by_distance[4] >= MATE_IN_FOUR_FLOOR);
    // mate in five: the same shape iff its shipping count is non-zero,
    // otherwise it stays in the MESSAGE and the comment says which and why.

Precondition, asserted before the search as the suite already does per row:
the defender node is not in check and its static score clears the margin (the
`mate_case_t` rows carry the defender nodes and the lead). Observed red:
rebuild with `RFP_MAX_DEPTH` at 15 in a throwaway worktree, the mechanism of
`adocs/data/S154_floor_margin_sweep.py`'s `red` mode, and watch `REQUIRE( 0
>= N )`. A shipping count of exactly 1 gives a floor with no margin
(question 5).

Goldens touched and the script for each (DEC-142): `MATE_IN_THREE_FLOOR` --
`adocs/data/S154_floor_margin_sweep.py floor` and `red` over the 82 rows, the
procedure `adocs/data/S168_floor_sweep.log` records; `EXACT_FLOOR` --
`adocs/data/S156_mined_floor_sweep.py`; `expected_mate_lines` --
`adocs/data/S170_replay.py` over `adocs/data/S170_cases.tsv`; the
`golden_defaults` row for `RfpMaxDepth` in `tests/test_search_params.cpp`, a
transcription of the table with no script (S192 owns that class). Each
re-derivation is placed strictly between its two ends with the margin stated
in the comment.

Mate-safety instruments (DEV_MANUAL "Mate safety"): `test_engine`'s
mate-safety suite, `test_mate_breadth`, `test_mate_pv` ("the constructed set
reports no mate it cannot show") and `test_mate_carry` run in the gate; the
SPRT carries `-check-mate-pvs`, so count `Incomplete mating PV` lines per side
in the run's log as S171 does -- a candidate count above the reference's is a
BUGS-rule finding before any verdict is read.

INV-6 command, both binaries, for the record and not for neutrality:

    python3 tools/search_bench.py ./build/src/chesso 9
    python3 tools/search_bench.py ./build/src/chesso 12

Debug self-play (DEC-141 clause 1; the canonical line is S190's to document,
this is `adocs/testing_strategy.md` R2's shape):

    cmake --build build-debug -j12
    fastchess -engine cmd=build-debug/src/chesso name=dbg-a \
      -engine cmd=build-debug/src/chesso name=dbg-b \
      -each tc=4+0.04 option.Hash=16 option.Threads=1 \
      -openings file=books/UHO_Lichess_4852_v1.epd format=epd order=random \
      -rounds 4 -repeat -concurrency 4 -pgnout file=.tuning/s148_debug.pgn \
      -log file=.tuning/s148_debug.log level=trace > .tuning/s148_debug.out 2>&1
    grep -c 'Assertion' .tuning/s148_debug.log .tuning/s148_debug.out    # 0 and 0
    grep -c -iE 'disconnect|crash|illegal' .tuning/s148_debug.out       # 0

Time forfeits between two Debug binaries are not the reading; an abort is. The
stamp says "0 Assertion over 8 Debug games".

### 7. Measurement

Lane: SPRT. Candidate is the working tree with `RFP_MAX_DEPTH` at C1;
reference is HEAD at 15. `./fastchess.sh --nonreg`: `elo0=-5 elo1=0
alpha=0.05 beta=0.05`, `model=normalized` (nElo), 8+0.08, Hash 16, the UHO
book, `-repeat`, `-check-mate-pvs`, 20000 rounds.

Why `{-5, 0}`. No gain is claimed: S085's tuner moved this axis toward more
pruning at 2+0.02, the joint +21.02 had 15 in the vector and is upward-biased
like every early-stopped estimate (DEC-063), and the one published experiment
of this shape was SPRT-neutral at both controls. The expected effect is a small
loss or nothing, so `{0, 5}` cannot contain it and random-walks to the cap
(S068 spent 6 h 36 m that way); `{-5, 0}` straddles it and is the harness's
own mode for "a constant moved for another reason" (DEV_MANUAL "Which
bounds"). The cheaper `{-5, 5}` (10465 games worst case, 4.5 h) asks whether
the sign is positive and would reject a one-nElo cost the owner might pay for
the mates, which is why the pair is the owner's call (question 1).

Cost (DEC-143, `adocs/testing_strategy.md` 1.1): `D / (e1 - e0)^2` with
`D = 1046535` gives 41861 games with the truth at the midpoint, **17.9 h at
2337 games an hour**; 25591 games and **10.9 h** with the truth on a bound.
Abort rule: `--nonreg`'s 20000 rounds cap the run at 40000 games, about
17.1 h, below the midpoint case, so a run that reaches the cap ends as "no
verdict" by construction and is read as such; the watcher's hard ceiling is
36 h; the run is stopped early only for a forfeit rate over 1.0 % on a side
(`python3 tools/forfeit_report.py <dir>/games.pgn --max-pct 1.0`) or an
`Incomplete mating PV` line from the candidate that the reference does not
match, both of which are findings and not results.

The three outcomes, pre-registered in `adocs/data/S148_sprt.sh` before a game
is played:

- **H1 accepted** -- C1 is not a regression of 5 nElo or more (about 3.5 to
  4.0 logistic Elo; take the ratio off the run's own `Elo` and `nElo` lines).
  Ship C1; the magnitude is not established, no gain is claimed, the mate
  counts are what the change is for, and the stamp names the verdict as
  inside S151's scope.
- **H0 accepted** -- C1 costs 5 nElo or more; C1 is rejected. If C2 = 6 was
  pre-registered and bought, run it under its own script with the same
  reading; otherwise **keep 15** and record the deep-mate loss as the measured
  price of S085's ceiling.
- **No verdict** at 40000 games -- record as zero (S005, S006, S015) and
  **keep 15**: the incumbent has a verdict behind it and the candidate is not
  inert in play (the bench moves), so "cannot tell" does not displace it. No
  re-run at other bounds without a decisions.md entry. If the owner would
  rather ship on a null for the mate property, that reading is written into
  the script before launch, not chosen after (question 1).

Script shape, from `adocs/data/S165_sprt.sh`'s header: the one integer, from
and to; the HEAD sweep table and the `search_bench.py` counts on both builds;
the pair with its worst-case games, hours and abort rule (DEC-143, new since
S165); the reading of each outcome; then `set -uo pipefail`, `cd
/home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }` and `exec
./fastchess.sh --nonreg`. Launch and watch:

    nohup adocs/data/S148_sprt.sh > .tuning/sprt_s148.log 2>&1 &
    echo $! > .tuning/sprt_s148.pid
    # the WATCHERS poll loop verbatim, RUN_LOG=.tuning/sprt_s148.log,
    # RUN_PID from the pid file, ceiling 129600, armed through Monitor with
    # persistent: true; a marker written before arming is still caught.

Records: `adocs/data/S148_rfp_ceiling_sweep.py` and `.log`,
`adocs/data/S148_sprt.sh` (and `S148_sprt_c2.sh` if run), the run's stdout as
`adocs/data/S148_sprt.log`, each with a row in `adocs/data/README.md`; the
step file gets the HEAD table, the chosen value and why, the verdict block
(`LLR`, `Elo`, `nElo`, games, wall time, forfeits, `Incomplete mating PV`
counts) and the re-derived floors with their reds.

### 8. Completion checklist

- Gate: `cmake --build build -j12 && ctest --test-dir build -L fast
  --output-on-failure && cmake --build build-tune -j12 && ctest --test-dir
  build-tune -L fast --output-on-failure && ./clang-format.sh --check`. The
  fast-suite log's `mate in 4: ... mate in 5: ...` line goes in the stamp.
- DEC-141 clause 3's `tools/gate_extra.sh` is S197's and lands after this
  step; run its parts by hand: `cmake --build build-debug -j12 && ctest
  --test-dir build-debug -L fast`, `ctest --test-dir build -L slow`,
  `python3 tools/plan_prose_check.py --prose`, `--citations`, `--params`.
- Commit message: S189 precedes this step in the Open list, so the completing
  commit ends with `Bench: <nodes>` from `./build/src/chesso bench` when the
  default moved, and `No functional change` when only comments in `src/`
  changed; `tools/gate.sh` checks it. If S189 has somehow not landed, quote
  the three `search_bench.py` counts in the stamp instead.
- Docs, the five places 15 is written: `adocs/specs.md`'s "with a remaining
  depth of 15 or less" and "S085's move of that bound from 6 to 15 is the
  trade to re-measure and S148 is where it is"; `MANUAL.md`'s `RfpMaxDepth`
  row and its known-bug entry ("only where 15 or fewer plies remain"), which
  also carry the mate in four and five counts at the decided value;
  `DEV_MANUAL.md` "Mate safety" table of counts on the shipping build,
  re-taken ("Which bounds" is unaffected); the `RFP_MARGIN` comment in
  `src/search_params.hpp`; the `MATE_IN_THREE_FLOOR` comment in
  `tests/test_engine.cpp`. `tools/plan_prose_check.py --params` green.
  `test_uci_surface` builds the option line from the table, so no golden
  refresh is owed, but MANUAL.md's row must agree with it (SURFACE rule).
- Stamp: the HEAD sweep per value and distance; C1 and its rule; the verdict
  block and wall time; the bench and `search_bench.py` counts on both builds;
  "0 Assertion over 8 Debug games"; each golden re-derived with its script and
  its red; the promoted floor or the sentence saying why it stayed a
  `MESSAGE`; the S151 note; that `RfpMinPly` was 3 throughout.
- `plan.md`, `status.md` and the decision entry go through the coordinator.

### 9. Sources read

- https://www.chessprogramming.org/Reverse_Futility_Pruning -- definition,
  pseudo-code, skip conditions, "constant multiple of depth", no Elo.
- https://www.chessprogramming.org/Futility_Pruning -- frontier and
  pre-frontier origin, Heinz's extended form, depth-scaled margins.
- https://api.github.com/repos/official-stockfish/Stockfish/commits/09b6d283
  and `/commits/dbd7f602` -- the two cap removals, dates, "passed STC and
  LTC", the `<-2.50,0.50>` bounds; commit messages only, no source read.
- https://api.github.com/repos/official-stockfish/Stockfish/commits/dabaf222
  -- the revert and its "sf14 2427, master 1246, patched 2467" table.
- https://api.github.com/repos/official-stockfish/Stockfish/issues/3641/comments
  -- vondele's "patchPart1 1282, patchPart2 1630", the figures the step body
  quotes; confirmed here, they are not in the commit message or PR body.
- https://api.github.com/repos/official-stockfish/Stockfish/issues/3627 and
  its comments -- the mate-finding report behind the revert; no counts.
- https://api.github.com/repos/official-stockfish/Stockfish/commits/fa8b6add
  -- the dynamic cutoff exists and passed non-regression; its constants not
  taken. The step body's claim that Stockfish's source has carried "The depth
  condition is important for mate finding" ever since is **unverified** here:
  reading that source is barred (DEC-016); the record-side evidence is
  SFisGOD's PR #3641 comment asking for exactly that comment.
- Repository: `src/search.cpp`, `src/search_params.hpp`, `src/search.hpp`;
  the tests named above; `fastchess.sh`, `tools/search_bench.py`,
  `tools/plan_prose_check.py`; `adocs/data/` S145, S154, S165, S149 scripts
  and logs, `README.md`, the test review's `mutants.py`; `plan_done/` S145,
  S142, S085, S165, S108, S033; `plan_todo/` S151, S198, S189, S190, S191,
  S196, S197, S184; `specs.md`, `MANUAL.md`, `DEV_MANUAL.md` "Test",
  "Measure", "Play games", "Which bounds", "Mate safety";
  `adocs/testing_strategy.md` 1.1 and R2; `plan.md`, `status.md`,
  `.moltke.local.md`; DEC-019, DEC-063, DEC-095, DEC-105, DEC-116, DEC-134,
  DEC-135, DEC-140 to DEC-144.

Corrections to the existing sections. The table under "What S145 measured" is
the `14748c9` reading and every count has moved since: S154's re-take at
`fc5526e` reads 25 of 48 at ceilings 10, 15 and 63 (not 24), 40 at 0 (not
34), 30 at 6 (not 27), deep classes at 0 of 6 of 8 and 5 of 8 (not 4 and 3);
the set is 82 rows since S168 and the ceiling has never been swept over them,
so the HEAD sweep of section 3 is the number this step decides on. "10 is
already indistinguishable from 63" holds on every re-take -- and one value
lower on the finer grid, from 10 rather than from 15. The sentence that quoted
Stockfish's shipped constants for the excluded dynamic-cutoff feature is
reworded to name the shape without the numbers, 2026-09-08, question 4
answered: DEC-134 makes a constant read off an engine's commit message that
engine's constant wherever it appears, and this one seeds nothing here. The header carries `author:` in `plan_todo/`
and has no `done:` field; S184's worklist owns that (its F10).

### 10. Questions deferred to the owner, answered 2026-09-08

1. **The pair.** `{-5, 0}` as recommended (17.9 h worst case, accepts a cost
   up to about 2.5 nElo for the mates), or `{-5, 5}` (4.5 h, ships only a
   positive sign)? And on a null: keep 15 as written, or ship the challenger
   for its mate property? Either reading is written before the run.
2. **One verdict or two.** C1 alone, or C1 with C2 = 6 as a pre-registered
   fallback if C1 is rejected -- up to 36 h of machine against 18. Elbow-first
   is recommended because a pass at C1 is the better mate result; 6-first is
   the cheaper Elo risk.
3. **The accepts says `adocs/data/S145_rfp_sweep.log` is "re-run at the
   shipped value"**; that file's header and `adocs/data/README.md` say it is
   never rewritten, and S154 already re-took the ceiling into its own log.
   Proposed: the re-run lands in `S148_rfp_ceiling_sweep.log` and the accepts
   is read that way; a one-word amendment if the owner prefers it explicit.
4. **The step body quotes Stockfish's dynamic-cutoff constants** ("19 ...
   13"). The feature is excluded, so they seed nothing, but DEC-134 says a
   constant quoted from an engine's commit message is that engine's constant
   wherever it appears. Reword the sentence to "a table that shrinks the
   ceiling as scores approach the decisive band", or leave it as history?
5. **A promoted floor at a count of 1** satisfies the accepts' "non-zero" but
   has no margin, which DEC-116 rejected for the mate in three. Promote at 1
   with the zero margin stated, or keep that class in the `MESSAGE` until a
   later change lifts it to 2?

**The answers, taken 2026-09-08 before the sweep finished and before any game
was played.** Every one of the five is the file's own recommendation:

1. **The pair is `{-5, 0}`**, `--nonreg`. And on a null: **keep 15**, as
   written -- the incumbent has a verdict behind it and the candidate is not
   inert in play.
2. **C1 alone.** No C2 = 6 fallback is bought; a rejection ends the step at
   "keep 15" with the measured price recorded.
3. **The re-run lands in `adocs/data/S148_rfp_ceiling_sweep.log`**, and the
   accepts' "`S145_rfp_sweep.log` re-run at the shipped value" is read that
   way: that file is evidence and is never rewritten. Decided by the agent as
   a low-stakes reading, and it is what the file already proposed.
4. **Reworded**, and the sentence in the header section now names the shape
   without Stockfish's two constants. Also the agent's call, under DEC-134.
5. **A class whose count is exactly 1 stays in the `MESSAGE`.** A floor of 1
   has no margin between its ends, which is what DEC-116 rejected for the mate
   in three. This binds: at C1 = 4 the mate in five count is exactly 1, so
   **only the mate in four class is promoted** and the mate in five stays
   recorded, with the comment saying which and why.
