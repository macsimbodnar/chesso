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
done:

## What this is

`evaluate_mobility_and_king_safety()` tapers its two terms separately, one
integer division each (`src/evaluation.cpp:951` mobility, `:953` king safety),
then sums them. Summing the two middlegame sums and the two endgame sums first
and dividing once is the same term through one division instead of two.

The same argument is already written in this file for the pawn terms, at
`src/evaluation.cpp:635-639`: summed into the accumulated pair before the
interpolation rather than tapered on its own, "one integer division instead of
two [...] and one truncation towards zero instead of two -- which is what keeps
test_eval_model's one-centipawn slack against the tuner's floating-point model
from having to grow." This step applies it one function further down.

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
does not even have a reported figure behind it. A verdict of zero is recorded as
zero, and the change may still be kept with the reason stated — S005, S006 and
S015 all were.

**The saved division is not the prize either.** One integer division on a
function the search calls once per non-shortcut node is far under the 3 % noise
floor that `CLAUDE.md` foundation 2, point 5 sets, and under the resolution
`bench_movegen` and `bench_eval` report for themselves. Measure it with
`bench_eval` and `tools/search_bench.py`, but do not expect it to clear
anything, and do not let a sub-resolution number decide the step.

## The real prize is the guard's bound

`test_eval_model`'s tolerance is 3, because `evaluate()` divides by
`GAME_PHASE_MAX` four times and three of them can round today
(`src/evaluation.cpp:640` positional, `:668` tempo — exactly 0 while
`tempo_mg == tempo_eg == 0`, `:951` mobility, `:953` king safety), bounding the
disagreement with `tools/eval_model.hpp` at 3 x 23/24 = 2.875. Removing one
division puts the bound at 2 x 23/24 = 1.917, so the tolerance returns to 2.

That is worth a centipawn of guard resolution: the guard is the only thing
stopping the model drifting from `evaluate()`, and at a tolerance of 3 a real
model error of up to about 2.8 cp per position hides inside legitimate rounding.
Over a 1.49 M position fit that is a systematic bias the fit absorbs silently.
DEC-053 records the two routes and why the slack was raised first.

**Ordering: after S042, before S029.** The tighter bound protects fits of the
hand-crafted evaluation, and S029 is where the network takes over from the HCE
as the thing being fitted, so the tightening is worth more before S029 than
after it.

## The model already does it this way

Checked at S038 rather than assumed: `tools/eval_model.hpp:973-974` is

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
+/-`LAZY_EVAL_MARGIN` (`src/evaluation.cpp:984`) and the model clamps in the
same place. The clamp is applied after the taper on both sides today; merging
the divisions must not move it to before, or the two implementations stop
clamping the same quantity — which the model's own comments at
`tools/eval_model.hpp:987-1007` warn about term by term.

S039 re-decides `LAZY_EVAL_MARGIN` and sits ahead of this step in plan order. If
it changes the margin, nothing here has to change: this step alters what is
clamped by at most 1 cp and not where the clamp is.

## Technical details (SOTA research, 2026-08-19)

**State of the art.** CPW's Tapered Eval is one interpolation over the
*complete* accumulated mg/eg pair — `(mg*phase + eg*(PHASE_MAX-phase)) /
PHASE_MAX`, documented over a 0–256 phase with an optional half-denominator
rounding add; chesso uses the common 24-point granularity (minor 1, rook 2,
queen 4) with pure truncation. Per-term tapering is not the published shape.
This step moves stage two to the canonical form that `evaluate_cheap()`
(src/evaluation.cpp:640-643) and the float model (tools/eval_model.hpp:973-974)
already have. Division count is observable because C++ integer division
truncates toward zero ([expr.mul]/4).

**Shape for chesso.** The two sites: src/evaluation.cpp:951-952 (mobility) and
:953-954 (king safety), each dividing a phase blend by `GAME_PHASE_MAX` = 24
(src/evaluation.hpp:304); `phase = game_phase()` is `board->phase` clamped to
[0,24] (src/evaluation.cpp:1050-1053). The guard today:
tests/test_eval_model.cpp:378 `CHECK(|model - engine| <= 3.0)`, comment
:337-359 naming the four divisions; the non-vacuity case :405-446 asserts the
tempo-unfitted precondition (:410-415, message "4 x 23/24 = 3.833 ... has to
be 4"), per-pin `difference > 2.0` (:436) and `worst > 2.8` (:444) over
`truncation_positions` (:231-236), four FENs at exactly 69/24 found by
`build/tools/truncation_scan` over `.tuning/selfplay_v2_dedup.tsv` (present on
this machine). Transformation: sum the two mg halves and the two eg halves,
blend once, divide once; the model changes nothing (:973-974 is already merged).

**Implementation sketch.** In `evaluate_mobility_and_king_safety`:
`stage_two = ((mob_mg + saf_mg) * phase + (mob_eg + saf_eg) * endgame) / 24`.
Rounding analysis: let A, B be the two integer blends, `r = x % 24` the
toward-zero remainder (sign of dividend, |r| <= 23). The change is
`d = trunc((A+B)/24) - trunc(A/24) - trunc(B/24) = (rA + rB - r(A+B))/24`, an
integer. Same-sign blends: `d = sign` iff `|rA + rB| >= 24`, else 0 — merged
loses less, score weakly farther from zero. Mixed signs: d in {-1, 0, +1},
either direction. So **|d| <= 1 cp per position, no parity invariant**, and at
phase 0 or 24 the blend is divisible by 24 and d = 0. Guard bound: after the
merge two divisions round (:640-643 and the merged one; tempo still 0/24
exact), each losing at most 23/24 toward zero, worst case aligned:
**2 x 23/24 = 46/24 = 1.9167**, so 2 is the tightest integer tolerance the
arithmetic can never legitimately exceed (headroom 0.083 cp dwarfs double
rounding; today's is 3 - 2.875 = 0.125). Analog thresholds: per-pin `> 1.0`
(one division alone loses < 23/24 < 1, so past 1.0 proves both truncated) and
`worst > 1.9` (vs 1.9167); the tempo message becomes 3 x 23/24 = 2.875 with
tolerance 3, as the accepts states. Re-pin by re-running `truncation_scan`
with `--min` near 1.8 at this step's HEAD; DEC-057 forbids lowering thresholds
to whatever came out — find positions that reach the new maximum.

**Constants and seeds.** None. No weight moves, nothing to seed or refit
(DEC-084 not engaged).

**Pitfalls.**
- Truncation is toward zero, so the loss direction flips with the blend's
  sign, and the blends are routinely negative: knight mg is -1 (:690), queen
  eg -6 (:691), and the sums are mover-signed (:901-902). The +/-1 lands on
  most positions; direction depends on the sign mix, per the analysis above.
- **The collect path is the trap the file does not name.** `<collect=true>`
  hands back tapered mobility and safety separately (:956-958) through
  `evaluate_expensive_terms` (:1005) to tools/eval_spread.cpp:174 and
  tests/test_evaluation.cpp:464-496, which REQUIREs
  `clamp(mobility + safety) == evaluate() - evaluate_cheap()` **exactly**
  (:481-484). Post-merge the reported pair must sum to the merged total: taper
  one term, hand back the other as `stage_two - that term`, and state which
  term carries the +/-1 residue — do not weaken the test. eval_spread's
  per-term worsts shift by <= 1 cp; S039 reads them.
- The clamp stays after the taper (:984; model :976-977) — already stated above.
- INV-4: same loop, same accumulated sums, one arithmetic site changed, no
  recompute added; `evaluate_cheap()` untouched.
- The stale comments at :638-639 and :662 are this step's to fix — DEC-053's
  consequences say "S055 owns them".

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
- **S039**: `evaluate_lazy()`'s shortcut (:1038) tapers nothing — it compares
  the already-tapered cheap score against the window. Covered above.
- Order note: this file's "after S042, before S029" is DEC-053-era prose;
  plan.md owns order and now lists S055 (entry 32) before S042 (entry 36),
  with S029 parked — DEC-054 says S055 "keeps its value and loses its
  deadline". No conflict.

**References.**
- https://www.chessprogramming.org/Tapered_Eval — the phase-weighted blend,
  0–256 phase, taper applied once over the complete mg/eg pair.
- https://eel.is/c++draft/expr.mul — [expr.mul]/4, quotient truncates toward
  zero.
