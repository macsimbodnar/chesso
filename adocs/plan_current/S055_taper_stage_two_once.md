id:         S055
goal:       taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
accepts:    evaluate() performs one fewer integer division; the tuner-model tolerance returns to 2; the pinned truncation thresholds in tests/test_eval_model.cpp are re-measured in this step's own commit and re-pinned to the post-merge bound of 2 x 23/24 = 1.917, and the tempo precondition message's arithmetic becomes 3 x 23/24 = 2.875 with its tolerance of 4 becoming 3 -- the thresholds are a non-vacuity guard on the pinned corpus and the guard is kept with new numbers rather than relaxed; SPRT verdict recorded, zero recorded as zero
            (Folded in from the retired S056 and S081 by DEC-086. S056's four
            merged-disagreement literals are deliberately not restated here:
            they were measured before S065's refit and this step re-measures on
            the truncation_positions the test loads at its own HEAD.)
touches:    src/evaluation.cpp, tools/eval_model.hpp, tests/test_eval_model.cpp
excludes:   any other evaluation term
decisions:  DEC-053
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:

## What this is

`evaluate_mobility_and_king_safety()` tapers its two terms separately, one
integer division each (`src/evaluation.cpp` `evaluate_mobility_and_king_safety`
mobility, `:953` king safety), then sums them. Summing the two middlegame sums
and the two endgame sums first and dividing once is the same term through one
division instead of two.

The same argument is already written in this file for the pawn terms, at
`src/evaluation.cpp` `evaluate_cheap`: summed into the accumulated pair before
the interpolation rather than tapered on its own, "one integer division instead
of two [...] and one truncation towards zero instead of two -- which is what
keeps test_eval_model's one-centipawn slack against the tuner's floating-point
model from having to grow." This step applies it one function further down.

## It alters play, so it is an SPRT

`(a + b) / 24 != a / 24 + b / 24` in integer arithmetic. Two truncations
towards zero become one, so the score changes by at most 1 cp on any position
where the two remainders do not both vanish — and it changes on the positions
where they do not, which is most of them. That is a different evaluation, a
different tree, and INV-6's first test (identical node counts and best moves)
cannot be satisfied. It is decided by `./fastchess.sh --fast`, or
`REF=<sha> ./fastchess.sh`.

**Expected verdict is 0**, and the step completes on the number either way. The
change is pure rounding of at most 1 cp per position; nothing about which move
looks best is being altered on purpose. DEC-019 is the standing warning that a
reported figure decides what to try and never what to conclude, and this one
does not even have a reported figure behind it. A verdict of zero is recorded
as zero, and the change may still be kept with the reason stated — S005, S006
and S015 all were.

**The saved division is not the prize either.** One integer division on a
function the search calls once per non-shortcut node is far under the 3 % noise
floor that `CLAUDE.md` foundation 2, point 5 sets, and under the resolution
`bench_movegen` and `bench_eval` report for themselves. Measure it with
`bench_eval` and `tools/search_bench.py`, but do not expect it to clear
anything, and do not let a sub-resolution number decide the step.

## The real prize is the guard's bound

`test_eval_model`'s tolerance is 3, because `evaluate()` divides by
`GAME_PHASE_MAX` four times and three of them can round today
(`src/evaluation.cpp` `evaluate_cheap` positional, `:668` tempo — exactly 0
while `tempo_mg == tempo_eg == 0`, `:951` mobility, `:953` king safety),
bounding the disagreement with `tools/eval_model.hpp` at 3 x 23/24 = 2.875.
Removing one division puts the bound at 2 x 23/24 = 1.917, so the tolerance
returns to 2.

That is worth a centipawn of guard resolution: the guard is the only thing
stopping the model drifting from `evaluate()`, and at a tolerance of 3 a real
model error of up to about 2.8 cp per position hides inside legitimate
rounding. Over a 1.49 M position fit that is a systematic bias the fit absorbs
silently. DEC-053 records the two routes and why the slack was raised first.

**Ordering: after S042, before S029.** The tighter bound protects fits of the
hand-crafted evaluation, and S029 is where the network takes over from the HCE
as the thing being fitted, so the tightening is worth more before S029 than
after it.

## The model already does it this way

Checked at S038 rather than assumed: `tools/eval_model.hpp` `evaluate` is

```
double stage_two = (mobility_mg_sum + king_safety_mg_sum) * mg_weight +
                   (mobility_eg_sum + king_safety_eg_sum) * eg_weight;
```

— the two terms summed per phase and tapered **once**. So the model needs no
arithmetic change: this step moves the engine to the shape the model has always
had, which is why the bound falls rather than merely moves. `eval_model.hpp` is
still in `touches:` because the correspondence has to be re-read after the
change and because its comments describe an engine that tapers twice.

Two comments in `src/evaluation.cpp` state the old tolerance and are false
as of S038, which left them alone because its `excludes:` forbade touching the
file at all:

- `:638-639` — "which is what keeps test_eval_model's one-centipawn slack
  against the tuner's floating-point model from having to grow". The slack is
  not one centipawn; S038 raised it to 3.
- `:662` — "test_eval_model allows two and this widens the worst case by one".
  It allows 3. Under this step it allows 2 again, which makes the sentence true
  by coincidence rather than by statement, so restate it against the division
  count rather than against the number.

## Watch the clamp, not just the sum

`evaluate_expensive()` clamps the summed stage-two score to
+/-`LAZY_EVAL_MARGIN` (`src/evaluation.cpp` `evaluate_expensive`) and the model
clamps in the same place. The clamp is applied after the taper on both sides
today; merging the divisions must not move it to before, or the two
implementations stop clamping the same quantity — which the model's own
comments at `tools/eval_model.hpp` `evaluate` warn about term by term.

S039 re-decides `LAZY_EVAL_MARGIN` and sits ahead of this step in plan order.
If it changes the margin, nothing here has to change: this step alters what is
clamped by at most 1 cp and not where the clamp is.

## Technical details (SOTA research, 2026-08-19)

**State of the art.** CPW's Tapered Eval is one interpolation over the
*complete* accumulated mg/eg pair — `(mg*phase + eg*(PHASE_MAX-phase)) /
PHASE_MAX`, documented over a 0–256 phase with an optional half-denominator
rounding add; chesso uses the common 24-point granularity (minor 1, rook 2,
queen 4) with pure truncation. Per-term tapering is not the published shape.
This step moves stage two to the canonical form that `evaluate_cheap()`
(`src/evaluation.cpp` `evaluate_cheap`) and the float model
(`tools/eval_model.hpp` `evaluate`) already have. Division count is observable
because C++ integer division truncates toward zero ([expr.mul]/4).

**Shape for chesso.** The two sites are both in `src/evaluation.cpp`
`evaluate_mobility_and_king_safety`, one for mobility and one for king safety,
each dividing a phase blend by
`GAME_PHASE_MAX` = 24 (`src/evaluation.hpp` `GAME_PHASE_MAX`); `phase =
game_phase()` is
`board->phase` clamped to [0,24] (`src/evaluation.cpp` `game_phase`). The guard
today: `tests/test_eval_model.cpp` "the model reproduces evaluate() on every phase" `CHECK(|model - engine| <= 3.0)`, comment
`tests/test_eval_model.cpp` "the model reproduces evaluate() on every phase"
naming the four divisions; the non-vacuity
case `tests/test_eval_model.cpp` "the pinned positions reach the truncation
bound" asserts the tempo-unfitted precondition
(`tests/test_eval_model.cpp` "the pinned positions reach the truncation bound",
message "4 x 23/24 = 3.833 ... has to be
4"), per-pin `difference > 2.0` (`tests/test_eval_model.cpp` "the pinned
positions reach the truncation bound") and `worst >
2.8` (`tests/test_eval_model.cpp` "the pinned positions reach the truncation
bound") over `truncation_positions`
(`tests/test_eval_model.cpp` `truncation_positions`), four FENs at exactly
69/24 found by
`build/tools/truncation_scan` over `.tuning/selfplay_v2_dedup.tsv` (present on
this machine). Transformation: sum the two mg halves and the two eg halves,
blend once, divide once; the model changes nothing
(`tools/eval_model.hpp` `evaluate` is already merged).

**Implementation sketch.** In `evaluate_mobility_and_king_safety`: `stage_two =
((mob_mg + saf_mg) * phase + (mob_eg + saf_eg) * endgame) / 24`. Rounding
analysis: let A, B be the two integer blends, `r = x % 24` the toward-zero
remainder (sign of dividend, |r| <= 23). The change is `d = trunc((A+B)/24) -
trunc(A/24) - trunc(B/24) = (rA + rB - r(A+B))/24`, an integer. Same-sign
blends: `d = sign` iff `|rA + rB| >= 24`, else 0 — merged loses less, score
weakly farther from zero. Mixed signs: d in {-1, 0, +1}, either direction. So
**|d| <= 1 cp per position, no parity invariant**, and at phase 0 or 24 the
blend is divisible by 24 and d = 0. Guard bound: after the merge two divisions
round (`src/evaluation.cpp` `evaluate_cheap` and the merged one; tempo still
0/24 exact),
each losing at most 23/24 toward zero, worst case aligned: **2 x 23/24 = 46/24
= 1.9167**, so 2 is the tightest integer tolerance the arithmetic can never
legitimately exceed (headroom 0.083 cp dwarfs double rounding; today's is 3 -
2.875 = 0.125). Analog thresholds: per-pin `> 1.0` (one division alone loses <
23/24 < 1, so past 1.0 proves both truncated) and `worst > 1.9` (vs 1.9167);
the tempo message becomes 3 x 23/24 = 2.875 with tolerance 3, as the accepts
states. Re-pin by re-running `truncation_scan` with `--min` near 1.8 at this
step's HEAD; DEC-057 forbids lowering thresholds to whatever came out — find
positions that reach the new maximum.

**Constants and seeds.** None. No weight moves, nothing to seed or refit
(DEC-084 not engaged).

**Pitfalls.**
- - Truncation is toward zero, so the loss direction flips with the blend's
  sign, and the blends are routinely negative: knight mg is -1
  (`src/evaluation.cpp` `mobility_mg`), queen eg -6 (`src/evaluation.cpp`
  `mobility_eg`), and the sums are mover-signed (`src/evaluation.cpp`
  `evaluate_mobility_and_king_safety`). The +/-1 lands on most positions;
  direction depends on the sign mix, per the analysis above.
- - **The collect path is the trap the file does not name.** `<collect=true>`
  hands back tapered mobility and safety separately (`src/evaluation.cpp`
  `evaluate_mobility_and_king_safety`) through `evaluate_expensive_terms`
  (`src/evaluation.cpp` `evaluate_expensive_terms`) to `tools/eval_spread.cpp`
  `main` and `tests/test_evaluation.cpp` "the unclamped terms are the engine's
  own", which REQUIREs `clamp(mobility + safety) == evaluate() -
  evaluate_cheap()` **exactly** (`tests/test_evaluation.cpp` "the unclamped
  terms are the engine's own"). Post-merge the reported pair must sum to the
  merged total: taper one term, hand back the other as `stage_two - that term`,
  and state which term carries the +/-1 residue — do not weaken the test.
  eval_spread's per-term worsts shift by <= 1 cp; S039 reads them.
- - The clamp stays after the taper (`src/evaluation.cpp` `evaluate_expensive`;
  model `tools/eval_model.hpp` `evaluate`) — already stated above.
- INV-4: same loop, same accumulated sums, one arithmetic site changed, no
  recompute added; `evaluate_cheap()` untouched.
- - The two stale comments in `src/evaluation.cpp` `evaluate_cheap`
  are this step's to fix — DEC-053's consequences say "S055 owns them".

**Measurement.** The accepts is unambiguous: **SPRT, zero recorded as zero.**
DEC-083's timing-only lane requires identical node counts and best moves
(INV-6's proof of neutrality), which a +/-1 cp change on most positions cannot
produce — different scores, different tree. No "behaviour-neutral with bounded
drift" category exists in INV-6 or DEC-083: the guard bound caps *model*
divergence, not play. The alternative reading — timing-only on the bounded-1cp
argument — would need a decision amending INV-6 and the accepts already chose
the SPRT. Run `./fastchess.sh --fast` at the S105 settings; record `bench_eval`
and `tools/search_bench.py` numbers for the file, expecting sub-noise (the
division by constant 24 is already a multiply-shift, not a `div`).

**Interactions.**
- **S117 (next in plan order): do S055 first, do not fold.** Packing mg/eg
  into one integer makes the single division automatic — one packed stage-two
  accumulator unpacks to one blend and one division, and the two-division form
  cannot survive packing without deliberately unpacking twice. But S117's
  accepts demands identical scores and node counts, no SPRT owed; folding this
  step's rounding change in breaks that and puts two changes under one
  verdict. S055 first lets S117 stay bit-identical against the merged form.
  This answers S117's open question ("do S055 first or fold it in").
- **S121** replaces the mobility weights with per-count tables but still feeds
  the same mg/eg sums: the merged taper and the bound of 2 survive; the pinned
  FENs get re-measured at S121's fit anyway (DEC-057's standing rule).
- **S122** rewrites king safety with a quadratic finalizer, unclamped. If the
  finalizer emits mg/eg halves, the merged blend survives; if it applies after
  the taper, this site is rewritten and S122 must re-derive the guard's
  division count. One line of rework either way.
- - **S039**: `evaluate_lazy()`'s shortcut (`src/evaluation.cpp`
  `evaluate_lazy`) tapers nothing — it compares the already-tapered cheap score
  against the window. Covered above.
- Order note: this file's "after S042, before S029" is DEC-053-era prose;
  plan.md owns order and now lists S055 (entry 32) before S042 (entry 36),
  with S029 parked — DEC-054 says S055 "keeps its value and loses its
  deadline". No conflict.

**References.**
- https://www.chessprogramming.org/Tapered_Eval — the phase-weighted blend,
  0–256 phase, taper applied once over the complete mg/eg pair.
- https://eel.is/c++draft/expr.mul — [expr.mul]/4, quotient truncates toward
  zero.

## Report (implementer, 2026-10-05)

Implemented by one Opus subagent briefed by the coordinator. Nothing
committed and no match run. Evidence is under `.tuning/S055/` (gitignored).

### Stops and deviations first

1. **`test_mate_carry` is red in both builds. It needs a decision before
   landing.** The new tree silenced five of the six S170 cases at their
   budgets: only B reported, against a majority of 4. The budget rule
   (`adocs/data/S203_case_sweep.sh`, DEC-156 as DEC-162 left it) was re-run
   and its answer applied to `adocs/data/S170_cases.tsv`: A 500000 -> 1000000,
   C 1500000 -> 1200000, D 2000000 -> 1500000, E 100000 -> 300000, F 100000 ->
   300000, B unchanged. The grid is `adocs/data/S055_sweep_s170.txt`. At the
   rule's budgets every case reports and `unreached.empty()` holds. **E's new
   cell has 2 mate lines, both short, against E's ceiling of 0.** Raising a
   ceiling relaxes a test, and the file says it "needs a decision"
   (DEC-246/249/250 precedent), so `short_line_ceiling()` was not touched.
   What `--ceilings` gives:
   - S202's grid plus this one: 0, 5, 4, 0, **2**, **1** (E 0 -> 2, F 0 -> 1).
   - This grid alone: 0, 1, 1, 0, 2, 1. This applies the "WHICH GRIDS" rule
     that a grid must come from today's engine; S202's grid no longer does.
2. **Scripted goldens re-derived outside `touches:` (DEC-142, DEC-233).**
   - `tests/test_search.cpp` "a side in check may not stand pat": 198 -> 197.
     `adocs/data/S192_anchors.py` was changed to taper the sum once, as the
     engine now does, and reads 10 of 10.
   - "pruning does not hide a forced mate": capture-mate rows 3 and 4 moved,
     re-derived by the seven sweeps (see Tests).
   - `DEV_MANUAL.md`'s golden table changed with them.
3. **The pinned positions live in two files.** `truncation_positions` is a
   subset of `tests/test_eval_positions.hpp`'s `positions`, so the new four
   were added there too. S076's four were **kept** in `positions`: they are
   the non-vacuity evidence below.
4. **`tools/truncation_scan.cpp`'s header, usage text and default `--min`**
   (2.8 -> 1.9) said the old maximum. The text was updated; its arithmetic did
   not change.
5. **Finding (tooling, DEC-171 filler): the mutant files have been stale since
   S020.** `5ad8837` made `is_check_move` a call, `is_check_move()`. The `old`
   strings of S091 `C02` and `R01`, of `tools/mutants/S109_shallow_pruning.py`
   and of `tools/mutants/search.py` still read the bare name, so
   `tools/mutation_check.py` refuses those mutants. The sweep driver applied
   C02 and R01 with the call form, which injects the same bug. This cannot be
   reached in ordinary play.
6. **Finding (tooling, DEC-171 filler): `adocs/data/S192_anchors.py` clamps
   at 150.** `LAZY_EVAL_MARGIN` has been 184 since S085. The clamp does not
   bind on any of the script's cases, so no anchor is wrong today.

### What changed

- `src/evaluation.cpp` `evaluate_mobility_and_king_safety`: `stage_two =
  ((mob_mg + saf_mg) * phase + (mob_eg + saf_eg) * endgame) / 24`. The clamp
  in `evaluate_expensive` still comes after the taper.
  - The collect path tapers mobility alone and reports king safety as
    `stage_two - mobility`, so **king safety carries the residue**. It stays
    within 1 cp of its own separate taper, and the pair sums exactly to the
    engine's total, which is what `test_evaluation`'s exact check needs. The
    test was not edited.
  - Both stale comments in `evaluate_cheap` were restated against the
    division count, not a tolerance number.
- `tools/eval_model.hpp`: comments only. The model already tapered the summed
  pair once, so no arithmetic changed.
- The accepts' "one fewer integer division" was checked in the binary. Count
  of `0x2aaaaaab` multiply-shift sequences in `evaluate_expensive` (where the
  `<false>` instantiation is inlined): 2 -> 1. `evaluate_cheap` is unchanged
  at 1.

### Tests

- **Tolerance 3 -> 2. Pinned thresholds: per-pin `> 1.0`, worst `> 1.9`.
  Tempo message: 3 x 23/24 = 2.875, tolerance 3.** These are the arithmetic
  thresholds the step file derived before anything was measured, not values
  read off a run (DEC-057).
- **New pins.** Found by `build/tools/truncation_scan --data
  .tuning/selfplay_v2_dedup.tsv --min 1.8` at this tree: 10795695 rows, 0 past
  2.0, 18462 past 1.8, **1816 at the maximum 46/24 = 1.916667** (phases 1, 5,
  7, 11, 13, 17, 19, 23). Each pin is the first in corpus order at the maximum
  for its phase and side to move:
  - `7k/8/5p1P/3KpB2/8/8/5PP1/8 b - - 10 47` (phase 1)
  - `6r1/1Bp5/2n1k2p/1p6/1b1Pn3/1P6/5P1P/R1B2K1R w - - 0 23` (11)
  - `5r1k/1b1p3p/pp2p1p1/2p5/2PNP3/P1NPb2P/KP5q/R2Q1B2 b - - 0 24` (17)
  - `r2qkb1r/1ppb1ppp/4pn2/p2p4/7P/3P1N2/PPPBPPP1/RN1QKB1R w KQkq - 0 8` (23)
- **Non-vacuity, observed.** The new tests were built against the parent's
  `src/evaluation.cpp` in a throwaway worktree. `test_eval_model` failed 6
  assertions:
  - `CHECK(|model - engine| <= 2.0)` failed at 2.875 on all four S076
    positions, so the tolerance of 2 rejects the three-division engine.
  - Per-pin `> 1.0` failed at 0.917 on two new pins (phases 1 and 23). On the
    parent engine the four new pins read 0.917, 1.917, 1.917, 0.917. No
    corpus row past 2.0 exists on either side of the change, so the old four
    are what makes the tolerance-2 assertion able to fail.
- **Capture-mate table, the seven sweeps.** Shipped plus C02, C05, C06, C07,
  R01, R02, depths 3 to 12, over `adocs/data/S230_table_fens.txt`; driver
  `.tuning/S055/capmates.sh`.
  - Shipped profiles: `d9-12`, `d8-12`, `d11 d12`, `d9-12`.
  - Rows: {9, "no S091 mutant, since S116"} unchanged; {8, "no S091 mutant,
    since S095"} unchanged; **{11, "C05, R02, since S055"}**, was 10 "C02,
    C05, R02"; **{9, "C02, C05, C07, R02, since S055"}**, was 10 "no S091
    mutant".
  - No mate distance moved. C06 and R01 are separated by no row.
- **Gate**, `export CLANG_FORMAT_MAJOR=22`: both builds compile and 40 of 41
  pass in each. The one red is `test_mate_carry`, on E's ceiling only
  (stop 1). `./clang-format.sh --check` is clean.
- **DEC-141 second tier is not owed.** Nothing in `make_move`, `unmake_move`,
  the generator or the search changed, only the evaluation's arithmetic.

### Numbers

- **`bench` 4081329 -> 3562703** (-12.71 %). Replies: the fifth position
  d8d6 -> d8e7, the other seven unchanged.
- `bench 12` 1860699 -> 1741481. Kiwipete d5e6 -> e2a6, the fifth d8d7 ->
  d8e7.
- `tools/search_bench.py`, node counts:

  | depth | parent | this tree |
  |---|---|---|
  | 9 | 32932 / 70095 / 25178 | 17240 / 69834 / 18029 (midgame c3d5 -> g5f6) |
  | 12 | 67792 / 280873 / 137893 | 78212 / 234517 / 113207 (kiwipete d5e6 -> e2a6) |

- `bench_eval`, three interleaved pairs, parent then this tree: 49.64 / 50.43,
  49.97 / 50.05, 51.16 / 50.37 ns per call, resolution 0.1 to 2.7 %. Same
  checksum on both sides. That is sub-noise, as the step file predicted, and
  it decides nothing.

### Docs

- `DEV_MANUAL.md`: the bench history paragraph, the truncation section's
  command, maximum and margins, and the golden table row 198 -> 197.
- `MANUAL.md`: checked; it says nothing about the taper or the tolerance, so
  no change.
- `adocs/specs.md`, proposed for the coordinator, at the end of the
  evaluation row's clamp sentence: "Since S055 the two are summed per phase
  and tapered through one integer division, then clamped, the form the
  tuner's model computes; the tuner-model guard's tolerance is 2 (two
  rounding divisions, 2 x 23/24)."

## Measurement, pre-registered 2026-10-05

Written before any game. The shape follows S042's.

**Run.** Once the landing commit exists:

```
REF=<landing^> CAND=<landing> OUT=.tuning/sprt_s055_<stamp> \
  nohup ./fastchess.sh --nonreg > .tuning/sprt_s055.log 2>&1 &
```

If no commit is made first, the working-tree form against `HEAD` works the
same way. Settings: `elo0=-5 elo1=0` nElo, `alpha=beta=0.05`,
`model=normalized`, 20000 rounds (40000-game cap). The regime is DEC-189's:
8+0.08, `Hash=16`, `books/noob_3moves.epd`, all 12 threads.

**Why `--nonreg` and not `--fast` or `{-5, 5}`.** The expected truth is 0,
because the change is pure rounding of at most 1 cp per position.
- `--fast` is `{0, 10}`. With the truth at elo0 it ends H0, which only says
  "not a 10 nElo gain", and that cannot decide keep or revert.
- `{-5, 5}` (DEC-063's small-change pair) puts a zero truth at its midpoint.
  It is cheap, 10465 games or 4.96 h worst case, but H0 and H1 are then a
  coin flip, so an H0 there would not justify a revert.
- `--nonreg` asks the one thing this change owes: is it a regression of 5
  nElo or more. S042, the latest non-regression step, ran it.

**Cost (DEC-143), nElo run-length formula, at 2110 games/h:**
- Truth on a bound, which is where 0 sits: **25591 games, 12.13 h**.
- Truth at the midpoint, -2.5: 41861 games. That is past the 40000 cap, so
  about **18.96 h** at the cap.
- Four hours or more, so a night run (DEC-155). Launch detached and arm a
  watcher with the four exits; ceiling 2x the worst case, about 40 h.

**Abort rule.**
- Before launch: the one-minute load from `/proc/loadavg` must be well under
  12, and nothing else may run.
- During the run: abort if the forfeit rate passes 1.0 % on either side,
  counted by `tools/forfeit_report.py` over the run's own PGN.
- This step changes nothing in the harness, so it owes no A/A (DEC-143).

**Readings, fixed now:**
- **H1**: not a regression of 5 nElo or more. **Kept.** No magnitude is
  claimed: an SPRT that stops on a favourable swing is biased upward
  (DEC-063).
- **H0**: a 5 nElo regression is more likely than none. Check forfeits and
  the harness first. If it stands, **revert**: the merged taper, and every
  golden this step moved, byte for byte:
  - tolerance 3, thresholds `> 2.0` / `> 2.8`, the tempo message 4 x 23/24;
  - the S076 pins in `truncation_positions`, and the four added rows taken out
    of `positions`;
  - anchor 198 and `S192_anchors.py`'s two separate truncations;
  - capture-mate rows 3 and 4 as S116 left them;
  - the S170 budgets as S116 left them, and whatever ceiling the decision in
    stop 1 set;
  - `truncation_scan`'s text and default, and the `DEV_MANUAL.md` paragraphs.
  A 1 cp rounding change measuring as a 5 nElo regression would itself be a
  finding to record.
- **No verdict at the cap: recorded as zero and kept**, with the reason
  stated. The step's prize is the guard's bound: tolerance 2 instead of 3, one
  centipawn of model-guard resolution ahead of every later HCE fit. The
  single taper is also the canonical form, the one `evaluate_cheap` and the
  model already use, and it has one fewer division. S005, S006 and S015 are
  the precedent for keeping a measured zero, and the step file allows it.

**Open findings during the run (DEC-171).** Fillers 5 and 6 above (stale
mutant `old` strings since S020; `S192_anchors.py`'s clamp at 150). Neither
can be reached in ordinary play or move a reported score. `status.md` had no
filler open before this step.
