id:         S108
goal:       every node that is not in check computes its static evaluation once, stores it in its table entry and reads it back, so improving and the pruning margins have an input
accepts:    an SPRT verdict, recorded whatever it is; `improving` is `static_eval(ply) > static_eval(ply - 2)`, falling back to ply-4 when the ply-2 node was in check and defaulting to true when neither exists -- and the fallback is **tested**, because a broken improving calculation costs Elo without ever crashing; the eval is computed once per node and never twice, and the table entry no longer overwrites a stored evaluation with TT_EVAL_NONE; INV-4 holds -- the number comes from the accumulators `make_move` maintains and nothing stops maintaining them; the nps cost is measured and recorded next to the verdict
touches:    src/search.cpp negamax, src/transposition_table.cpp, src/data_structures.hpp
excludes:   consuming improving, which is S109 and the reduction work; correction history, which is S099
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Why this is a prerequisite and not a feature

`improving`, futility pruning, razoring, ProbCut's margin and correction
history all read a static evaluation at their own node. Today the main search
computes one **only at reverse-futility nodes** -- `static_eval` is
`TT_EVAL_NONE` everywhere else, by design, because S094 and S103 deliberately
declined to add a call.

So S092 as originally ordered -- the improving flag, first in the pending
order -- had no input to read and no consumer to feed. It is folded into this
step, which supplies the input, and its consumers arrive at S109.

## The bug this also fixes

`negamax` stores `static_eval` on every path, and `static_eval` is
`TT_EVAL_NONE` at every node except a reverse-futility one. So a main-search
store **overwrites a real evaluation that quiescence had already recorded** for
the same position. The field was added at S094 and is read at S103 and the
overwrite has been there since. Small, and it is exactly the kind of thing that
makes the hit rate at the S103 site lower than it should be -- 23.1 % over 300
positions at depth 10, 10.7 % on the three search_bench positions at depth 12.

## The cost, measured 2026-08-19

The lazy shortcut is load-bearing and this step spends part of it. `go movetime 2000`
from the start position, tune build, `LazyEvalMargin` at three settings:

| margin | nps | what it means |
|---|---|---|
| 0 | 6.55 M | expensive terms effectively off |
| 150, shipping | 5.46 M | the shortcut fires on most nodes |
| 2000 | 4.82 M | full evaluation at every node |

Full evaluation everywhere costs **11.7 %** against shipping. This step does not
go that far -- it computes at non-check main-search nodes and reuses through the
table -- but the number bounds the worst case, and `-march=native` at S104 has
already paid +16.7 % toward it.
