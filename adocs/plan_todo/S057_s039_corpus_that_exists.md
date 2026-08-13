id:         S057
goal:       S039's accepts names a corpus that exists in this tree, with its spread figures re-measured at HEAD
accepts:    S039's first acceptance criterion names a corpus that exists on this machine, with the exact command that reads it; the figures in "What it costs now" (0.364 % past 150, worst 279, the three worst-case FENs) are replaced by figures re-measured at the commit S057 completes on, or the step states the datagen run and its cost as its own first item of work; nothing in S039 still points at .tuning/selfplay_v1.tsv as an input the step assumes it has
touches:    adocs/plan_todo/S039_lazy_margin_redecide.md
excludes:   re-deciding LAZY_EVAL_MARGIN, which is S039 itself; any edit to src/evaluation.hpp; generating a new dataset
decisions:  DEC-039, DEC-049
closes:     2026-08-13_plan_review.2-F02
blocks:
paused_by:
done:

## What is there

`S039_lazy_margin_redecide.md:3` accepts "the margin is chosen from an
eval_spread run over the full corpus at current weights and the run is
recorded". The corpus that phrase means is `.tuning/selfplay_v1.tsv`, which
`DEV_MANUAL.md:217` documents as the input to `eval_spread` and which does not
exist here: `.tuning` is gitignored (`.gitignore:11`) and `DEV_MANUAL.md:512`
says so. The work moved machines at DEC-049 and the dataset did not come with
it.

Regenerating is not a substitute. `DEV_MANUAL.md:400-401` gives the run as
`--games 20000 --nodes 100000 --seed 20260810`, `adocs/status.md:37` records 95
minutes for it on the old machine, and S027 and S028 changed the engine that
generates it, so the result is a different corpus and S039's quoted figures are
not reproducible at all.

The premise itself is intact — the step is still worth doing. Re-measured at
HEAD over the tracked corpus, whose ninth field is the FEN:

```
$ tail -n +2 adocs/data/S028_raw.tsv | cut -f9 > /tmp/s028_fens.tsv
$ build/tools/eval_spread --data /tmp/s028_fens.tsv
  positions  5582
  150             7 0.125%      0 0.000%     29 0.520%
  200             0 0.000%      0 0.000%      2 0.036%
  mobility      176  6rk/1P2b3/8/3p3p/3P1p2/3b1P1q/1P3R1P/6BK b - - 1 41
  king safety   113  4r1k1/5ppp/p1b5/2bR4/2P5/1P1Q4/4K3/1R4q1 w - - 0 36
  combined      242  r1b2Q2/1pkr3p/2nq4/1R6/p3B3/3P2P1/P4P1P/2R3K1 w - - 5 27
```

The conversion is not optional: `eval_spread` reads the FEN as the first
tab-separated field (`tools/eval_spread.cpp:163-165`), so pointing it straight
at `adocs/data/S028_raw.tsv` prints "no positions read from
adocs/data/S028_raw.tsv". S039 has to carry the command that works, not the one
`DEV_MANUAL.md` documents against a dataset this tree does not have.

Without this, a session starting S039 discovers the missing input after
promoting the step, and its choices are an unplanned datagen run or quietly
calling a corpus of a different size "the full corpus" — while `plan.md` prices
S039 as one cheap run before an SPRT.

Full evidence: 2026-08-13_plan_review.2-F02.
