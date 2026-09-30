id:         S249
goal:       three golden comments read what their trees report -- `test_mate_breadth`'s shipping count, `MATE_IN_THREE_FLOOR`'s comment, and S132's sweep docstring's offsets
accepts:    `tests/test_mate_breadth.cpp`'s GOLDEN comment quotes the count the shipping tree reports (145 written, 147 read on `cd50c7a`'s parent) with the command that took it; the comment beside `MATE_IN_THREE_FLOOR` in `tests/test_engine.cpp` quotes its count (12 written, 13 read) the same way; `adocs/data/S132_node_share_census.py`'s docstring names the offsets its aspiration rows use (0/1/2, not S021's 0/37/71) or is corrected to what the script does; no floor and no assertion changes; both fast suites green
touches:    tests/test_mate_breadth.cpp (a comment), tests/test_engine.cpp (the comment beside `MATE_IN_THREE_FLOOR`), adocs/data/S132_node_share_census.py (a docstring)
excludes:   the floors, the counts' derivation rule, any behaviour
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-09-30)

S115's build read the mate guards against the tree and found two golden
comments quoting shipping counts the parent no longer reads -- both floors
still hold, so nothing is red -- and its sweep script, executing S021's
picker, found S132's docstring calling offsets 0/1/2 "the aspiration rows'
300" where S021's rows are at 0/37/71. Comments and a docstring, no reach
into play: a filler behind the next strength step (DEC-171), named by id in
`adocs/data/S115_sprt.sh`'s open findings while open.
