id:         S079
goal:       S063 covers every pending step file and cites symbols, and S058 stops requiring a literal line number
accepts:    `S063`'s `touches:` names every pending step file that carries a stale citation, not three of them, and its `excludes:` no longer forbids the six the audit listed; its `accepts:` asks for a symbol or a test title where one exists and for a line only where nothing else identifies the site; `S058`'s `accepts:` stops requiring S030's paragraph to name "the write at src/search.cpp:507" and names the write by symbol instead; the 17 misses in the audit's table are listed in S063 so the pass has a checklist; a re-resolution of every `file:line` in `plan_todo/` after S063 completes reports none missing
touches:    adocs/plan_todo/S063_step_file_line_citations.md, adocs/plan_todo/S058_s030_prev_move_piece_remedy.md
excludes:   performing the repairs, which is S063's own job and happens right after this; the body evidence in S060 and S061, which is S078's; any change under `src/` or `tests/`
decisions:
closes:     2026-08-16_plan_review-F02
blocks:
paused_by:
done:

## Why this exists

S063 was created for three stale citations (`2026-08-13_plan_review.2-F08`). The
2026-08-16 re-run resolved every `file:line` in the 22 pending step files and
found **17 misses across six files**, of which **nine are outside S063's
`touches:`**, which names only `S024`, `S030` and `S039` and whose `excludes:`
forbids editing anything else. Its `accepts:` asks that "every other file:line
citation in the pending step files still resolves, checked in the same pass" --
a check whose remedy its own scope puts out of reach.

The set grew because `6bd650e` (S033) moved `src/search.cpp` by 57 lines and
`33aa3b4` (S065) moved `tests/test_eval_model.cpp` by about 20.

| step file | citation | what is at HEAD | what was meant |
|---|---|---|---|
| S024:16 | `src/search.cpp:507` counter-move write | `if (reduction < 0) { reduction = 0; }` | `:564` |
| S024:17 | `src/evaluation.cpp:1091-1094` | `:1091` is `killer_moves[1]` | `:1093-1096` |
| S030:21 | `src/evaluation.cpp:1092,1097` | both blank | `:1094`, `:1099` |
| S039:20 | `src/evaluation.cpp:731-734` | a collinearity comment | `:733-736` |
| S056:17 | `tests/test_eval_model.cpp:381-422` | mid-comment through the loop | `:401-442` |
| S056:18 | `:412` `CHECK_MESSAGE(difference > 2.0` | blank | `:432` |
| S056:19 | `truncation_positions` at `:215-220` | inside `positions` | `:227-232` |
| S056:20 | `:420` `CHECK_MESSAGE(worst > 2.8` | `REQUIRE(load_FEN(...))` | `:440` |
| S056:32 | `:388-391` tempo precondition message | a comment about the 2.0 threshold | `:408-411` |
| S056:34 | `:366-373` "the old 2.0" comment | the tempo/counts comment | `:378-385` |
| S057:17 | `DEV_MANUAL.md:217` eval_spread input | a node-count paragraph | `:254` |
| S057:18 | `DEV_MANUAL.md:512` | mid-sentence, and about v1 only | `:508-512` |
| S057:22 | `DEV_MANUAL.md:400-401` datagen command | `grep -c "^Finished game"` | **gone** |
| S057:23 | `adocs/status.md:37` "95 minutes" | DEC-033 prose | `:57` |
| S058:27 | `src/search.cpp:503` history write | the LMR guard | `:560` |
| S058:3, :28 | `src/search.cpp:507` counter-move write | `if (reduction < 0)` | `:564` |
| S061:23 | `tests/test_evaluation.cpp:643-704` bands | case runs `:652-713` | `:652-713` |

S057's rows are S080's to resolve, since S080 rewrites that file outright; S061's
row is S078's, which replaces the range with the case title. The rest are S063's.

## One of these is actively harmful

S063 sits at list entry 49 and S058 at 62. S063 repairs S030's citations; S058
then rewrites S030's hazard paragraph and its `accepts:` (`S058:3`) **requires**
that paragraph to name "the write at src/search.cpp:507". A session that
satisfies S058 as written puts a stale line number back into S030 after S063 has
just removed one. That is why S058's gate is edited here and not left to S063.

## Symbols, not lines

```
$ grep -n "MOVE_PIECE" src/search.cpp
560:            state->history_moves[MOVE_PIECE(moves[i])][MOVE_TO(moves[i])];
564:          state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)] =
```

`grep -n MOVE_PIECE src/search.cpp`, `CHECK_MESSAGE(difference > 2.0` and a
`TEST_CASE_FIXTURE` title all survive an edit above them. A line number does not,
and this is the second audit in a row to find the same class. S071 applies the
same remedy to `plan.md`'s citation of the checker.

## Cost

Minutes for this step. S063 then pays one pass over the pending step files.
