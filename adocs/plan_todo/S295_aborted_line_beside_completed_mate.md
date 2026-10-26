id:         S295
goal:       a search that ends inside an iteration never prints a line beside a mate score that does not mate at the claimed distance; and the completed iteration's mate distance, one move short of the oracle's on S145 mined row 168, is traced to its node and rule
accepts:    (1) on S294's cell (`adocs/data/S294_aborted_long_line.txt`: S145 mined row 168, cold table, Hash 16, `go nodes 150000`) a case in the fast suite observed red on the shipped tree and green after: every line printed beside `score mate N` ends in checkmate at exactly 2N - 1 plies (2|N| for a mate received) or is left short as DEC-122 allows, and `bestmove`/`ponder` are that line's first two moves; the cell is a golden named at its site with the script that re-derives it (DEC-142); the fix is in the reporting layer only and is INV-6 identical (bench total and `tools/search_bench.py` at 9 and 12, nodes and best moves); a mutant it kills (DEC-141); the S170 cells and `test_mate_carry` read as before or tighter, a ceiling it lowers restated under DEC-162's rule, none raised; (2) the completed depth-8 iteration's `mate 4` on that cell (Stockfish at depth 20: mate in 5 by h1g1) traced from the engine's own output to the node and the rule that drops or refutes h1g1, every chess claim from a tool; if it is a recorded price (DEC-158 or another), say which with the evidence; if not, stop with the figures for the coordinator before any search change; MANUAL.md's `depth` and `pv` rows true afterwards; both builds and both fast suites green
touches:    src/search.cpp (the end-of-search line choice, the aborted-line fallback S202 wrote, DEC-251 (c1)); tests/test_mate_pv.cpp (a case); tools/mutants/ (one mutant); adocs/data/S295_*; adocs/data/README.md; MANUAL.md (`depth`, `pv` rows if restated); DEV_MANUAL.md (the goldens row)
excludes:   any change that moves a node (a search change for reading (2) is its own step, measured alone); the S170 budgets (DEC-299); the completion walk's move choice (DEC-251 (c2)) unless the trace shows it is the cause, then stop and report
decisions:  DEC-317, DEC-171, DEC-122, DEC-251, DEC-162, DEC-141, DEC-142, DEC-158
closes:
blocks:
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-26, DEC-317)

S294 found it while sweeping for MW02's cell. At every budget from 80000 to
200000 nodes on S145's mined row 168, a search that stops inside its ninth
iteration prints the aborted iteration's 9-ply line beside the depth-8
iteration's `score mate 4`. python-chess finds no mate at ply 7 and
checkmate at ply 9, and `ponder` names the aborted line's reply (h1g1)
where the completed line's is h1h2. S202's fallback (DEC-251 (c1)) does
not run, because it catches only an aborted line the walk cannot complete;
this one completes, to a longer mate. Stockfish at depth 20 reads the root
mate in 5, so the completed `mate 4` is one move short as well. MANUAL.md's
`depth` row says the pair never disagrees and its `pv` row says the line is
never wrong. It moves a reported line wherever a search stops inside an
iteration, so it is BUGS-binding under DEC-171 (c) and goes before S136
(DEC-317).
