id:         P13 (proposed; the S-id is allocated at adoption)
goal:       the static evaluation shrinks toward zero as the halfmove clock grows, so the search prefers progress over shuffling in positions it cannot win
accepts:    (1) after correction, `eval · (C − clock) / C`, never applied to a mate or draw score, sign preserved; (2) a reach census first (DEC-239: reach, not value) -- the share of evaluated nodes with a clock of 20 or more over a 1000-game A/A's positions -- and bounds chosen from it by DEC-063 (`{0, 5}` if the change is expected to clear 5, else `{-5, 5}`); (3) tests: the scaled value is monotone in the clock and never changes sign; INV-5's colour symmetry holds; (4) one SPRT
touches:    src/search.cpp, src/evaluation.cpp, src/search_params.hpp, tests/test_evaluation.cpp, tests/test_search.cpp, adocs/specs.md
excludes:   any change to the fifty-move draw rule itself (S162)
closes:
paused_by:
author:
done:

## Description it is implemented from

Common practice; no published description was read while writing this. Pin
one at step start or record the design as the project's own.

## Seeds (DEC-134)

`C`: a derivation from the rule itself, a multiple of the hundred-ply limit
stated in the pre-registration (form 2). No engine constant.
