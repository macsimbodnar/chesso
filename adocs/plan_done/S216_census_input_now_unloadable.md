id:         S216
goal:       the S159 killer-slot census stops silently measuring the wrong position, and the instrument refuses to report a position the engine would not load
accepts:    `adocs/data/S159_census_run.py`'s reader treats a `position fen` refusal as a failure of that row -- the `info string refused [position fen]` line is detected and the row is reported as refused with a non-zero exit, rather than being scraped for `info ... nodes` and silently inheriting the previous board's numbers; the `promo-mess` row of `adocs/data/S159_census_positions.txt`, which holds the pre-S208 `KILLER_POS` and is now refused, is resolved one of three ways and the choice recorded as a decision -- replaced by the legal `KILLER_POS` (which re-derives the row and is the `either end moved` trigger of DEC-142), dropped from the set, or kept with the census re-run and the row reported as refused; `adocs/data/`'s append-only convention is respected, so a changed input or a changed reader lands as a new file beside the old with the old one named and dated rather than edited, unless a decision says otherwise; whether the census is re-derived at all is decided against what it is cited for -- DEC-160 reads it as the evidence that refuted the ageing reading of S149's 11 Elo before a game was played, and the step states whether one refused row of sixteen can move that reading; `adocs/data/README.md` gains a row for whatever file this lands
touches:    adocs/data/, adocs/decisions.md
excludes:   the S208 load bound and `KILLER_POS` itself, both decided at DEC-177; re-running any match; any `src/` change
decisions:  DEC-142, DEC-160, DEC-177
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185) for the reader and the row, 2026-09-11 evening while S219's match held the machine; a Sonnet 5 subagent (DEC-188) for the guard fix, the verification run and the gate, 2026-09-12; stamped by the coordinator
done:       2026-09-12 02:05. `adocs/data/S216_census_run.py` fails a refused `position fen` row loudly and exits 1, observed red first against S159's reader on HEAD at depth 2 (the refused row inherited the start position's 452 nodes and `d2d4`, illegal on its own board by python-chess) and byte-identical to S159's on legal input; `adocs/data/S216_census_positions.txt` replaces the pre-DEC-177 `promo-mess` row by the legal `KILLER_POS` as `promo-mess-s208`, DEC-186; the recorded census (`S159_census.md`, pinned `git archive 99000c1`) is deliberately not re-derived -- it was taken on a board that loaded, and DEC-186 carries the arithmetic showing one row cannot move DEC-160's reading. Tier-1 finding closed: the refusal guard keys on the refusal line, not the reason's truthiness, shown before (`42 nodes best e2e4`, exit 0) and after (`REFUSED`, exit 1) on a synthetic stub. Verification run over `S159_census_positions.txt` at HEAD: exactly one `REFUSED` row, exit 1, ten rows searched, 18021401 nodes, 2.75 s. Gate green in both builds (33/33, `clang-format.sh --check` silent). `DEV_MANUAL.md` and `MANUAL.md` checked, nothing names this reader; `README.md` human-owned, untouched. Reader and rows by an Opus 5 subagent, completion by a Sonnet 5 subagent (DEC-185, DEC-188)

## Why this exists

Found by the Tier-1 fast check over S208's completing commit `8aff8ac`, on
2026-09-11, and confirmed at the file.

DEC-177 made `KILLER_POS` legal by deleting its h3 pawn, because S208's load
boundary refuses more than 16 pieces of a colour and the old constant carried
17. `adocs/data/S159_census_positions.txt` row 16 -- `promo-mess` -- is the
**old** FEN, verbatim, and it is now refused.

What that does to the instrument is worse than an error.
`adocs/data/S159_census_run.py` writes `position fen <row>` then `go depth 12`
and scrapes only lines starting `info` and containing ` nodes `. A refusal is
an `info string` line with no ` nodes ` field, so it is skipped; the `go` then
searches **whatever board survived the refusal**, which is the row above --
`tactical` -- and its numbers are printed under the name `promo-mess`. The
census reports sixteen rows, fifteen positions, and says nothing about it.

`specs.md` and DEC-160 both read that census as the evidence that refuted the
ageing reading of S149's 11 Elo *before* a match was spent, so a
re-derivation that silently measures a different set is exactly the class
DEC-142 exists to stop: **either end moving is the trigger, and this end
moved.**

## Two things to settle, and the second is a judgement

**The reader.** A refusal must fail the row loudly. That is not a
re-derivation, it is the instrument refusing to lie, and it should land whether
or not the census is re-run. Any other row of any other census that stops
loading is caught by it from then on.

**The row.** Replacing it with the legal `KILLER_POS` keeps the set's shape --
it is there as the promotion-heavy position, and the legal constant keeps all
twelve promotions and the `f5e6` en-passant capture (DEC-177) -- but it changes
a recorded input, and `adocs/data/` is append-only by the convention
`adocs/data/S198_pairs.py` states in its own comment. So it is a new file
beside the old, or a decision that says otherwise. Dropping the row instead
shrinks a set that a recorded decision rests on.

**Whether the census is re-derived** is the judgement. One row of sixteen was
measuring a duplicate, which inflates the total and misnames one row; whether
that can move DEC-160's reading is a question about the margin in that reading
and not about this row, and the step answers it from the recorded numbers
rather than by re-running first.

## Cost

Documents and a small script change. An hour, plus a census re-run only if the
step decides one is owed -- which is minutes, not a match.

## Findings, 2026-09-11

Written by the subagent, while S219's match held all 12 threads: engine use was
two depth-2 readers and two four-command probes, under a minute of one core.

### The reader, and the defect observed red first

`adocs/data/S216_census_run.py` lands beside S159's, which is untouched. Both
readers were fed the same two rows at depth 2 against the Release binary at
HEAD -- row 1 the start position, row 2 the pre-DEC-177 constant the engine now
refuses. The old one:

```
  startpos       d2            452 nodes  best d2d4
  promo-mess     d2             83 nodes  best d2d4
  TOTAL                         535 nodes
```

`d2d4` is legal in the start position and **not legal** on the `promo-mess`
board -- checked with python-chess 1.11.2, not reasoned about -- so the second
row reports a move its own position cannot play. The board that survived the
refusal was read directly: `position fen` startpos, `position fen` the refused
row, then the engine's own `fen` command prints the **start position** back,
which is `game = previous` in `src/chesso.cpp` `set_position`. The new reader
on the same input:

```
  startpos       d2            452 nodes  best d2d4
  promo-mess     d2              -  REFUSED  rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1, more than 16 pieces of one colour (17 white, 14 black)
  TOTAL                         452 nodes
  1 of 2 rows refused: this census measures a different set than its input names
```

exit 1, and the file carries `promo-mess<TAB>2<TAB>REFUSED<TAB>-` plus a
`REFUSED_ROWS` trailer. The row is never searched: an `isready` between
`position fen` and `go` is the synchronisation point the refusal is read
against, and `command_isready` only replies, so it touches no search state. On
an all-legal three-row input the two readers' output files are **identical**,
which is the comparability the step asks for.

### The row: the three options priced, and none of them decided here

Recommended: **replace, as a new positions file**, under a name that is not
`promo-mess`. None of the three moves DEC-160 (below), so the question is only
what the next census can be run on.

- **Replace** by DEC-177's legal constant in `S216_census_positions.txt`, old
  file named and dated in its header. Keeps the set's shape and its eleventh
  row's purpose: python-chess reads the legal constant as valid, 16 white, and
  it keeps **12 promotions and the f5e6 en passant** against the old row's 42
  legal moves at 48. The row is a *different position*, so it must not keep the
  old name -- `promo-mess-s208` or `killer-pos` -- or a future table's
  `promo-mess` line will be read against the recorded 142852 as if it were the
  same board. That is the same silent-difference trap one level up.
- **Drop** it. Ten rows, strictly less overlap with the recorded set than
  replacing, and the only row carrying en passant goes; promotions survive in
  the `underpromo` row. Cheapest and worst for the next comparison.
- **Keep and report refused.** The input is untouched, but every future run on
  it is a red run by construction, so the recorded input becomes permanently
  un-runnable against any current build without a further decision.

### The census is not re-derived, and the arithmetic is not about the margin

**The recorded census is not contaminated: the row loaded when it was taken.**
`S159_census.md`'s reproduction pins `git archive 99000c1`, S159's own
completing commit of 2026-09-09, and the 16-a-side bound landed at `8aff8ac` on
2026-09-11 -- `grep` for its refusal text in `src/chesso.cpp` at 99000c1 finds
nothing. The output proves it independently of the source: the recorded
`promo-mess` row reads 142852 nodes and `g7h8q` against the row above it at
648113 and `d7c8q`, and `g7h8q` is **not legal** on that row's board
(python-chess). So the recipe still reproduces exactly, and the defect reaches
only a census pointed at a post-S208 binary.

The margin does not decide it, but it is stated anyway. The row is **0.79 % of
HEAD's 18166063 nodes** and 0.94 % of the guard build's 16016996. DEC-160 rests
on three ratios: the distinct-offer rate **45.4 % against 88.8 %**, a gap of
43.4 points, where deleting the whole row moves the HEAD figure to between
44.6 % and 46.0 % even if every node in it were an offer or none were; the
candidate-A headroom **0.89 %**, which cannot exceed 0.90 % with the row gone;
and the stale share **1.96 % against 1.91 %**. The stale share is the one a
single row could move -- its numerator is 90575, smaller than the row itself,
so the recorded totals alone bound it only to [0 %, 2.02 %] -- and that is
exactly why the provenance above is what answers the question and not the
arithmetic.

Re-running is also not available as a comparison: 41 commits of search change
separate 99000c1 from HEAD, and the instrumentation patch is written against
99000c1, so a census taken today is a new experiment and not a re-derivation.
DEC-142's "either end moved" is discharged by fixing the instrument.

### Two corrections to this file's own prose

The set is **eleven rows** and `promo-mess` is the sixth of them; "sixteen rows,
fifteen positions" above counts the line number, 16, in a file with five header
lines. The refusal is therefore one row of eleven. And S159's six evidence
files have no row in `adocs/data/README.md` -- the index gained a row for the
new reader only.

## Completion, 2026-09-12

**The guard fix.** The Tier-1 fast check over `f302b8c` found that the refusal
check tested `refusal` -- the reason text after the `REFUSAL` prefix -- for
truthiness, so a refusal whose line carried no reason (and no FEN) would read
as falsy and the row would fall through to `go depth N` unrefused: the same
silent-inheritance class this file exists to stop, unreachable at HEAD because
every refusal here names a reason, but latent. Fixed by a `was_refused` flag
set inside the `readyok` loop when the `REFUSAL` line is seen, checked instead
of `refusal`'s content -- two lines changed, nothing else. Demonstrated with a
synthetic one-row stub engine (scratchpad only, not committed) that answers
`isready` after `position fen` with exactly `info string refused [position
fen]`, no reason, no FEN. Before the fix: `stub-row d1 42 nodes best e2e4`,
exit 0 -- the row was silently searched and reported as clean. After the fix:
`stub-row d1 - REFUSED`, exit 1. A header comment was added noting the fix and
that the Findings' depth-2 demonstration above was produced at `f302b8c`,
before it, so that output stays attributable to the pre-fix reader.

**The verification run.** `python3 adocs/data/S216_census_run.py
build/src/chesso adocs/data/S159_census_positions.txt <out>`, HEAD, Release,
this machine (i7-8700K, the one `S159_census.md` was taken on). All eleven
rows attempted; exactly one `REFUSED` -- `promo-mess`, d12, the pre-DEC-177
constant, reason "more than 16 pieces of one colour (17 white, 14 black)"; the
other ten searched at their own depth: startpos d13 3292523 nodes (e2e4),
kiwipete d13 6638840 (e2a6), midgame d13 926398 (c3d5), tactical d13 648113
(d7c8q), cmk d13 2759923 (d8e7), lasker d18 12355 (a1b2), locked-pawns d22
39783 (f2f3), perpetual d16 2695927 (c3g3), 9bishops d14 283371 (g1h2),
underpromo d14 724168 (h7g8q). Total 18021401 nodes over the ten loaded rows.
Exit 1, matching `REFUSED_ROWS 1`. Wall time 2.75 s real (`time`), one core --
well inside the 15-minute budget, so no re-derivation question is raised by
cost. This is the optional verification DEC-186's Consequences promised, not a
re-derivation of the recorded census: `S159_census.md`'s numbers stay pinned to
`git archive 99000c1` plus the instrumentation patch, untouched.

**The gate.** Both builds, per `AGENTS.md`'s TESTS rule, `CLANG_FORMAT_MAJOR=22`
set (DEC-146, this machine only): `cmake --build build -j8` clean, `ctest
--test-dir build -L fast` 33/33 passed (82.70 s), `cmake --build build-tune -j8`
clean, `ctest --test-dir build-tune -L fast` 33/33 passed (83.48 s),
`./clang-format.sh --check` silent, exit 0. Combined chain exit 0. `src/` and
`tests/` are untouched by this step, as expected, so a red gate would have been
reported unfixed; it was green.

**Documents check.** `grep -n
"S159_census_run\|S216_census_run\|S159_census_positions\|S216_census_positions"
DEV_MANUAL.md MANUAL.md` matches nothing in either file (exit 1). The generic
word "census" appears several times in `DEV_MANUAL.md` -- S190's walk census,
S155's motif census, a mate-count census (`mate_trace`), and the PGN/match
census `rating.sh` and the fastchess wrapper read -- none of them this reader
or its positions file, checked by reading each hit's surrounding paragraph.
No change needed to either document: the reader's only documented row is
`adocs/data/README.md`'s, written under DEC-186, and this step changes nothing
in the documented contract -- same inputs, same outputs, the same
byte-identical-on-legal-input guarantee, the same `REFUSED`/exit-1 shape on a
refused row. It closes a case that was never reachable and never documented.

**Files touched.** `adocs/data/S216_census_run.py` (the guard fix and the
header note) and this step file (this section). `git status --short` shows
exactly those two paths modified and no new untracked file in the repository.
