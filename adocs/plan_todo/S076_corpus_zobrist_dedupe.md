id:         S076
goal:       the fit runs on a corpus deduplicated by zobrist key, so a repeated position stops carrying repeated weight
accepts:    a tracked tool reduces an existing corpus to one row per distinct position without replaying anything, keyed on the engine's own zobrist key rather than on the FEN text, and prints how many rows it dropped; the duplicate count on `selfplay_v2.tsv` is recorded in the step, since nobody has measured it; the deduplicated corpus is refitted and **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is; the dedupe pass is deterministic -- run twice, same output file byte for byte
touches:    tools/, .tuning/, src/eval_tables.hpp, src/evaluation.cpp
excludes:   regenerating any corpus, which is S082's and S083's; changing datagen's filters; the label blend, which is S075's; the held-out split, which S066 fixed
decisions:  DEC-056
closes:
blocks:
paused_by:
done:

## Why this exists

`adocs/eval_tuning_strategy.md` section 2.6 lists dedupe among the filters "all
of which matter": "Deduplicate by Zobrist key; heavily repeated positions bias
the fit."

`tools/datagen` does not dedupe. Every game starts from 8 uniform random plies
and every position after a capture-free opening repeats across games; the
resulting weight on early and drawish positions is whatever the opening
randomiser happened to produce, which is not a modelling choice anyone made.

**How big the effect is here is unknown**, and that is the first thing this step
produces. 11003693 rows from 120000 games at 8 random opening plies could hold
almost no duplicates or a great many; the step records the number before it
records a verdict, because a dedupe that drops 0.1 % of rows is not worth an
SPRT and one that drops 20 % changes what the fit is fitting.

## Why key on the engine's zobrist and not on the FEN string

Two FENs that differ only in halfmove clock or fullmove number are the same
position for evaluation purposes, and the en passant field is written by
`make_move` whether or not a capture is available -- which is
`2026-08-13_adversarial-F08` and S042, still open. Keying on the text would treat
those as distinct and would inherit the same defect the plan is already fixing
elsewhere. The engine's key is one call and is the definition the rest of the
project uses.

## Interaction with the steps after it

S082 changes which position is labelled, and S083 changes how many there are.
Both make this pass part of corpus preparation rather than a one-off, which is
why it is a tracked tool and not a shell pipeline in a scratchpad -- the shape
`2026-08-16_plan_review-F04` found and S072 is cleaning up.

## Cost

A pass over 715 MB, minutes. One fit, minutes. One SPRT if the duplicate count
justifies it, three to four and a half hours -- and if it does not, that is
recorded as the result and no match is spent.
