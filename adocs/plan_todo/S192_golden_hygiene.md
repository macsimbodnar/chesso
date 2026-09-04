id:         S192
goal:       every golden number in `tests/` is named as one with the script that re-derives it, the piece anchors are re-derivable from the repository, and the soft-limit scaling test asserts the rule on a constructed history
accepts:    an inventory in this file of every golden in `tests/` -- the static-score anchors 563/567/198/-505/-569 and the piece anchors 135/244/325/563/787, the `test_mate_carry` per-game floors, the `test_mate_breadth` floor, the mate-in-three floor, the node budgets of "ordering keeps the tree small", the `test_search.cpp` table-independence claim -- each with a comment at its site naming it a golden and the command that re-derives it; `.tuning/anchors.py` committed as `adocs/data/S192_anchors.py` (rewritten if it is lost) and shown to reproduce 135/244/325/563/787 and 563/567 at the shipped weights; `tests/test_engine.cpp`'s "the iteration loop scales its soft limit by what the search found" replaced by a case that feeds the loop's scaling from a constructed stability and score-drop history and asserts the rule, with the tree-dependent assertions removed; `tests/test_search.cpp`'s "the table never changes the answer" comment names the S130 stand-pat substitution and quiescence answering from main-search entries as the property's known exceptions; the fault-injection driver re-run over M06a, M09, M29 and M30 shows each still caught, by a golden or by S191's cases; fast suite green in both builds
touches:    tests/, adocs/data/, .gitignore, DEV_MANUAL.md
excludes:   changing any golden's value; any retune or refit
decisions:  DEC-139, DEC-142
closes:     2026-09-04_test_review-F03
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F03`. A large share of the suite's sensitivity comes
from golden numbers that every legitimate search or evaluation change also
moves: the eval-sign mutant turned 16 cases red, twelve of them on the static
anchors; `test_mate_carry` went red on 21 of the 22 search mutants; the
soft-limit scaling case went red on eight search mutants that were not
time-management defects, because it asserts on a fixed position's tree rather
than on the rule. The numbers are right today and they did their job in the
pass. The cost is that every pending search step and every refit will redden
several of them for no defect, each needing a re-derivation, and a floor
re-derived under time pressure is how a gate gets weakened -- S145 recorded
two surveyed engines that disabled their mate tests rather than their pruning.
DEC-116 states the rule for one floor; DEC-142 generalises it and this step
applies it.

## Cost

Machine-free, about a day. `adocs/data/S154_floor_margin_sweep.py` is the
pattern for a re-derivation script.
