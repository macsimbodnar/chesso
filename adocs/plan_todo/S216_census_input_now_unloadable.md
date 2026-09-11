id:         S216
goal:       the S159 killer-slot census stops silently measuring the wrong position, and the instrument refuses to report a position the engine would not load
accepts:    `adocs/data/S159_census_run.py`'s reader treats a `position fen` refusal as a failure of that row -- the `info string refused [position fen]` line is detected and the row is reported as refused with a non-zero exit, rather than being scraped for `info ... nodes` and silently inheriting the previous board's numbers; the `promo-mess` row of `adocs/data/S159_census_positions.txt`, which holds the pre-S208 `KILLER_POS` and is now refused, is resolved one of three ways and the choice recorded as a decision -- replaced by the legal `KILLER_POS` (which re-derives the row and is the `either end moved` trigger of DEC-142), dropped from the set, or kept with the census re-run and the row reported as refused; `adocs/data/`'s append-only convention is respected, so a changed input or a changed reader lands as a new file beside the old with the old one named and dated rather than edited, unless a decision says otherwise; whether the census is re-derived at all is decided against what it is cited for -- DEC-160 reads it as the evidence that refuted the ageing reading of S149's 11 Elo before a game was played, and the step states whether one refused row of sixteen can move that reading; `adocs/data/README.md` gains a row for whatever file this lands
touches:    adocs/data/, adocs/decisions.md
excludes:   the S208 load bound and `KILLER_POS` itself, both decided at DEC-177; re-running any match; any `src/` change
decisions:  DEC-142, DEC-160, DEC-177
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found by the Tier-1 fast check over S208's completing commit `8aff8ac`, on
2026-09-11, and confirmed at the file.

DEC-177 made `KILLER_POS` legal by deleting its h3 pawn, because S208's load
boundary refuses more than 16 pieces of a colour and the old constant carried
17. `adocs/data/S159_census_positions.txt` row 16 -- `promo-mess` -- is the
**old** FEN, verbatim, and it is now refused.

What that does to the instrument is worse than an error.
`adocs/data/S159_census_run.py` writes `position fen <row>` then `go depth 12`
and scrapes only lines starting `info` and containing ` nodes `. A refusal is
an `info string` line with no ` nodes ` field, so it is skipped; the `go` then
searches **whatever board survived the refusal**, which is the row above --
`tactical` -- and its numbers are printed under the name `promo-mess`. The
census reports sixteen rows, fifteen positions, and says nothing about it.

`specs.md` and DEC-160 both read that census as the evidence that refuted the
ageing reading of S149's 11 Elo *before* a match was spent, so a
re-derivation that silently measures a different set is exactly the class
DEC-142 exists to stop: **either end moving is the trigger, and this end
moved.**

## Two things to settle, and the second is a judgement

**The reader.** A refusal must fail the row loudly. That is not a
re-derivation, it is the instrument refusing to lie, and it should land whether
or not the census is re-run. Any other row of any other census that stops
loading is caught by it from then on.

**The row.** Replacing it with the legal `KILLER_POS` keeps the set's shape --
it is there as the promotion-heavy position, and the legal constant keeps all
twelve promotions and the `f5e6` en-passant capture (DEC-177) -- but it changes
a recorded input, and `adocs/data/` is append-only by the convention
`adocs/data/S198_pairs.py` states in its own comment. So it is a new file
beside the old, or a decision that says otherwise. Dropping the row instead
shrinks a set that a recorded decision rests on.

**Whether the census is re-derived** is the judgement. One row of sixteen was
measuring a duplicate, which inflates the total and misnames one row; whether
that can move DEC-160's reading is a question about the margin in that reading
and not about this row, and the step answers it from the recorded numbers
rather than by re-running first.

## Cost

Documents and a small script change. An hour, plus a census re-run only if the
step decides one is owed -- which is minutes, not a match.
