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
-- **a record of where the technique landed, and nothing more** (S232,
2026-09-19): no number out of that pull request appears anywhere in this file
and none seeds anything in it, here or at S127, because a constant another
engine ships is that engine's wherever it is republished (DEC-105, DEC-134)
-- and that CPW's page carries no Elo number for any variant
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

## Constants and seeds

**Written 2026-09-19 by S232**, under DEC-105 as DEC-134 states it, so this
file names its seeds before it is ever started. Three forms, and each seed
says which: **(a)** a value from a publication *about the technique*, with
its URL; **(b)** a derivation over chesso's own data or scale, run by this
step at its start; **(c)** the range midpoint or the off value, stated as
such. No publication about the technique states a value for any constant
here, so **(a)** is unavailable throughout and is not claimed.

- **The two index offsets** -- **not a seed at all.** "The move played two
  and four plies ago" is this step's form, fixed by its own `goal:`, and not
  a tunable with a starting value, so nothing is owed for it. A variant with
  other offsets is a separate verdict and a separate step.
- **Blend weights across the correction tables**, one named constant each --
  **(c), the range midpoint, stated as such**, which is what this step's
  `accepts:` already requires: constants in `src/search_params.hpp` with
  ranges, fitted at S127 and not taken from anywhere (DEC-084, as DEC-105
  and DEC-134 now read it). Each is a table's applied share expressed as a
  **percentage of the pawn table's**, range **0 to 200** -- 0 is that table
  switched off, 200 is twice the pawn table's share -- and the seed is the
  midpoint **100**, equal weight, so no table is presumed worth more than
  another before the fit says so. A percentage is the unit because a blend
  weight is a ratio between two corrections and not a quantity in the
  material scale. S110 states the same for the second table.
- **Weight cap, EMA scale, maximum applied correction, grain, entry clamp
  and entry count** -- **(b), the values S099's and S110's own fits return**,
  read at this step's HEAD. The table is the same parameterised struct
  instantiated a third time, so these are chesso's own fitted output and not
  a number from anywhere else; the entry count is the census S099 defines,
  re-run for the continuation key.
- **The correction pointer a null move installs** -- **not a constant.** The
  `accepts:` requires a null move to install a pointer rather than leave the
  child indexing garbage. Which pointer is a correctness choice with a test
  behind it, not a value with a range.

**Anti-seeds -- records, not seeds.** Stockfish pull request #5617 and
Tcheran's "Added 1-ply continuation correction history (8.23 +- 5.04)" are
records -- where the technique landed, and one engine's single measurement.
Under DEC-019 a record says which direction is worth trying and never what to
start from. The unsourced +1.8 to +4.6 is not even that.

seeds re-derived 2026-09-19 under DEC-105 (DEC-134)
