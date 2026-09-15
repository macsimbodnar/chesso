id:         S222
goal:       one-ply continuation history retried with a scale of its own -- its bonus, malus and bound as tune-build parameters and the weight of the continuation term against plain history, fitted in a narrow SPSA lane of its own before the SPRT, together with plain history's six never-fitted coefficients, directly before S098 which reads the sum (DEC-198) -- because S024's table, on plain history's scale, measured H0 (DEC-194)
accepts:    the one-ply table of S024 verdict 1 (`cace216`'s shape: `cont_hist[12][64][12][64]`, one index helper, the guard on a previous move existing, the sentinel cases and the two mutants) rebuilt with its own `CONT_HIST_BONUS`/`CONT_HIST_MALUS`/`ContHistMax` and a `ContHistWeight` (or an equivalent parameterisation stated in the file) declared in `src/search_params.hpp` the way existing tunables are, so the tune build can move them; the two hypotheses DEC-194 wrote are tested by the fit rather than argued -- that summing two equal-weight terms doubled plain history's share of the quiet band, and that the shared gravity bound clipped the table; **the fit runs first**: one narrow SPSA pass over the new parameters, `QuietHistoryMax` and the six `HISTORY_BONUS_*`/`HISTORY_MALUS_*` coefficients of plain history -- never fitted, `tools/spsa_s085.json` carries no history axis -- at S085's regime, pre-registered in its own script's header with the estimate from the measured throughput, its result recorded whatever it is; **then one gainer SPRT** `{0, 5}` nElo at the harness regime against the commit before it, pre-registered per DEC-143 with the fitted values named; the band-clearance case re-stated for the new bound; `bench` line; Debug self-play; the census driver `adocs/data/S024_census_run.py` re-run on the fitted build and its shares recorded beside S024's (97.6 / 96.2 / 27.1 %); H1 keeps it and opens the two-ply table as its own follow-up step; H0 records the zero and the technique leaves the plan with a decision saying why
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp, src/search_params.hpp, tests/, adocs/data/
excludes:   the two-ply table (its own step after this one passes); any constant taken from another engine (DEC-084, DEC-105); reusing S024's unfitted result as evidence for or against the fitted one
decisions:  DEC-194, DEC-198, DEC-019, DEC-084, DEC-105, DEC-143, DEC-141
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator for phases one and two (DEC-185, DEC-199); the SPSA night and the SPRT are the coordinator's; started 2026-09-14 12:55 on the idle machine
done:       2026-09-15 07:00. **H1: S222 whole is worth +11.13 +/- 6.90 Elo at `8+0.08`, decided in 6278 games.** `adocs/data/S222_sprt.sh` ran as pre-registered -- candidate `d0a6667` (phase three's landing) against `d785b89`, the tree before S222 (DEC-210), both identity lines printed, `{0, 5}` nElo, `8+0.08`, `Hash=16`, `noob_3moves.epd`, seed `20260915040001`, 12 cores -- from 04:00:01 to 06:55:09: **LLR 2.95, `Elo 11.13 +/- 6.90`, `nElo 13.87 +/- 8.59`, LOS 99.92 %, `Ptnml(0-2) [291, 686, 1060, 735, 367]`, W 2138 L 1937 D 2203, 6278 games in 2 h 55 m 08 s at 2151 an hour**, 0 time forfeits either side, 4578 adjudications and 1701 natural ends over the 6279 games the PGN holds, `Incomplete mating PV` 0 and 0, pair-score mean 1.0320 and variance 0.3218 over 3139 pairs (`adocs/data/S222_sprt_pairs.txt`). Read as pre-registered: H1 at the gainer pair, **the whole vector and the table stay** -- the one-ply continuation history on its own fitted scale (bonus 17, malus 18, weight 26 against the 32767 bound) together with plain history's six coefficients, `QuietHistoryMax` 8831 and `HistPruneCoeff` 612, one vector under one verdict (DEC-210); the stopping run's Elo is upward-biased and what may be written is at least 5 nElo (DEC-063). What S024's table lacked was its scale and the history fitted beside it: the same table on plain history's scale measured -5.48 +/- 7.20 (DEC-194), and on its own it clears the gainer pair; which part of the vector carries the gain is not attributed, by design (DEC-210), and the lane's weight reading -- 26, near its seed -- says the equal-authority sum was not the fault. **The two-ply table opens as S231**, behind S098 as the H1 clause requires. Second tier closed before the match: `gate_extra` 5 stages 1073 s, the four mutants 4 of 4 killed at the fitted defaults, Debug self-play 8 games 0 `Assertion`; the census on the fitted build 96.62 / 95.15 / 19.15 % beside S024's 97.56 / 96.19 / 27.14, with the same-tree control that puts the third figure's fall on the smaller tree. Taken with S194 open (Open entry 1, no reach into play), as the pre-registration names. Evidence `adocs/data/S222_sprt.log`; the lane's `S222_spsa_trajectory.tsv`, `S222_spsa_run.json`, `S222_spsa.log`; `S222_census.txt`. Phases one and two implemented by an Opus 5 subagent and a repair agent, phase three by another Opus 5 subagent; the lane, the readings, the SPRT and the stamp are the coordinator's (DEC-185, DEC-199).

## Why this exists

S024 verdict 1 built the one-ply table on plain history's own formula and
bound, with no constant of its own, and the gainer SPRT accepted H0 at
-5.48 +/- 7.20 nElo over 8954 games while the census showed the table
exercised in 96 % of quiet reads (DEC-194). So the technique as the published
record describes it did not transfer on this engine at that scale, and the
plan's rule for a technique whose value the record puts between +2 and +45
Elo is to fit before concluding (DEC-019, DEC-084): the fit was S127's lane
and this step waited behind it rather than guessing; since DEC-198 the fit is
a narrow lane of this step's own and the step sits directly before S098,
which reads the sum.

## Moved before S098 with its own lane, 2026-09-12, DEC-198

S098 scales the reduction by the history sum and its file forbids measuring
against a table about to change; with this step at entry 51 that is what S098
would have done, on plain history alone, and been measured again when this
landed. And plain history's own scale is a seed: bonus = malus = depth
squared, S085 tuned none of the six coefficients, nothing before S127 fits
them. So the lane here is the six coefficients, `QuietHistoryMax` and this
step's own tunables, one night at S085's regime, pre-registered with the
estimate from `.moltke.local.md`'s throughput, then the SPRT above. S127
still refits every axis after the block.

## Cost

One SPSA pass (a night, S085's regime, a lane of its own -- DEC-198) and one gainer SPRT (2.5 h if the
effect is real, 19.8 h at the wall), plus the rebuild from `cace216`'s shape,
which `git show cace216` holds in full.

## Note 2026-09-13, from S212: the lane's fastchess config carries `twosided=true`

S212 made resignation two-sided in both harnesses (DEC-174). `adocs/data/S085_spsa_run.json`'s `extra` is one-sided and is a frozen record of S085's run, not a template: the SPSA config this lane writes carries `-resign movecount=3 score=400 twosided=true`, the regime every verdict is taken in from S212 on, and its pre-registration says so.

## Note 2026-09-13, from S109: history pruning ships nearly inert, DEC-205

S109's history pruning rule reads plain quiet history below `-HistPruneCoeff * lmr_depth` (576, a range midpoint) and removes 0.8 % of the nodes at depth 10 over 300 positions, because the continuation table the rule was written for was reverted (DEC-194) and plain history alone rarely reaches the threshold. This lane fits the history scale and lands the table; `HistPruneCoeff` belongs in its axis list, and the lane's SPRT re-prices the rule.

## Phases one and two landed 2026-09-14 14:14

Built by an Opus 5 subagent on the idle machine (DEC-185, DEC-199). Phase
three -- the fitted defaults, the census re-run and the gainer SPRT -- follows
the night and is briefed separately. The step stays in `plan_current/` and
`done:` is empty.

### The parameterisation, and why it is three axes and not four

`accepts` names `CONT_HIST_BONUS`, `CONT_HIST_MALUS`, `ContHistMax` and
`ContHistWeight`, "or an equivalent parameterisation stated in the file". This
is the equivalent, and the reason is arithmetic rather than taste.

`history_gravity_update` is `e + b - e|b|/M`. Scale `b` and `M` together by
`k` and every entry scales by `k`; the read then multiplies by the weight. So
(bonus, malus, bound, weight) over one table reaches the search only through
(bonus/bound, malus/bound, weight x bound) -- **three degrees of freedom, and
the fourth direction is a gauge**. An SPSA lane carrying all four would walk
that direction without moving the engine and land a meaningless endpoint in
the vector the SPRT judges, which is exactly why DEC-094 dropped
`OrderHistoryMax` from S085 and DEC-200 dropped `TmHardPercent` from S127 and
from this lane.

So the gauge is fixed instead of tuned. **The table's bound is a definition**,
`CONT_HIST_BOUND` in `src/data_structures.hpp`, at the `int16_t` ceiling the
entry's own type sets -- which is also the widest band the table can have, and
therefore the strongest available answer to DEC-194's second suspect: a table
clipped by a bound it shares with another table cannot arise when the bound is
its own storage. `search_params.hpp`'s own header already excludes definitions
from the tunable set and this joins that class.

**DEC-194's two suspects are then one axis, and that is a finding of this
phase.** "Two terms of equal weight doubled plain history's share of the quiet
band" and "the shared bound clipped the table" are both statements about how
much of the quiet band the continuation term spans. That span is
`ContHistWeight x CONT_HIST_BOUND / 100`, plain history's is
`QuietHistoryMax`, and both are axes of the lane. The fit tests both suspects
by moving their ratio; neither is argued.

The three axes, in `src/search_params.hpp` beside plain history's six:

| symbol | UCI name | default | range |
|---|---|---|---|
| `CONT_HIST_BONUS` | `ContHistBonus` | 15 | 0 to 1000 |
| `CONT_HIST_MALUS` | `ContHistMalus` | 15 | 0 to 1000 |
| `CONT_HIST_WEIGHT` | `ContHistWeight` | 25 | 0 to 2000 |

The two shares are in **thousandths of the table's own band at
`CONT_HIST_REF_DEPTH`**, 11, which is chesso's own median remaining depth as
`RfpMaxDepth`'s comment records it from S085's control. The depth grading stays
the published quadratic; the unit buys resolution. A bare `QUAD * depth *
depth` coefficient in `HistoryBonusQuad`'s shape has its whole useful region
inside the integers 0 to 4, and an SPSA axis whose smallest perturbation is a
quarter of its own useful region cannot be fitted -- the same coarseness plain
history's six coefficients still have, stated in the lane's own header rather
than fixed there.

### Ranges and seeds, DEC-084 as amended by DEC-105

Ranges are by stated purpose, never a guess at where the good values are. 0 is
a true off value on all three: nothing is written and the ordering term is
identically 0. 1000 is one whole band closed by a single update at the
reference depth, past which the clamp inside `history_gravity_update` makes
every larger value the same engine. `ContHistWeight`'s 2000 is the
band-clearance ceiling and it is CLAUDE.md's one-way door: the quiet band
becomes `[-(QuietHistoryMax + w * CONT_HIST_BOUND / 100), +the same]` and must
stand 100 clear of the countermove band at 700000. At both declared maxima that
is `32767 + 655340 = 688107`, a clearance of **11893**; at 2100 it would be
720901 and the band would swallow the countermove and both killers.

Seeds are **(b), a derivation over chesso's own history scale** -- no engine's
constant is behind any of them, and S024's reuse of plain history's numbers is
not evidence for these:

- `ContHistBonus` and `ContHistMalus` at **15**: plain history's shipped
  grading closes `11 * 11 / 8192` of its own band at the reference depth, which
  is 14.77 thousandths. The table starts at the same *rate* as the table it
  sits beside, on a band of its own, and the lane moves it.
- `ContHistWeight` at **25**: `25 * 32767 / 100 = 8191` against plain history's
  shipped `QuietHistoryMax` of 8192, so the two terms start with equal
  authority. That is precisely the equal-weight sum S024 measured, which is the
  null the first suspect is tested against.

### `HistPruneCoeff` and the sum: what was done, and why not more

S109's history pruning still reads the **raw plain entry**, not the sum, and
`src/search.cpp` says so at the rule. DEC-205 asks for `HistPruneCoeff` in the
lane's axis list and it is there; it does not ask for the rule's input to
change, and two things argue against changing it here. One verdict already
prices a new table and a fitted history scale, and a third change inside it
could not be attributed (MEASUREMENT). And `HistPruneCoeff`'s declared range is
derived from the plain band's own edge -- `QuietHistoryMax/64` to
`QuietHistoryMax/8`, range top at twice that edge -- so against a sum whose
span is two axes' product that derivation has to be taken again, with its own
seed. S098 is the step that reads the sum by design and its file carries the
amendment.

### Tests, red first, and four mutants killed

`tools/mutants/S222_continuation_history.py`, applied by hand and reverted byte
for byte (`tools/mutation_check.py` needs a linked worktree at a commit that
carries this work, which does not exist until the coordinator commits;
re-validate there, S024's and S207's precedent). `load_mutants` + `validate`
pass against the working tree, every anchor unique.

| mutant | what went red |
|---|---|
| `H01_cont_hist_malus_sign` | "a quiet cutoff with a previous move grades the continuation table": `CHECK( 259 < 0 )`, "A quiet tried before the cutoff scores 259 in the continuation table and not a malus", three times |
| `H02_cont_hist_no_prev_guard` | both sentinels: "a quiet cutoff maluses the quiets tried before it" `CHECK_EQ( 4, 0 )` and "the cutoff move is credited and the quiets before it are charged" `CHECK_EQ( 5, 0 )` |
| `H03_null_child_keeps_prev` | "the node after a null move has no previous move to index": `CHECK_EQ( 1, 0 )` |
| `H04_cont_hist_unread` | "the declared history ceiling clears the band above it": `REQUIRE_EQ( 32767, 40958 )` |

The sentinels are the pair `accepts` asks for. Ply 0 is two cases -- the direct
call and the drive through `negamax` -- each scanning the **whole** table and
requiring it at zero, because the failure a dropped guard causes is a write to
the wrong cell and not a crash: move 0 decodes to the legitimate `(W_PAWN, a8)`
cell. The node after a null move is a case of its own in the guard suite, which
S024 left to a structural argument: the pass makes the side to move White again
under a White previous move, and no ordinary node can write that pair, so the
White half of `PREV_MOVE`'s row has to be zero. It is driven through
`negamax_probed` with `probe.null_move_made` asserted first, on a position
where no capture exists for either side so the fail-high after the pass must
come from a quiet.

The band case, `tests/test_evaluation.cpp` "the declared history ceiling clears
the band above it", is re-stated for the sum: both tables driven to their own
bounds at once through the self-referencing cell, the compiled band asserted
exactly (which is what `H04` fails), the clearance asserted against the
**widest** band the declared ranges admit and not against the compiled one, and
the arithmetic fact that the widest band exceeds `int16_t` -- which is why the
sum is carried in `int`.

### One mate row moved, measured both ways, and it is not a relaxation

`tests/test_search.cpp` "pruning does not hide a forced mate" went red on this
change and the cause was measured rather than guessed. S091's `capture_mates`
table pins a depth per position, and its own paragraph defines that depth as
"where the shipped build reports the mate and the mutant does not" -- a
measurement, which a change to move ordering can move. `search_fen()` over
depths 3 to 12, on all four positions, in this tree and in a worktree at
`f4f70c4`, on the shipped build and under each of the six S091 mutants:

| position | parent, shipped | here, shipped |
|---|---|---|
| `3krb1r/Np2pppp/...` | d7 d9 d10 d11 d12 | unchanged |
| `2b5/4k2P/...` | d7 d9 d10 d11 d12 | unchanged |
| `3N1bk1/3Q3p/...` | **d8** d9 d10 d11 d12 | d9 d10 d11 d12 |
| `6qk/7p/...` (S109's) | every depth 3 to 12 | unchanged |

So one row of four lost one depth. The row is removed and the labels are
re-derived with it: the first position separates C02 and C05 at depth 7 and no
longer R02, the second separates C02, C05 **and** R02 at depth 7, and the third
separates no S091 mutant at any depth 3 to 12 and its column says exactly that.
**R01 is no longer separated by any of the four**, where the second position
carried it at depth 7 at the parent -- measured both ways -- and that is not a
hole: run under R01 the whole fast suite fails at "a capture that gives check is
not reduced", S091's own direct guard, and nowhere else. What was lost is a
second, incidental kill. Nothing else in the mate suites moved:
`test_mate_carry`'s ceilings and `test_engine`'s 48-position mate safety are
both green unchanged.

`tests/test_search.cpp` "ordering keeps the tree small" reads **17332** against
17451 at the parent, 0.7 % inside a band of `[3490, 69804]`, so its golden pair
is not re-derived (DEC-142's own rule: re-derive when the count leaves the
middle half).

### Measurements

`chesso bench`: **6267842 -> 5950740**, -317102 nodes, -5.06 %. The commit
carries `Bench: 5950740`.

Throughput, four interleaved pairs on the idle machine: before 3889177,
3908460, 3832251, 3960101; after 3771750, 3771310, 3755609, 3731702. The groups
do not overlap and the means are 3897497 against 3757593, **-3.6 % of nodes per
second** -- two extra dependent loads per scored quiet, one extra graded update
per cutoff, and a 1.125 MiB table. Just outside what CLAUDE.md calls noise, and
reported as measured.

`tools/search_bench.py`, before -> after:

| depth | midgame | kiwipete | tactical |
|---|---|---|---|
| 9 | 74327 -> 74825, `c3d5` | 146873 -> 147048, `e2a6` | 27855 -> 25019, `d7c8q` |
| 12 | 172303 -> 184125, `c3d5` | 596588 -> 577720, `e2a6` -> **`d5e6`** | 190823 -> 97318, `d7c8q` |

Node counts move by construction, so INV-6's discharge is not available and the
SPRT is the only thing that can decide this step. Kiwipete's best move at depth
12 moves, which S109 also did on the same position.

Debug self-play (DEC-141 clause 1): four rounds at 4+0.04, `noob_3moves.epd`,
concurrency 8 -- **8 games in 25 s, 0 `Assertion`, 0 `disconnect`**, both the
`level=trace engine=true` log and the tee'd stdout. Time forfeits are not the
failure condition at this control.

Gate, both builds, `CLANG_FORMAT_MAJOR=22`: **38 of 38 in `build`, 38 of 38 in
`build-tune`, `./clang-format.sh --check` clean**. `tools/gate_extra.sh` is the
coordinator's and is not run here.

### The lane, written and checked, not run

`tools/spsa_s222.json`: eleven axes -- the three above, `QuietHistoryMax`,
plain history's six coefficients and `HistPruneCoeff` -- at S085's regime, 1250
iterations x 24 pairs = 60000 games at 2+0.02, Hash 16, concurrency 12,
`r_end` 0.004, seed 222, adjudication `fastchess.sh`'s line verbatim including
`twosided=true` (S212, DEC-174). No `Tm*` axis and no `TmHardPercent`, with
DEC-094 and DEC-200 named in the config.

**The book is `books/UHO_4060_v3.epd` and the brief said `noob_3moves.epd`;
this is the one place the brief was not followed and the coordinator decides
before launch.** Three documents say tune and verify must not share openings --
`adocs/eval_tuning_strategy.md` par.7; `DEV_MANUAL.md`, whose own sentence has
the run tuning on the UHO book while fastchess.sh verifies on the harness one;
and this step's own "at S085's regime". `books/fetch_book.sh` pins the second
UHO-class book for exactly that. Changing it is one field in the config and one
line in the pre-registration.

`python3 tools/spsa_driver.py check tools/spsa_s222.json --engine
build-tune/src/chesso`, on the idle machine, no warnings:

```
setoption reaches the search: ContHistBonus 0 -> 63019 nodes, 1000 -> 86678 (depth 9, midgame)
setoption reaches the search: ContHistMalus 0 -> 53126 nodes, 1000 -> 66740 (depth 9, midgame)
setoption reaches the search: ContHistWeight 0 -> 74327 nodes, 2000 -> 72222 (depth 9, midgame)
setoption reaches the search: QuietHistoryMax 1 -> 56700 nodes, 32767 -> 74825 (depth 9, midgame)
setoption reaches the search: HistoryBonusQuad 0 -> 31609 nodes, 1024 -> 55155 (depth 9, midgame)
setoption reaches the search: HistoryBonusLin 0 -> 74825 nodes, 4096 -> 52564 (depth 9, midgame)
setoption reaches the search: HistoryBonusConst -32768 -> 44942 nodes, 32767 -> 49875 (depth 9, midgame)
setoption reaches the search: HistoryMalusQuad 0 -> 47010 nodes, 1024 -> 19176 (depth 9, midgame)
setoption reaches the search: HistoryMalusLin 0 -> 74825 nodes, 4096 -> 19451 (depth 9, midgame)
setoption reaches the search: HistoryMalusConst -32768 -> 41762 nodes, 32767 -> 19451 (depth 9, midgame)
setoption reaches the search: HistPruneCoeff 0 -> 44607 nodes, 16384 -> 74825 (depth 9, midgame)
probed 11 of 11 parameters in 0.7 s
11 parameters, 1250 iterations x 24 pairs = 30000 pairs, 60000 games
SPSA-DONE
```

`adocs/data/S222_spsa.sh` is the pre-registration and the runner, with `OUT`
under `.tuning/`. **Estimate 8 h 30 m, ceiling 17 h.** From measured throughput
and not a guess (DEC-155): S085 ran this exact shape on this machine in 8 h
21 m -- 24.05 s an iteration, 7186 games an hour -- against its own pre-run
23.47 s an iteration; a game at a fixed clock costs `2 * (base + moves * inc)`
whatever the engine's speed, which is S085's own finding, so only game length
can move the wall and 10 % longer games would add 3.7 %. Cross-checked the
other way from `.moltke.local.md`'s 2110 games an hour at 8+0.08, which prices
60000 games at 2+0.02 at about 7.1 h -- lower, so the estimate is the
conservative one. Past DEC-155's four-hour line: a night. The abort rule is
forfeits over 1.0 % on a side, `SPSA-FAILED`, or the machine losing mains or
gaining a second load; a sick trajectory is read and recorded, never adjusted.
Mid-run reads at about 25 % and 50 %. `SPSA-DONE`/`SPSA-FAILED` is the terminal
marker on every exit path.

### Documents

- `MANUAL.md`: three option rows added, name, default, range and effect. The
  tune build's surface grew by three and `test_uci_surface` requires it.
- `DEV_MANUAL.md`: the S222 lane documented beside S085's worked example, with
  the invocation, the axes, the estimate and the two books; and the bench
  ledger gains `S091` 6267842 and `S222` 5950740 with the nps pairs.
- `tests/test_search_params.cpp`: the golden table goes 41 rows to 44.
- `README.md`: human-owned, untouched.
- `adocs/specs.md`: not edited (hard limit). Wording proposed below.

### Fast check, 2026-09-14, repaired before the landing commit

Five findings, none blocking, all fixed in the same commit by the coordinator
and a repair agent (AGENTS: trivial and in scope, fixed now, noted here for
the stamp):

- `search_state_t` grew to 1267272 bytes with `cont_hist`, and
  `iterative_deepening_search` built it as a local on `search_thread`'s
  stack -- 8 MiB on this machine, 512 KiB for a `std::thread` on macOS, where
  the first `go` would have overflowed with nothing to detect it. The one
  struct built per `go` is heap-owned there now; the table stays a value
  member so the rebuild still clears it with `quiet_history` and
  `counter_moves`. `tools/datagen.cpp`'s `run_search`, called from its
  worker threads, had the same defect and takes the same fix; the tests build
  theirs on the main thread. Bench unchanged, as heap against stack must be.
- R02's and R01's mutation notes in `tests/test_search.cpp` still described
  the mate table's rows before this step re-derived their labels (DEC-209):
  R02's incidental kill is the second row now, and R01 is separated by no row
  until S230. Both notes follow the labels.
- The lane's pre-registration read a `ContHistWeight` fitted well below 25
  only as DEC-194's first suspect. It gains the reading for a fit at or under
  5 with an H1: the verdict is then the six history coefficients', and a
  second SPRT of the fitted vector against itself with the weight pinned at 0,
  gainer {0, 5}, decides the table before the two-ply step opens. Fixed before
  the fit so the reading is not chosen after the number.
- `adocs/data/S222_spsa.sh` ended in `exec` with a fallback marker after it
  that could never run. The shell stays the watched pid and prints
  `SPSA-FAILED` itself when the driver exits non-zero without one.

### Second tier after the landing commit `96fdc19`, 2026-09-14

- `tools/mutation_check.py tools/mutants .ref-builds/mut --only
  H01_cont_hist_malus_sign H02_cont_hist_no_prev_guard
  H03_null_child_keeps_prev H04_cont_hist_unread`: 4 of 4 killed, each by one
  fast-suite case (1/38 red under every mutant), the bench moved under H01,
  H03 and H04 and stayed under H02 -- the dropped write guard hits no bench
  position, which is why a test and not the signature kills it. 130, 170,
  129 and 127 s. Log `.tuning/mutation_s222.log`.
- `tools/gate_extra.sh`: GATE-EXTRA-DONE, 5 stages, 1101 s (prose 0, citations
  0, debug 347, sanitize 696, perft 58), run beside the mutation pass. Log
  `.tuning/gate_extra_2026-09-14_S222.log`. DEC-141's second tier is closed
  for phases one and two; phase three re-runs whatever it touches.

### Proposed `specs.md` wording, for the coordinator

For the **search** row, replacing the S024 sentence's closing clause and added
after the S091 sentence:

> **One-ply continuation history returned with a scale of its own on
> 2026-09-14, S222 (DEC-194, DEC-198)**: the table of S024 verdict 1 --
> `cont_hist[12][64][12][64]` on `search_state_t`, keyed on the previous move's
> (piece, to) and this move's, reached through one helper `continuation_entry`,
> written at every quiet cutoff and summed into `score_move`'s quiet return --
> rebuilt on its own constants. `ContHistBonus` and `ContHistMalus` grade it in
> thousandths of its own band at chesso's median remaining depth 11, its bound
> is the `int16_t` ceiling as a definition rather than a setting, and
> `ContHistWeight` decides how much of the quiet band the term spans, so the
> band is `[-(QuietHistoryMax + ContHistWeight x 32767 / 100), +the same]` and
> still clears the countermove band by 100 at both declared maxima -- 688107
> against 700000, asserted at both edges. Three axes and not four because a
> bound and a weight over one table are one degree of freedom: DEC-194's two
> suspects, the doubled share and the shared bound, are the same statement
> about that span and the fit tests them on it. Ply 0 and the node after a null
> move pass no previous move and touch nothing, each asserted with the mutant
> that breaks it killed. History pruning still reads the raw plain entry.
> `bench` 6267842 -> 5950740, -5.1 %, at 3.6 % fewer nodes per second; the
> defaults above are first settings and S222's own SPSA lane
> (`tools/spsa_s222.json`, `adocs/data/S222_spsa.sh`) fits them together with
> `QuietHistoryMax`, plain history's six never-fitted coefficients and
> `HistPruneCoeff` before the gainer SPRT decides the step.

For the **absent, search** row: `continuation history (reverted at S024,
returns as S222)` leaves the list.

## Phase three, 2026-09-15: the fit lands, the census is re-run, the SPRT is pre-registered

Built by a fresh Opus 5 subagent on the idle machine (DEC-185, DEC-199). The
step stays in `plan_current/` and `done:` is still empty: what completes this
step is the SPRT's verdict, and that run is the coordinator's.

### The fit, and which pre-registered reading it lands in

The lane ran **2026-09-14 18:05:58 to 2026-09-15 02:43:29 -- 8 h 37 m 31 s
against the 8 h 30 m estimate**, 1.5 % over, 24.84 s an iteration against
S085's measured 24.05. 1250 iterations x 24 pairs = 30000 pairs = **60000
games** on `books/UHO_4060_v3.epd`, W 21663 L 21536 D 16801, `SPSA-DONE`.
Evidence: `adocs/data/S222_spsa_trajectory.tsv`, `S222_spsa_run.json`,
`S222_spsa.log`.

The vector is the driver's own rounded JSON block at the end of that log and
no number in it was re-rounded by hand (`final_vector` rounds at the UCI
boundary, which is where the run itself sent every value):

| axis | incumbent | fitted | theta before rounding |
|---|---|---|---|
| `ContHistBonus` | 15 | **17** | 17.46 |
| `ContHistMalus` | 15 | **18** | 18.06 |
| `ContHistWeight` | 25 | **26** | 25.69 |
| `QuietHistoryMax` | 8192 | **8831** | 8831.48 |
| `HistoryBonusQuad` | 1 | **6** | 6.33 |
| `HistoryBonusLin` | 0 | **19** | 18.82 |
| `HistoryBonusConst` | 0 | **2** | 1.63 |
| `HistoryMalusQuad` | 1 | **0** | 0.18 |
| `HistoryMalusLin` | 0 | **17** | 17.06 |
| `HistoryMalusConst` | 0 | **36** | 35.96 |
| `HistPruneCoeff` | 576 | **612** | 611.88 |

**Not a stuck run.** Every axis moved off its seed, so S085's rule -- a rounded
vector equal to the incumbent is recorded as one and owes no SPRT -- does not
apply and the run is owed.

**The weight ended near 25, which is the fifth of the lane's five
pre-registered readings.** Neither of DEC-194's suspects is what the fit
found: the continuation term was not over-weighted at an equal-authority sum
and it was not starved of band. At 26 it spans `26 * 32767 / 100` = 8519
against plain history's fitted 8831, the same near-equal authority the seed had
at 8191 against 8192. **The "at or under 5" row is not triggered, so there is
no pinned-zero attribution SPRT** and `adocs/data/S222_sprt_pinned.sh` is
deliberately not written. One gainer SPRT `{0, 5}` decides the step.

What did move is what had never been fitted: plain history's six coefficients
and `QuietHistoryMax`. The fit splits the bonus from the malus for the first
time and does it asymmetrically -- bonus `6d^2 + 19d + 2`, malus **linear** at
`17d + 36` with its quadratic coefficient fitted to zero. That is a hypothesis
and not a result until the SPRT says otherwise (DEC-019), and the two quadratic
axes were the lane's coarsest by its own header's admission.

### The three mid-run reads, taken over the whole trajectory

The lane pre-registered them and said they are recorded whatever they say.

- **`c_scale` decays as designed**: 2.055 -> 1.150 at a quarter -> 1.072 at a
  half -> 1.000 at the end.
- **`y` has real spread and is centred**: mean 0.102, standard deviation 6.05,
  range -35 to +35, and only 7.0 % of iterations at exactly zero. Not the
  "barely changing" trajectory the fishtest wiki calls useless.
- **No axis lives at a bound.** The only one that touches one is
  `HistoryMalusQuad`, at its floor of 0 for **18.8 %** of iterations, which is
  also where it landed; `HistoryBonusLin` and `HistoryMalusLin` touch their
  floors on 0.3 % and 0.2 %. S085's `RfpMinPly` sat at a bound for 72.5 % and
  that run was still read.

### What landed, and how each number got there

`src/search_params.hpp`'s eleven X-macro rows carry the fitted values, written
by a script that reads the driver's JSON rather than retyping it, and the built
tune binary's own `uci` option lines were then compared back against that same
JSON, all eleven equal. The four comment blocks that explain those defaults say
the value is this project's SPSA fit of 2026-09-14 on `UHO_4060_v3.epd`
(`adocs/data/S222_spsa_trajectory.tsv`) **and nothing else** -- no other
engine's number is behind any of them, and the seeds they replace are named as
the seeds they were (DEC-084 as amended by DEC-105). `HistPruneCoeff`'s block
keeps its old midpoint derivation as history and adds the check that the fitted
612 still sits inside that region at the fitted band, 138 to 1104.

`tests/test_search_params.cpp`'s golden table was re-derived from the engine's
own table rather than typed: the 44 rows were read back out of the built tune
binary's `uci` reply, compared name by name and in order against the golden
list, and only the numbers substituted, with every column width preserved. Its
GOLDEN block names S222's eleven beside S085's ten. **A deviation from the
brief worth stating**: the brief asked for that golden to be re-derived "with
its own script", and the golden's own site says the opposite -- "there is no
script and none is owed, because `src/search_params.hpp` is the derivation and
a diff of the two is the re-derivation". The site's rule was followed and the
mechanical route above taken as well, so both readings are satisfied; the site
is unchanged.

`MANUAL.md`'s eleven option rows were updated the same mechanical way, from the
binary, and four descriptions that had gone stale were rewritten: the malus no
longer "ships equal to the bonus", `ContHistWeight`'s row reads the fitted
spans instead of the seed's, `HistPruneCoeff`'s row no longer calls 612 a
midpoint, and `QuietHistoryMax`'s says where its value comes from.
`tools/plan_prose_check.py --params` flagged exactly the eleven table rows
before the edit and passes after it.

### Measurements

`chesso bench`: **5950740 -> 5685915**, -264825 nodes, **-4.5 %**. The commit
carries `Bench: 5685915`. Nothing was added to the search in this phase; a
differently graded history orders quiets differently, and every count
downstream of the order moves with it.

`tools/search_bench.py`, phases one and two -> fitted:

| depth | midgame | kiwipete | tactical |
|---|---|---|---|
| 9 | 74825 -> 51189, `c3d5` | 147048 -> 146616, `e2a6` | 25019 -> 39389, `d7c8q` |
| 12 | 184125 -> 143205, `c3d5` | 577720 -> 570238, `e2a6` | 97318 -> 148060, `d7c8q` |

The three positions disagree in direction -- two shrink, tactical grows by
half -- which is the ordinary signature of a reordering and another reason the
verdict is games. **Kiwipete's best move at depth 12 moves back to `e2a6`**,
where it sat before phase two moved it to `d5e6`; the other two are unchanged
at both depths. Node counts move by construction, so INV-6 is not available and
the SPRT is the only thing that can decide this step.

`tests/test_search.cpp` "ordering keeps the tree small" reads **17321** against
17332 at phase two and 17451 at the parent, 0.7 % inside a band of
`[3490, 69804]`, so its golden pair is not re-derived (DEC-142's own rule:
re-derive when the count leaves the middle half).

Debug self-play (DEC-141 clause 1): four rounds at 4+0.04, `noob_3moves.epd`,
concurrency 8 -- **8 games in 20 s, 0 `Assertion`, 0 `disconnect`**, on both the
`level=trace engine=true` log and the tee'd stdout.

Gate, both builds, `CLANG_FORMAT_MAJOR=22`: **39 of 39 in `build`, 39 of 39 in
`build-tune`, `./clang-format.sh --check` clean**. `tools/gate_extra.sh` is the
coordinator's; this phase changes eleven integers and four comment blocks in
`src/`, so the second tier's mutation pass and sanitizer run are re-run on the
landing commit by the coordinator, as phases one and two's were on `96fdc19`.

### The census on the fitted build, and the control that attributes it

`adocs/data/S024_census_run.py run` over its own 400 positions at depth 10,
Hash 16, on a Release build of this phase's `src/search_params.hpp` with
S024's five throwaway counters patched back in, built in a detached worktree
that was removed after the run. Output and the instrumentation diff:
`adocs/data/S222_census.txt`. **S024's own `adocs/data/S024_census.tsv` was not
touched** -- the script writes that path, so it was run from the worktree's
copy and never from this tree's.

| share | S024, 2026-09-12 | S222 fitted | S222 at the incumbent vector |
|---|---|---|---|
| writes with a previous move | 97.56 % | **96.62 %** | 96.66 % |
| quiet reads consulting the term | 96.19 % | **95.15 %** | 95.14 % |
| of those, reading non-zero | 27.14 % | **19.15 %** | 18.94 % |

The third figure fell eight points against S024 and **the fit is not what moved
it**, which is a measurement here and not an explanation: the same instrumented
binary was rebuilt with `bdd82cc`'s incumbent vector and the same 400 positions
re-run, and it reads 18.94 % -- so the fitted vector finds a non-zero entry
slightly *more* often, by 0.21 points, and the whole fall belongs to the tree.
That tree is a third of the size S024's census measured, 30474552 nodes against
112638096 over the same positions, because S109's pruning block and S091's
capture rules landed between the two. Per-position spread 2.04 % to 38.88 %,
median 17.12 %, so no handful of positions carries the average. The reading is
S024's: the table is consulted on nearly every quiet score and returns a real
number on a substantial share of those, so a verdict on the fitted vector is
not a verdict on inert wiring.

### The lane script's double marker, fixed

`adocs/data/S222_spsa.sh` ran `spsa_driver.py check` before the run and the
driver ends that stage with `SPSA-DONE` too -- one of its exit paths, and
DEC-061 asks every exit path for a marker -- so the run log carried one marker
before the first game and two at the end, and the watcher armed on the night
had to be told to count to two. Counting is what WATCHERS says a watcher must
not do. `check` now writes `${OUT}.check.log` and only its non-marker lines are
echoed into the run log, so the probe evidence still lands where it is read and
the first `SPSA-(DONE|FAILED)` in that log is the run's. A dated comment says
why, and **the header's pre-registration text is untouched**: it is the record
of the run that happened. Verified by running the script's own check path with
its run stage stubbed -- 0 markers in the run log, all eleven probe lines
present, the marker in the check log. The same run re-confirmed that all eleven
axes still reach the search from the fitted defaults, 11 of 11.

### The SPRT, pre-registered

`adocs/data/S222_sprt.sh`, modelled on `adocs/data/S091_sprt.sh`, written
before a game is played. `{0, 5}` nElo at `8+0.08`, `Hash 16`, concurrency 12,
`books/noob_3moves.epd` -- **not the lane's `UHO_4060_v3.epd`**, which is the
whole point of the two books (DEC-209 clause 1). Worst case 41861 games at the
interval's midpoint and 25591 on a bound: **19.5 h and 11.9 h** at the 2150
games an hour the five runs since S212 average, 19.8 h and 12.1 h at
`.moltke.local.md`'s 2110. Abort on time forfeits over 1.0 % on either side
(`tools/forfeit_report.py`); a crash or disconnect voids it and `fastchess.sh`
says so itself. **Open findings this run is taken while open: none** -- S211,
S213, S224, S225, S228, S229 and S230 all completed 2026-09-12 to 2026-09-14
and `adocs/plan.md`'s Open list carries no filler behind this entry.

`REF` defaulted to `bdd82cc`, the commit before this phase's landing, when
this section was written; **the coordinator re-pinned it to `d785b89`, the
tree before S222, before any game was played (DEC-210)**: the phase-two
landing took the SPRT path for INV-6 and was never a verdict, so measured
against it the table itself would stay unpriced, and the step's accepts --
"against the commit before it" -- means the commit before S222. `CAND`
defaults to `HEAD`, because the landing commit does not exist when the file is
written; the coordinator pins it to the sha after committing, and
`fastchess.sh`'s banner prints both shas with their dates before the first
game.

The three outcomes are written in that file. The one with work in it is H0:

- **H1** -- keep the whole vector, table and history scale together. The
  stopping figure is upward-biased and the claim is "at least 5 nElo"
  (DEC-063). The step's H1 clause then opens the two-ply table as its own step,
  behind S098.
- **H0** -- the scale was not the cause. The lane fitted it on its own axes,
  found the weight where the seed put it, and the vector built on it does not
  gain: that is the lane's fifth pre-registered reading and the step's accepts
  then binds, so **the technique leaves the plan with a decision that says
  why** (S005, S006, S015 are the precedent that a zero is recorded as a zero
  and may still be kept). **But the other eight axes have no verdict of their
  own.** The six plain-history coefficients, `QuietHistoryMax` and
  `HistPruneCoeff` were fitted in the same vector; an H0 over the sum says the
  eleven together do not clear 5 nElo, not that any one of them costs. **The
  reading proposed to the coordinator, who decides: keep the eight, revert the
  three.** The three continuation axes are inert once the table is gone and go
  with it; the eight are a fit of parameters this engine already shipped, over
  60000 of its own games, and reverting them to their seeds would be reverting
  a measurement to a guess on no evidence. What that costs is named and not
  hidden: the eight were fitted *with* the table present, so they are
  conditioned on a tree that would no longer exist, and keeping them without a
  verdict is a change of unknown sign. Two ways to close it, either acceptable:
  a second gainer SPRT of the eight alone against the pre-S222 commit, another
  night; or revert all eleven and let S127 -- which refits the whole parameter
  set after the block -- take them on the tree that ships. **What is not
  acceptable is keeping the eight and quoting this run as evidence for them.**
- **No verdict** -- recorded as zero, decided with the reason stated. The
  eight-axis question is open in the same terms.

### Documents

- `MANUAL.md`: eleven option defaults, four descriptions.
- `DEV_MANUAL.md`: the bench ledger gains `S222` phase three's `5685915` with
  what moved it; the S222 lane paragraph gains the run's measured 8 h 37 m and
  the check-log plumbing.
- `adocs/data/README.md`: six rows -- `S222_spsa.sh` (which phase two left
  without one), `S222_spsa_run.json`, `S222_spsa_trajectory.tsv`,
  `S222_spsa.log`, `S222_census.txt`, `S222_sprt.sh`.
- `tests/test_search_params.cpp`: the eleven golden defaults.
- `README.md`: human-owned, untouched.
- `adocs/specs.md`, `adocs/plan.md`, `adocs/status.md`, `adocs/decisions.md`:
  not edited (hard limit). Wording proposed below.

### Proposed `specs.md` amendment, for the coordinator

The search row's S222 passage ends "the defaults above are first settings and
S222's own SPSA lane ... fits them together with `QuietHistoryMax`, plain
history's six never-fitted coefficients and `HistPruneCoeff` before the gainer
SPRT decides the step." That clause is now history. Replacing it:

> The defaults above were first settings and **S222's own SPSA lane fitted
> them on 2026-09-14/15** -- `tools/spsa_s222.json`, `adocs/data/S222_spsa.sh`,
> eleven axes, 1250 iterations over 60000 games at 2+0.02 on
> `UHO_4060_v3.epd` so tuning and verification share no openings, 8 h 37 m,
> `adocs/data/S222_spsa_trajectory.tsv`. Every axis moved, so the run is not
> stuck and the SPRT is owed: `ContHistBonus` 15 -> 17, `ContHistMalus`
> 15 -> 18, `ContHistWeight` 25 -> 26, `QuietHistoryMax` 8192 -> 8831,
> `HistoryBonusQuad` 1 -> 6, `HistoryBonusLin` 0 -> 19, `HistoryBonusConst`
> 0 -> 2, `HistoryMalusQuad` 1 -> 0, `HistoryMalusLin` 0 -> 17,
> `HistoryMalusConst` 0 -> 36, `HistPruneCoeff` 576 -> 612 -- plain history's
> six coefficients and its band fitted for the first time in this project's
> history, and the bonus and malus split asymmetrically, the bonus quadratic
> at `6d^2 + 19d + 2` and the malus linear at `17d + 36`. **The weight landed
> near its seed**, which is the lane's own pre-registered reading that neither
> of DEC-194's suspects -- the doubled share, the shared bound -- is what the
> fit found, and the "at or under 5" row that would have owed a second
> attribution run is not triggered. `bench` 5950740 -> 5685915. The census on
> the fitted build reads 96.62 / 95.15 / 19.15 % against S024's 97.56 / 96.19 /
> 27.14, with a same-tree control at the incumbent vector at 18.94 % that
> attributes the fall to S109's and S091's pruning and not to the fit
> (`adocs/data/S222_census.txt`). One gainer SPRT `{0, 5}` at the harness
> regime against the commit before the landing decides the step
> (`adocs/data/S222_sprt.sh`).

**Ruled by the coordinator before any game, DEC-210, 2026-09-15**: the
proposal above -- keep the eight, revert the three -- is refused. Under H0 the
whole vector reverts with the table to the measured baseline `d785b89`,
because a tree carrying the eight without the three has been played by no run
and a change of unknown sign is not shipped on the argument that a fit beats a
guess; the eight axes' fit becomes a step of its own (a lane without the
table, then one SPRT), seeded from `adocs/data/S222_spsa_trajectory.tsv`.
`adocs/data/S222_sprt.sh`'s header carries that reading in place of the
proposal.

### Proposed decision, for the coordinator

A `DEC` is owed for two choices made here that a future reader would otherwise
re-derive: **(1)** that the eleven-axis fit lands as one vector under one
verdict rather than being attributed axis by axis, with S085's twelve-axis
precedent and the reason attribution is unavailable -- every axis prices the
same quiet ordering score; and **(2)** the H0 reading above, that the eight
never-before-fitted axes have no verdict of their own and what may and may not
be concluded about them. Both are stated in `adocs/data/S222_sprt.sh` before a
game is played, which is where the record has to be; the decision entry is the
coordinator's to write. **Written as DEC-210 on 2026-09-15**: (1) as proposed,
one vector under one verdict; (2) against the proposal, the whole vector
reverting under H0 and the eight axes filed as their own step; and the
reference re-pinned to the tree before S222.
