id:         S265
goal:       a late capture or promotion that SEE does not call losing can be reduced on its own, smaller schedule, adjusted by its capture-history score; the extra ply a SEE-losing capture already takes stays on top
accepts:    runs only if S023 is kept -- on S023's H0 or no verdict this step goes to the reserve, and a form without capture history is a different candidate with its own pre-registration, never an automatic fallback; (1) a reduction for captures and promotions with its own base and divisor, adjusted by S023's capture-history score; (2) the existing exclusions hold and each is asserted: no reduction for a checking capture, in check, at the root, below the quiet reduction's minimum depth, or for the first moves of the list; (3) a direct guard test of the new rule and a mutant it kills (`tools/mutation_check.py`, DEC-141); the mate suites green; the second tier (Debug self-play, `tools/gate_extra.sh`) before completion; (4) one `{0, 5}` nElo SPRT with DEC-143's worst case and abort rule pre-registered; (5) every new constant in `src/search_params.hpp` `CHESSO_SEARCH_PARAMS` with a stated range, the UCI surface golden refreshed only after `adocs/specs.md` and `MANUAL.md` describe the options
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, tools/mutants/, MANUAL.md, adocs/specs.md
excludes:   the quiet reduction and its terms; capture history inside SEE or futility margins; any change to the SEE-losing extra ply S091 added
decisions:  DEC-258, DEC-134, DEC-141, DEC-143, DEC-221
closes:
blocks:
paused_by:
author:
done:

## Why

Only quiet moves take the late-move reduction; a late capture that SEE calls
safe is searched at full depth however unlikely it is to matter. The 2026-10-08
ruling puts capture history and its two consumers in the main order as one
family of three verdicts after correction history (DEC-258): S023, then S025,
then this.

## Description it is implemented from

CPW *Late Move Reductions*: several engines reduce captures and promotions on
a separate, smaller formula and adjust it by history. Stash's changelog,
v31: one ply of reduction allowed for captures and promotions, +4.9. An
Ethereal commit message (2020-09-26, "Use Capture History to apply LMR to
some Tactical Moves", +7.22 +/- 4.72 at 10+0.1, +2.37 +/- 1.90 at 60+0.6,
quoted in S023's file) describes reducing tacticals with a negative capture
history. Its reduction amounts are that engine's constants and seed nothing
(DEC-134).

## Seeds (DEC-134)

The base and divisor come from a census over the bench positions of the move
number at which capture cutoffs occur -- the tune build's cutoff census
(`src/search.cpp` `census_cutoff`) counts quiet cutoffs today and is
extended to captures -- with the derivation written before the census runs
(form b), or the range midpoints (form c). The history divisor comes from
S023's stated capture-history bound (form b).

## From the record

S091 (the SEE-losing extra ply, part of a +46.52 verdict), S098 (the quiet
reduction's terms), DEC-213 (history-scaled quiet reduction measured zero
twice). Ported from the 2026-10-07 proposal's P10.
