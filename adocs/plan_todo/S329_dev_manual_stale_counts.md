id:         S329
goal:       three statements in `DEV_MANUAL.md` that S325 and S327 found stale say what is true, history kept
accepts:    "The second fit's constants are the ones in `eval_tables.hpp`" re-read against every commit to `src/eval_tables.hpp` since `33aa3b4` and made true, dated or put in the past tense; the S187 count, "six more were wrong when written", re-read against S187's own list and `adocs/data/S187_symbols.tsv` and made true in `DEV_MANUAL.md`, with `plan_done/S187`'s stamp left unedited and the difference stated beside it; the "Measured on `selfplay_v2.tsv`" paragraph's 207998 dropped, S076's figure, dated as S076's beside what a current `corpus_dedupe` drops (208490 by S327's run, re-read); each old and new text in this step; `tools/plan_prose_check.py`'s checks and `test_plan_citation_freshness` green
touches:    DEV_MANUAL.md
excludes:   `adocs/plan_done/`; shortening a passage or removing history (DEC-348)
decisions:  DEC-171, DEC-348
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-03 by S325's implementer. Six commits since `33aa3b4` changed `eval_tables.hpp`, but the sentence is in
the present tense. S187's stamp says six more citations were wrong when written. Its own list names eight, and three of
them, the ones at line 674 of `src/chesso.cpp`, were right when written (S325, by `adocs/data/S169_recitations.tsv`),
so the list supports at most five; `DEV_MANUAL.md` repeats the six. Filler by DEC-171: documents only.

Also found 2026-11-03 by S327's implementer: `DEV_MANUAL.md`'s "Measured on `selfplay_v2.tsv`" paragraph gives S076's
207998 dropped in the present tense; since S042 a current binary drops 208490 on the same input (S327's
`adocs/data/S327_verify.txt`).
