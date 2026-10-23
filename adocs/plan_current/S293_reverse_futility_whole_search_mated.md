id:         S293
goal:       close the defect S291 found -- reverse futility failing high at a node whose entry is a mated upper bound or exact score from a node that skipped no move, DEC-306's shape -- as its own measured change, with S292's condition reused, or show by measurement that no guard of that kind is correct
accepts:    mined row 171's loss under S291's scratch guard (`adocs/data/S291_reach.txt`: m5 -> cp 2104) traced from the engine's own output to its node and rule before anything ships; if the guard hides a mate the shipped tree reports and no narrower form avoids it, stop with the figures for the coordinator; otherwise a case observed red first on the shipped tree -- a node driven so its entry holds S292's mated whole-search bound and reverse futility's margin clears beta, S191's probe `rfp_cutoff` false once guarded -- and green after; a mutant it kills (DEC-141); both builds green; bench and `tools/search_bench.py` d9/d12; every moved golden by its script inside DEC-299's guard; gate_extra; the Debug self-play command written for the coordinator; S291's four mate sets and its census re-read with the shipped form; implemented from published descriptions, each named and quoted (DEC-221): reverse futility as static null-move pruning, and the null move's premise in David-Tabibi and Netanyahu, ICGA J. 25(3), 2002; a `{-5, 0}` SPRT against its parent pre-registered in `adocs/data/S293_sprt.sh` in S292_sprt.sh's shape, with the worst-case expected games from the nElo formula, the abort rule, the three outcomes (H0 reverting it whole), and the expectation that with 0 ordinary-play firings at depth 10 the run sits near a true zero and may reach its cap; the fast suite green in both builds
touches:    src/search.cpp (reverse futility's guard), tests/, tools/mutants/, adocs/data/S293_*, DEV_MANUAL.md, MANUAL.md if a stated behaviour moves
excludes:   reverse futility's margins and depth ceiling (DEC-158, S127); the null move and DEC-310's guard; the returns on bounds from nodes that skipped a move (not a proof, DEC-310); S083's constants
decisions:  DEC-311, DEC-310, DEC-306, DEC-171, DEC-158, DEC-143, DEC-221, DEC-141, DEC-142, DEC-299
closes:
blocks:     S083
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-23, DEC-311)

S291 found reverse futility returning a fail-high, with beta outside the
mate band, at nodes whose entry is a mated upper bound or exact score stored
by a node that skipped no move. That is the comparison DEC-306 read as a
zugzwang for the null move, and DEC-310 guarded the null move against it.
DEC-158 priced reverse futility only where nothing is proved. S291 counted
90 such returns over S289's four mate sets (45, 2, 13, 30) and none over
11503 ordinary-play positions at depth 10. Over the mate sets its scratch
guard moved eight reported scores and two best moves. S145 root 76, a
proved mate in 5 for the side to move, reports cp 0 with b7c7 shipped and
cp 529 with b7b2 guarded. It gained and lost no exact mate in S145's 186 defender
nodes, and it lost mined row 171 (m5 -> cp 2104)
(`adocs/data/S291_reach.txt`, `S291_census.txt`, S291 section 5).

DEC-311 reads it as BUGS-binding under DEC-171 (c): the defect moves
reported scores and best moves, and (c) carries no ordinary-play limit. So
it is fixed before S083's run, and S083 is paused by it. The scratch
guard's loss of row 171 says the obvious form is not yet known to be
correct. Tracing that loss comes first.
