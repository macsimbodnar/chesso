id:         S231
goal:       a two-ply continuation history table -- keyed on the move two plies back and this move -- beside S222's one-ply table, on its own scale, fitted in a narrow SPSA lane and decided by one gainer SPRT, because S222's H1 (DEC-210) is what the two-ply table was waiting on
accepts:    `cont_hist2[12][64][12][64]` (or the shape the implementing agent states) on `search_state_t`, keyed on the (piece, to) of the move two plies back and this move's, written at every quiet cutoff beside the one-ply table and summed into the quiet ordering score with its own `ContHist2Weight`, `ContHist2Bonus` and `ContHist2Malus` declared in `src/search_params.hpp` the way S222's are (the bound a definition, three axes -- DEC-209's parameterisation); the guard on the move two plies back existing (**ply 0, ply 1 and the node two plies after a null move pass none -- amended 2026-09-18 by DEC-219 from "the two nodes after a null move", which was true only read one table at a time; the node one ply after a pass does carry a two-ply key, because the pass consumes a ply and the table's parity survives it**), each guard with a sentinel case and a mutant `tools/mutation_check.py` kills; the band-clearance case re-stated for both weights at their declared maxima; **the fit first**: one narrow SPSA lane over the three new axes together with S222's three, on `UHO_4060_v3.epd`, pre-registered in its own script's header with the estimate from the measured throughput, its result recorded whatever it is; **then one gainer SPRT `{0, 5}`** nElo at the harness regime against the commit before the step's first landing -- DEC-210's reading: one vector under one verdict, H0 reverts the whole step -- pre-registered per DEC-143 with the fitted values named; `bench` line; Debug self-play; `adocs/data/S024_census_run.py` re-run with the second table counted; H1 keeps it, H0 records the zero and the two-ply idea leaves the plan with a decision saying why; after H1, the killer slots and then the countermove table are each measured for removal at `{-5, 0}`, one at a time, or the step states why not (DEC-222)
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp, src/search_params.hpp, tests/, tools/mutants/, tools/, adocs/data/
excludes:   a third ply; any change to S222's table or values except through the shared lane; S098's reading of the sum (S098 owns it); any constant from another engine (DEC-084, DEC-105)
decisions:  DEC-210, DEC-209, DEC-194, DEC-198, DEC-143, DEC-141, DEC-084, DEC-105, DEC-218, DEC-219, DEC-222
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator for phase one (DEC-185, DEC-199); the SPSA lane night and the SPRT are the coordinator's; started 2026-09-18 11:20 on the idle machine
done:       2026-09-20 10:57 -- built, fitted, measured and reverted on a zero. Phase one (`b83fb1d`, `4c727b2`, `af8b9f0`): `cont_hist2[12][64][12][64]` on `search_state_t`, keyed on the (piece, to) of the move two plies back and this move's, written at every quiet cutoff beside the one-ply table and summed into the quiet score on `ContHist2Weight`, with `ContHist2Bonus` and `ContHist2Malus` declared as S222's are (three axes, DEC-209); the guards at ply 0, ply 1 and the node two plies after a null move (DEC-219's wording), each with a sentinel case and a mutant, 4 of 4 killed by `tools/mutation_check.py`; the band-clearance case restated for both weights at their declared maxima (DEC-218). The fit first: the six-axis lane on `UHO_4060_v3.epd` (`adocs/data/S231_spsa.sh`, pre-registered), 60000 games in 8 h 40 m 45 s, 0 forfeits, every axis moved and none touched a bound, `ContHist2Weight` 26 -> 24 in the pre-registered near-26 reading; phase three (`55891bb`) landed the six defaults from the driver's JSON, `bench` 5443203 -> 4393575, the census re-run with the second table counted (`adocs/data/S231_census.txt`: two-ply 94.24 / 95.72 / 27.51 % with a same-tree control). Then one gainer SPRT `{0, 5}` nElo at the harness regime against `3a649c0`, pre-registered with the fitted values named (`adocs/data/S231_sprt.sh`): **H0, `Elo -2.65 +/- 4.82`, `nElo -3.40 +/- 6.20`, LLR -2.95, 12070 games in 5 h 37 m, 0 forfeits** (`b06a5c8`, the first commit carrying DEC-220's block, gate-checked against `adocs/data/S231_sprt.log`). The zero is recorded as a zero and the two-ply idea leaves the plan with DEC-224; `src/` returned to `3a649c0` whole in `9c5a38f`, `bench` 4646334 to the node and `search_bench` identical at depths 9 and 12 against a `3a649c0` worktree, so the one-ply three are S222's fitted 17 / 18 / 26 again and nothing is stranded; the killer and countermove removals DEC-222 attached to an H1 are not owed. Debug self-play 8 games and 0 `Assertion` at phase one and again on `55891bb`; `gate_extra` 5 stages green on `55891bb` 2026-09-20 (the weekly). Goldens: `test_search_params` re-derived from the tune binary at each landing; the S192 node-budget golden re-derived by its script on the reverted tree (69804 / 3490 -> 65024 / 3251), the trigger having stood since S222. Fast checks on phase one, phase three and the record, each earning its keep; the revert verified byte-identical by the coordinator. Tier-1 gate at completion: 40 of 40 in both builds and format clean at `9c5a38f`. Two findings left as filler, test-side, no reach into play: phase one recorded 17321 for `3a649c0`'s node-budget count where the byte-identical tree reads 16256; `adocs/data/S192_node_budget.py`'s drift line reads OUTSIDE for a band it has just derived. S231's I03 mutant gap went with the code.
## Why this exists

S222's accepts: "H1 keeps it and opens the two-ply table as its own follow-up
step". Its SPRT read H1 on 2026-09-15 -- `Elo 11.13 +/- 6.90` over 6278
games against the tree before S222, one vector under one verdict (DEC-210).
The published record treats the two-ply table as the second half of the same
idea; whether it transfers here is the measurement and not the record
(DEC-019). Placed behind S098, as S222's pre-registration required: S098
scales its reduction by the sum these tables make and is measured against the
tree S222 left, and a second table landing first would move that baseline
under it.

## Cost

Agent work, about a day for the table, its guards, tests and mutants; a lane
night of about nine hours at S085's regime (`.moltke.local.md`'s 24 to 25 s an
iteration); the SPRT up to twenty hours worst case (41861 games at 2150 an
hour). Memory: a second 1.125 MiB table on `search_state_t`, heap-owned since
S222's fast check moved the struct off the thread stack.

## Phase one landed 2026-09-18

Built by an Opus 5 subagent on the machine the coordinator holds (DEC-185,
DEC-199). The step stays in `plan_current/` and `done:` is empty: what completes
it is the lane and then the SPRT, and both are the coordinator's.

**The SPRT's reference is `3a649c0`**, the tree before this step's first
landing, and it is recorded here so phase three cannot get it wrong. One vector
under one verdict is DEC-210's reading and this step is written to it: the
table, its three axes and whatever the lane does to S222's three are judged
together, and H0 reverts `src/` to `3a649c0` whole.

### The shape, and the one number that is not `accepts`'s

`accepts` names `cont_hist2[12][64][12][64]` and that is what shipped, on
`search_state_t`, `int16_t`, 1.125 MiB, a value member beside `cont_hist` so
the zeroed rebuild at the top of every `iterative_deepening_search()` clears it
with the other four tables and `ucinewgame` needs no clear of its own. The
struct is now **2446920 bytes, 2.33 MiB**, measured and not estimated, against
1267272 with one table; and the heap ownership S222's fast check imposed is
what keeps that legal. `iterative_deepening_search()` and `tools/datagen.cpp`'s
`run_search()` already own theirs through a `unique_ptr`, which is the property
the brief asked to keep true, and it is true unchanged -- neither call site
needed editing. The tests build theirs on the main thread, 8 MiB on both
platforms, and the deepest case holds two at once -- `guard_fixture_t`'s member
and a `search_fen()` local -- for 4.7 MiB. That is inside 8 MiB and the whole
suite is green in both builds, but it is **half the headroom gone in one
step**, so the next table on this struct is the one that has to be a pointer.
Stated here rather than found later.

One index, `continuation2_entry()` in `src/data_structures.hpp`, a second
function rather than a table argument on `continuation_entry()`: a call site
naming the wrong table then names the wrong *function* and reads as one, and
the write in `history_on_quiet_cutoff` and the read in `quiet_history_sum`
cannot disagree about which pair comes first.

### The plumbing: a parameter, not a per-ply stack

`negamax_at` takes `prev_move2` as a trailing parameter and every recursion
states it; the two entry points `negamax` and `negamax_probed` default it to 0,
which is `cut_node`'s own precedent from S098 verdict 2 -- defaulted there and
nowhere else.

**Why not a move stack on `search_state_t`.** A parameter costs no read. Every
node already holds its own `prev_move` and hands it down as its child's
`prev_move2`, so the two-ply key is produced by an argument the recursion was
already building; a `move_t moves_played[MAX_PLY]` would add a store per move
made *and* a load per node on top of it, and the field beside these is the one
`search_state_t` records at **1.49 % of nodes per second for exactly one read
per node** (`probe`, S191). The brief asked for the cheaper one and this is it.

**Behaviour-identical for every existing caller, and that is measured and not
argued.** With `ContHist2Weight` compiled at 0 the tree is `3a649c0`'s to the
node: `chesso bench` **4646334**, exactly HEAD's total. Every test that drives
`negamax`, `negamax_probed` or `score_move` without the new argument therefore
searched the same tree it always did; the parameter alone moves nothing.

### The guards, and the reading the null move forces

Three classes pass no two-ply key: **ply 0**, **ply 1**, and **the node two
plies after a null move**. The third falls out of the plumbing for free -- the
pass hands its child a 0 `prev_move`, and that child hands the 0 on as its own
child's `prev_move2` -- which is one reason the parameter is the right shape.

**The node one ply after the pass is not in that list, and stating the reading
is the point.** `accepts` says "the two nodes after a null move pass none", and
that is true read one table at a time: the first passes none to the *one-ply*
table (its `prev_move` is 0, S222's own guard) and the second passes none to
the *two-ply* table. What the first node does pass is a real two-ply key, its
parent's `prev_move` -- and that is correct rather than tolerated, because the
pass consumes a ply and therefore **preserves the parity this table rests on**:
at a node where side S is to move, the move two plies back is S's own, and
under the pass the side to move is again the side whose move two plies back it
is. The table stays a follow-up table across a null move. The alternative
reading -- zero the key at the null child too -- throws a legitimate,
parity-correct cell away, and it is what mutant `I03_null_child_drops_prev2`
does.

0 decodes to the legitimate `(W_PAWN, a8)` cell for this index as for the
other, so a dropped guard is a silent wrong-cell write and not a crash. Every
guard therefore has a sentinel that scans a whole table or a whole row, never
one cell.

### The three axes

| symbol | UCI name | default | range |
|---|---|---|---|
| `CONT_HIST2_BONUS` | `ContHist2Bonus` | 17 | 0 to 1000 |
| `CONT_HIST2_MALUS` | `ContHist2Malus` | 18 | 0 to 1000 |
| `CONT_HIST2_WEIGHT` | `ContHist2Weight` | 26 | 0 to 1000 |

Three and not four, for DEC-209's reason unchanged: a bound and a weight over
one table are one degree of freedom, so the bound stays the definition
`CONT_HIST_BOUND` and **no second bound is added**. The unit is
`ContHistBonus`'s unit -- thousandths of the table's own band at
`CONT_HIST_REF_DEPTH` 11 -- so the two pairs of shares are directly comparable
and the lane can move one against the other.

Ranges by stated purpose, identical to the one-ply axes' for that reason. 0 is
a true off value on all three: nothing is written and the ordering term is
identically 0. 1000 closes one whole band in a single update at the reference
depth for the two shares, past which the clamp inside `history_gravity_update`
makes every larger value the same engine. 1000 on the weight is the **shared**
band-clearance ceiling; the next section is its arithmetic.

**Seeds, DEC-084 as amended by DEC-105, all three (b) -- a derivation over
chesso's own numbers.** No engine's constant is behind any of them, none was
read off a table, a wiki page or a release note, and none was seeded from a
published figure.

- `ContHist2Bonus` **17** and `ContHist2Malus` **18** are the one-ply table's
  own fitted shares. The two tables are written at the same call site, over the
  same two spans, at the same reference depth, into bands of the same width, so
  "start at the same rate as the table it sits beside" is the derivation S222
  itself used against plain history -- with the rate now this project's own
  SPSA fit of 2026-09-14 rather than a shipped seed. The lane moves all six.
- `ContHist2Weight` **26** is equal authority with the term beside it: at 26 the
  two-ply term spans `26 * 32767 / 100 = 8519`, exactly what the one-ply term
  spans at its fitted 26, against plain history's fitted `QuietHistoryMax` of
  8831. The quiet band therefore ships as **8831 / 8519 / 8519** over its three
  terms.

**Not 0, and that is the coordinator's ruling followed rather than a choice
made here.** An SPSA axis that starts at a bound is a known pathology --
S085's `RfpMinPly` sat at one for 72.5 % of that run's iterations -- and the
lane exists to fit this weight. The consequence is stated rather than hidden:
phase one's landing **alters play**, `bench` moves, nothing in this step claims
otherwise, and the SPRT after the lane is what decides it.

### The band-clearance one-way door, and the decision the coordinator has to record

This is the trap the brief named and the arithmetic is the whole argument.

Once a second weighted term enters the sum the quiet band is
`[-(QuietHistoryMax + (w1 + w2) * CONT_HIST_BOUND / 100), +the same]` and it
must stand **100 clear of the countermove band at 700000**. The clearance is a
property of the two weights' **total**, not of either one. At
`ContHistWeight`'s old declared 2000 the one-ply term alone spans 655340, the
band is 688107 and the clearance is 11893 -- so a second weight had 11893 of
band to declare a ceiling inside, which is no useful range at all.

**Resolution, verified arithmetically: halve the band-clearance ceiling of each
weight. `ContHistWeight` 0..2000 becomes 0..1000, and `ContHist2Weight` is
declared 0..1000.**

    32767 + 1000 * 32767 / 100 + 1000 * 32767 / 100
      = 32767 + 327670 + 327670
      = 688107          against 700000, a clearance of 11893

which is **exactly the number the single 2000 ceiling gave**, because 2000 and
1000 + 1000 span the same 655340. At 1050 each the band would be **720873** --
`32767 + 344053 + 344053`, each weighted term through its own integer division
because that is how `quiet_history_sum` takes it, and not the 720874 that
combining the two weights first and dividing once would give -- and would
swallow the countermove and both killers, which is the same edge S222's comment
names at 2100.

That ceiling was never a statement about where good values lie --
`search_params.hpp` calls it "the band-clearance ceiling" and nothing else --
so with two tables sharing one band the per-axis share of it is naturally
halved. **The fitted 26 is untouched**, which is what this step's `excludes`
protects: only the declared maximum moves.

**Proposed decision, for the coordinator to record; agents do not write
`adocs/decisions.md`.** Two clauses:

1. `ContHistWeight`'s declared maximum is halved from 2000 to 1000 because the
   quiet band now carries two weighted terms and the band-clearance ceiling is
   a property of their sum. The compiled default and every fitted value are
   untouched; `tests/test_evaluation.cpp` "the declared history ceiling clears
   the band above it" asserts the clearance at **both** weights' declared
   maxima at once and reads the same 11893 it read before.
2. The consequence, which is correct and is not to be repaired:
   `tools/spsa_s222.json` declares that axis 0 to 2000 and
   `spsa_driver.py check` compares a config's bounds against the binary's, so
   running `check` on that config would now refuse it by name. It is the frozen
   record of a run already taken, not a template, and it is **not edited**.
   `src/search_params.hpp` says so at the axis and `adocs/data/S231_spsa.sh`
   repeats it, so a mid-run read does not mistake it for a defect.

### Tests, and what each one is for

Every new assertion is a scan of a whole table or a whole row, because the
failure a dropped guard causes is a write to the wrong cell.

| case | what it pins |
|---|---|
| "a quiet cutoff two plies into the tree grades the two-ply continuation table" (new) | the bonus and the malus spans on the two-ply table's own key; that the key is the move two plies back and **not** the previous move (asserted directly, `continuation2_entry(&state, prev_move, cutoff) == 0`); another key's row untouched; exactly four cells in each table from one call |
| "a quiet cutoff with a previous move grades the continuation table" (extended) | the **ply-1 sentinel**: a previous move and no move two plies back writes four one-ply cells and **zero** two-ply cells. The four are the counted precondition |
| "a quiet cutoff maluses the quiets tried before it" (extended) | the ply-0 sentinel for both tables at once; the five butterfly cells are the counted precondition |
| "a quiet move that gives check enters the ordering tables" (extended) | the driven ply-1 sentinel: `negamax` at ply 1 leaves the two-ply table empty while the one-ply cell moved, and every child is quiescence, which writes nothing |
| "the cutoff move is credited and the quiets before it are charged" (extended) | the driven ply-0 sentinel for the two-ply table |
| "the node after a null move has no previous move to index" (extended) | the `(W_PAWN, a8)` row of the two-ply table is empty through the **real** null-move block, with a counted precondition that the table was written elsewhere in the drive |
| "the node two plies after a null move has no move two plies back to index" (new) | the guard itself, driven at the null child with the exact preconditions below |
| "a search fills the ordering tables" (extended) | the two-ply table is filled by real play: a depth-8 search that leaves it empty has plumbing that never reaches it |
| "the declared history ceiling clears the band above it" (rewritten) | all three tables driven to their bounds at once, the compiled band asserted **exactly** at both edges, and the clearance asserted at every declared maximum at once |

**The null-move case's preconditions are exact and were measured, not
assumed.** The drive makes the pass by hand and calls `negamax` with the
arguments `negamax_at` gives its null child -- 0 for the previous move,
`PREV_MOVE` for the two-ply key -- at depth 3 from ply 2. After the pass White
is to move, so a **one-ply** cell keyed on a *White* mover can only have been
written by a node at ply 3, 5, 7 ...; at this depth real search nodes exist at
plies 2, 3 and 4 and everything at ply 5 is quiescence, which writes no
history. So a White-keyed one-ply cell is a quiet cutoff at a node two plies
after the pass, and the count of them is the precondition. Measured in this
tree: **3** at depth 3, 3 at depth 4, 8 at depth 5, and 0 at depths 1 and 2 --
which is why the depth is 3 and not 2. The driven node writes no one-ply cell
of its own, its previous move being 0, so none of the three is its. A further
precondition asserts all eight White pawns still on the second rank, so no
White pawn can legitimately key either table on `(W_PAWN, a8)`: reaching a8
takes six pawn moves and this drive gives White two.

### Mutants

`tools/mutants/S231_continuation_history2.py`, four, in the shape of S222's
four one ply lower. `load_mutants` and the anchor check pass against the
working tree, every anchor unique.

| mutant | what it breaks |
|---|---|
| `I01_cont_hist2_malus_sign` | the two-ply malus is credited instead of charged |
| `I02_cont_hist2_no_prev2_guard` | the guard on the move two plies back is dropped, so ply 0, ply 1 and every node two plies after a pass write the `(W_PAWN, a8)` cell |
| `I03_null_child_drops_prev2` | the null child is handed 0 as its two-ply key instead of the move before the pass -- the opposite reading of the null move to the one this step took, and a silent one |
| `I04_cont_hist2_unread` | the read term leaves `quiet_history_sum`, so the table is written at every cutoff and orders nothing: exercised and inert, the shape DEC-194 needed a census to rule out for the first table |

**All four killed, on a linked worktree at `b83fb1d`, and each verdict is the
tool's own** (`.tuning/mutation_s231.log`, `.tuning/mutation_s231_retry.log`,
`.ref-builds/mut/build/mutation/results.tsv`). `MUTATION-RUN-DONE`, score
100 %.

| mutant | verdict | fast | bench | killed by, named | s |
|---|---|---|---|---|---|
| `I01_cont_hist2_malus_sign` | killed | 1/39 | moved | "a quiet cutoff two plies into the tree grades the two-ply continuation table", `CHECK( continuation2_entry(&state, prev_move2, move) < 0 )` and two more | 131.7 |
| `I02_cont_hist2_no_prev2_guard` | killed | 1/39 | **same** | five cases at once -- "a quiet move that gives check enters the ordering tables", "a quiet cutoff maluses the quiets tried before it", "a quiet cutoff with a previous move grades the continuation table", "the cutoff move is credited and the quiets before it are charged", each `CHECK_EQ( continuation2_entries(state), 0 )`, and "the node two plies after a null move has no move two plies back to index", `CHECK_EQ( under_null_key, 0 )` | 129.0 |
| `I03_null_child_drops_prev2` | killed | 1/39 | moved | **only** "pruning does not hide a forced mate", `REQUIRE( result.mate_found )` | 127.9 |
| `I04_cont_hist2_unread` | killed | 2/39 | moved | "the declared history ceiling clears the band above it", `REQUIRE_EQ( s_history, live_span )`, and the mate case | 151.3 |

**I02's row is the one the step is built for.** Its bench signature is
*unchanged* -- the dropped guard writes the `(W_PAWN, a8)` cell on no bench
position -- so nothing but a sentinel that scans a table or a row can see it,
which is exactly the argument the guards section makes and now a measurement
rather than an argument. Five cases catch it.

**I03's row is a gap and is recorded as one rather than dressed up.** The only
case that kills it is `capture_mates`, and that table's own depths are a
measurement that moves under every ordering change -- DEC-209 clause 4 is the
ruling that an incidental kill there is not a guard. So the reading of the null
move that this step chose, and that `I03` breaks, **has no direct guard test**:
the case written for the null move (`under_null_key`) asserts the *negative*
two plies after the pass, which `I03` leaves true. A direct guard needs a drive
in which the null child is the only writer of the two-ply table, which needs
the null search to fail high so the drive node returns before its own move
loop; the drive this step has does not, measured (`probe.move_count` 14 at
every depth 5 to 11 in that position), so the null child's write cannot be
told from an ordinary ply-2 node's. **Proposed to the coordinator as filler
behind the next strength step**, not fixed here: one change at a time, and the
mutant does die today.

**Running them also found a real defect in S222's registry, which is the most
useful thing this pass did.** `I04` came back **stillborn** on its first run --
it does not compile, because deleting the two-ply read orphans the
`prev_move2` parameter and `-Werror=unused-parameter` refuses the build. That
is the class the tool's own docstring describes and the fix is the `(void)` it
prescribes.

**The same argument applies one table over, and that is the finding: S222's
`H04_cont_hist_unread` was silently disarmed by this step.** Adding a second
guarded term to `quiet_history_sum` means that deleting the *first* one now
orphans `prev_move` in exactly the same way, so `H04` stopped compiling the
moment S231 landed.

**Which of the three ways a mutant can go dead this was, stated precisely,
because it decides the repair.** Not the first: the code `H04` targets is
still there, character for character -- the one-ply read is exactly where it
was. Not the second: no case lost its reach, and the proof is that once the
mutant builds again it dies by **two** named cases and not one. It is the
third and least visible way -- **the mutant stopped being buildable, so it
never ran at all**, and `stillborn` is not a failure the suite reports as a
hole. It would have sat there looking like part of the registry indefinitely.

**So it is not an equivalence and the S098 leg-1 qualification precedent does
not apply here.** That precedent is for a mutant that builds, runs and changes
nothing observable on one configuration; this one changed plenty and was never
given the chance to. Nothing is deleted and nothing is qualified: both mutants
gain the `(void)` of the parameter they orphan, and both were then **re-run
and observed**, not assumed --

| mutant | verdict | fast | bench | killed by, named | s |
|---|---|---|---|---|---|
| `H04_cont_hist_unread` (S222's, repaired) | killed | 2/39 | moved | "the declared history ceiling clears the band above it", `REQUIRE_EQ( s_history, live_span )`, and "pruning does not hide a forced mate" | 152.8 |
| `I04_cont_hist2_unread` | killed | 2/39 | moved | the same two | 151.3 |

Both registries' notes now say when the `(void)` arrived and why, so the next
step that adds a term to this function finds the trap written down instead of
walking into it. **This is the second edit this step makes to another step's
mutant file and it is a repair, not a drive-by: without it S222's registry is
broken by S231's code.**

### Two fixtures re-derived, both by their own scripts, neither relaxed

The two-ply table reorders every quiet move, which is "any change to ordering",
and two pinned measurements moved with it. Both were re-derived mechanically
and both are recorded here with the numbers either way.

**`capture_mates`, all four rows, the seven sweeps taken again.** The case went
red on row 2 at depth 8. The whole pass was re-taken rather than that row
re-picked -- the shipped build and all six `S091_capture_see` mutants, depths 3
to 12, over `adocs/data/S230_table_fens.txt`, driven by
`adocs/data/S230_mine_r01_row.py depths`, evidence in
`.tuning/coord/S231_capture_mates/` -- and the table's own rule applied by
`.tuning/coord/S230_leg1_rows.py` rather than by eye. Shipped profiles here:
`d8 d9 d10 d11 d12`, `d9 d10 d11 d12`, `d10 d11 d12`, `d9 d10 d11 d12`.

| row | depth before | depth after | label before | label after |
|---|---|---|---|---|
| `3krb1r/Np2pppp/...` | 9 | **8** | no S091 mutant | **C02, C05, C07 and R02** |
| `2b5/4k2P/...` | 8 | **9** | C02, C05, C07 and R02 | **no S091 mutant** |
| `3N1bk1/3Q3p/...` | 11 | **10** | C07 and R02 | **C05 and R02** |
| `1r3r1k/2p1n1pp/...` | 10 | **9** | R02 | R02 |

All four depths moved and **no mate distance did**. Rows 1 and 2 very nearly
swapped roles. R01 is separated by no row at any depth of this pass either,
which is the fourth consecutive pass reading that way.

**"a reduced move that beats alpha is searched again": depth 6 to 4.** The
fixture is a measurement -- the case's own comment says the position "was found
by scanning the 400 committed census positions rather than picked" -- and no
script existed, which DEC-142 asks for. `adocs/data/S231_research_witness.py`
is that script: it drives this node line for line over the same 400 committed
positions and applies a rule stated **before** its own sweep -- keep the
position if it still re-searches somewhere in depths 4 to 8 and take the lowest
such depth, otherwise take the first position of the corpus that re-searches at
every one of them.

`researched` on the pinned position, over depths 4 to 8, measured both ways in
this tree (the parent's column is this tree with `ContHist2Weight` 0, which is
the parent's engine to the node, `bench` 4646334):

| depth | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|
| parent | 7 | 2 | **4** | 3 | 2 |
| here | 7 | 2 | **0** | 4 | 3 |

Depth 6 is the one depth of the five that emptied and it was the one the case
pinned. Rule 1 applies, the position is kept, and the depth is 4 -- where 7 of
23 reduced moves are searched again, the **largest** count in the swept range
and not the smallest, which is what a depth picked to scrape past would look
like.

**"ordering keeps the tree small" is not re-derived and that is the rule, not
an exemption.** It reads **16256** against 17321 at the parent, -6.1 % against
the 17451 the band `[3490, 69804]` was derived from, so the count has not left
the middle half and DEC-142's trigger has not fired. S222 recorded the same
reading at 17332 for the same reason.

### Measurements

`chesso bench`: **4646334 -> 5443203**, +796869 nodes, **+17.2 %**. The commit
carries `Bench: 5443203`. **A bigger tree and not a smaller one, which is the
opposite sign to S222's landing and is the single number the coordinator should
carry into the lane.** S222's table took 5.1 % off the bench; this one puts
17.2 % on. A second history term reorders quiets again and every count
downstream of the order moves with it, so the sign is a statement about the
seeded weight and not about the wiring -- and the wiring is pinned by the
grading case, which asserts the two-ply table is keyed on the move two plies
back and not on the previous move, and by the parity the guard section sets
out. What it does suggest is that **equal authority may be more authority than
the second table earns**, which is precisely the question `ContHist2Weight` is
in the lane to answer and which the lane's "at or under 5" reading is
pre-registered for. It is not evidence either way about strength: only games
say whether the tree is a better one (DEC-019), and a bigger tree at the same
depth is not the same thing as a worse move.

`tools/search_bench.py`, parent (weight 0) -> here:

| depth | midgame | kiwipete | tactical |
|---|---|---|---|
| 9 | 21995 -> 43974, `g5f6` -> **`c3d5`** | 104682 -> 104700, `e2a6` | 29842 -> 29323, `d7c8q` |
| 12 | 155612 -> 167581, `c3d5` | 683624 -> 648155, `e2a6` | 152138 -> 128534, `d7c8q` |

The three positions disagree in direction, which is the ordinary signature of a
reordering, and **midgame's best move at depth 9 moves**, from `g5f6` to
`c3d5` -- the move it already played at depth 12 on both trees.

Node counts move by construction, so INV-6's discharge is not available and the
SPRT is the only thing that can decide this step.

Throughput, interleaved pairs on the machine at the load `.moltke.local.md`
describes:

before 3736998, 3728551, 3717272, 3875452, 3670493, 3707966
after  3717778, 3564005, 3576959, 3576527, 3555427, 3577747

Paired, that is **-3.84 % of nodes per second with a standard error of
0.95 %** over the six pairs (per-pair -0.51, -4.41, -3.77, -7.71, -3.13,
-3.51), and the group means are 3739455 against 3594740, -3.87 %. It is
S222's own 3.6 % shape repeated for the same three causes: one more dependent
load per scored quiet, one more graded update per cutoff, and a second
1.125 MiB table. **Two caveats, both stated rather than buried.** The groups
overlap -- pair 1's after-reading is above the slowest before-reading -- so the
paired form is the one to read, not the group minima. And **the machine was not
idle**: the load average was 2.0 on twelve threads for the whole run, which
CLAUDE.md's rule 4 says to check first and which is why the figure is quoted
with its interval and not as a point. It is outside the 3 % noise line either
way.

### The census driver

`adocs/data/S024_census_run.py` counts the second table. The census line's
counters go from five to **eight**, the three new ones **appended** --
`writes_with_prev2`, `reads_with_prev2`, `reads_nonzero2` -- so a five-counter
line stays exactly what it always was and S024's and S222's runs remain
reproducible from their own instrumentation. `parse_go_output` accepts five or
eight and **refuses anything else** rather than misreading a share; the two-ply
shares are printed only when the binary counted them, because three zeroes from
an uninstrumented binary would read as an inert table. The TSV gains three
columns and its header explains them. Smoke-tested on synthetic census lines of
all three shapes: five parsed, eight parsed, three refused by name.

**Not run here**, as the brief says: the run belongs to phase three, on the
fitted build, and the instrumentation itself is a throwaway patch applied in a
detached worktree. The command, from a worktree copy of the script so that
`adocs/data/S024_census.tsv` is not clobbered (what S222 did):

    ~/.venv/chess/bin/python adocs/data/S024_census_run.py run <instrumented Release binary>

### The lane, written and checked, not run

`tools/spsa_s231.json`: **six axes** -- `ContHistBonus`, `ContHistMalus`,
`ContHistWeight`, `ContHist2Bonus`, `ContHist2Malus`, `ContHist2Weight` -- at
S085's regime, 1250 iterations x 24 pairs = 60000 games at 2+0.02, Hash 16,
concurrency 12, `r_end` 0.004, **seed 231** (this step's id, traceable; a seed
chosen for its trajectory would be a result chosen after the number), book
`books/UHO_4060_v3.epd`, adjudication `fastchess.sh`'s line verbatim including
`twosided=true`. No `Tm*` axis and no `TmHardPercent` (DEC-094, DEC-200), and
no second bound (DEC-209's gauge argument applied unchanged).

**Plain history's six coefficients, `QuietHistoryMax` and `HistPruneCoeff` are
deliberately not in this lane** and the config says so: S222's lane fitted them
on 2026-09-14 over 60000 of this project's own games and S127 refits everything
after the block, so carrying them here would spend the night's resolution on
axes that already have a fit and widen the vector one SPRT has to carry. The
one-ply three **are** in it, because the two tables share one quiet band and
one reference depth and a lane that pinned one pair would be fitting a ratio it
had fixed by hand.

`adocs/data/S231_spsa.sh` is the pre-registration and the runner, `OUT` under
`.tuning/`, `check` writing its own log so the first `SPSA-(DONE|FAILED)` in
the run log is the run's. **Estimate 8 h 45 m, ceiling 18 h**, from measured
throughput and not a guess (RUNS, DEC-155): S222's lane ran this exact shape on
this machine in **8 h 37 m 31 s**, 24.84 s an iteration, and the axis count
does not enter because SPSA plays two evaluations an iteration whatever the
dimension; what can move the wall is game length, since a game at a fixed clock
costs `2 * (base + moves * inc)` whatever the engine's speed. Cross-checked the
other way from `.moltke.local.md`'s 2110 games an hour at 8+0.08, which prices
60000 games at 2+0.02 at about 7.1 h -- lower, so the estimate is the
conservative one. Past DEC-155's four-hour line: a night.

The abort rule, the mid-run reads and **six pre-registered readings** are in
that header, written before a game is played. Two of them differ from S222's on
purpose and the difference matters:

- **A stuck lane still owes the SPRT.** S222's header said a rounded vector
  equal to the incumbent owes no run; here the reference is the tree *before*
  the step, so what the run prices is the table itself and not the lane's
  movement.
- **An H0 reverts cleanly.** S222's H0 would have stranded eight never-fitted
  axes without a verdict; here the six are three new ones and three whose
  pre-S231 values are exactly what `3a649c0` carries, so `src/` goes back
  whole and nothing is left unmeasured.

The `ContHist2Weight` readings are pre-registered at **at or under 5**, **near
26** and **well above 26**, with the "at or under 5 and the SPRT says H1" row
owing a second attribution SPRT of the fitted vector against itself with the
weight pinned at 0 -- the threshold fixed before the fit so the reading is not
chosen after the number.

`python3 tools/spsa_driver.py check tools/spsa_s231.json --engine
build-tune/src/chesso`, at a load average of 1.05 on twelve threads, no
warnings and nothing refused:

```
setoption reaches the search: ContHistBonus 0 -> 57385 nodes, 1000 -> 42747 (depth 9, midgame)
setoption reaches the search: ContHistMalus 0 -> 43938 nodes, 1000 -> 41575 (depth 9, midgame)
setoption reaches the search: ContHistWeight 0 -> 64438 nodes, 1000 -> 42209 (depth 9, midgame)
setoption reaches the search: ContHist2Bonus 0 -> 43971 nodes, 1000 -> 40135 (depth 9, midgame)
setoption reaches the search: ContHist2Malus 0 -> 22002 nodes, 1000 -> 41681 (depth 9, midgame)
setoption reaches the search: ContHist2Weight 0 -> 21995 nodes, 1000 -> 38200 (depth 9, midgame)
probed 6 of 6 parameters in 0.3 s
6 parameters, 1250 iterations x 24 pairs = 30000 pairs, 60000 games
SPSA-DONE
```

`ContHist2Weight` at 0 searches 21995 nodes on the probe's own position, which
is the parent's depth-9 midgame count exactly -- the same equality `bench`
shows, read from a third place.

### Documents

- `MANUAL.md`: three option rows added; `ContHistWeight`'s range and its
  sentence rewritten for the halved ceiling and the three-term sum.
- `DEV_MANUAL.md`: the S231 lane documented beside S085's and S222's; the bench
  ledger gains this landing.
- `adocs/data/README.md`: rows for `S231_spsa.sh` and
  `S231_research_witness.py`.
- `tests/test_search_params.cpp`: the golden table goes 50 rows to 53 and
  `ContHistWeight`'s maximum with it. Re-derived the way that GOLDEN block's
  own site defines -- `src/search_params.hpp` is the derivation and a diff is
  the re-derivation -- and cross-checked mechanically besides: all 53 rows read
  back out of the built tune binary's `uci` reply and compared name, default,
  min and max, in order, against the golden list. They match row for row.
- `tests/test_uci_surface.cpp`: no edit owed. Its tune option lines are
  generated from the same table and its golden is the **release** build's count
  of 5, which the tune-only axes do not touch.
- `README.md`: human-owned, untouched.
- `adocs/specs.md`, `adocs/plan.md`, `adocs/status.md`, `adocs/decisions.md`:
  not edited (hard limit). The wording `specs.md` needs is proposed below.
  **One sentence in its search row is now false and this is exactly which
  one**, in the S222 passage:

  > "`ContHistWeight` decides how much of the quiet band the term spans, so the
  > band is `[-(QuietHistoryMax + ContHistWeight x 32767 / 100), +the same]`
  > and still clears the countermove band by 100 at both declared maxima --
  > 688107 against 700000, asserted at both edges."

  Two clauses of it: the band now carries a third term, and there are three
  declared maxima and not two. **688107 is still exactly right**, which is the
  point of the halving and is why the sentence reads plausibly at a glance.
  Nothing else in `specs.md` moved; the S222 lane's eleven fitted values are
  untouched and its own sentences stay true.

### What is still owed, and by whom

- **`tools/mutation_check.py` over the four mutants**, on a linked worktree at
  the commit that carries this work. **One trap found doing it, worth knowing
  before the next run:** the tool refuses at "the unmutated worktree is red"
  unless `CLANG_FORMAT_MAJOR=22` is exported into it, because
  `test_clang_format_script` is part of the fast suite it runs and this machine
  cannot supply the pinned major 23 (DEC-146, `.moltke.local.md`). `nohup`
  does not carry an exported variable from an interactive shell into a
  detached child on its own, so the invocation is
  `nohup env CLANG_FORMAT_MAJOR=22 python3 tools/mutation_check.py ...`. The
  refusal is correct behaviour -- a red baseline would make every mutant look
  killed -- and it cost one baseline build to find.
- **Debug self-play, DEC-141 clause 1**, which is a match and therefore the
  coordinator's. The exact command is in the report.
- **`tools/gate_extra.sh`**, owed before the step *completes* -- phase three,
  not phase one.
- **The lane**, then the census re-run on the fitted build, then the SPRT
  against `3a649c0`.

### Proposed `specs.md` wording, for the coordinator

For the **search** row, after the S222 passage:

> **A two-ply continuation history table joined it on 2026-09-18, S231**:
> `cont_hist2[12][64][12][64]` on `search_state_t`, keyed on the (piece, to) of
> the move **two** plies back and this move's, reached through one helper
> `continuation2_entry`, written at every quiet cutoff beside the one-ply table
> and summed into `score_move`'s quiet return on a weight of its own. The move
> two plies back reaches a node as a parameter of `negamax_at` and not as a
> per-ply stack, because a parameter costs no read: every node hands its own
> `prev_move` down as its child's `prev_move2`. `ContHist2Bonus` and
> `ContHist2Malus` grade it in the same thousandths of the same band at the
> same median depth 11, its bound is the same definition `CONT_HIST_BOUND`
> rather than a second setting, and `ContHist2Weight` decides how much of the
> quiet band it spans -- so the band is
> `[-(QuietHistoryMax + (ContHistWeight + ContHist2Weight) x 32767 / 100),
> +the same]` and still clears the countermove band by 100 at every declared
> maximum at once: **688107 against 700000, a clearance of 11893, the same
> number one weight gave at 2000**, because `ContHistWeight`'s ceiling was
> halved to 1000 when the second weight was declared at 1000. Three axes and
> not four, DEC-209's gauge argument unchanged. Ply 0, ply 1 and the node
> **two** plies after a null move pass no key and touch nothing, each asserted
> with the mutant that breaks it; the node **one** ply after a pass does pass a
> key, because a pass consumes a ply and the table's parity survives it. Seeds
> are this project's own numbers and nothing else: the two shares from the
> one-ply table's fitted 17 and 18, the weight at 26 for equal authority with
> the term beside it -- 8519 against 8519 against plain history's 8831. `bench`
> 4646334 -> 5443203. The defaults are first settings and S231's own SPSA lane
> (`tools/spsa_s231.json`, `adocs/data/S231_spsa.sh`) fits all six continuation
> axes before one gainer SPRT `{0, 5}` nElo against `3a649c0` decides the step
> -- one vector under one verdict, DEC-210's reading.

## Owed after the verdict, DEC-222: two removal verdicts

Once this step's gainer SPRT reads H1 and the fitted stack is in, the ordering
slots the history sum displaces are measured for removal, one at a time, each
a `{-5, 0}` non-regression whose truth sits on the bound -- 25,591 expected
games (DEC-143), eleven to twelve hours, a night each. First the killer slots
(`src/data_structures.hpp` `killer_moves`, two per ply: S149 and S159 already
stopped investing in them, and the open-source record shows the same removal at +0.50 over 47,676 games once its history stack was in), then the
countermove table (`src/data_structures.hpp` `counter_moves`, which the
one-ply continuation table subsumes in the published record, no number). A
removal that reads H0 stays in the tree and is recorded as such. The
selection rule (DEC-222): a removal is scheduled when it saves measurable
nodes per second or memory, or unblocks a later step, never as a sweep -- here
the saving is a store per cutoff and two reads per quiet, measured with
`hyperfine` before the run and stated in the pre-registration. If this step's
SPRT reads H0 the removals are not owed and the reason is that the stack they
were to be measured against did not ship.


## The lane, launched 2026-09-19 18:41 (coordinator)

Launched by the coordinator at 18:41:56 CEST on the idle workstation, exactly
as the header says: `nohup adocs/data/S231_spsa.sh > .tuning/spsa_s231.log
2>&1 &`, pid 3430959 written to `.tuning/spsa_s231.pid`, output directory
`.tuning/spsa_s231_20260919_184156`. HEAD `0c3f0eb`, 0 dirty tracked paths;
the tune binary rebuilt by the script hashes `844e184b3819760a...` (first 16 of
the sha256 the banner prints). Load 2.27 before launch on twelve threads with
no match or engine process alive, desktop on mains, governor `performance` as
found -- recorded and not set (DEC-195). `check` passed 6 of 6 in 0.4 s with
every node count identical to phase one's check on 2026-09-18 (`ContHist2Weight`
at 0 still 21995, the parent's depth-9 count), so the config and the binary
agree to the node.

**Open findings this run is taken while open**, as the header asks the launch
note to name (BUGS, DEC-171): no defect reachable in play, on the UCI surface
or able to move a reported score is open. S231's own `I03_null_child_drops_prev2`
gap -- the mutant dies only by an indirect case, structurally -- is a test gap
and filler behind the next strength step. The 2026-09-19 study review's
findings concern the analysis document and the plan's order, not the engine's code, and
are being turned into DEC-220 to DEC-222 and S232 to S238.

**Estimate 8 h 45 m, so an expected end near 03:27 on 2026-09-20; ceiling 18 h,
12:42.** Watcher armed in the coordinator's session (`Monitor`, persistent):
polls the whole log every 60 s, never a follow, four exits -- the first
`SPSA-(DONE|FAILED)` line, the pid gone without a marker, the 18 h ceiling, and
a manual stop that is a belt -- and announces the two mid-run reads at
iterations 313 and 625 off `trajectory.tsv`. Abort rule unchanged from the
header: forfeits over 1.0 % either side, `SPSA-FAILED`, mains or a second load.

**Beside it, agent-only work and no machine**: S233 (the DEC-220 result block in
the gate, and the ledger script) runs in a fresh Opus 5 subagent while the lane
holds the machine, because this step's own verdict commit is the first one that
block is owed in -- PLAN's "strictly necessary" clause, as `status.md` recorded
before launch. Its brief forbids every build, test run and match until the lane
ends; the Tier-1 gate over its result is the coordinator's, after `SPSA-DONE`.

**Mid-run read at a quarter, 20:52, iteration 313 of 1250 (coordinator).**
The three pre-registered readings, recorded whatever they say: `c_scale`
2.055 -> **1.150**, the same figure S222's lane showed at its quarter; `y`
centred with real spread -- mean -0.67, standard deviation 5.64, range -17 to
+18, 8.9 % of iterations at exactly zero -- not the "barely changing" trajectory
the fishtest wiki calls useless; **no axis has touched a bound** (0.0 % pinned
on all six). The vector itself has barely moved: `ContHistBonus` 17 -> 18,
`ContHistMalus` 18 -> 19, `ContHistWeight` 26 -> 24 (range so far 22 to 26),
`ContHist2Bonus` 17 -> 20, `ContHist2Malus` 18 -> 18, `ContHist2Weight`
26 -> 26 (range 25 to 29). W 5121 L 5331 D 4620 over 15072 games in the
trajectory; `tools/forfeit_report.py` over the run's PGN: **0 forfeits on either
side** of 15089 games. Load 11.8 on twelve threads, no second load. Nothing
here is a result (DEC-019): the fit is read at the end and the SPRT decides.

**Mid-run read at a half, 23:02, iteration 625 of 1250 (coordinator).**
`c_scale` **1.072**, again the figure S222's lane showed at its half, decaying
towards 1 as designed; `y` mean -0.21, standard deviation 5.45, range -17 to
+18, 7.7 % of iterations at exactly zero; **no axis has touched a bound**.
Vector at the half: `ContHistBonus` 18, `ContHistMalus` 18, `ContHistWeight`
25 (range so far 22 to 26), `ContHist2Bonus` 18, `ContHist2Malus` 20,
`ContHist2Weight` **23** (range 21 to 29). W 10394 L 10527 D 9127 over 30048
games; **0 forfeits either side** of 30078 games in the PGN. Load 12.1, no
second load. The pace holds at 25.2 s an iteration, so `SPSA-DONE` is expected
near 03:27. Nothing here is a result (DEC-019).

## Phase three, 2026-09-20: the fit lands, the census is re-run, the SPRT is pre-registered

Built by a fresh Opus 5 subagent on the idle machine (DEC-185, DEC-199). The
step stays in `plan_current/` and `done:` is still empty: what completes this
step is the SPRT's verdict, and that run is the coordinator's.

### The fit, and which pre-registered readings it lands in

The lane ran **2026-09-19 18:41:56 to 2026-09-20 03:22:41 -- 8 h 40 m 45 s
against the 8 h 45 m estimate**, 0.8 % under, **25.00 s an iteration** against
S222's measured 24.84 for the same shape. 1250 iterations x 24 pairs = 30000
pairs = **60000 games** on `books/UHO_4060_v3.epd`, W 20794 L 20774 D 18432,
one `SPSA-DONE` at the end of the run log (the check stage's went to its own
log, as this lane's script was written to make it). `tools/forfeit_report.py`
over `games.pgn`: **0 forfeits on either side of 60000 games**, against the
header's 1.0 % abort line -- the abort rule was not approached, let alone
breached. Evidence: `adocs/data/S231_spsa_trajectory.tsv`, `S231_spsa_run.json`,
`S231_spsa.log`.

The vector is the driver's own rounded JSON block at the end of that log and no
number in it was re-rounded by hand (`final_vector` rounds at the UCI boundary,
which is where the run itself sent every value):

| axis | incumbent | fitted | theta before rounding |
|---|---|---|---|
| `ContHistBonus` | 17 | **18** | 18.479 |
| `ContHistMalus` | 18 | **17** | 17.284 |
| `ContHistWeight` | 26 | **24** | 23.649 |
| `ContHist2Bonus` | 17 | **18** | 17.564 |
| `ContHist2Malus` | 18 | **20** | 20.162 |
| `ContHist2Weight` | 26 | **24** | 23.758 |

**Not a stuck lane.** All six axes moved off their seeds. The stuck-run row
would not have excused the run in any case -- that is the first of the two
places this lane's header deliberately differs from S222's, because the
reference is the tree *before* the step and what the SPRT prices is the second
table itself.

**`ContHist2Weight` ended near 26, at 24**, which is the fifth of the header's
six pre-registered readings. Equal authority was about where the fit wanted the
second table. **The "at or under 5" row is not triggered, so no pinned-zero
attribution SPRT is owed** and `adocs/data/S231_sprt_pinned.sh` is deliberately
not written, the way `S222_sprt_pinned.sh` is not.

**`ContHistWeight` moved 26 -> 24, which is two units and not "a long way"**,
so that row is not triggered either. Its consequence is carried anyway, because
that is what the row exists for: **S222's fitted 26 is no longer the shipping
value**, and `adocs/data/S231_sprt.sh` names every carried value for that
reason. At 24 each the two weighted terms span `24 * 32767 / 100 = 7864`
apiece, against plain history's fitted `QuietHistoryMax` of 8831 -- so the
quiet band ships as **8831 / 7864 / 7864** over its three terms where the seeds
shipped 8831 / 8519 / 8519. The two tables still carry equal authority with
each other and plain history still carries slightly more than either; what the
fit did was take a little band off both continuation terms at once.

**The whole vector sits within three units of its seeds**, which is a fact
about this lane's resolution at 60000 games and is not a result (DEC-019). The
SPRT against `3a649c0` prices the table itself, and it is owed whatever the
vector did.

### The three mid-run reads, taken over the whole trajectory

The lane pre-registered them and said they are recorded whatever they say. The
coordinator's reads at a quarter and a half are above; these are over all 1250
rows of `adocs/data/S231_spsa_trajectory.tsv`.

- **`c_scale` decays as designed**: 2.055 -> 1.150 at a quarter -> 1.072 at a
  half -> 1.000 at the end. The same three figures S222's lane showed, to the
  third decimal on two of them.
- **`y` has real spread and is centred**: mean **0.016**, standard deviation
  **5.48**, range **-17 to +18**, and **8.1 %** of iterations at exactly zero.
  Not the "barely changing" trajectory the fishtest wiki calls useless. The
  spread is narrower than S222's, which read 6.05 over -35 to +35 at the same
  24 pairs an iteration; the two lanes differ in dimension and in which axes
  they perturb, and nothing here separates those, so the comparison is recorded
  and not explained.
- **No axis touched a bound on any iteration.** 0.0 % pinned at both ends on
  all six, where S222's `HistoryMalusQuad` sat at its floor for 18.8 % and
  S085's `RfpMinPly` for 72.5 %. Per-axis ranges over the run: `ContHistBonus`
  17-19, `ContHistMalus` 17-20, `ContHistWeight` 21-26, `ContHist2Bonus` 17-21,
  `ContHist2Malus` 17-22, `ContHist2Weight` 21-29.

### What landed, and how each number got there

`src/search_params.hpp`'s six X-macro rows carry the fitted values, written by
**`adocs/data/S231_apply_fit.py`**, which reads the driver's rounded JSON out
of `adocs/data/S231_spsa.log` rather than retyping it and refuses a
substitution that would change a column's width -- that table is aligned and
its continuation backslashes sit in a fixed column. The built tune binary's own
`uci` option lines were then read back and compared against that same JSON by
**`adocs/data/S231_verify_fit.py`**: all six equal, and all 53 of the header's
rows equal to the binary's on name, default, minimum and maximum.

The two comment blocks that explain those defaults now say the value is this
project's own SPSA fit of 2026-09-19/20 on `books/UHO_4060_v3.epd`
(`adocs/data/S231_spsa_trajectory.tsv`) **and nothing else**. The seeds they
replace are named as the seeds they were and not as values in force (DEC-084 as
amended by DEC-105): the one-ply block keeps its original 15-thousandths
derivation and S222's fit of it as history, the two-ply block keeps its
equal-authority derivation at 26 and says the band *started* at
8831 / 8519 / 8519. No engine's constant is behind any of the six and none was
read off a table, a wiki page or a release note.

**The band-clearance derivation and the declared ranges do not move in this
phase**, which is what this step's `excludes` protects: 0 to 1000 on all six
(DEC-218), the bound still the definition `CONT_HIST_BOUND`, the ceiling
arithmetic word for word. One sentence inside it was stale and is corrected:
"the fitted 26 is nowhere near either edge and is untouched" now says the
halving moved no value and that S231's lane then moved the axis to 24 on its
own evidence. `tests/test_evaluation.cpp` "the declared history ceiling clears
the band above it" reads the declared maxima out of `search_param_info` rather
than the defaults, so it is untouched by a default moving -- run directly on
the fitted build, **46 assertions, all passing, band [-688107, 688107] at the
declared maxima, the same 11893 of clearance**.

`tests/test_search_params.cpp`'s golden table carries the six new defaults. Its
own site says there is no script and none is owed, "because
`src/search_params.hpp` is the derivation and a diff of the two is the
re-derivation"; that rule was followed, and the mechanical cross-check taken
besides -- all **53** rows read back out of the built tune binary's `uci` reply
and compared name, default, minimum and maximum, in order, against the golden
list. They match row for row. The GOLDEN block's own history sentence gains
S231's six beside S085's ten and S222's eleven.

### Measurements

`chesso bench`: **5443203 -> 4393575**, -1049628 nodes, **-19.3 %**. The commit
carries `Bench: 4393575`. **The sign is the one phase one did not have**: the
fitted tree is also **5.4 % smaller than `3a649c0`'s own 4646334**, the tree
the two-ply table was added to. Phase one recorded a 17.2 % *bigger* tree at
the seeded equal authority and asked whether equal authority was more authority
than the second table earns; the fit's answer, read as nodes and not as Elo, is
that two units off each weight take the whole of that growth back and 5 % more
besides. Nothing was added in this phase: six integers moved, and every count
downstream of a quiet ordering moves with them.

`tools/search_bench.py`, phase one -> fitted:

| depth | midgame | kiwipete | tactical |
|---|---|---|---|
| 9 | 43974 -> 43949, `c3d5` | 104700 -> 104912, `e2a6` | 29323 -> 28832, `d7c8q` |
| 12 | 167581 -> 170594, `c3d5` | 648155 -> 624405, **`e2a6` -> `d5e6`** | 128534 -> 128731, `d7c8q` |

The three positions disagree in direction at both depths and the moves are much
smaller than phase one's, which is what two units of weight should look like
beside a whole new table. **Kiwipete's best move at depth 12 moves to `d5e6`**,
which is where it sat before S222's own phase three moved it back to `e2a6`.
Node counts move by construction, so INV-6's discharge is not available and the
SPRT is the only thing that can decide this step.

`tests/test_search.cpp` "ordering keeps the tree small" reads **16246** against
16256 at phase one and the 17321 phase one measured at its parent `3a649c0`. That is 0.06 % off phase one's count --
and it is **outside** the middle half of its own band, which is the next
section.

### A golden whose re-derivation trigger has been standing for three steps

`adocs/data/S192_node_budget.py` is the band's own script and it was run on the
fitted build rather than the count being read by eye. It reports:

    count         16246 nodes, depth 5 on KIWIPETE_POS, cold table
    budget        64984  (4x the count)
    floor         3249  (the count over 5)
    shipping      budget 69804, floor 3490
    middle half   [20068, 53226]: the count is OUTSIDE it

**So DEC-142's trigger has fired, and it did not fire in this phase.** The band
`[3490, 69804]` was derived from 17451 after S091; the middle half of that band
is [20068, 53226] and **no count since has been inside it** -- S222 recorded
17332, phase one read 16256 against 17321 at its parent, and this phase reads
16246. Two step files, S222's phase
three and this file's phase one, state that the count "has not left the middle
half". Checked against the script that defines the phrase, that statement was
wrong both times; the arithmetic is above and is not in dispute.

**It is recorded and not repaired here, on purpose**, and each reason stands on
its own:

- It is not this phase's doing. 16256 -> 16246 is 0.06 %, and the count was
  already outside the band's middle half two steps ago.
- Re-deriving moves a test's numbers inside the commit an SPRT is about to
  judge, and MEASUREMENT says one change at a time.
- Under H0 `src/` reverts to `3a649c0` whole, and a band re-derived from this
  tree would be left describing a tree that no longer exists.
- The script itself says so: "what to do about a count that has left the middle
  half of the band is the step's decision, not this script's."

The case passes and is not weakened by leaving it: at 16246 against a budget of
69804 it bounds the tree at 4.3x rather than 4x, which is a looser bound and
not a broken one, and its floor at 3490 is 4.7x below the count. **Proposed to
the coordinator as filler behind the next strength step** -- one re-derivation
by `S192_node_budget.py` on whatever tree ships after this step's verdict,
whichever way the verdict goes -- and **named as an open finding in
`adocs/data/S231_sprt.sh`** before a game is played, as BUGS scoped by DEC-171
requires. Its reach: none into play, none onto the UCI surface, none into any
reported score, move or line. It is a test band.

### The census on the fitted build, and the control that attributes it

`adocs/data/S024_census_run.py run` over its own 400 positions at depth 10,
Hash 16, on a Release build of this phase's `src/search_params.hpp` with the
**eight** throwaway counters patched in -- S024's five and S231's three
appended, in `CENSUS_FIELDS`' own order. Built in a detached worktree at
`abe251b` with the fitted header copied in, its own `build/`, removed after the
run; the script was run from the worktree's copy, so **this tree's
`adocs/data/S024_census.tsv` was never touched** -- what S222 did. Output, both
summaries, the fitted run's 400 rows and the instrumentation diff verbatim:
`adocs/data/S231_census.txt`.

| share | S024, 2026-09-12 | S222 fitted | **S231 fitted** | S231 at the incumbent vector |
|---|---|---|---|---|
| one-ply writes with a previous move | 97.56 % | 96.62 % | **96.53 %** | 96.53 % |
| one-ply quiet reads consulting the term | 96.19 % | 95.15 % | **95.30 %** | 95.36 % |
| of those, reading non-zero | 27.14 % | 19.15 % | **19.89 %** | 20.22 % |
| two-ply writes with a move two plies back | -- | -- | **94.24 %** | 94.32 % |
| two-ply quiet reads consulting the term | -- | -- | **95.72 %** | 95.79 % |
| of those, reading non-zero | -- | -- | **27.51 %** | 27.31 % |

**The two-ply table is not inert wiring**, which is the question DEC-194 needed
a census to answer for the first table and which mutant `I04_cont_hist2_unread`
exists for. The move two plies back is there at 94.24 % of quiet cutoffs and
the term is consulted on 95.72 % of quiet `score_move()` evaluations -- within
a point and a half of the one-ply table's own figures -- and **27.51 % of those
consultations read a non-zero entry against the one-ply table's 19.89 % on the
same run**. The table that sees further is the one that more often has
something to say here. That is a fact about this tree's shape and not a claim
about strength: what a table returns is not what it is worth (DEC-019).

**The control attributes all of it to the tree and none to the fit.** The same
instrumented binary was rebuilt with `abe251b`'s own incumbent header
(17/18/26 twice) and the same 400 positions re-run: every share is within four
tenths of a point of the fitted run's, the one-ply non-zero share 0.34 points
*higher* at the incumbent vector and the two-ply one 0.20 points lower. What
the fit moved is the size of the tree, 29450590 nodes against 30121175, 2.2 %.
Both runs were repeated once and reproduced to the counter. Per-position
spreads: one-ply 3.02 % to 40.92 %, median 18.14 %; two-ply 4.91 % to 54.29 %,
median 25.82 % -- so no handful of positions carries either average.

The one-ply figures are S222's own profile again, a third of a point apart on
all three, on a tree 3.4 % smaller than S222's 30474552.

### The SPRT, pre-registered

`adocs/data/S231_sprt.sh`, modelled on `adocs/data/S222_sprt.sh`, written
before a game is played. `{0, 5}` nElo at `8+0.08`, Hash 16, one thread a side,
concurrency 12, `books/noob_3moves.epd` -- **not the lane's
`UHO_4060_v3.epd`**, which is the whole point of the two books (DEC-209
clause 1). `OUT` under `.tuning/` in the exec line, because this machine wipes
`/tmp` at boot and two runs were lost to it.

**`REF` is pinned in the file at `3a649c0`**, S098's completing commit and the
last tree without the two-ply table, because this step file pinned it on
2026-09-18 so phase three could not get it wrong (DEC-210: one vector under one
verdict). Measured against `HEAD` the table itself would stay unpriced and only
six integers would be on trial. Three commits between `3a649c0` and the
candidate touch `src/`: `b83fb1d` (phase one), `af8b9f0` (one arithmetic
correction inside a comment) and this phase's landing. `CAND` defaults to
`HEAD` because the landing commit does not exist when the file is written; the
coordinator pins it to the sha after committing, and `fastchess.sh`'s banner
prints both shas with their commit dates before the first game.

**The cost (DEC-143)**: 41861 expected games with the truth at the interval's
midpoint, 25591 on a bound, which at **2110 games an hour** --
`.moltke.local.md`'s standing figure for this control, book and governor -- is
**19.8 h** and **12.1 h**. S098's five verdicts, the most recent family and the
one on a tree this size, measured 2119 to 2128, which gives the same figures to
a tenth; the conservative figure is the one budgeted from. A
faster run would not surprise and a slower one is the thing to watch: this
candidate benches 4393575 against the 4.6 M tree those rates were measured on,
about 5 % smaller, where phase one's 5443203 would have been 18 % larger.

**Abort rule**: forfeits over 1.0 % on either side, a crash or disconnect
voiding the run outright (`SPRT-RUN-INVALID`), mains, a second load. Nothing
else.

**Open findings this run is taken while open** (BUGS as scoped by DEC-171):
no defect reachable in ordinary play, on the UCI surface, or able to move a
reported score, move or line. Two test gaps are, both named in the
pre-registration and both filler behind the next strength step: S231's own
`I03_null_child_drops_prev2`, which dies only by an indirect case; and the
ordering-band staleness this phase found, above. `adocs/plan.md`'s Open list
carries no other filler behind this entry.

The three outcomes are written in that file. **H1** keeps the whole vector and
the table, the claim being "at least 5 nElo" (DEC-063), and the step then owes
DEC-222's two removal verdicts, one at a time, killer slots first and the
countermove table second, each `{-5, 0}` and a night, each with `hyperfine`
before the run. **H0** reverts `src/` to `3a649c0` whole and the two-ply idea
leaves the plan with a decision saying why -- and the revert is clean in a way
S222's would not have been, because the one-ply three's pre-S231 values *are*
exactly what `3a649c0` carries (S222's own fit, which S222's own SPRT already
read H1 on), so no axis is stranded without a verdict and nothing that had one
is thrown away. **No verdict** is recorded as a zero and decided with the
reason stated. DEC-222's removals are not owed under H0: the stack they were to
be measured against did not ship.

### Documents

- `MANUAL.md`: the six option rows' defaults read off the binary, and every
  sentence that called the two-ply three "first settings ... not yet fitted"
  rewritten -- they are fitted now, and the seeds are named as seeds.
  `ContHistWeight`'s and `ContHist2Weight`'s rows carry the new spans and the
  three-term band. `tools/plan_prose_check.py --params` passes.
- `DEV_MANUAL.md`: the bench ledger gains this landing's `4393575` with what
  moved it; the S231 lane paragraph gains the measured wall, the W/L/D, the
  six axis moves, the no-axis-at-a-bound reading and which two of the six
  pre-registered readings it landed in.
- `adocs/data/README.md`: seven rows -- `S231_spsa_run.json`,
  `S231_spsa_trajectory.tsv`, `S231_spsa.log`, `S231_census.txt`,
  `S231_apply_fit.py`, `S231_verify_fit.py`, `S231_sprt.sh`.
- `tests/test_search_params.cpp`: the six golden defaults and one sentence of
  the GOLDEN block's history.
- `tests/test_uci_surface.cpp`: no edit owed, for phase one's reason unchanged
  -- its tune option lines are generated from the same table and its golden is
  the release build's count of 5.
- `README.md`: human-owned, untouched.
- `adocs/specs.md`, `adocs/plan.md`, `adocs/status.md`, `adocs/decisions.md`:
  not edited (hard limit). Wording proposed below.
- Gate, both builds, `CLANG_FORMAT_MAJOR=22` (DEC-146): **40 of 40 in `build`,
  40 of 40 in `build-tune`, `./clang-format.sh --check` clean**.
  `tools/plan_prose_check.py --params`, `--citations` (0 flagged over 50 files)
  and `--touches` (0 flagged) all pass. Debug self-play (DEC-141 clause 1) and
  `tools/gate_extra.sh` are the coordinator's, on the landing commit.

### Proposed `specs.md` amendment, for the coordinator

The search row's S231 passage ends "The defaults are first settings and S231's
own SPSA lane (`tools/spsa_s231.json`, `adocs/data/S231_spsa.sh`) fits all six
continuation axes before one gainer SPRT `{0, 5}` nElo against `3a649c0`
decides the step -- one vector under one verdict, DEC-210's reading." That
clause is now history. Replacing it:

> The defaults above were first settings and **S231's own narrow lane fitted
> them on 2026-09-19/20** -- `tools/spsa_s231.json`, `adocs/data/S231_spsa.sh`,
> six axes, 1250 iterations over 60000 games at 2+0.02 on `UHO_4060_v3.epd` so
> tuning and verification share no openings, 8 h 40 m 45 s, 0 forfeits,
> `adocs/data/S231_spsa_trajectory.tsv`. Every axis moved and none touched a
> bound: `ContHistBonus` 17 -> 18, `ContHistMalus` 18 -> 17, `ContHistWeight`
> 26 -> 24, `ContHist2Bonus` 17 -> 18, `ContHist2Malus` 18 -> 20,
> `ContHist2Weight` 26 -> 24. **The weight landed near 26**, which is the
> lane's own pre-registered reading that equal authority is about where the fit
> wanted the second table, and the "at or under 5" row that would have owed a
> second attribution run is not triggered. At 24 each the two terms span
> `24 x 32767 / 100 = 7864` apiece against plain history's 8831, so the quiet
> band ships as **8831 / 7864 / 7864** where the seeds shipped 8831 / 8519 /
> 8519; the declared maxima and the 688107 band they give are untouched.
> `bench` 5443203 -> **4393575**, which is 5.4 % *below* the 4646334 of
> `3a649c0` -- the seeded equal authority was what had grown the tree. The
> census on the fitted build reads 96.53 / 95.30 / 19.89 % for the one-ply
> table and **94.24 / 95.72 / 27.51 % for the two-ply one**, with a same-tree
> control at the incumbent vector putting every share within four tenths of a
> point, so neither table is inert wiring (`adocs/data/S231_census.txt`). One
> gainer SPRT `{0, 5}` nElo at the harness regime against `3a649c0` decides the
> step (`adocs/data/S231_sprt.sh`) -- one vector under one verdict, DEC-210's
> reading.

Nothing else in `specs.md` moved. The S222 passage's sentence that phase one
already flagged as false in two clauses is unchanged by this phase and is still
owed whatever the coordinator decided for it.

### Proposed decision, for the coordinator

**A `DEC` for the fit itself is probably not owed.** The lane's header
pre-registered six readings; the fit landed in two of them -- "the vector
moves" and "`ContHist2Weight` ends near 26" -- and both were written before a
game was played, so nothing here is a choice a future reader would re-derive.
The one substantive consequence, that S222's fitted 26 is no longer the
shipping value, is a fitted value replacing a fitted value under the lane the
step's `accepts` asked for, and it is recorded in the file that ships it and in
the SPRT's pre-registration.

**One thing may be worth an entry, and it is not about the fit**: the ordering
band above. Three step files in a row have read `[3490, 69804]`'s middle half
by eye and got it wrong, and the reason is that the phrase has a script and the
script was not run. If the coordinator wants that written down rather than left
to the filler step, the entry is one line -- *a golden with a script is read by
running the script, never by eye* -- and it is DEC-142 restated rather than
amended. Agents do not write `adocs/decisions.md`; this is a proposal.

## Second tier on the landing commit `55891bb`, 2026-09-20 (coordinator)

**Debug self-play, DEC-141 clause 1**, on the Release-fitted tree's Debug
build: four rounds of `fastchess` at 4+0.04 on `books/noob_3moves.epd`,
concurrency 8, `-log level=trace engine=true` (the form that captures engine
stderr), outputs under `.tuning/coord/s231_p3_debug_selfplay/` -- **8 games in
15 s, 0 `Assertion`, 0 `disconnect`**, trace log 104707 lines with 930
`bestmove` lines so the log really carried the engines' output. Script
`.tuning/coord/S231_debug_selfplay.sh`. Time forfeits are not the failure
condition at this control and were not counted.

`tools/gate_extra.sh` launched detached at 04:26 on `55891bb`
(`.tuning/gate_extra_2026-09-20_s231p3.log`, stages prose, citations, debug,
sanitize, perft), watcher armed with four exits and a 90-minute ceiling; its
marker is recorded below before the SPRT starts. `CAND` pinned to `55891bb` in
`adocs/data/S231_sprt.sh` (`f949759`), `REF` `3a649c0` as the file always said.

**`tools/gate_extra.sh` on `55891bb`: `GATE-EXTRA-DONE 5 stages 1123 s`**
(04:26 to 04:45, `.tuning/gate_extra_2026-09-20_s231p3/`), prose, citations,
debug, sanitize and perft all green; this is also the weekly run, so the next
is due by 2026-09-27.

**The SPRT, launched 2026-09-20 04:45:41 (coordinator).** `nohup
adocs/data/S231_sprt.sh > .tuning/sprt_s231.log 2>&1 &`, pid 894939 in
`.tuning/sprt_s231.pid`, output `.tuning/sprt_s231_20260920_044538`. The
banner: `cand-55891bb  Chesso 55891bb native` against `ref-3a649c0  Chesso
3a649c0 native`, 8+0.08, Hash 16, concurrency 12 of 12, `noob_3moves.epd`,
seed 20260920044538, bounds elo0=0 elo1=5 alpha=beta=0.05 -- exactly the
pre-registration. Load 1.4 before launch with no engine alive, mains (the
only power-supply entry is a Logitech peripheral's), 17 GB free. Watcher
armed (`Monitor`, persistent): every new `LLR:` line, the first
`SPRT-RUN-(DONE|FAILED|INVALID)`, the pid gone without a marker, a 40-hour
ceiling (2026-09-21 20:45) -- at least twice the 19.8 h worst case at 2110
games an hour. Abort rule as pre-registered: forfeits over 1.0 % either side,
crash or disconnect (`SPRT-RUN-INVALID`), mains, a second load; nothing runs
beside it until the marker. Open findings named in the header: S231's I03
mutant gap and the S192 node-budget golden outside its middle half, both
test-side, neither reachable in play.

## The verdict: H0, 2026-09-20 10:23:54 (coordinator)

**`SPRT-RUN-DONE` at 10:23:54 after 5 h 37 m 42 s and 12070 games: H0
accepted, LLR -2.95 against (-2.94, 2.94), `Elo -2.65 +/- 4.82`, `nElo -3.40
+/- 6.20`**, W 3654 L 3746 D 4670, `Ptnml(0-2) [556, 1464, 2097, 1352, 566]`,
LOS 14.08 %, draw ratio 34.75 %, pairs ratio 0.95; 2144.5 games an hour on a
tree 5.4 % smaller than the reference's. 0 time forfeits on either side over
the PGN's 12071 games (8364 adjudications, 3707 natural ends); `Incomplete
mating PV` 1 candidate against 1 reference, no asymmetry. `S105_pairs.py`: 6035
complete pairs, pair score mean 0.9967, variance 0.3105, sd 0.5572, 115.4 plies
and 19.6 s a game. Evidence `adocs/data/S231_sprt.log` and
`adocs/data/S231_sprt_pairs.txt`. Slow class: the truth sat inside the pair and
the run walked to the bound, as DEC-063 says a `{0, 5}` pair does to a zero.

**The pre-registered H0 reading applies and binds** (`adocs/data/S231_sprt.sh`,
written before a game was played): S231 whole -- the two-ply table on its
fitted scale together with the one-ply three fitted beside it -- does not gain
5 nElo over the tree before it. The nElo interval is [-9.60, +2.80] and the Elo
interval [-7.47, +2.17]: the centre sits below zero and the top does not reach
the bound, which is the shape DEC-194 did not keep. The census on the fitted
build read the two-ply table non-zero on 27.51 % of quiet scores with a
same-tree control, so the zero is about the technique as built here and not
about inert wiring -- DEC-194's own argument, S005, S006 and S015 the precedent
for recording a zero as a zero. **Consequences, all pre-registered:** `src/`
returns to `3a649c0` whole -- table, three new axes, and the one-ply three back
to 17 / 18 / 26, which are exactly `3a649c0`'s and S222's fitted values, so no
axis is stranded without a verdict; the two-ply idea leaves the plan with a
decision that says why; the two removal verdicts of DEC-222 are **not owed**,
because the stack they were to be measured against did not ship. The one-ply
table keeps its own H1 (DEC-210) untouched: this run measured the pair against
the tree that already carries it.

**What this costs the plan and what it bought:** one lane night (8 h 40 m),
one verdict (5 h 38 m), 72070 games, about two agent-days; a measured zero on
the published record's "second half of the same idea", and the finding that the
seeded equal authority had grown the tree by 17 % where the fit took it 5 %
below the reference -- neither of which transferred to Elo. DEC-019's rule
holds again: a figure decides what to try, never what to conclude.

The revert is a fresh agent's job (`.tuning/coord/S231_h0_revert_brief.md`),
proved by `bench` 4646334 to the node; the S192 node-budget golden is
re-derived on the reverted tree in that commit; S095's stashed, uncompiled work
is re-applied on the reverted tree afterwards. This commit is the first to
carry DEC-220's result block, and `tools/gate.sh` checked it against the log.

## H0, 2026-09-20: the revert

Carried out by a fresh Opus 5 subagent on the idle machine, briefed by the
coordinator (`.tuning/coord/S231_h0_revert_brief.md`, DEC-185, DEC-199). The
coordinator stages and commits; nothing below was staged here. Shape: DEC-194's
own revert, `ecdfadb` -- `src/` and the test files return to the pre-step
content, the step's mutant script goes, the evidence stays under `adocs/data/`,
the manuals say so, and a decision records why.

### What returned, what went, what stayed

`git log 3a649c0..HEAD` was read per file before a byte moved: every change to
each of the files below since `3a649c0` belongs to one of S231's four commits
(`b83fb1d`, `4c727b2`, `af8b9f0`, `55891bb`), so **no hunk of another step's
was caught in the revert** and none had to be kept back.

| path | what happened | why |
|---|---|---|
| all of `src/` -- `data_structures.hpp`, `evaluation.cpp`, `evaluation.hpp`, `search.cpp`, `search.hpp`, `search_params.hpp` | **returned to `3a649c0`'s content, byte for byte** | the pre-registered H0 reading: one vector under one verdict, so the table, its three axes, the halved `ContHistWeight` ceiling and the one-ply three all go together. The one-ply three land on 17 / 18 / 26, which are `3a649c0`'s and S222's own fitted values, so no axis is stranded |
| `tests/test_evaluation.cpp`, `tests/test_search_params.cpp` | returned to `3a649c0`'s content | the band case at three tables goes back to two; the golden option table goes 53 rows to 50 |
| `tests/test_search.cpp` | returned to `3a649c0`'s content, **then one golden re-derived** (below) | the two-ply cases, the null-child case, the re-derived `capture_mates` depths and the depth-4 research witness were all S231's and all go |
| `tools/mutants/S222_continuation_history.py` | returned to `3a649c0`'s content | `4c727b2`'s `(void) prev_move;` in `H04_cont_hist_unread` existed **only** because S231 gave `quiet_history_sum` a second guarded term. With that term gone the orphan is gone, and the pair source-plus-registry is exactly the one S222's own mutation run observed `H04` killed on |
| `tools/mutants/S098_research_rule.py` | returned to `3a649c0`'s content | its only change since was S231 widening `D08_site_ignores_the_rule`'s anchor for the new `prev_move` argument |
| `tools/mutants/S231_continuation_history2.py` | **deleted** | the four mutants target code that no longer exists; DEC-194 deleted S024's the same way |
| `MANUAL.md` | returned to `3a649c0`'s content | the three `ContHist2*` rows leave and `ContHistWeight`'s row is `3a649c0`'s again: range 0 to 2000, defaults 17 / 18 / 26 |
| `tests/CMakeLists.txt`, `tests/test_gate_script.sh`, `tests/test_ledger.py` | **untouched** | S233's, landed in `1b7c9be` between S231's phases |
| everything under `adocs/data/` -- the lane, the census, the SPRT, the two fit scripts, the research witness | **untouched, every row of `adocs/data/README.md` kept** | DEC-194's precedent: the evidence of a measurement outlives the code it measured. `tools/spsa_s231.json` stays for the same reason S222's does -- the frozen record of a run already taken, not a template |

### The proofs

**`chesso bench` on the reverted Release build: `4646334`** -- `3a649c0`'s own
total to the node, which is the whole-revert proof and what the commit's
`Bench:` line carries. The `3a649c0` worktree's own binary, rebuilt today,
prints the same `4646334`.

**`tools/search_bench.py`, reverted tree against a `3a649c0` worktree build,
interleaved -- identical counts and identical best moves at both depths, every
position** (INV-6's form of the same proof):

| depth | midgame | kiwipete | tactical |
|---|---|---|---|
| 9 | 21995, `g5f6` | 104682, `e2a6` | 29842, `d7c8q` |
| 12 | 155612, `c3d5` | 683624, `e2a6` | 152138, `d7c8q` |

Those six counts are also exactly the parent column phase one recorded before
the table landed.

**The gate, both builds, `CLANG_FORMAT_MAJOR=22` (DEC-146): 40 of 40 in
`build`, 40 of 40 in `build-tune`, `./clang-format.sh --check` clean.**
`tools/plan_prose_check.py --params`, `--citations` (0 flagged over 50 files)
and `--touches` (0 flagged) all pass.

**`tools/mutation_check.py`'s own `validate` over the remaining registry, run
against the reverted tree: 84 mutants over 10 files, every anchor occurring
exactly once in the file it names, no duplicate id.** The four S231 mutants are
gone with their file; S222's four (`H01` to `H04`) are back in the form
`3a649c0` carries. No full mutation pass was taken and none is owed: the tree
it would run on is `3a649c0`'s `src/` byte for byte, which had its own.

**Second tier, DEC-141: no Debug self-play and no `tools/gate_extra.sh` are
owed here, and the reason is the same one.** `src/` is not "equivalent to" the
tree that already passed both -- it is that tree, byte for byte, and the only
delta in the whole commit beyond it is two integers and a comment in one test
case. `gate_extra` last read `GATE-EXTRA-DONE 5 stages 1123 s` on `55891bb`
this morning, so the weekly is in hand to 2026-09-27 either way.

### The one edit beyond `3a649c0`: the node-budget golden, re-derived

`adocs/data/S192_node_budget.py` on the reverted Release build:

    count         16256 nodes, depth 5 on KIWIPETE_POS, cold table
    budget        65024  (4x the count)
    floor         3251  (the count over 5)

So the golden pair in `tests/test_search.cpp` "ordering keeps the tree small"
goes **69804 and 3490 -> 65024 and 3251**, applied at the case's own site with
the script named there as its re-derivation (DEC-142), the GOLDEN block's
history line gaining the 2026-09-20 reading, and the failure message's
"derived from 17451" becoming "derived from 16256". `DEV_MANUAL.md`'s golden
index carries the new pair and the same script. This closes the finding
`adocs/data/S231_sprt.sh` named before a game was played: the trigger had been
standing for three steps while two step files recorded it as not fired.

**Two things were found doing it and neither is repaired here.**

1. **The parent's recorded count does not reproduce.** Phase one recorded
   17321 for `3a649c0`. On a tree whose `src/` and whose case text are
   `3a649c0`'s byte for byte the script reads **16256**, twice in a row, and
   **16256 again from the tune build** -- so it is not a build-configuration
   artefact. 16256 is also the number phase one recorded for its *own* tree.
   One of those two labels is wrong and this revert cannot say which without
   rebuilding phase one's tree, which is out of its scope. What ships is
   derived from the tree that ships, measured today.
2. **The script's drift line cannot read "inside" for a freshly derived
   band, and that is arithmetic rather than drift.** A band of
   `[count / 5, 4 x count]` puts the count at `0.8 / 3.8` = 21.05 % of its own
   span, below the lower quartile, so `middle half [18694, 49581]: the count
   is OUTSIDE it` is what the script prints **on the pair it has just
   derived** -- verified after the edit, with `shipping` now equal to the
   derived pair and the count at 5.00x the floor and 0.25 of the budget. Three
   step files in a row have read that line as a verdict on the band. It is a
   defect in the trigger, not in the tree: reach none into play, none onto the
   UCI surface, none into any reported score, move or line. **Proposed to the
   coordinator as filler behind the next strength step** (BUGS as scoped by
   DEC-171), one change at a time -- either the ratios or the line, not both,
   and not inside this commit.

### Documents

- `MANUAL.md`: reverted whole, which is exactly the change owed -- three rows
  out, `ContHistWeight` back to 0 to 2000 at 26. `plan_prose_check --params`
  passes against the reverted header.
- `DEV_MANUAL.md`: the bench ledger's S231 entry gains the revert and its
  `4646334`, read as the ledger's second equality after S098 verdict 1's
  `5685915` and as the whole-revert proof; the S231 lane paragraph stays as
  history and gains the verdict, the fact that the six fitted values are not
  in the engine, and the fact that **`ContHistWeight` is declared 0 to 2000
  again** -- so `tools/spsa_s222.json` passes `spsa_driver.py check` once more,
  where the halving refused it by name for two days. The golden index's row
  for this case carries the new pair.
- `adocs/data/README.md`: no row leaves, none added.
- `README.md`: human-owned, untouched.
- `adocs/specs.md`, `adocs/plan.md`, `adocs/status.md`, `adocs/decisions.md`:
  not edited (hard limit). Wording proposed below.

### Proposed `specs.md` amendment, for the coordinator

The whole S231 passage in the search row -- the one added on 2026-09-18 and
amended by phase three -- becomes history. Replacing it:

> **A two-ply continuation history table was measured here between 2026-09-18
> and 2026-09-20 and is not in the engine (S231).** It was
> `cont_hist2[12][64][12][64]` on `search_state_t`, keyed on the (piece, to) of
> the move two plies back and this move's, written at every quiet cutoff beside
> the one-ply table and summed into `score_move`'s quiet return on a weight of
> its own; its three axes were fitted together with the one-ply three in a
> six-axis SPSA lane over 60000 games, and the gainer SPRT `{0, 5}` nElo
> against `3a649c0` -- one vector under one verdict, DEC-210's reading --
> **accepted H0 on 2026-09-20 at `nElo -3.40 +/- 6.20` over 12070 games**
> (`adocs/data/S231_sprt.log`). The pre-registered reading returned `src/` to
> `3a649c0` whole, so `ContHistWeight` is declared 0 to 2000 again and the
> one-ply three are S222's own fitted 17 / 18 / 26. The census on the fitted
> build read the two-ply term consulted on 95.72 % of quiet scores and non-zero
> on 27.51 % of those, against the one-ply table's 19.89 %
> (`adocs/data/S231_census.txt`), so the zero is about the technique as built
> here and not about inert wiring. The lane and the verdict are kept as
> evidence; `bench` is `4646334`, the reference's own total to the node.

The S222 passage's band sentence returns to one weight, which is what phase one
flagged as false in two clauses and what the revert makes true again:

> "`ContHistWeight` decides how much of the quiet band the term spans, so the
> band is `[-(QuietHistoryMax + ContHistWeight x 32767 / 100), +the same]` and
> still clears the countermove band by 100 at both declared maxima -- 688107
> against 700000, asserted at both edges."

That is `3a649c0`'s own wording and `tests/test_evaluation.cpp` "the declared
history ceiling clears the band above it" asserts it again unchanged.

### Proposed decision, for the coordinator

Agents do not write `adocs/decisions.md`; this is the proposal the step's
`accepts` asks for -- "H0 records the zero and the two-ply idea leaves the plan
with a decision saying why".

> **The two-ply continuation history table leaves the plan, on its own
> measurement.** S231's gainer SPRT `{0, 5}` nElo against `3a649c0` accepted H0
> at `LLR -2.95`, `Elo -2.65 +/- 4.82`, `nElo -3.40 +/- 6.20` over 12070 games
> with 0 time forfeits. The nElo interval is [-9.60, +2.80] and its centre sits
> below zero, which is the shape DEC-194 did not keep.
>
> **Why the verdict is about the technique and not about the wiring**, which is
> the question DEC-194 needed a census to answer for the first table and which
> `I04_cont_hist2_unread` was written for: on the fitted build the move two
> plies back exists at 94.24 % of quiet cutoffs, the term is consulted on
> 95.72 % of quiet `score_move()` evaluations, and **27.51 % of those
> consultations read a non-zero entry against the one-ply table's 19.89 % on
> the same run**, with a same-tree control at the incumbent vector putting
> every share within four tenths of a point
> (`adocs/data/S231_census.txt`). The table was exercised, it had more to say
> than the table beside it, and it still bought nothing. Nor was it unfitted:
> its own narrow lane moved all six continuation axes over 60000 games and no
> axis touched a bound.
>
> **Recorded as a zero and kept as a zero** -- S005, S006 and S015 are the
> precedent that a measured zero is recorded as one, and DEC-194 the precedent
> for not keeping a change whose interval sits below the bound. The idea does
> not return without a reason the record does not already contain; the
> published treatment of it as "the second half of the one-ply idea" is exactly
> the kind of figure DEC-019 says decides what to try and never what to
> conclude, and this is the fourth time it has not transferred.
>
> **What is not owed as a consequence**: DEC-222's two removal verdicts, the
> killer slots and the countermove table, because the history stack they were
> to be measured against did not ship. **What is untouched**: the one-ply
> table's own H1 (DEC-210), because this run measured the pair against a tree
> that already carried it.
>
> **What it cost and what it bought**: one lane night (8 h 40 m), one verdict
> (5 h 38 m), 72070 games, about two agent-days; a measured zero on a published
> idea, the finding that the seeded equal authority grew the tree 17 % where
> the fit took it 5 % below the reference with neither transferring to Elo, and
> one golden whose re-derivation trigger had been standing unread for three
> steps.

### Proposed commit text, for the coordinator

`tools/gate.sh`'s block check is triggered by an `SPRT |` line and this commit
carries none: the verdict was closed by `b06a5c8`, which carries DEC-220's
block. This one owes `Bench:` alone, and the gate verifies it against the
built binary.

```
Revert S231's two-ply continuation history on H0, DEC-194's shape

The gainer SPRT against 3a649c0 on noob_3moves.epd accepted H0 at LLR
-2.95, nElo -3.40 +/- 6.20 over 12070 games with no forfeits, recorded in
b06a5c8, and the pre-registered reading binds: one vector under one
verdict, so the table, its three axes, the halved ContHistWeight ceiling
and the one-ply three go together. The one-ply three land on S222's own
fitted 17 / 18 / 26, which is what 3a649c0 carries, so no axis is left
without a verdict.

The census on the fitted build read the two-ply term non-zero on 27.51 %
of quiet scores against the one-ply table's 19.89 %, with a same-tree
control, so the zero is about the technique as built here and not about
inert wiring. src, the three test files and the two mutant registries
return to 3a649c0's content byte for byte; S231's own mutant script goes;
the lane, the census and the SPRT evidence stay under adocs/data.

One golden moves with the tree and is the only edit beyond 3a649c0:
"ordering keeps the tree small" re-derived by adocs/data/S192_node_budget.py
from a count of 16256, the pair 69804 and 3490 becoming 65024 and 3251.
That trigger had been standing since S222 while two step files recorded it
as not fired, which is the finding S231_sprt.sh named before a game was
played.

Bench: 4646334
```

