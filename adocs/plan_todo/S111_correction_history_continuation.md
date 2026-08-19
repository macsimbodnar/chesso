id:         S111
goal:       correction tables indexed by the move played two and four plies ago
accepts:    an SPRT verdict, recorded whatever it is; the sentinel plies are valid at ply 0 through 3 and a test covers them; a null move installs a correction pointer rather than leaving the child to index garbage; the corrected score still cannot reach the decisive band; the blend weights across the correction tables are constants in src/search_params.hpp with ranges, fitted at S127 and not taken from anywhere (DEC-084)
touches:    src/search.cpp, src/data_structures.hpp, src/search_params.hpp
excludes:   the pawn and non-pawn tables, which are S099 and S110
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## The free second use

Once the tables exist, `abs(correction)` is a complexity signal that costs
nothing to read: a position the correction disagrees with the static score
about is a position worth reducing less in and worth a wider reverse-futility
margin at. Both are one-line consumers and both are reported as small positive.
They belong to S098 rather than here, but they are the reason S098 is ordered
after this and not before it.
