id:         S039
goal:       re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
accepts:    the margin is chosen from an eval_spread run at the weights that ship at this step's own HEAD, over .tuning/selfplay_v2.tsv where it is on disk and over the tracked 5582-row corpus otherwise -- the file is gitignored and a machine move loses it again, so which corpus was read is recorded with the figures; the run is recorded in adocs/data/; the LAZY_EVAL_MARGIN comment in src/evaluation.hpp is re-measured for its mobility sentence as well as its king-safety sentence, cited by the symbol rather than by a line range; an SPRT against the preceding commit returns a verdict if the margin changes
            (Folded in from the retired S057, S080 and S063 by DEC-086. The
            figures those steps corrected -- 0.364 % past 150, worst 279 -- are
            stale twice over: the real spread was four times that, and S065 has
            since refitted 827 constants. Nothing here is taken on the old
            numbers. This step is now a prerequisite for S122 rather than a
            micro-tune: the clamp it sizes is what caps king safety.)
touches:    src/search_params.hpp, where the value itself has lived since S073
            and where a re-decision has to land; src/evaluation.hpp
            LAZY_EVAL_MARGIN and its comment, which is what the number means;
            adocs/data/S039_eval_spread.log for the run the accepts records;
            tools/eval_spread.cpp, whose candidate margins must include the value
            that ships (F25)
excludes:   the structure of the lazy shortcut itself, which is S034 and is done
decisions:  DEC-039
closes:     2026-08-13_adversarial-F05, 2026-09-10_adversarial-F16, 2026-09-10_adversarial-F25
blocks:
paused_by:
done:

## What is stale

`src/evaluation.hpp` `evaluate_expensive_terms` says 150 is above the largest
correction observed, and adds that the figure "is still the whole correction
only because king safety ships at zero weight; the margin is re-decided from
measured data once it is fitted, and tools/eval_spread is what measures it".

King safety has not shipped at zero weight since S027 (`src/evaluation.cpp`
`king_safety_mg`). The margin is 184, moved by S085's SPSA and not by a spread
measurement. The re-decision the comment promises has not happened.

## What it costs now

`eval_spread` over 50000 corpus positions at the current weights:

```
  150            66 0.132%      0 0.000%    182 0.364%
  200             3 0.006%      0 0.000%     24 0.048%
  250             0 0.000%      0 0.000%      9 0.018%
worst position per term
  mobility      201  r1b2qk1/3p2p1/p2p4/1p1P1B2/5p1p/1QB4P/PP3P1K/4R1R1 b - - 0 27
  king safety   134  r1b4r/1p1k1pp1/1Qn2q1p/p2N4/3p4/5B2/P2B1PPP/4RRK1 b - - 0 21
  combined      279  r1b2q1r/ppppk3/5n1p/2P3p1/8/1P5N/PBQ3BK/3R1R2 b - - 5 21
```

The combined correction exceeds the margin on 0.364 % of positions and reaches
279, so `evaluate()` discards up to 129 cp of a term it has already computed on
roughly one position in 275.

## Not an unsoundness

The clamp is applied inside `evaluate_expensive()`, so `evaluate()` *is* the
clamped function and the shortcut's bound holds by construction — an audit check
over 100000 positions found the gap never above 150. What is lost is
expressiveness, and it lands exactly where king safety was added for: 134 in a
position with a king on d7 under fire, where the clamp is what stops mobility
and king safety both being paid.

## Why it is a step and not an edit

Raising the margin costs search time in `evaluate_lazy` — fewer positions take
the shortcut. That trade is an SPRT question, not an argument. Re-run
`eval_spread` over the full corpus first, pick the margin from that
distribution, then measure.

## Amended 2026-09-11, DEC-170 and DEC-172: moved beside S122, and two audit findings folded in

`2026-09-10_adversarial-F16` re-measured the spread at the shipping weights
over 1.5 M rows: p99 **153**, max **401**, against the `src/evaluation.cpp`
comment's "p99 128 and max 330" -- the figures in "What it costs now" above
are older still. And it observed that this step sized the margin "at the
weights that ship at this step's own HEAD" while the Open order placed S121,
S123, S125 and S101 -- four steps that change the very sum being clamped --
between it and S122. **So this step now sits directly before S122**, its
consumer, and the `eval_spread` run is taken at the weights those four steps
leave behind. DEC-169 already binds the other side: a move of
`LAZY_EVAL_MARGIN` owes a fresh `truncation_scan` reading, recorded beside the
new margin.

`2026-09-10_adversarial-F25`: `tools/eval_spread.cpp`'s candidate margins are
`{150, 200, 250, 300, 400}` and do not include the 184 that has shipped since
S085 -- in the tool whose whole job is this re-decision. The candidate list
gains the shipping value, read from the binary or from `search_param_info()`
rather than typed, before the run is taken.
