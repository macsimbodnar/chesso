id:         S055
goal:       taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
accepts:    evaluate() performs one fewer integer division; the tuner-model tolerance returns to 2 and test_eval_model passes over the pinned corpus at that tolerance; SPRT verdict recorded, zero recorded as zero
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
