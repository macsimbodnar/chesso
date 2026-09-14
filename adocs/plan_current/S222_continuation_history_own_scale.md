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
done:

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
