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

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki defines the technique and gives no number for anything this step
decides.** CPW *Lazy Evaluation*
(https://www.chessprogramming.org/Lazy_Evaluation, fetched 2026-09-13): "If
after performing all the tasks for a given stage score exceeds beta by a
certain margin or if it falls below alpha by the same margin, the score is
returned." The page states **no margin value, no method for choosing one, no
warning about the bound, and no Elo figure**. So there is nothing published to
seed the margin from and nothing to check the choice against -- the number can
only come from this engine's own spread, which is precisely what the accepts
says and why this step exists.

**Two departures from the wiki's form, both deliberate and both already in the
tree.** First, the page's condition is a *stage* test that returns early;
chesso's clamp is applied inside `src/evaluation.cpp` `evaluate_expensive`, so
`evaluate()` **is** the clamped function and the shortcut's bound holds by
construction. That is stronger than the published form and it is why "Not an
unsoundness" above is correct. Second, the published form's margin guards a
return; chesso's also bounds the *expressiveness* of the two terms inside it,
which is the cost S122 cannot pay. Neither departure is on the wiki and both
are stated here so the implementer does not read the page and expect the
simpler shape.

**No located source prices a lazy-evaluation margin in Elo, at any band.** The
2026-09-04 literature check recorded lazy evaluation as present in Berserk 4.3
and in chesso and found no figure; this pass fetched the CPW page and confirms
it carries none. Searched: CPW *Lazy Evaluation*; the review's part B3 row.
The step does not need one -- its verdict is its own SPRT -- and no ordering
argument here rests on a published number.

### 2. Shape for chesso

The number lives in `src/search_params.hpp` `CHESSO_SEARCH_PARAMS` as
`LAZY_EVAL_MARGIN` and has since S073; `src/evaluation.hpp`'s comment is what
it *means* and is the sentence the accepts asks to re-measure. The distribution
that decides it is `tools/eval_spread.cpp`'s, run at the weights that ship at
this step's own HEAD -- which, after the DEC-172 move, is the weights S121,
S123, S125 and S101 leave behind.

### 3. Implementation sketch

- `tools/eval_spread.cpp`'s candidate list gains the shipping value, read from
  `search_param_info()` rather than typed (F25). A tool whose job is this
  re-decision that cannot report the incumbent cannot answer the question.
- The run goes in `adocs/data/S039_eval_spread.log` with the corpus it read
  named, because `.tuning/selfplay_v2.tsv` is gitignored and a machine move
  loses it.
- The comment above `LAZY_EVAL_MARGIN` is rewritten from the new run's
  figures, both sentences -- mobility and king safety -- and cited by symbol.
- DEC-169 binds the other side: a move of the margin owes a fresh
  `truncation_scan` reading recorded beside the new value.

### 4. Constants and seeds

One constant, and it is **(b) a derivation over chesso's own data, run by this
step at its start**. The procedure, stated so the implementer does not
improvise:

1. Build `tools/eval_spread` at this step's HEAD and run it over the largest
   corpus on disk, with the candidate list including the shipping value.
2. Report, per candidate margin, the share of positions whose combined
   mobility-plus-king-safety correction exceeds it, and the worst position per
   term with its FEN.
3. Choose the candidate from that distribution with the rule stated in the
   step -- the share it is willing to clamp -- and record the share, not only
   the margin.
4. SPRT the change against the preceding commit if the margin moves.

**No published value seeds this and none exists** (section 1). Another
engine's lazy margin would be that engine's tuned constant wherever it is
quoted (DEC-105, DEC-134) and is not used. The figures inside "What it costs
now" above are this project's own older measurements and are superseded by the
new run, not averaged with it.

### 5. Pitfalls

- **Three generations of figures now sit in this file and they disagree.** "What
  it costs now" gives 0.364 % past 150 and a worst of 279; the
  `src/evaluation.cpp` comment says p99 128 and max 330; F16's re-measurement
  over 1.5 M rows says p99 **153** and max **401**. Only the run this step
  takes counts, and the older three are kept as history, not as inputs.
- **Raising the margin costs search time.** Fewer positions take the shortcut
  in `src/evaluation.hpp` `evaluate_lazy`. That is the trade and it is an SPRT
  question, never an argument.
- **The measurement moves under the step.** Four terms-changing steps now sit
  between this file's original writing and its run. Taking the spread at the
  old weights would size a clamp for a sum that no longer exists, which is
  exactly what F16 found and DEC-172 moved the step to fix.
- **The clamp is on the real score, not only on the shortcut.** Anyone reading
  this as a pure speed knob will under-weight what S122 loses.

### 6. Measurement

An `eval_spread` run recorded in `adocs/data/`, then one SPRT against the
preceding commit **if the margin changes** -- and no SPRT if it does not,
because nothing moved. A `truncation_scan` reading beside the new margin
(DEC-169). The spread run is minutes; the SPRT is the S105 regime and is
daytime work under DEC-155.

### 7. Interactions

- **S122 (immediately after, its consumer)**: the clamp is what caps king
  safety, and S122 states which of the three outcomes it inherited.
- **S121, S123, S125, S101 (before, by DEC-172)**: each changes the sum being
  clamped, so the spread is taken behind all four.
- **S120 (ordered ahead)**: retiring the clamp costs a measured 11.7 % of nps;
  the evaluation cache and S104 are what pay for it.
- **DEC-169 (truncation)**: a margin move owes a fresh scan.

### 8. References

- - https://www.chessprogramming.org/Lazy_Evaluation -- the condition quoted
  above; **no margin value, no selection method, no Elo figure**. Fetched
  2026-09-13.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` part B3 -- lazy
  evaluation present in Berserk 4.3 and chesso, no figure located.
- - `adocs/audit/2026-09-10_adversarial.md` findings F16 and F25 -- the p99 153
  / max 401 re-measurement and the missing candidate value. Local evidence.
