id:         S096
goal:       extend a node that gives check, so a forcing line is not cut at the horizon
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); a bound on total extension so a checking sequence cannot make the tree unbounded, stated as a constant in src/search_params.hpp with a range that is arithmetic rather than a guess (S073); tests/test_search.cpp keeps a case that search depth is finite on a position with a long series of checks, observed to hang or to blow the ply limit with the bound removed; the mate cases in the fast suite pass and the mate-in-two distance is unchanged; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   singular extensions, which are S097; extending on anything other than check
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Note

This is the first thing in the search that makes the tree *larger*. Everything
in the block before it prunes or reduces, so a node-count comparison against
HEAD is expected to move the other way and that is not a regression by itself.
The bound is what keeps it finite and the test is on the bound, not on the
gain.
