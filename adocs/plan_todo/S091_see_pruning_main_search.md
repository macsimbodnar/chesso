id:         S091
goal:       skip captures the exchange evaluation says lose material, in the main search rather than in quiescence alone, and reduce a negative-SEE move by an extra ply
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); separate margins for captures and for quiets, both constants in src/search_params.hpp with stated ranges (S073), and the depth scaling stated as what it is rather than as a flag; a position with a forced mate inside the pruned depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; nothing is pruned at a PV node or while in check, with the precondition asserted; see() and see_ge() are unchanged and tests/test_search.cpp's exchange cases still pass unmodified; the fast suite green
touches:    src/search.cpp negamax move loop, src/search_params.hpp, tests/test_search.cpp
excludes:   any change to see() or see_ge() themselves; quiescence, where see_ge already declines losing captures at src/search.cpp:205; delta pruning, which is S022
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## The precedent that decides how this is read

S015 measured quiescence SEE pruning at **0 Elo** and it was kept anyway with
the reason recorded. That verdict was taken when `see()` cost 12.1 % more than
it does today, and the main search is a different caller with a different
alternative -- a move pruned here is not searched at all, where in quiescence it
was only declined. So this is a new measurement and not a re-run of that one,
and a verdict of zero here is recorded as zero as well.


## Re-scoped 2026-08-19

Quiet-move SEE pruning moved into S109, where it is one of four rules measured
as a block (DEC-082). What is left here is the capture side plus the extra
reduction for a negative-SEE move, and it is not a leftover: the +151 release
this step's ordering rests on is Leorik 2.4. **The second review tempers the
attribution (DEC-087): that release bundled SEE in the main search with a
reverse-futility cutoff inside null move and with drawn-material recognition,
so +151 is the bundle's number and not SEE's alone.** The step keeps its
position -- Ethereal's removal ledger prices its SEE pruning at -41.5 on its
own -- and DEC-019 keeps the expectation honest.

The margins differ by move type in every implementation surveyed -- a margin
scaled by depth for quiets, by depth squared for captures -- and both are ours
to fit (DEC-084).
