id:         S063
goal:       S024, S030 and S039 cite the src/evaluation.cpp lines that exist at HEAD
accepts:    every file:line citation in the three step files resolves to what the sentence around it claims — S024's counter-move read, S030's history and counter-move indexing, S039's king safety weights — or the sentence cites the symbol instead of the line; every other file:line citation in the pending step files still resolves, checked in the same pass
touches:    adocs/plan_todo/S024_continuation_history.md, adocs/plan_todo/S030_move_encoding_16_bit.md, adocs/plan_todo/S039_lazy_margin_redecide.md
excludes:   implementing any of the three steps; any edit to src/
decisions:
closes:     2026-08-13_plan_review.2-F08
blocks:
paused_by:
done:

## What is there

Every `file:line` citation in the pending step files was resolved against HEAD.
Three miss, all in `src/evaluation.cpp` and all by the same two lines:

| step file | citation | what is there at HEAD | what was meant |
|---|---|---|---|
| S024 | `:1091-1094` "read as the fixed `ORDER_COUNTER` band" | `:1091` is the `killer_moves[1]` test | the counter-move read is `:1093-1096` |
| S030 | `:1092,1097` "indexes `history_moves` and `counter_moves`" | `:1092` and `:1097` are blank | `:1094` and `:1099` |
| S039 | `:731-734` "King safety has not shipped at zero weight since S027" | a comment about collinearity | the weights are `:733-736` |

Cause: `8b89411` ("Complete S037") changed `src/evaluation.cpp` by +3/-1 at
19:13, after S045, S046 and S049 rewrote those three step files at 18:23-18:32.
Everything else resolves exactly, S031's four xor sites and S055's six
`src/evaluation.cpp` lines included.

Cosmetic for a reader who greps. It matters here because S030's whole neutrality
argument is a list of pointers — see S058, which corrects what that list says —
and a pointer landing on a blank line is one more reason to rebuild the site set
by hand rather than trust the file. Citing the symbol is the durable form where
the file is likely to move again.

Full evidence: 2026-08-13_plan_review.2-F08.
