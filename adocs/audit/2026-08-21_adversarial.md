# Audit 2026-08-21 — adversarial

Type: `adversarial`.
Commit under review: `8b4f63d3e29a495dc8530c7843f1df963d3da075`.
Report id stem: `2026-08-21_adversarial`.
Requested as: assess the work done toward 3000 — the latest completed steps, the
code, the tests and the gain — against the literature and the open-source record.

## Scope

**In scope and examined.** The thirteen steps completed since the S088 anchor of
2026-08-18: S089, S094, S103, S104, S105, S106, S107, S100, S084, S137, S085,
S138, S145. Their code in `src/`, their tests in `tests/`, their data under
`adocs/data/`, and the claims they left in `adocs/specs.md`, `MANUAL.md`,
`DEV_MANUAL.md` and `adocs/plan.md`. The measurement instrument
(`fastchess.sh`, the SPSA driver, the bounds). `adocs/decisions.md` was grepped
by topic, never read whole.

**Method.** Every finding reproduces from a command quoted with it. Where a
claim needed the published record it was fetched, not recalled. Prior reports
under `adocs/audit/` were not read as a brief.

**Machine state.** No timing, nps or match figure below is new — the ones quoted
are the project's own, because the review itself loaded the machine. The
instrumented counts in F01 were taken on scratchpad builds after the machine
went idle and are reported with their method.

**NOT COVERED, and this is a hole rather than a scoping choice.** Two of the
commissioned investigations were lost to a session limit and were not re-run by
the owner's decision:

| surface | state |
|---|---|
| the Elo-accounting ledger and the plan's forecast arithmetic re-derived | **not done.** The appendix table is transcription from the `done:` stamps, not a re-derivation, and the plan's +180-to-280 / +90-to-160 / +40-to-90 bands were **not** checked against the published per-feature record |
| a per-parameter survey of the shipped values against comparable open engines | **not done.** No claim below says a shipped value is inside or outside published practice |

Treat their absence as absence. In particular, nothing here validates or
challenges the plan's own Elo forecast.

**Nothing in the repository was modified except this report and the step files
it names.**

## Verdict

**One real defect in shipped code, found in the move ordering and not in
anything the recent steps touched — F01. Everything else holds.**

The three claims that carry the recent gain survive:

- **S085's +21.02 Elo is real.** The SPRT ran the shipping build on both sides
  (`CHESSO_TUNE=OFF`, `Release`, `native`, PGO off — verified from both
  `CMakeCache.txt` files), against the immediate parent `3488506` whose only
  executable difference is the twelve constants, on a book and a control the
  tuning run did not use, at bounds `[0.00, 5.00]` — the *gainer* form, not a
  non-regression. Every printed figure reproduces from the pentanomial
  `Ptnml [121,289,527,363,173]` → 53.021 % → `Elo 21.02`, `±9.86`,
  `nElo 26.81`. The optional-stopping correction is **smaller** than the step
  file assumes, not larger: at `P(H1) = 1.0000` only overshoot remains, about
  **+1.3 Elo**, so the true effect is near **+19.7**. The 2946-game stopping
  time corroborates it — a true +5 effect takes a published ~25 600 games at
  these bounds.
- **S104's +18.22 % is held out and node-identical**, and the project states it
  as a conversion rather than a verdict, which is the correct treatment.
- **S145's mate set survived independent re-proof in full.** All 48 positions
  re-verified from scratch by two oracles written without reference to the
  repository script: 48 of 48 confirmed as forced mates at exactly the claimed
  distance by an AND/OR enumeration iterated to distance 6, and 48 of 48 by
  Stockfish at 20 M nodes. 0 disagreements. The construction script regenerates
  the tracked TSV **byte-identically** (`md5 de60ad1d1cf9c04935d2a27dbc50b6db`),
  the fence is non-vacuous and goes red at exactly the value the tuner sat at,
  all 104 defender nodes clear the reverse-futility margin and none is in check,
  and the suite is really in the `fast` label (18/18, 15.72 s).

The rest of the findings are about what the record claims versus what the
artefact does (F02, F03, F06, F08) and where the plan spends its scarcest
resource (F04, F05, F09).

**Read F01 first and note what the house rules say about it.** AGENTS.md §0: "A
bug that has been found gets fixed before anything else starts. Not noted, not
scheduled, not carried into the next change." F01 is a found bug in shipped
code, so by the project's own rule it jumps ahead of S142.

## Findings

10 findings: 1 high, 5 medium, 3 low, 1 informational.

---

### 2026-08-21_adversarial-F01 — high — a repeated fail-high destroys the second killer slot, on 44 % of nodes, and the test that exists to prove the slot works cannot see it

Status: accepted — DEC-098, S149

**The defect.** `src/search.cpp:761-762`, the whole store:

```cpp
if (!is_capture) {
  state->killer_moves[1][ply] = state->killer_moves[0][ply];
  state->killer_moves[0][ply] = moves[i];
```

There is no guard for `moves[i] == state->killer_moves[0][ply]`. When the same
quiet move fails high twice at the same ply, slot 0 is copied into slot 1 and
**both slots then hold the same move**. `grep -n killer_moves src/*.cpp src/*.hpp`
returns exactly four lines — the two above and the two reads — so no dedupe
exists anywhere.

**Why that costs ordering.** `src/evaluation.cpp:1160-1161` tests slot 0 first:

```cpp
if (move == state->killer_moves[0][ply]) { return ORDER_KILLER_0; }
if (move == state->killer_moves[1][ply]) { return ORDER_KILLER_1; }
```

With both slots equal, the second test can never award `ORDER_KILLER_1`
(800000) to a distinct move. The second-best refutation at that ply falls to the
countermove band (700000) or into history (≤ 600000, `ORDER_HISTORY_MAX`), so it
is searched later than the design intends. The slot is not merely redundant, it
is dead.

**Why it is the common case and not an edge case.** `src/chesso.cpp:647` builds
`search_state_t state = {}` **once per `go`**, so killer slots persist across
every iteration of iterative deepening. A quiet move that refutes a node at
depth *d* is overwhelmingly the same move that refutes it at *d+1*, and each
iteration stores it again — the second store is the one that duplicates the
slot. The structure guarantees the frequency.

**Measured.** Instrumented copies of the engine under
`/tmp/.../scratchpad`, release build, 11 positions at depth 12–22, ~19 M nodes:

```
killer stores that re-store the move already in slot 0:  351422 / 532133  = 66.0 %
negamax nodes with both slots holding the same non-zero move: 5115505 / 11531069 = 44.4 %
```

Reproduced on an independently configured sanitizer/tune build: 62.8 % and
22.3 %. The two builds differ in the search they actually run, so the second is
corroboration of the mechanism, not a repeat of the same number.

**Why nobody noticed, which is the part worth keeping.**
`tests/test_search.cpp:477-499` is the test written for exactly this slot:

```cpp
for (size_t ply = 0; ply < MAX_PLY; ++ply) {
  if (state.killer_moves[0][ply] != 0) { killers_0++; }
  if (state.killer_moves[1][ply] != 0) { killers_1++; }
}
...
// The second slot only fills once a ply produces a second killer, which
// is what the shift down from slot 0 is for.
REQUIRE(killers_1 > 0);
```

It counts slots that are **non-zero**, never slots that are **distinct**. A
duplicated slot 1 is non-zero, so the assertion passes while the property it
describes in its own comment is false. This is the vacuity §6 warns about — "a
test asserting X does not happen must first establish the precondition that
would make X happen" — in the version that is hardest to see, because the test
does fire and does pass.

**Published form.** The Chess Programming Wiki's *Killer Heuristic* page states
the rule directly: "The replacement scheme ought to ensure that all the
available slots contain **different** moves." Its stated rationale is exactly
the capability lost here — "most of the cutoffs come from the first killer slot.
But occasionally opponent does something important, like attacking a queen …
That's where the second slot comes in handy."
https://www.chessprogramming.org/Killer_Heuristic

**Not recorded anywhere.** `grep -rn killer adocs/ --include=*.md` filtered for
duplicate, same-move or both-slots returns nothing. S107's stamp records only
the promotion wrinkle; `adocs/plan_todo/S093_history_malus_gravity.md` does not
mention it; `decisions.md` does not.

**Attribution, stated so the ledger stays honest.** This is **pre-existing and
not caused by any recent step** — the store has had this shape since the killer
tables were written. S107 is adjacent, not culpable: by admitting checking
quiets it widened the class of moves entering the table, so the slot is consumed
more often, but the missing guard predates it.

**What the fix costs.** Two lines and a verdict. It alters play, so INV-6 is not
available and it owes an SPRT; the direction is expected positive but the
project's own DEC-019 ledger is three published figures that measured 0, 0 and
*slower*, so the number is not predictable from the argument. Note the ordering
constraint: S093 rewrites this same block (history malus, gravity, butterfly
indexing), so the guard should land **before** S093 or be folded into it
deliberately — not after, or S093's verdict is measured on top of an unfixed
slot. And the test has to be re-targeted from "non-zero" to "distinct" in the
same change, red first.

**Resolution, appended 2026-08-21 — accepted, not fixed. DEC-098, S149.** The
finding's mechanism reproduced exactly: the same four counters at the same
placement, over the same 11 positions, read 351422/532133 stores and
5115505/11531069 nodes at `ac4c588`, to the digit. The guard this finding asks
for was implemented -- two lines, CPW's replacement rule -- and it worked: 0
duplicated nodes of 11146351 after it, negamax nodes -3.34 %, total nodes
-2.87 %. It then measured **-11.02 +/- 10.53 Elo, nElo -14.21, LLR -2.97 at
[-5, 5], H0 accepted over 2522 games in 1 h 05 m, 0 forfeits**, and was
reverted. The duplication is engine behaviour by measurement.

The finding's second half stands and was acted on regardless of the verdict:
the test counted **non-zero** slots and now counts **duplicated** ones,
asserting what shipped and carrying the number that decided it, so an agent who
re-guards the store goes red and finds this instead of repeating the night. Its
non-vacuity was checked in both directions -- green on the shipped store, red
under a re-applied guard.

What the run did *not* establish is why 11 Elo. The unguarded shift also ages
slot 1, discarding it on every repeat; the guard preserves a stale killer for a
whole `go`. That is a hypothesis, unmeasured, and it is S159.

---

### 2026-08-21_adversarial-F02 — medium — three parameter values S085 shipped are still stated at their old values in `specs.md` and `MANUAL.md`, and no check in the tree can catch that class

Status: closed — S150

**Evidence.** `src/search_params.hpp` is the single source of the defaults. The
live values and the prose disagree in three places:

```
$ grep -o "Aspiration windows search the root of each iteration from depth [0-9]* in a band \*\*[0-9]* centipawns\*\*[^.]*\." adocs/specs.md
Aspiration windows search the root of each iteration from depth 5 in a band **50 centipawns** either side of the previous iteration's score, doubling the failing side alone and going to the full window past 400; ...

$ grep -o "quiescence is capped at [0-9]* plies" adocs/specs.md
quiescence is capped at 8 plies

$ sed -n '250p' MANUAL.md
  *Aspiration windows* are present since S021 (2026-08-17): from depth 5 the
```

against the shipped table — `MaxQsearchDepth` **19**, `AspirationMinDepth` **2**,
`AspirationDelta` **21**, `AspirationMaxDelta` **437**
(`src/search_params.hpp:51`, `:168-170`).

So `adocs/specs.md:171` states 5 / 50 / 400 where the engine runs 2 / 21 / 437,
`adocs/specs.md:178` states a quiescence cap of 8 where the engine runs 19, and
`MANUAL.md:250` states depth 5 where the engine runs 2. `MANUAL.md`'s own option
table at `:125-134` **is** correct, so `MANUAL.md` contradicts itself eleven
lines apart.

This is not general staleness. S085's commit `21b4a21` claims the doc sweep was
done — "Doc claims move with the code … and specs.md's margin all follow." The
margin sentence in that same table row was updated; the aspiration triple and
the quiescence cap were not, and `S085_spsa_first_run.md` mentions `specs.md`
nowhere. `specs.md` is first in the precedence order of §1, so this is the
document a reader is told to trust over plan and status.

**Why nothing caught it, which is the more useful half.** Three checks exist and
none of them can:

- `tools/plan_prose_check.py` has exactly two modes, `--prose` (a completed step
  described as pending) and `--citations` (a `path:line` citation that no longer
  holds what it is cited for). Neither compares a number in prose to a number in
  code.
- `tests/test_uci_surface.cpp:88-91` builds its expected option lines *from*
  `search_param_info()`, so by construction it cannot detect a value drift — it
  restates whatever the code says.
- `tests/test_search_params.cpp`'s golden list does hold the live defaults, and
  it did its job: it went red on exactly the ten changed values. It guards the
  code against itself, not the prose against the code.

The four pending document steps do not close it either: S139 checks `accepts`
fields, S140 retired ids and undefined invariants, S141 `touches` fields, S144
citation paths. **None checks a value.** Yet §7's own words are "Doc claims are
claims about code".

**One honest limit on the fix.** The two `specs.md` sentences never name the
parameter, so a name-adjacency scan alone misses them — I ran one over
`specs.md`, `MANUAL.md`, `DEV_MANUAL.md`, `CLAUDE.md` and `plan.md` and it found
only legitimate historical records and example invocations. A checker has to key
on the English phrase, which means the phrase set is the maintenance cost.

---

### 2026-08-21_adversarial-F03 — medium — every verdict in the plan is taken at one short control, the target list plays one about an order of magnitude longer, and the published record says that transfer is poor

Status: open

**Evidence.** The regime is single-control by construction. `fastchess.sh` plays
`8+0.08` (S105, DEC-088). S085's SPSA ran at `2+0.02`. The target is stated in
`adocs/plan.md`: "at least 3000 on the CCRL Blitz scale … the 1CPU entry
(DEC-089)" — a list whose games are 2 min + 1 s, roughly 180 s a side against
the SPRT's ~13 s a side.

No decision, plan section or step requires a second verdict at a longer control:

```
$ grep -niE "\bLTC\b|long time control|longer control|does not scale" \
    adocs/decisions.md adocs/plan.md adocs/specs.md DEV_MANUAL.md
adocs/plan.md:107:  ... An 8-thread search is worth about +180 at LTC on the published ...
adocs/specs.md:85:  ... the published 1.43 Elo per percent of nps at long time control and 2.10 at ...
adocs/decisions.md:5118: ... an 8-thread search is worth about +180 at LTC on the ...
adocs/decisions.md:5463: ... removing its depth limit as passing SPRT at both STC and LTC while ...
```

Every hit is a citation of someone else's LTC result. The practice is not
adopted, and DEC-095's own amendment quotes the phrase "passing SPRT at both STC
and LTC" while describing Stockfish.

**What the published record says.** Fetched, not recalled:

- vondele, `nevergrad4sf` README: "It appears that the optimal parameters are
  often time sensitive, i.e. can be verified to be a gain at the VSTC used for
  tuning, but regress at STC or LTC", and "Ideally, tuning is performed at the
  TC that is most relevant." https://github.com/vondele/nevergrad4sf
- fishtest wiki, *Creating my first test*, on SPSA: "A short TC is appropriate
  to get faster approximations or for checking the tune's correctness and
  parameters. A long TC (`60+0.6`) is best to get **better scaling values**."
  https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html
- Stockfish issue #2600, "STC-LTC correlation ?": "Out of the 40 last LTC tests
  with elo-gaining bounds, there has been **23 reds, 16 yellows, 1 green**."
  https://github.com/official-stockfish/Stockfish/issues/2600

**Why this bites here specifically, rather than in general.** S085's vector
moves every affected axis in the *same* direction — more pruning and more
reduction:

| axis | before | after | direction |
|---|---|---|---|
| `RfpMaxDepth` | 6 | 15 | reverse futility fires at every depth this engine reaches |
| `LmrBase` / `LmrDivisor` | 75 / 225 | 52 / 182 | a smaller divisor is a **larger** reduction at every depth and move number |
| `MaxQsearchDepth` | 8 | 19 | deeper quiescence |
| `AspirationMinDepth` | 5 | 2 | narrow windows three iterations earlier |

That is the archetype of a short-control-favourable vector, and it was tuned at
`2+0.02`, four times faster again than the verification control — precisely the
regime `nevergrad4sf` warns about. The verification SPRT at `8+0.08` establishes
the vector beats its predecessor *at 8+0.08*; nothing establishes it at 2+1, and
2+1 is the scale the 3000 target is denominated in.

**What is already recorded, and what is not.** DEC-094 excluded the nine `Tm*`
parameters from S085 for exactly this reason, citing a published
+23.8-at-20+0.2 / −22.9-at-10+0.1 case — so the project knows the failure mode
and has acted on it once, for one family. Nowhere recorded: that the same risk
applies to the pruning and reduction axes that *were* tuned, or that the plan
has no gate for it.

**Cost of closing it, in the project's own units.** Do **not** add an LTC gate
to all 45 to 55 verdicts — that would roughly double the plan's machine budget,
and DEC-063 already prices what bounds cost. Add it to the one vector S085
shipped, and state the rule narrowly: a change that moves a pruning or reduction
parameter has its verdict re-taken at a control at least 4x longer before the
number is banked.

---

### 2026-08-21_adversarial-F04 — medium — no absolute-strength checkpoint exists between the 2559 anchor and plan position 62

Status: open

**Evidence.** The anchor: S088, 2026-08-18, "chesso ≈ 2559 CCRL Blitz, 95 %
±25, SOFT: 3340 rated games at 10+0.2 over five engines and three families,
concurrency 6, **5 h 02 m 49 s**". The only pending step that re-rates:

```
$ grep -l "rating.sh" adocs/plan_todo/*.md
adocs/plan_todo/S128_rating_run_at_blitz_time_control.md
adocs/plan_todo/S132_time_management_node_fraction.md   # says "rating.sh-style", not a run
```

S128 sits at **position 62 of 74** in `plan.md`'s order — after the whole search
block, the speed block, the corpus work and the entire evaluation block. Between
here and there the plan owes, by its own count, "roughly 45 to 55 SPRT
verdicts". S128's own file defers it because "it costs about five hours and buys
zero Elo".

**Why the reasoning is the wrong way round for this project.** Five hours against
the plan's own budget of "roughly 75 to 110 machine-hours" is about 5 %. What it
buys is not Elo, it is the only available check that the accumulated per-patch
deltas *are* the engine's strength — and the project has been burned by exactly
that gap: DEC-020 is the contamination where "one run reported +301 Elo and
meant nothing". The plan itself takes "a fifth off for interaction", which is an
admission that summed per-patch deltas are not the total, carried as an estimate
that nothing measures.

**The honest counter, which belongs with the finding.** A rating run resolves
±25 at best, and S088 measured 121.8 Elo of spread between rating families. So a
mid-plan checkpoint can only detect drift larger than roughly 50 Elo. That is
not small — but it is the size the block forecast is written in (+180 to +280 for
the search block), so a checkpoint *after the search block* measures a quantity
it can resolve. After a single patch it would not, and should not be proposed.

**Concrete suggestion.** One rating run after S132, the last step of block 1,
before block 2 begins. Cost 5 h. If it reads inside the forecast band the
remaining ~30 verdicts proceed on a confirmed base; if it does not, that is
found 30 verdicts earlier than S128 would find it.

---

### 2026-08-21_adversarial-F05 — medium — the front of the queue is four consecutive zero-match steps, and the binding constraint is idle while they run

Status: closed — DEC-113, S153

**Evidence.** `plan.md`'s order, positions 15 to 19:

```
15. S142  the two declared parameter ranges that contradict the purpose ...
16. S139  every pending accepts field states something the harness can produce ...
17. S140  no plan, specs or status document routes work to a retired id ...
18. S141  every pending step's touches field names the file its change lands in
19. S093  history gets a malus ...   [the first Elo step of block 1]
```

All four are document or process work. S142's own `accepts` states the shipping
build is "byte-identical"; S139, S140 and S141 touch only `adocs/` and `tools/`.
None owes a match. `.moltke.json` sets `plan_active_max: 1` and plan steps run in
sequence, so the machine has nothing to do for their duration.

The utilisation that produces, summed from the `done:` stamps and the run logs —
the project's own recorded durations, not new measurements:

| run | recorded duration |
|---|---|
| S089 SPRT, 500 games | 0 h 22 m |
| S094 SPRT #1, 3000 games | 2 h 07 m |
| S094 SPRT #2, 1954 games | 1 h 23 m |
| S107 SPRT, 3812 games | 1 h 38 m |
| S085 SPSA, 60000 games | 8 h 21 m |
| S085 verification SPRT, 2948 games | 1 h 15 m (`.tuning/sprt_S085.pid` 03:52 → `sprt_S085.log` 05:07) |
| **total, exactly stamped** | **15.1 h** |

Wall clock from the anchor (2026-08-18 20:33) to this report: **65.4 h**, so 23 %
of the window carried a stamped run. Adding the passes whose durations are not
stamped — S103's interleaved timing and hit-rate counting, S104's ten rotating
triples, S105's two 1000-game calibrations, S100's 10.8 M-row corpus pass,
S145's sweeps over 6347 and 318 positions — puts it at roughly 20 to 24 h, about
a third. The remaining two thirds is agent-only work on a free machine.

**Verdict rate and what it implies.** Five SPRT verdicts in 65.4 h, one per 13 h.
The plan owes 45 to 55. At the observed rate that is **24 to 30 days of
calendar** against 75 to 110 hours of actual compute — the gap is idle time, not
work.

**Stated fairly: this is the owner's sequencing rule, not a mistake.**
`status.md` records "the instruction that plan steps run in sequence", and the
document work is not busywork — S138 found 107 of 201 citations stale and eight
steps pointing at the wrong mate test, which would have misdirected real work.
DEC-096 already protects machine time *from* document work ("No machine time is
booked for the audit's numeric half", because "Measurement capacity is the
binding constraint on the plan"). What is nowhere recorded is the converse: an
agent-only step leaves that same constraint idle, and the two classes contend for
nothing.

**Suggestion, one line of configuration.** Raise `plan_active_max` to 2 with the
rule that at most one active step may hold the machine. Then a document step and
a match step overlap by construction. If the strict sequencing is deliberate on
other grounds it deserves a `decisions.md` entry saying so, because it is
currently the largest single lever on the plan's calendar.

---

### 2026-08-21_adversarial-F06 — medium — `MATE_IN_THREE_FLOOR` has one position of margin, and thirteen tree-reshaping steps are queued next

Status: open

**Evidence.** `tests/test_engine.cpp` sets `MATE_IN_THREE_FLOOR = 7` and asserts
`exact_by_distance[3] >= MATE_IN_THREE_FLOOR`. The counts, from
`adocs/data/S145_rfp_sweep.log` and **independently reproduced** on
`build-tune/src/chesso` at `go depth 2d-1+8`:

| setting | mate-in-3 exact |
|---|---|
| shipping (`RfpMinPly` 3) | **8** of 16 |
| `RfpMinPly` 1 or 0 | 6 of 16 |
| `RfpMinPly` 4 | 11 of 16 |

The floor sits one position below the shipping value. The comment claims the
placement makes it robust — "placed strictly between the shipping value and the
removed-guard value so it fails when the guard fails **and not when the tree
shifts underneath it**". One position of margin out of sixteen does not support
that sentence: any change that costs a single mate-in-3 detection turns the fast
suite red with no guard having failed.

**Why that is not hypothetical.** Plan positions 19 to 31 — S093, S130, S108,
S024, S109, S091, S098, S095, S099, S097, S112, S131, S022 — all change the shape
of the tree, several substantially (S109 lands four pruning rules in one verdict;
S098 rebuilds the reduction). The failure mode that follows is the one S145
itself documented in the surveyed field: "Two projects wrote exact mate-distance
tests, watched pruning break them, and **switched the tests off rather than the
pruning**." A floor that reddens on an unrelated tree change is the mechanism
that produces that outcome here.

**A related doc inaccuracy, in the safe direction.** The step and the sweep log
characterise the `RfpMinPly` 1 failure as "13 of 16" mate-in-two, but the
assertion also requires `first_exact == 2d-1`, so **9 of 16** pass and 7 fail.
The fence is stronger than documented, not weaker. Worth correcting in the same
change so the next reader sizing the margin uses the real number.

**Suggestion, cheap either way.** Either widen the margin deliberately and state
what tree-shift size it tolerates, or keep 7 and add the sensitivity measurement
the comment currently asserts without evidence — how many mate-in-3 detections a
known-neutral tree change actually moves. The harness for the second already
exists.

---

### 2026-08-21_adversarial-F07 — low — the S145 constructed set is 48 positions and one motif, which is the criticism the step was written to answer

Status: open

**Evidence.** S145's goal line indicts the gate it replaced: "so the floor that
fences the tuner rests on evidence rather than on **three positions and one
motif**", and its body at `:105` says "three positions are **one geometry,
mirrored, plus one padded variant**".

The replacement is 48 positions over two geometries. Every case in
`adocs/data/S145_mate_set.tsv` is a fully blocked pawn wall (`p1p1p` against
`P1P1P`) with a rook-bishop-rook-bishop battery behind it, one queen, two kings,
and `lead` = 760 in every row. The `family` column names the only variation:
`shift0` / `shift2` (which file the wall sits on), `_flip` (mirror), `_black`
(colour swap).

**This is partly forced, and the report should say so.** The construction the
`accepts` inherited from S033 requires the mated side to be far *ahead* on
material while unable to act — that is what makes the static score clear
`RFP_MARGIN * depth` at a node that is a forced loss. A blocked wall with
immobile pieces is close to the only way to build it, which is why every row
reads `lead` 760 (verified as standard material: 2R+2B+3P = 1960 against
Q+3P = 1200). The criticism is not that the work was careless, and the set is
sound — every position was independently re-proved.

**What is wrong is the claim, not the set.** Neither the step file nor the test
comment says the replacement carries the same monoculture, and the `done:` stamp
reads "48 proved mates spanning distances 2 to 5" — breadth in mate distance,
which is real, presented without the qualifier that breadth in motif is absent.
The consequence bounds what the gate can ever catch: no back-rank mate, no
smothered mate, no king hunt, no open-line sacrifice, no position with a
realistic material balance. A future pruning rule that hides mates in any of
those shapes passes this suite.

---

### 2026-08-21_adversarial-F08 — low — the mined breadth set the `accepts` asked for is not asserted anywhere

Status: open

**Evidence.** S145's `accepts` requires "a mined breadth set built from
`.spsa/S085/games.pgn` at one position per game, labelled by `stockfish`, scored
as a **count with a floor** and not per-position pass/fail".

The data and the script exist — `adocs/data/S145_mined_set.tsv` is 318
positions, `adocs/data/S145_mined_set.py` regenerates it. Nothing consumes them:

```
$ grep -rn "mined_set\|S145_mined" tests/ src/ CMakeLists.txt
tests/test_engine.cpp:1694:  // that does not occur in play. adocs/data/S145_mined_set.py covers breadth
```

One comment. No assertion, no floor, no ctest registration. The `done:` stamp
does not mention the mined set, and its numbers appear only as prose in the step
body ("the mined set of 318 positions reads 146 exact at `RfpMinPly` 2 and 3 and
139 at 1 and 0").

**Reading it fairly.** "Scored as a count with a floor" may have meant a
*measurement* method rather than a gate, and on that reading the clause is met by
the body text. But the same phrase is what the constructed set's real assertion
uses — `MATE_IN_THREE_FLOOR` — so within this step the phrase means an asserted
floor everywhere else it appears. Either the clause is discharged and the stamp
should say how, or the floor is owed. This is the class S139 exists for.

---

### 2026-08-21_adversarial-F09 — low — the SPRT bounds are normalized Elo, and eight recorded conclusions restate them as plain Elo

Status: closed — S157

**Evidence.** `fastchess.sh:226` passes `model=normalized`, and the installed
binary documents what that means:

```
$ /usr/games/fastchess --help | grep -A4 'model'
    model: 
      · normalized - Uses nElo (default). 
      · logistic - Uses regular/logistic Elo. 
      · bayesian - Uses BayesElo.
```

So `elo0=-5 elo1=0` is a bound in **nElo**. The conversion for a real run comes
from that run's own two printed figures — S085 printed `Elo 21.02` and
`nElo 26.81`, a ratio of 1.2755 — so at that draw rate 5 nElo is **3.92 logistic
Elo**.

Eight recorded sentences translate the bound as plain Elo:

```
$ grep -oE "not a regression of 5 Elo or more|a regression of 5 Elo or more is excluded" \
    adocs/plan_done/*.md adocs/specs.md | sort | uniq -c
      2 adocs/plan_done/S021_aspiration_windows.md:not a regression of 5 Elo or more
      1 adocs/plan_done/S068_rfp_margin_retune.md:not a regression of 5 Elo or more
      1 adocs/plan_done/S076_corpus_zobrist_dedupe.md:not a regression of 5 Elo or more
      2 adocs/plan_done/S107_killers_for_checking_quiets.md:not a regression of 5 Elo or more
      1 adocs/specs.md:a regression of 5 Elo or more is excluded
      1 adocs/specs.md:not a regression of 5 Elo or more
```

Each excludes a regression of about 3.9 logistic Elo, not 5. `DEV_MANUAL.md:1274`
records that "every SPRT verdict here runs `model=normalized` and reports nElo" —
the reporting is documented; the bounds' scale is never connected to the
conclusions drawn from them.

**The model choice itself is correct and must not be changed.** The CPW table the
comment cites — fetched from
https://chessprogramming.org/Sequential_Probability_Ratio_Test — lists gainer
bounds `[0, 2]` for "Stockfish STC" and `[0.5, 2.5]` for "Stockfish LTC", which
are fishtest's own bounds, and fishtest expresses bounds in normalized Elo
("Test bounds in Fishtest are expressed in terms of 'normalized Elo'"). The table
states no scale, but its Stockfish rows identify it: it is nElo throughout, so
`model=normalized` with `{0,5}` matches the cited source exactly.

**Do not read this as an instrument defect.** `elo0` is scale-invariant at 0, so
every "the sign is positive" verdict recorded so far stands untouched. What is
owed is the wording of eight conclusions and one sentence in `DEV_MANUAL.md`
saying the bounds are nElo, so the next reader does not redo this.

---

### 2026-08-21_adversarial-F10 — informational — the shipped opening book's digest, for S146

Status: closed — S158

**Evidence.** S146 records that `src/openings.book` is compiled into the shipped
binary with no recorded origin or licence. The step needs an identifier to close
against; here it is, decoded from the hex `#define`:

```
decoded bytes 2610256   entries 163141 (16-byte Polyglot)
sha256        47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5
first entry   key 00000883b144421f  move 0dae  weight 002d (45)  learn 00000000
```

`Use Book` defaults false (`src/chesso.cpp:930`), so this is a provenance and
licence exposure only — it cannot have contaminated any measurement and no
verdict is at risk. Recorded here because the digest is what makes the file
searchable against a published book, and computing it costs nothing.

## Checked and clean

Listed because a negative result is a result, and because several of these are
where a reviewer would expect to find something.

**S085's verification SPRT.**
- Both sides are the shipping build. `build/CMakeCache.txt` and
  `.ref-builds/3488506/build/CMakeCache.txt` agree on `CHESSO_TUNE:BOOL=OFF`,
  `CMAKE_BUILD_TYPE=Release`, `CHESSO_ARCH=native`, `CHESSO_PGO=off`,
  `SANITIZER=OFF`. Confirmed behaviourally: the reference binary exposes only
  `Use Book`, `Hash` and `Threads` over UCI, so no `setoption` could have reached
  a search parameter on either side.
- The base is the immediate parent. `git rev-parse 21b4a21^` = `3488506`, and
  `git log 3488506..21b4a21` is one commit whose only executable change is the
  twelve constants (`src/evaluation.hpp`'s diff is comment text only).
- The reference side really is the old vector: `tools/search_bench.py` on it reads
  164302 midgame nodes, the figure the step's own pre-run probe recorded at
  `RfpMargin` 75.
- Bounds `[0.00, 5.00]`, the gainer form (`.tuning/sprt_S085.log:7102-7119`).
- The SPSA run's PGN holds exactly 60000 `[Result` lines and **0**
  `[Termination "time forfeit"]`.

**The optional-stopping caveat is present**, in the step body at `:745-748`. Only
the one-line `done:` stamp omits it, and the correction runs in the project's
favour — see the Verdict.

**No double counting.** `+21.02` appears in the step file and nowhere in
`plan.md`, `specs.md` or `status.md` as a banked figure. The `+21.10` those files
carry is S065's evaluation fit, a different step.

**The SPSA schedule constants are the published ones.** `alpha 0.602`,
`gamma 0.101`, `A = 0.1 x iterations` are Spall 1998 verbatim (IEEE T-AES
34(3):817-823: "Practically effective … values for α and γ are 0.602 and 0.101";
"we frequently take [A] to be 10 % … of the maximum number of expected/allowed
iterations"). 1250 iterations x 24 pairs = 30000 pairs = 60000 games.

**Games per parameter is above the only published anchor.** 60000 over 12 axes is
5000/parameter. Kiiski's announcement of the method — which CPW's *Stockfish's
Tuning Method* page quotes — is "7-35 variables at the same time" and
"30000-100000 super-fast games", i.e. 2857 to 4286 per parameter at the widths he
names. No source states an "X games per parameter" rule; figures of
10k-40k/parameter circulate but were confirmed nowhere.

**Boundary absorption on the other eleven axes: refuted.** Over all 1250
trajectory rows, ten of twelve axes touched neither declared bound at any
iteration. Only `RfpMinPly` (72.5 % at its min) and `AspirationMinDepth` (55.6 %)
did, both are stated in the step file, and the asymmetric treatment is coherent:
`RfpMinPly`'s 0 is illegal in purpose, fails 3 of 18 mate cases and is
byte-identical to 1, while `AspirationMinDepth`'s 2 is an arithmetic floor (depth
1 has no previous score to centre a band on). `tests/test_engine.cpp:1617-1622`
documents the side effect the move had on the exempt-region test rather than
hiding it.

**Vector integrity.** `src/search_params.hpp` at HEAD, the tuner's returned
vector, and the golden list in `tests/test_search_params.cpp` agree on all twelve
axes: 19 / 63 / 15 / 3 / 3 / 6 / 52 / 182 / 184 / 2 / 21 / 437.

**Reverse futility has its own verdict on this engine**, so "the rule is
unmeasured here" would have been false: S033 measured `+59.98 ± 17.24`, H1
accepted, 1012 games.

**S085's new values create no new reachability defect.** Quiescence at
`MaxQsearchDepth` 19 is bounded by `ply + 1 >= MAX_PLY` (`src/search.cpp:270`)
independently of the parameter, and quiescence indexes no per-ply array;
measured `max_qply` 19 with the cap fully reached. `RFP_MARGIN * depth` at 15
reached a measured maximum of 945 against `MATE_MIN` 48000 with **0** mate-band
returns over 19 M nodes, and even the declared maxima (2000 × 63 = 126000) do
not overflow int32 — they only make the rule inert. A sanitizer + assertions +
tune build driven at the declared extremes (`MaxQsearchDepth 64`,
`RfpMaxDepth 63`, `RfpMinPly 0`) over 6 positions produced **0 ASan, 0 UBSan and
0 assertion failures**.

**S103's stored-eval read is exact.** The reverse-futility site compared against
a fresh `evaluate()` on the same node: **923383 reads, 0 mismatches, max diff 0**
(44611 / 0 on a second build). That rules out collision (`tt_get_entry` compares
the full 64-bit key), bound-instead-of-score, and staleness at once.
`TT_EVAL_NONE == INT16_MIN` and the store clamps a real eval to
≥ `INT16_MIN + 1`, so "no eval" is distinguishable from `eval == 0`.

**S094's table discipline is correct.** `tt_entry_answers` de-normalizes before
comparing and is the only reader; the mate round trip
`de_normalize_score(normalize_score(s, ply), ply) == s` held over 19 M nodes with
**0** failures; `TT_DEPTH_QS = -1` never answers the main search, because the
lowest depth negamax is entered with is 0 — a margin of exactly one, worth
knowing before anyone stores at −1 elsewhere; and stand-pat cannot be raised by a
bound.

**The move-ordering bands still clear.** `MVV_PAWN` 100, `MVV_KING` 100000,
`ORDER_CAPTURE` 1000000 → worst capture 900100 > `ORDER_KILLER_0` 900000.
`piece_values_abs` is built from the `MVV_*` macros, which `search_params.hpp:29`
excludes from the tuned set, so S085 did not move them. S107's promotion
argument also verifies: `score_move` returns the capture band for any promotion
at `src/evaluation.cpp:1158` *before* the killer tests, so a quiet promotion in a
killer slot is a dead slot and never a mis-ordered move. (The stamp cites
`:1097/:1099`; the current lines are `:1158/:1160-1161`.)

**S145 verified end to end.** See the Verdict. Additionally: the script's
`verify` mode reports "stockfish corroborates 46 of 48 at 4000000 nodes", and the
two that miss are a node-budget artefact rather than a property of the positions
— at `go mate d nodes 20000000` in fresh processes it is 48 of 48. The
independent enumeration's node counts also reproduce the TSV's `proof_nodes`
column exactly.

**The book overlap between tuning and verification is not zero but is
negligible.** S085 asserts the two books are disjoint; measured on position
identity, 1441 of the 30000 openings the SPSA played (4.80 %) also appear in
`books/UHO_Lichess_4852_v1.epd`. The SPRT drew 1473 rounds from 2632036
openings, so the expected number of verification rounds landing on an opening
the tuner actually played is 0.8. The substantive rule — never tune and test on
the same opening set — is met, and nothing follows from the wording.

**The `static_assert(ASPIRATION_MIN_DEPTH >= 2)` was converted to a runtime
`REQUIRE`, not deleted** (`tests/test_engine.cpp:1628`), with the reason in the
comment above it, and S143 owns the gate gap it exposed. No test was weakened.

**The release build takes the tuned values through the same `X()` table** as the
tune build, as `inline constexpr int`. No divergence beyond the intended
constant-versus-variable difference the file's header states.

**`fastchess.sh` really passes `-check-mate-pvs`** (`:105`), verified against the
installed `fastchess` alpha 1.8.1.

**The SPSA-near-`RfpMinPly`-0 wrinkle is acknowledged, not glossed.** Commit
`21b4a21` states the other eleven values were tuned jointly with that axis near
0, and DEC-095 explicitly defers the re-run on the narrowed bound as premature
rather than forgetting it.

**`plan.md`'s "+180 at LTC" citation is accurate as a citation.** The published
figure is 178.6 ± 14.0 Elo for 8 threads against 1 at 60+0.6 over 1000 games
(Stockfish wiki *Useful data*, measurement by vondele, January 2020, pre-NNUE).
It is one match from one era, and the rating-list evidence for 1→4 cores
extrapolates lower — but the plan cites it to *retire* threading from phase one,
so nothing turns on its precision.

**The plan's central premise has at least one independent existence proof.**
Stash v27 is rated about 3057 on CCRL Blitz with no network, which is the claim
`plan.md` rests on — that 3000 is reachable with a hand-crafted evaluation.

## Appendix — what has been measured since the anchor

Transcription from the `done:` stamps, **not** a re-derivation — see the scope
note. The point of the table is the instrument column.

| step | claim | instrument | alters play |
|---|---|---|---|
| S089 | `+45.42 ± 23.47`, recorded as "not a regression, sign positive" | SPRT `[0,10]`, 500 games | yes |
| S094 | **0** and **0** | two SPRTs, 3000 and 1954 games | yes, kept at zero (DEC-079) |
| S103 | `+2.47 %` speed, node-identical | interleaved timing, 24 pairs | no (INV-6) |
| S104 | `+18.22 %` speed, held out; `+26.1` Elo **stated as a conversion** | 10 rotating triples, 400 unseen positions | no (INV-6) |
| S105 | harness `x1.67` throughput | two 1000-game A/A calibrations | n/a |
| S106 | nothing found, recorded as such | source walk + 9 red-first tests | no |
| S107 | `+12.67 ± 8.65`, recorded as "not a regression of 5, sign positive" | SPRT `[-5,0]`, 3812 games | yes |
| S100 | diagnosis, no weight shipped | 10.8 M-row refit, finite differences | no |
| S084 / S137 | driver and observability | synthetic gate, real-binary check | no |
| S085 | **`+21.02 ± 9.86`, H1 at `[0,5]`** | SPRT, 2948 games, shipping build both sides | yes |
| S138 / S145 | citations, mate set | checkers and two oracles | no |

**One verified positive verdict with a magnitude (S085), two sign-only positive
verdicts (S089, S107), two recorded zeros (S094), and two speed results of which
one is converted to Elo and labelled as a conversion.** That is the state of the
gain since 2559. Nothing in the tree overstates it — which, together with F01
being pre-existing rather than recently introduced, is the substance of this
review.
