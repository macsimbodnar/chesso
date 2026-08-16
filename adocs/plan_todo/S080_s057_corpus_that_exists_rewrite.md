id:         S080
goal:       S057 argues from the corpus on disk and the spread measured at HEAD, and S039's stale premise is named in full
accepts:    `S057`'s body names `.tuning/selfplay_v2.tsv` and the `eval_spread` invocation `DEV_MANUAL.md` documents for it, states that the file is gitignored and that a machine move loses it again -- so the 5582-row tracked corpus is the fallback and not the first choice -- and carries the spread re-measured at the commit S057 completes on, over the full corpus and over the tracked one; `S039`'s figures are replaced by the same measurement and its `accepts:` requires that `src/evaluation.hpp`'s mobility sentence be re-measured, not only its king-safety sentence; every `file:line` in S057 resolves after the rewrite
touches:    adocs/plan_todo/S057_s039_corpus_that_exists.md, adocs/plan_todo/S039_lazy_margin_redecide.md
excludes:   choosing `LAZY_EVAL_MARGIN`, which is S039's decision and its verdict; editing `src/evaluation.hpp`'s comment, which S039 does in its own commit; regenerating any corpus
decisions:
closes:     2026-08-16_plan_review-F01
blocks:
paused_by:
done:

## Why this exists

S057 exists to hand S039 corrected evidence. Every load-bearing claim in it is
false at HEAD, and it is the highest-severity finding of the 2026-08-16 audit.

**The corpus exists.** `S057:17-20` says it "does not exist here: `.tuning` is
gitignored and `DEV_MANUAL.md:512` says so. The work moved machines at DEC-049
and the dataset did not come with it."

```
$ ls -la .tuning/selfplay_v2.tsv
-rw-rw-r-- 1 max max 715409623 Aug 14 08:13 .tuning/selfplay_v2.tsv
```

S065 regenerated it (DEC-055), the FEN is the first tab-separated field so no
conversion is needed, and `DEV_MANUAL.md:254-255` documents the exact run S039
asks for. The `DEV_MANUAL.md` line S057 cites now says the opposite of what S057
draws from it: it scopes non-existence to `selfplay_v1.tsv` and distinguishes it
from v2.

**The replacement figures are stale by S065's refit.** S057 prints a run "Re-measured
at HEAD" giving `29 0.520%` / `2 0.036%` and a worst combined of 242. Re-run
verbatim at `02bb6a5` over the same 5582 tracked rows: `40 0.717%` / `17 0.305%`
/ `96 1.720%`, worst combined **326**. S057 was written on 2026-08-13 and S065
(`33aa3b4`) refitted 827 constants on 2026-08-15.

**The real corpus is worse still.** Over all 11003693 rows at HEAD:

```
$ build/tools/eval_spread --data .tuning/selfplay_v2.tsv
  positions  11003693
                 mobility  king safety     combined
  p99.9               176          164          233
  max                 267          363          489
  150         51006 0.464%  20565 0.187% 171424 1.558%
  200          1491 0.014%   2068 0.019%  32524 0.296%
  300             0 0.000%     17 0.000%    995 0.009%
  400             0 0.000%      0 0.000%     22 0.000%
```

S039 records "0.364 %" past 150 and a worst of 279, "so `evaluate()` discards up
to 129 cp". Measured: **1.558 %** and **489**, so up to **339 cp** is discarded,
on four times as many positions.

**And the shipped comment's leading claim is false, which S039 does not say.**
`src/evaluation.hpp:274-278`:

> 150 is above the largest correction observed over 149084 positions of S028
> self-play, where the tapered mobility term ran p50 19, p95 60, p99 81,
> p99.9 107 and a maximum of 143 centipawns.

Mobility alone now reaches **267** and king safety alone **363**. S039's "What is
stale" section names only the king-safety-at-zero-weight sentence; the sentence
above it is false too.

## What this changes about S039's decision

S039 picks a margin. A margin chosen from a 0.5 % tail when the real tail is
1.6 %, and from a worst case of 242 when it is 489, is chosen from the wrong
distribution -- and `2026-08-13_adversarial-F05` is still open behind it.
S057's `accepts:` does say "figures re-measured at the commit S057 completes on",
so a careful session recovers; the body reads as already-done work and argues
for the smaller corpus on a premise that no longer holds, which is what makes it
worth a step rather than a note.

## Cost

Minutes plus two `eval_spread` runs, 18.6 s for the full corpus. No match.
