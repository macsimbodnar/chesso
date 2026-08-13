id:         S045
goal:       S024 distinguishes continuation history from the countermove heuristic and scopes the one-ply table first
accepts:    S024's step file states that chesso has the countermove heuristic (one move_t per previous piece and target, fixed ORDER_COUNTER band) and no continuation history at any depth; its scope reads one-ply table first, two-ply second, one SPRT verdict each
touches:    adocs/plan_todo/S024_continuation_history.md
excludes:   implementing any part of S024 itself
decisions:
closes:     2026-08-13_plan_review-F02
blocks:
paused_by:
done:      S024's note distinguishes the countermove heuristic (present, counter_moves[12][64], flat ORDER_COUNTER band) from continuation history (absent at every depth), and its accepts now asks two SPRT verdicts, one-ply table first, two-ply second, each against the commit before it. Gate green.

## What is there

`S024_continuation_history.md`: "One-ply continuation history is the
countermove table already present. Two-ply is the usual next step." What is
present — `move_t counter_moves[12][64]` at `src/data_structures.hpp:434`,
written on beta cutoff at `src/search.cpp:507`, read as a fixed band at
`src/evaluation.cpp:1091-1094` — is the countermove *heuristic* (Uiterwijk,
1992): one remembered refutation move with a flat bonus.

Continuation history is a different device: a score table indexed by
(previous move's piece, target) × (current move's piece, target),
accumulating graded bonuses for every quiet move. Reference engines carry
both at once, and the one-ply table is consistently reported the stronger
half of the pair. As written, the note steers S024 into building only the
two-ply table; a zero verdict on that would be recorded against a technique
whose strongest form was never built — the DEC-019 failure mode made locally.

Full evidence: 2026-08-13_plan_review-F02.
author:    Maksym Bodnar
