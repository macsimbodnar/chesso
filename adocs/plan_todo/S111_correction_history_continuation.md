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

## Reserve, 2026-08-19, DEC-087

Demoted behind the 3000 push with S110: measured +1.8 to +4.6 per table and
only above ~3100 in the surveyed record. **Both halves of that sentence are
unverified.** The 2026-09-04 literature check searched for the continuation
figures and the band and found neither (row A22); what it did find is that
Stockfish pull request #5617 is where continuation correction history landed
and that CPW's page carries no Elo number for any variant
(https://www.chessprogramming.org/Static_Evaluation_Correction_History). The
demotion rests on S110's sourced pawn figure and on the reserve's own logic,
not on +1.8 to +4.6.

**Searched again 2026-09-13**, after S186's fast check: an open web search for
the continuation figures and for the rating band, the CPW page again, and the
engine changelogs that search turns up. **The band stays unverified** -- no
source anywhere states a rating floor for this feature. **One figure for the
feature itself is now sourced**, and it is a different engine's and a
different number: Tcheran's `CHANGELOG.md`, "Added 1-ply continuation
correction history (8.23 +- 5.04)", in its unreleased section
(https://raw.githubusercontent.com/tcheran-chess/tcheran/master/CHANGELOG.md;
changelog entry only, DEC-016, and the file states no time control for it). It
is above the +1.8-to-+4.6 range this file could not source, it is one engine's
single measurement, and it is direction only (DEC-019): the demotion stands on
S110's sourced pawn figure and the step is still gated on S099's verdict.

## The free second use

Once the tables exist, `abs(correction)` is a complexity signal that costs
nothing to read: a position the correction disagrees with the static score
about is a position worth reducing less in and worth a wider reverse-futility
margin at. Both are one-line consumers and both are reported as small
positive. They belong to S098, which now lands long before this step -- if
this table ever ships, revisiting that consumer is part of its scope.
