id:         S111
goal:       correction tables indexed by the move played two and four plies ago
accepts:    an SPRT verdict, recorded whatever it is; the sentinel plies are valid at ply 0 through 3 and a test covers them; a null move installs a correction pointer rather than leaving the child to index garbage; the corrected score still cannot reach the decisive band; the blend weights across the correction tables are constants in src/search_params.hpp with ranges, fitted at S127 and not taken from anywhere (DEC-084)
touches:    src/search.cpp, src/data_structures.hpp, src/search_params.hpp -- amended by the implementer 2026-10-21 (As built, deviation 9): src/correction_history.hpp (the keys, the reads, the blend, the update and clear), src/search.hpp (the game probe), src/chesso.cpp and src/uci.hpp (ownership), tools/datagen.cpp (one pair per worker), tests/test_search.cpp, tests/test_engine.cpp, tests/test_search_params.cpp, tools/mutants/S111_continuation_correction.py, tools/mutants/S099_correction_history.py (CH09 re-anchored, after the rebase onto S110's revert), MANUAL.md, DEV_MANUAL.md, adocs/data/S111_sprt.sh, adocs/data/S111_blend_gain.py and its reading adocs/data/S111_blend_gain.txt, adocs/data/README.md; and in phase 2, the goldens: tests/test_mate_carry.cpp, tests/test_mate_breadth.cpp, tests/test_engine.cpp, adocs/data/S170_cases.tsv, adocs/data/S111_sweep_s170.txt, adocs/data/S111_floor_sweep.log, adocs/data/S111_mined_floor.log, adocs/data/S111_first_mate.txt
excludes:   the pawn and non-pawn tables, which are S099 and S110
decisions:  DEC-071, DEC-084, DEC-258, DEC-297, DEC-300, DEC-302, DEC-215, DEC-134, DEC-221, DEC-260
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-22 10:40 -- H0, never committed, recorded as zero. SPRT H0 at {0, 10} nElo, 32+0.32, Hash 64, noob_3moves.epd (DEC-297), played from the worktree's uncommitted tree by DEC-303: CAND `03fa3ec+f6cb46046bc0` vs REF `03fa3ec`, Elo -1.01 +/- 6.15, nElo -1.52 +/- 9.20, LLR -2.96, 5484 games, 0 forfeits (`adocs/data/S111_sprt.log`, record `7b6f951`); the played diff kept as the record (`adocs/data/S111_candidate.diff`, blob f6cb46046bc0, DEC-254), main's src/, tests/ and tools/ never moved (bench 6042933); `EXACT_FLOOR` stays 163, so DEC-303's owner ruling is not needed; no follow-up run (DEC-297). As the candidate stood: two continuation correction tables of the pawn table's kind, 2^18 slots a side, keyed on (2 plies ago, 1 ply ago) and (4, 1) (DEC-302), averaged with the pawn read at CorrHistCont2Weight and CorrHistCont4Weight 100 (DEC-300), applied correction 0.808 of the pawn read alone (decides nothing); bench 6042933 -> 7343608; the off switch prints the parent's stream; goldens by their scripts inside DEC-302's guard (ceilings unmoved, budgets D and F moved, mate in three 11, first-mate 9 and 9, S192 13446) except the mined set, 162 against 163. Fast suite 49/50 in both builds in the worktree, `EXACT_FLOOR` the one red; format clean; Debug self-play four rounds at 4+0.04, 8 games, 0 Assertion; gate_extra prose, citations, debug and perft ok, sanitize red on the same floor alone; CC01-25 and CH09 each observed red first under its mutant, the 41-mutant pass not run (it needs a green baseline and waited for the ruling the H0 made moot). Fast checks: phase 1's three findings and phase 2's (CC13's docstring, mutants CC24 and CC25 for the UCI layer's two other clears, and the floor DEC-303 keeps), all applied. MANUAL.md, DEV_MANUAL.md and specs.md checked: main's are untouched and correct for the tree that ships. Implemented from the Chess Programming Wiki's description and S099's table (DEC-221) by an Opus 5.5 subagent briefed by the coordinator.

## Conditional, 2026-10-08, DEC-258

In the main order directly after S099, and run only if S099 reads H1. On
S099's H0 or no verdict this step returns to the reserve unchanged.

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


## Pre-registration (DEC-297, the coordinator's, 2026-10-21), copied

DEC-297: "S111 is pre-registered the same way" as S110. CAND = S111's commit,
REF = its parent, `32+0.32`, `Hash=64`, `noob_3moves.epd`, `{0, 10}` nElo,
alpha = beta = 0.05, through `fastchess.sh` with `SPRT_BOUNDS`. Worst case
about 10500 games (DEC-143's 41861 at `{0, 5}` scaled by the bounds' width
squared), about 19-20 h at the 534-545 games an hour measured at this
control; cap 5250 rounds; abort at 2x that (40 h) or any forfeit rate above
1 %. H1: ships. H0 or no verdict: reverted whole, recorded as zero. Open
DEC-171 findings during the run: the coordinator's to name on the day it
pins. `adocs/data/S111_sprt.sh` carries it, in S110's shape, refusing to
start until the coordinator pins the pair and fills its bracketed sentences
(S110's reading among them). S110 read H0 and was reverted (`eee722c`), so
the parent is that tree, the pawn table alone (bench 6042933); the dated
section "rebased onto S110's revert" at the end is the rebase.

## As built, phase 1 (2026-10-21, Opus 5.5 subagent, blind)

**Written on S110's tree, and since rebased onto S110's revert**: the dated
section "2026-10-21: rebased onto S110's revert, and phase 1b" at the end
supersedes every statement in this section and the next about S110's tables,
the five-read blend and its quarter shares, the NC re-anchors, the S110-shaped
blend-gain counter, the goldens' S110 values and the phase list; the key,
the sentinels, the null move, the tables, their ownership and the tests'
design stand.

Worktree `/home/max/ws/chesso-s111`, branch `s111` at `36ec2eb` (S110's
`39194e4` and its SPRT pin), beside S110's SPRT on all 12 cores. **Nothing was
compiled, built, tested, timed or formatted** (DEC-260): reading, grep, git
and editing only. Every number below that a run would give is "to be
measured"; every CPU step is a script in `.tuning/coord/S111/p2/`, listed
under "Phase 1b and phase 2" with its time. The change is staged, not
committed, and saved as `.tuning/coord/S111/s111_phase1.diff`.

### Deviations and choices beyond the brief, first

1. **The key is a pair of moves: the move two (four) plies ago with the move
   one ply ago.** The Chess Programming Wiki's Static Evaluation Correction
   History page indexes continuation correction history by the "Move on the
   previous ply and the ply before it" -- two moves -- and gives a key on one
   move alone its own row, last-move correction. The step's goal fixes the
   earlier move at two and four plies. So the two-ply table's key is the
   page's pair exactly, and the four-ply table's is the same shape with the
   earlier move two plies further back; the move one ply ago, the move that
   made the position, is the second member of both. A table on the earlier
   move alone would be a last-move table at a distance, 385 slots a side,
   which the page does not describe. **Accepted by the coordinator's fast
   check as DEC-302**; had the goal been read
   as the earlier move alone, it would have been one line,
   `continuation_correction_keys`' return.
2. **A move is coded by its piece type and destination**, `1 + type * 64 +
   to`, 0 for no move: `MOVE_PIECE` and `MOVE_TO`, as `cont_hist` and
   `counter_moves` key a move; the mover of a promotion is its pawn. The
   colour is dropped because parity gives it -- the move one ply ago is the
   opponent's, the moves two and four plies ago the side to move's -- and the
   side to move is the table's first index already. 385 codes.
3. **A direct index, no hash and so no census.** A key is `earlier * 385 +
   last`, below 148225; `CORRHIST_CONTINUATION_ENTRIES` is 2^18, the smallest
   power of two above it, because S099's struct takes the key's low bits as
   the slot (form (b) is not claimed: it is the index's range, not a seed).
   No two move pairs share a slot. 1 MiB a side, 2 MiB a table, 4 MiB for the
   pair -- sixteen times the non-pawn pair; 113919 slots a side are never
   touched. **The step file's constants section asked for S099's census
   re-run for this key**; the brief's "a direct index needs no census"
   supersedes it, and the size is the arithmetic in `src/data_structures.hpp`
   `CORRHIST_CONTINUATION_ENTRIES`.
4. **The moves are read off the game's history, not a new search stack.**
   `make_move` and `make_null_move` already push one `history_entry_t` a
   ply, the search's plies included, each carrying its move (the null move's
   is 0), and `search_state_t::root_history_size` marks the root. So the move
   `k` plies before a node is `history.entries[size - k]`, present when
   `size - k >= root_history_size`: `src/correction_history.hpp`
   `continuation_move`. No recursion site -- the move loop, the null move,
   ProbCut, the singular verification -- has a slot to write or forget, and a
   state that never set the root (`SIZE_MAX`, every test driving `negamax`
   directly) reads no move.
5. **The sentinel is code 0, a real slot.** Before the root (plies 0 to 3
   for the four-ply key, 0 to 1 for the two-ply, 0 for the last move) and at
   a null move the code is 0, so the keys are always defined -- (0, 0) at
   ply 0, (0, last) at ply 1, (two, last) and (0, last) at plies 2 and 3 --
   and the sentinel slots learn like any other: a node near the root teaches
   a last-move-like slot. **The moves a game played before the root are
   never read**, although the history holds them: the search must not depend
   on whether its position came as a FEN or as a move list.
6. **The null move installs the sentinel by construction**:
   `make_null_move` writes move 0 into the entry it pushes, and
   `continuation_move_code` maps 0 to the sentinel before decoding anything
   -- decoded, move 0 is a white pawn arriving on a8, code 1, a real slot.
   That is the "correction pointer" of the accepts in this design: the
   entry's move is the pointer, and the test reads it through the key.
7. **The blend extends S110's average by its own terms, in one function.**
   `src/correction_history.hpp` `correction_blend` gains three parameters
   with defaults (`continuation = {}`, `two_weight = 0`, `four_weight = 0`), so
   S110's three-argument calls are unchanged and are the same function at
   S111's weights 0. S110's two lines (`numerator`, `denominator`) are kept as
   S110 wrote them; S111 adds `total = numerator + 2 * (two_weight * two +
   four_weight * four)` and `shares = denominator + 2 * (two_weight +
   four_weight)`, and the rounding moves onto those. The integer form:
   `n = 200 * pawn + w_np * (white + black) + 2 * (w2 * two + w4 * four)`,
   `d = 200 + 2 * w_np + 2 * (w2 + w4)`, result `sign(n) * ((|n| + d / 2) /
   d)`. At `w2 = w4 = 0` both added terms are 0, so `n` and `d` are S110's
   term for term and the integer is S110's to the unit. **S110's NC01 and
   NC14 anchored on the old return line and are re-anchored** on the new one,
   the mutations unchanged (`tools/mutants/S110_nonpawn_correction.py`'s
   header says so); NC02 to NC04 anchor on S110's two kept lines.
8. **The continuation tables are read only beside the non-pawn pair** (an
   assert in Debug at `src/search.cpp` `corrected_static_eval`), as the pair
   is read only beside the pawn table: S111's arguments ride on S110's call,
   which keeps NC12's anchor and S110's block intact. Every owner attaches all
   five tables. The board-only probe `search_corrected_eval_probe` reads the
   continuation tables at the root's keys; a new `search_corrected_eval_game_probe`
   reads them at the node's, through the same function `negamax_at` calls.
9. **`touches:` amended**, listing the files the change lands in beyond the
   three named.
10. **No new UCI option in the release build**; the tune build gains
    `CorrHistCont2Weight` and `CorrHistCont4Weight`, documented in MANUAL.md's
    table, which `test_uci_surface` requires. `test_search_params`' golden
    list goes 76 -> 78.
11. **No mirror case of S111's own.** S110 added one through its tables; a
    position's mirror is reached by a mirrored line, which a case would have
    to build move by move. The blend's sign symmetry is held directly ("the
    blend weighs each continuation read at its own weight") and the read's
    side to move by CC08's case; the existing mirror cases attach no
    continuation table and are unchanged.

### What was built

- **The tables.** `src/data_structures.hpp`: `CORRHIST_CONT_MOVE_CODES`
  (385), `CORRHIST_CONT_KEYS` (148225), `CORRHIST_CONTINUATION_ENTRIES`
  (2^18) with a `static_assert` between them, `continuation_table_t`
  (S099's `correction_table_t` again, form (b)), `continuation_correction_t`
  (`by_offset[0]` the two-ply table, `by_offset[1]` the four-ply), and
  `search_state_t::continuation_correction`, null by default.
- **Keys, read, update, clear.** `src/correction_history.hpp`:
  `continuation_move_code`, `continuation_move`, `continuation_keys_t`,
  `continuation_correction_keys`, `continuation_root_keys`,
  `continuation_reads_t`, `continuation_correction_values` (S099's
  `correction_value` per table, so the read's CorrHistMax clamp is S099's),
  `continuation_correction_update` (S099's `correction_update` per table) and
  a `correction_reset` overload; `correction_blend` extended (deviation 7).
- **Read and update.** `src/search.cpp` `corrected_static_eval` takes the
  keys and averages the two reads in with their weights before the mate
  band's clamp (CH01's line, unchanged); a three-argument overload reads the
  keys off the game, and `negamax_at` calls it. The update sits beside the
  pawn and non-pawn updates under `correction_may_learn` -- the same gate, a
  third call -- with `best_so_far - raw_eval` at the node's keys, recomputed
  from the history after every move is unmade.
- **Ownership.** `src/chesso.cpp`: a static pair beside the other tables,
  attached to every `go`'s state, cleared in `reset_for_new_game` (so
  `ucinewgame` and every `bench` position), `uci_init` and `test`;
  `uci_continuation_correction()` through `src/uci.hpp` for the tests.
  `tools/datagen.cpp`: one pair per worker on the heap, cleared per game,
  passed to `run_search`.
- **The parameters.** `src/search_params.hpp`: `CORRHIST_CONT2_WEIGHT`
  (`CorrHistCont2Weight`) and `CORRHIST_CONT4_WEIGHT` (`CorrHistCont4Weight`),
  each 100 in 0 to 200, with the tables' dimensions and clamps stated beside
  them (the accepts' wording).
- **The record, DEC-302 (3).** `src/search.cpp` `record_blend_gain`, tune
  build only, appends S111's ten fields to S110's counter line, and
  `adocs/data/S111_blend_gain.py` reads them (the dated section at the end).

Seeds (DEC-134 forms):

| constant | value | form |
|---|---|---|
| `CorrHistCont2Weight`, `CorrHistCont4Weight` | 100 each, range 0 to 200 | (c), the range midpoint, stated as such: 0 that table off, 200 twice the pawn read's weight; at the seeds the pawn read, the non-pawn pair's mean and each continuation read count a quarter each |
| `CORRHIST_CONTINUATION_ENTRIES` | 2^18 a side | none: the direct index's range, 385 * 385, rounded up to the power of two S099's slot needs (deviation 3) |
| `CORRHIST_CONT_MOVE_CODES` | 385 | none: the sentinel and six piece types on 64 squares, a definition |
| weight cap, scale, grain, entry clamp, applied maximum | S099's at this HEAD: 144, 256, 256, `CorrHistMax * 256`, 47 | (b), S099's own, the same functions |
| the two offsets | 2 and 4 plies | not a seed: the step's own form |
| the null move's pointer | code 0, the sentinel | not a constant: a correctness choice with a test (deviation 6) |

Implemented from the description (DEC-221): the Chess Programming Wiki's
page's table row for continuation correction history, fetched 2026-10-21
(its code blocks are other engines' source and were not used), the step's
goal for the offsets, and S099's own table. No engine's code, constant or
table; the step file's anti-seeds stayed records.

### Tests, each to be observed red first in phase 1b

New, none run (phase 1 compiled nothing):

- `tests/test_search.cpp`, new suite "search: continuation correction
  history":
  - "a move's continuation code is its piece type and destination": codes
    written out (37, 37 for the black pawn to the same square, 83, 57 for a
    promotion's pawn, 1 for a pawn onto a8, 384 the largest), 0 for move 0.
    Kills CC05, CC06.
  - "the continuation keys are defined at every ply from the root": four
    moves before the root, then plies 0 to 4 below it -- **the accepts'
    sentinel plies, each of 0, 1, 2, 3 and also 4** -- the keys written out
    from the moves played, the pre-root moves read as no move although the
    unguarded keys show them there (the precondition), and the search's own
    read at each ply landing on exactly those two slots. Kills CC01-CC05.
  - "a null move is read as no move by the continuation keys": a move, a
    null move, three moves; the null move one, two and four plies above a
    node reads as the sentinel, the moves around it keep their codes; the
    precondition that move 0 decodes to a real move. **The accepts' null-move
    test.** Kills CC05.
  - "at both continuation weights 0 the blend is S110's to the unit": the
    identity, against S110's integer written out, over a grid of reads and
    non-pawn weights, with four sets of continuation reads. Kills CC11.
  - "the blend weighs each continuation read at its own weight": written-out
    expectations at weights the build does not ship, the rounding, saturated
    reads averaging to themselves at every weight, and range and sign
    symmetry over a grid. Kills CC10-CC13.
  - "a corrected score never reaches the mate band with every continuation
    table saturated": **the accepts' mate-band case**, all five tables
    planted ten units past the entry bound first, then at the bound and at
    int32's extremes, raw scores past the band's edge; the opposite-sign
    plants show the continuation reads live. Kills CH01, CC09, CC12, and
    CC20 at the first plant, by a wrong number (57 against 47) and not by
    the signed overflow the extremes would put through an unclamped blend.
  - "each continuation table is read at its own key": decoys in every slot a
    wrong read lands in, the four keys distinct as a precondition. Kills
    CC07, CC08, CC09.
  - "the search teaches the continuation tables exactly where the gate
    does": S110's site case with the node one to four moves below the root,
    against the published update at each table's key; floors on a moved
    slot, a refusal out of check (`refused > refused_in_check`, S110's
    twin's floor: CC14 keeps the in-check sentinel's refusal) and distinct
    keys. Kills CC14-CC17, CC22 (and CC21 through
    its read check).
  - "the continuation tables hide no forced mate": the three mate-in-two
    rows at depths 3 to 6, cold and with every slot of every table at
    +-the entry bound.
  - tune build only, "at both continuation weights 0 the continuation
    tables add nothing": saturated continuation tables, S110's three-argument
    blend as the expectation, one weight at 0 not enough. DEC-215 on the
    read; the whole-tree half is bench.sh's.
- `tests/test_engine.cpp` "ucinewgame clears the continuation correction
  tables": a `go depth 8` teaches both, `ucinewgame` empties both. Kills
  CC18, CC19.
- `tests/test_search_params.cpp`: the two golden rows; the range case holds
  both weights' ranges and the blend's int32 headroom with S111's terms
  (131600 at the range tops).

**Red first** is phase 1b's `red_first.sh`: each CC mutant, and S110's
re-anchored NC01 and NC14, applied alone to a scratch fixture, the S111 suite
(and S110's, and the `ucinewgame` case) run, the red cases recorded, the
unmutated baseline green last. The table goes here when it has run; every
mutant must be red where its case's `Mutation:` line names it.

### Where this is least sure to compile or be right

- **Formatting, and the anchors that depend on it.** No clang-format ran.
  The lines most likely to be reflowed: `correction_blend`'s `total`
  expression and one-line return, the extended call in
  `corrected_static_eval` (written one argument a line), the update call at
  the store, `continuation_correction_update`'s two calls, and the tests'
  long expressions (the tests carry no anchor). After `./clang-format.sh`
  rewrites anything in `src/`, `anchors.sh` says which of CC01-CC22, NC01 and
  NC14 lost its anchor; each is a one-line edit of the anchor string.
- **The tests' preconditions on the generator's first moves**: the read case
  skips a position whose four keys collide and needs one of two not to; the
  site case's floors (a moved slot, a refusal out of check, distinct keys)
  are the engine's answer. Not chess judgement -- whichever moves the
  generator lists first -- but not run.
- **Narrowing and sign warnings under -Werror**: `hash_t{size_t}` in the
  null-move case (unsigned to unsigned of the same width, not narrowing on
  either platform), `MOVE_TO(0) == a8` (both promote to int).
- **The average's applied scale.** At the seeds each read counts a quarter,
  so the pawn read's share falls from a half to a quarter, and if the
  continuation reads are small (cold or slow to learn) the applied
  correction shrinks, the opposite of what S110 measured for the pair
  (R_avg 0.787). Not acted on: DEC-300 fixed the form and S127 owns the
  gain. **Measured, as a record, by DEC-302 (3)**: S110's tune-only
  counter sees the continuation reads now, and phase 1b's
  `adocs/data/S111_blend_gain.py` reads S111's applied correction against
  S110's and the pawn read's on the same games, and how often each
  continuation slot read is cold. The figure is recorded beside the verdict
  and decides nothing.

### Goldens: phase 2's, by their scripts (DEC-142)

The UCI path's tree moves, so every golden S110 re-derived is re-derived
again, never read off a run. None was edited in phase 1. **The guard is
DEC-302's: DEC-299's guard word for word** --
apply what `S203_case_sweep.sh --ceilings` answers provided EXACT_FLOOR and
MATE_IN_THREE_FLOOR hold or rise and no ceiling rises by more than 2.
Outside it, the values the scripts answer are carried provisionally, as
DEC-301 carried S110's case E, each moved site citing the decision, and
**the owner rules before an H1 ships**; an H0 reverts them with the commit.

| golden | now | script |
|---|---|---|
| `tests/test_mate_carry.cpp` `short_line_ceiling` and `adocs/data/S170_cases.tsv` budgets | S110's 2, 5, 4, 1, 4, 2 and C 500000, D 1500000, E 1000000, F 100000; expected to move | `p2/sweep.sh` into `adocs/data/S111_sweep_s170.txt`, then `adocs/data/S203_case_sweep.sh --ceilings` over S202's, S261's, S286's, S110's and S111's grids, budgets by `.tuning/coord/S099/p2/pick_budgets.sh` |
| `tests/test_mate_breadth.cpp` `EXACT_FLOOR` | 163 | `p2/floors.sh` (`S156_mined_floor_sweep.py --floor 163`) |
| `tests/test_engine.cpp` `MATE_IN_THREE_FLOOR` | 11 | `p2/floors.sh` (`S154_floor_margin_sweep.py floor`, `red`) |
| `tests/test_engine.cpp` `first_mate_depth`, `mate_in` | 9, 9; 5 | `p2/floors.sh` (`S188_repair_goldens.py first-mate`) |
| `tests/test_search.cpp` "ordering keeps the tree small" | 13446, unmoved by construction (no table attached) | `S192_node_budget.py`, in `p2/floors.sh` |
| `tests/test_search.cpp` mined rows | unmoved by construction: no case attaches a table | their mutants' kills in a pass would confirm |
| `tests/test_search_params.cpp` `golden_defaults` | 78, written here | a diff of `src/search_params.hpp` is the derivation |
| bench | to be measured (S110's 6570375 the parent's) | `chesso bench`, the commit's `Bench:` |

### If S110 reverts

On S110's H0 or no verdict S110 is reverted whole and S111 rebases onto the
revert -- the pawn table alone. These hunks change; everything else applies
as it stands:

- `src/correction_history.hpp` `correction_blend`: S110's function and
  `nonpawn_reads_t` leave with S110, so the blend becomes S111's alone,
  `correction_blend(int pawn, continuation_reads_t continuation, int
  two_weight, int four_weight)`, with S110's two lines replaced by
  `numerator = 200 * pawn` and `denominator = 200` -- the rest of the body,
  the rounding included, unchanged. The comment loses S110's paragraph.
  `continuation_reads_t` stays where it is.
- `src/search.cpp` `corrected_static_eval`: S110's non-pawn block leaves;
  S111's block becomes `if (state->continuation_correction != nullptr) {
  correction = correction_blend(correction, continuation_correction_values(
  ...), CORRHIST_CONT2_WEIGHT, CORRHIST_CONT4_WEIGHT); }`, and the Debug
  assert reads "continuation only beside the pawn table". The tune-only
  census lines leave with S110, and so do the blend-gain counter's S110
  fields; its S111 fields then compare S111's applied correction with the
  pawn read alone (S110's blend has no reads to take), and
  `adocs/data/S111_blend_gain.py` reads the shorter line. The update block
  stays, after the pawn table's.
- `src/data_structures.hpp`: the continuation section follows the pawn
  table, and `search_state_t::continuation_correction` follows
  `pawn_correction`; the comments say "beside the pawn table".
- `src/chesso.cpp`, `src/uci.hpp`, `tools/datagen.cpp`: S111's lines sit next
  to the pawn table's instead of the non-pawn pair's; `run_search` takes
  `(game, tt, correction, continuation_correction, nodes, max_depth)`.
- `src/search_params.hpp`: the two rows follow `CORRHIST_WEIGHT_CAP`; the
  comment loses the non-pawn pair, and the seeds' shares are a third each.
- `tests/test_search.cpp`: the fixture attaches the pawn and continuation
  tables; the written-out expectations lose `2 * CORRHIST_NONPAWN_WEIGHT`
  from the denominator (the seeds' 15 becomes 20, the site case's 5 becomes
  7 -- re-derive, do not read off a run); the blend cases call the
  four-argument blend and their written-out values are re-derived (the
  quarters become thirds); the identity case compares against the pawn read
  itself, `blend(pawn, any, 0, 0) == pawn`; `cc_fill_all` loses the non-pawn
  loop; the tune case's expectation is `raw + 5`.
- `tests/test_search_params.cpp`: the rows follow `CorrHistWeightCap`, the
  count is 75 -> 77, the headroom check loses the non-pawn term.
- MANUAL.md and DEV_MANUAL.md: the rows and paragraphs lose S110's tables;
  `golden_defaults` reads 77.
- Mutants: S110's file leaves with S110 (its re-anchors with it); CC10-CC13
  anchor on the blend's own lines and survive; CC18's anchor becomes
  `proven_mate_line.length = 0;` followed by S111's reset; `anchors.sh`
  confirms the rest.

### specs.md wording, proposed for the coordinator (not edited here)

- The search row's correction paragraph, after S110's proposed sentence:
  "S111 adds two tables of the same kind keyed on moves: the move two plies
  ago with the move one ply ago, and the move four plies ago with the move
  one ply ago, each move by its piece type and destination, read off the
  game's history against the search's root -- a ply before the root or a
  null move is no move, a defined slot -- directly indexed, 2^18 slots a
  side; taught at the same nodes under the same gate and joined to the same
  weighted average at `CorrHistCont2Weight` and `CorrHistCont4Weight` (100
  each), so the correction stays inside CorrHistMax and then inside the mate
  band."
- The "absent, search" row: the continuation correction tables leave it.
- INV-5's row, if the coordinator wants it: the blend's sign symmetry with
  the continuation reads is held by `tests/test_search.cpp` "the blend
  weighs each continuation read at its own weight"; no mirror case of S111's
  own (deviation 11).

MANUAL.md and DEV_MANUAL.md checked and changed: MANUAL's two option rows,
the `CorrHistNonPawnWeight` row's shares sentence, the `RfpTtEstimate` and
`RazorMargin` rows' "static evaluation", the `ucinewgame` reset paragraph;
DEV_MANUAL's datagen paragraph and the `golden_defaults` row. The other
golden rows move in phase 2 with their readings. `adocs/data/README.md`
gains S111's section.

### Phase 1b and phase 2, in order, with machine time

Scripts in `.tuning/coord/S111/p2/`, adapted from S110's; each prints its own
DONE or FAILED marker last. Times are S110's recorded wall times at `-j12`
unless marked. **On S110's H0, rebase first** ("If S110 reverts"), then run
the same list with `REFBIN` and `EXPECT_REF` set for `bench.sh`.

Phase 1b (gets the blind code green, and DEC-302 (3)'s record; about 1 h):

1. `build.sh` -- configure and build Release, tune and Debug in the s111
   worktree: about 6 min cold (*est.*). Fix any compile error here.
2. `gate.sh gate1` -- both builds' fast suite and the format check: 4 min.
   Expect red only `test_mate_carry` (its budgets move with the tree) and
   possibly the format check; on a format failure run `./clang-format.sh`
   and go on.
3. `anchors.sh` -- every mutant anchor exactly once: seconds. Re-anchor any
   it names (CC01-CC22, NC01, NC14).
4. `bench.sh` -- bench and bench 12 on the parent's frozen binary and the
   candidate, **the off switch**: the tune build at `CorrHistCont2Weight` 0
   and `CorrHistCont4Weight` 0 must print the parent's bench stream (0 lines
   differing) and search_bench at 9 and 12 must read the parent's nodes and
   best moves; the tune build at its defaults must print the Release stream:
   5 min.
5. `red_first.sh` -- each new case red under its mutant, the baseline green:
   about 25 min (*est.*, 24 mutants at about a minute).
6. `blend_gain.sh` -- DEC-302 (3)'s record: the tune build's `bench` with
   `CHESSO_BLEND_GAIN` set must print the Release candidate's total (the
   counter only reads), then `adocs/data/S111_blend_gain.py` on the
   candidate's tune build into `adocs/data/S111_blend_gain.txt`: about 6
   min (S110's took about 6), the twelve cores. Recorded beside the verdict;
   decides nothing.

Phase 2 (the goldens and the second tier; about 2.5 h):

7. `fixture.sh fixA "S111 phase 2 fixture A"` -- the tree as it stands:
   seconds.
8. `sweep.sh` -- the S170 grid: 5 min. Then `adocs/data/S203_case_sweep.sh
   --ceilings` over S202's, S261's, S286's, S110's and S111's grids, and
   `.tuning/coord/S099/p2/pick_budgets.sh adocs/data/S170_cases.tsv
   adocs/data/S111_sweep_s170.txt` for the budgets, under DEC-302's guard
   (DEC-299's word for word; outside it carried provisionally, the owner
   ruling before an H1 ships).
9. `floors.sh 163` -- the two floors, first-mate, S192: 15 min.
10. Edit the moved goldens with their readings and citations, then
   `fixture.sh fixB "S111 phase 2 fixture B"` and `gate.sh gate2`: 5 min.
11. `mutants.sh` -- CC01-CC22, NC01-NC14, CH01-CH15 and G04 on fixture B: 52
    mutants at S286's 118 s each, about 100 min.
12. `selfplay.sh` -- Debug self-play, four rounds at 4+0.04, grep for
    `Assertion`: 10 min.
13. `gate_extra.sh` -- `tools/gate_extra.sh`, five stages: 17 min.
14. `tools/plan_prose_check.py --citations`, `--touches`, `--params`,
    `--gate`: seconds.
15. `tools/gate.sh --message <the commit text>` on the staged tree, last.
16. Commit; pin CAND and REF in `adocs/data/S111_sprt.sh`, fill its
    bracketed sentences, check the A/A rule (DEC-143) and the Open list;
    then `nohup adocs/data/S111_sprt.sh > .tuning/s111_sprt.log 2>&1 &`, a
    night (DEC-155), cap 5250 rounds, **19.7 h at 534 games an hour**,
    watcher ceiling 144000 s.

`chain.sh` runs steps 10 (fixture B and gate2) to 13 detached, as S110's did.

### Proposed `done:` stamp

`.tuning/coord/S111/done_stamp_s111.txt`.

### Commit text

`.tuning/coord/S111/commit_s111.txt`.

## 2026-10-21: the coordinator's fast check (DEC-302)

The coordinator accepted the pair key (deviation 1) as **DEC-302**, which also gives S111's goldens DEC-299's guard word for
word, outside it carried provisionally as DEC-301 did with the owner ruling
before an H1 ships (the goldens section above says so). Four changes, made
the same day under the same machine limits (still blind, DEC-260):

1. `tests/test_search.cpp` "the search teaches the continuation tables
   exactly where the gate does" counts in-check refusals apart and floors
   `refused > refused_in_check`, S110's twin case's floor: CC14's ungated
   update keeps the in-check sentinel's refusal, so only a refusal out of
   check can show it.
2. `tests/test_search.cpp` "a corrected score never reaches the mate band
   with every continuation table saturated" plants `(CorrHistMax + 10) *
   CORRHIST_GRAIN` first, both signs: every read clamps it to 47, an
   unclamped continuation read applies 57, so CC20 dies on a wrong number
   before any int32 extreme is planted, with no signed overflow in the
   mutant. CC20's note in `tools/mutants/S111_continuation_correction.py`
   names that plant.
3. `src/data_structures.hpp`: the correction table's comment no longer says
   the non-pawn and continuation tables are held in reserve and not built
   (stale since S110); it names S110's pair and S111's pair as built.
4. DEC-302 (3): S110's tune-only counter (`src/search.cpp`
   `record_blend_gain`, `CHESSO_BLEND_GAIN`) takes the node's keys and
   appends ten fields after S110's nine -- the continuation reads, S110's and
   S111's applied corrections and their difference, each continuation read's
   size, and each table's cold reads and entries -- so
   `adocs/data/S110_blend_gain.py` reads its line as before.
   `adocs/data/S111_blend_gain.py`, in S110's shape, plays S110's twelve
   games on the candidate's tune build and prints S111's applied correction
   against S110's and the pawn read's, and how often each continuation slot
   read is cold. **A record beside the verdict; it decides nothing.** Phase
   1b's step 6 runs it.

## 2026-10-21: rebased onto S110's revert, and phase 1b

S110 read H0 (nElo -4.88 +/- 11.28 over 3644 games, record `99a33a4`) and
`eee722c` reverted `39194e4` whole: src/, tests/, tools/, the manuals, specs
and `S170_cases.tsv` are `bb3d31d`'s again, bench 6042933. `03fa3ec` landed
DEC-302, adjusted for the revert. S111 is rebased onto `03fa3ec` in a fresh
worktree, `/home/max/ws/chesso-s111r`, branch `s111r`; `../chesso-s111`
stays untouched as the reference. The phase-1 diff was applied with `git
apply --3way` (S110's mutant file excluded: it left with S110), and every
S110-dependent hunk rewritten as "If S110 reverts" said. The machine was
free from here: builds, tests and node-count work at `-j12`,
`CLANG_FORMAT_MAJOR=22`.

### What the rebase changed

- **The blend** (`src/correction_history.hpp` `correction_blend`) is the pawn
  read and the two continuation reads alone, `(pawn, continuation,
  two_weight, four_weight)`: S110's two lines became `numerator = 200 * pawn`
  and `denominator = 200`, the rest -- the continuation terms, the doubled
  unit, the rounding -- unchanged. The integer form: n = 200 * pawn + 2 * (w2
  * two + w4 * four), d = 200 + 2 * (w2 + w4), sign(n) * ((|n| + d / 2) / d).
  **At w2 = w4 = 0 it is the pawn read exactly** (|200 p| + 100 over 200
  truncates to |p|), the pawn-only parent, held by `tests/test_search.cpp`
  "at both continuation weights 0 the blend is the pawn read" over every pawn
  read in +-2 CorrHistMax against five sets of continuation reads. At the
  seeds the pawn read and each continuation read count a third. The doubled
  unit changes no integer against the undoubled form (rounding half away
  from zero is the same rule at 2n / 2d), and is kept as the revert plan said.
- **The read**: the continuation reads sit beside the pawn read in
  `src/search.cpp` `corrected_static_eval` -- the pawn read is `int
  correction`, and where the continuation tables are attached the blend is
  assigned to it before the mate band's clamp (CH01's line, unchanged); a
  Debug assert holds them to the pawn table's company. S099's CH09 is
  re-anchored on the reflowed pawn read, as S110 once did; the mutation is
  unchanged. The update follows the pawn table's.
- **Ownership, datagen, parameters**: S111's lines sit beside the pawn
  table's; `run_search(game, tt, correction, continuation_correction, nodes,
  max_depth)`; the two X rows follow `CORRHIST_WEIGHT_CAP`, their comment
  saying a third each. `test_search_params` 75 -> 77 and the headroom check
  is the three-read numerator's, 94000 at the range tops.
- **Tests**: the suite's fixture attaches the pawn and continuation tables;
  every written-out expectation re-derived for the three-read average (the
  seeds' read case 20, the site case's corrected score 7, the blend cases'
  thirds); the mate-band case writes the rounding out for the opposite-sign
  plants instead of requiring divisibility. Nothing else in the design moved.
- **Mutants**: CC09 is now the blend computed and dropped (`(void)` on the
  call); CC18 anchors on the reset after `proven_mate_line.length = 0;`;
  **CC23 is new**, the blend's rounding not symmetric in sign (S110's NC14
  was that mutant and left with S110); NC01 and NC14's re-anchors left with
  S110's file. 240 mutants in `tools/mutants/`, every anchor exactly once
  (`anchors.sh`).
- **The blend-gain counter**: S110's left with its revert, so S111 gives
  `src/search.cpp` `record_blend_gain` its own twelve fields (tune build only,
  read-only) and `adocs/data/S111_blend_gain.py` reads R = |applied| / |p|
  against the pawn read alone, and each table's cold share (DEC-302 (3)).
- **Docs**: MANUAL's two rows (a third each), the `RfpTtEstimate` and
  `RazorMargin` rows and the `ucinewgame` paragraph; DEV_MANUAL's
  `golden_defaults` row (77) and datagen paragraph; the data README;
  `adocs/data/S111_sprt.sh`'s pair, readings (S110's H0 quoted) and the
  measured bench.
- **clang-format** (`./clang-format.sh`, major 22) reflowed two comment
  paragraphs and two blank lines in src/ and seven expressions in the tests;
  no code line in src/ moved, so no anchor did.

### Readings, phase 1b (2026-10-21, the idle machine, `-j12`)

Scripts in `.tuning/coord/S111/p2/`, updated for the rebased base; outputs
beside them.

- **Build** (`build.sh`): Release, tune and Debug configured fresh and built
  in 128 s, no warning and no error -- the blind code compiled as written.
- **The gate** (`gate.sh gate1`, `CLANG_FORMAT_MAJOR=22`): Release **49 of
  50**, tune **49 of 50**, red only `test_mate_breadth` in both, a golden
  (below); `test_mate_carry` green. The format check was red on two comment
  paragraphs and blank lines in src/ and seven test expressions;
  `./clang-format.sh` fixed them and the check is clean.
- **Anchors** (`anchors.sh`): 240 mutants, every anchor exactly once, after
  CH09's re-anchor.
- **Bench** (`bench.sh`; the parent is `.tuning/coord/S110/p2/bin/ref`,
  `bb3d31d`'s Release build, sha256 `b1c83f0b...`, whose src/ `eee722c`
  restores, checked at 6042933): `bench` **6042933 -> 7343608**, replies
  `c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6`, the second moved (the
  parent's `d5e6`); `bench 12` **2339074 -> 2460543**, replies `c3d5 d5e6
  d7c8q g7h8q a7a5 a1b2 e5e6 e5e6`, the fifth moved (the parent's `a7a6`).
  `tools/search_bench.py` depth 9 **44575 / 127190 / 52594**, best `c3d5
  e2a6 d7c8q` (the parent's moves); depth 12 **253407 / 550494 / 246634**,
  best `c3d5 d5e6 d7c8q` (the parent's). Which reply is better is no
  judgement made here.
- **The off switch (DEC-215)**: the tune build at `CorrHistCont2Weight` 0 and
  `CorrHistCont4Weight` 0 prints the parent's `bench` stream, 6042933, 121
  lines, **0 differing** but time and nps; search_bench there reads the
  parent's **62042 / 125888 / 34107** and **214973 / 595000 / 217107** with
  its best moves. One weight at 0 is not off: `CorrHistCont2Weight` 0 alone
  benches 7513144. The tune build at its defaults prints the Release
  stream, 0 lines differing.
- **Red first** (`red_first.sh`, `red_first.tsv`): each mutant applied alone
  to a scratch fixture of this tree, the S111 suite (S099's for CH09, the
  `ucinewgame` case for CC18 and CC19) run. **All 24 built and every one is
  red in the case its `Mutation:` line names; the unmutated baseline is
  green**:

| mutant | red under it |
|---|---|
| CC01, CC02, CC03 | keys at every ply, null move |
| CC04 | keys at every ply |
| CC05 | code, keys at every ply, null move |
| CC06 | code |
| CC07, CC08, CC21 | keys at every ply (its read), own key, site against gate |
| CC09 | keys at every ply, mate band saturated, own key, site against gate |
| CC10 | blend |
| CC11 | identity, blend |
| CC12, CC13 | keys at every ply, blend, mate band saturated, own key |
| CC14, CC15, CC16, CC17, CC22 | site against gate |
| CC18, CC19 | `ucinewgame` clears (tests/test_engine.cpp) |
| CC20 | mate band saturated (its first plant, no overflow) |
| CC23 | identity, blend, mate band saturated |
| CH09 (re-anchored) | S099's four pawn-table cases |

  The short names: code, keys at every ply, null move, identity, blend, mate
  band saturated, own key and site against gate are the suite's "a move's
  continuation code is its piece type and destination", "the continuation
  keys are defined at every ply from the root", "a null move is read as no
  move by the continuation keys", "at both continuation weights 0 the blend
  is the pawn read", "the blend weighs each continuation read at its own
  weight", "a corrected score never reaches the mate band with every
  continuation table saturated", "each continuation table is read at its own
  key" and "the search teaches the continuation tables exactly where the gate
  does". The site case's census on this tree: 2276 stores, 646 taught a moved
  slot, 1627 refused (143 in check), 1701 with the two keys distinct. "The
  continuation tables hide no forced mate" and the tune-only weights-0 case
  are green (the latter in the tune build, 10 of 10 there).

### Goldens red, not edited

- **`tests/test_mate_breadth.cpp` `EXACT_FLOOR`**: the mined set reads **162
  exact** at depth 10 (167 right sign, 0 wrong) against the floor 163.
  DEC-302's guard -- DEC-299's word for word -- requires both mate floors to
  hold or rise, so a lowered floor is **outside the guard**: phase 2's
  `floors.sh` re-measures it by `S156_mined_floor_sweep.py` (shipping and
  weakened, the separation), and the owner rules before an H1 ships, the
  value carried provisionally as DEC-301 did only if the coordinator so
  decides. Nothing in the test was touched.
- Every other golden is green on this tree: `test_mate_carry` (the S170
  budgets and ceilings as the revert left them), `MATE_IN_THREE_FLOOR`, the
  first-mate depths, S192's count, the mined rows. Phase 2's sweep and
  floors still re-derive them by their scripts (DEC-142).

### The blend-gain record, DEC-302 (3) -- decides nothing

`blend_gain.sh`: the tune build benches 7343608 with the counter on, the
Release candidate's total, so the counter only reads. Then
`adocs/data/S111_blend_gain.py` over S110's twelve games (201 s;
`adocs/data/S111_blend_gain.txt`), 1335434217 reads:

| reads | mean abs p | R = abs applied / abs p | abs (applied - p) / abs p | abs two / abs p | abs four / abs p | pawn 0 (entry) | two-ply 0 (entry) | four-ply 0 (entry) |
|---|---|---|---|---|---|---|---|---|
| all | 19.04 | **0.808** | 0.516 | 0.943 | 0.914 | 0.031 (0.001) | **0.085 (0.040)** | **0.075 (0.029)** |
| plies 1-40 | 14.55 | 0.754 | 0.643 | 0.909 | 0.874 | 0.046 (0.004) | 0.152 (0.084) | 0.130 (0.061) |
| plies 41-80 | 17.23 | 0.796 | 0.590 | 0.984 | 0.935 | 0.030 (0.000) | 0.064 (0.021) | 0.058 (0.015) |
| plies 81-120 | 25.87 | 0.851 | 0.385 | 0.932 | 0.923 | 0.016 (0.000) | 0.036 (0.011) | 0.034 (0.009) |

S111 applies **0.81 of the pawn read's scale**, close to S110's R_avg
(0.787), but not because its reads are small: each continuation read is
about as large as the pawn read (0.94 and 0.91 of it), and the average
shrinks where the three disagree in sign -- the applied correction moves
about half the pawn read's size away from it. Cold slots are rarer than the
phase-1 worry: 8.5 % of two-ply reads and 7.5 % of four-ply reads are 0
(4.0 % and 2.9 % of entries), most of them in the first third. Recorded
beside the verdict; DEC-302 reads an H0 as the conditioning and this scale
together, and S127 owns the gain.

### Phase 2 on the rebased tree, in order, with machine time

The scripts in `.tuning/coord/S111/p2/` point at `/home/max/ws/chesso-s111r`;
times are S286's and S110's recorded wall times at `-j12`. This list
supersedes "Phase 1b and phase 2" above.

1. `fixture.sh fixA "S111 phase 2 fixture A"`: seconds.
2. `sweep.sh`: the S170 grid into `S111_sweep_s170.txt`, 5 min. Then
   `adocs/data/S203_case_sweep.sh --ceilings adocs/data/S202_sweep_s170.txt
   adocs/data/S261_sweep_s170.txt adocs/data/S286_sweep_s170.txt
   adocs/data/S111_sweep_s170.txt` (the revert's command, S111's grid
   appended) and `.tuning/coord/S099/p2/pick_budgets.sh
   adocs/data/S170_cases.tsv adocs/data/S111_sweep_s170.txt`, under DEC-302's
   guard.
3. `floors.sh 163`: the two floors, first-mate, S192, 15 min. `EXACT_FLOOR`
   is red at 162 already (above): outside the guard, the owner's.
4. Edit the moved goldens with their readings and citations (DEC-142), then
   `fixture.sh fixB "S111 phase 2 fixture B"` and `gate.sh gate2`: 5 min.
5. `mutants.sh`: CC01-CC23, CH01-CH15 and G04 on fixture B, 39 mutants at
   118 s each, about 77 min.
6. `selfplay.sh`: Debug self-play, four rounds at 4+0.04, grep for
   `Assertion`: 10 min.
7. `gate_extra.sh`: five stages, 17 min.
8. `tools/plan_prose_check.py --citations`, `--touches`, `--params`,
   `--gate`: seconds.
9. `tools/gate.sh --message <commit_s111.txt>` on the staged tree.
10. Commit; pin CAND and REF in `adocs/data/S111_sprt.sh` and fill its
    bracketed sentence; then `nohup adocs/data/S111_sprt.sh >
    .tuning/s111_sprt.log 2>&1 &`, cap 5250 rounds, 19.7 h at 534 games an
    hour, watcher ceiling 144000 s.

`chain.sh` runs 4 (fixture B, gate2) to 7 detached. About 2.2 h of machine
before the SPRT.

### specs.md wording, proposed for the coordinator (supersedes the one above)

- The search row's correction paragraph: "S111 adds two tables of the pawn
  table's kind keyed on moves: the move two plies ago with the move one ply
  ago, and the move four plies ago with the move one ply ago (DEC-302), each
  move by its piece type and destination, read off the game's history
  against the search's root -- a ply before the root or a null move is no
  move, a defined slot -- directly indexed, 2^18 slots a side; taught at the
  same nodes under the same gate. The static evaluation's correction is a
  weighted average (DEC-300's form): the pawn read at 100 and each
  continuation read at `CorrHistCont2Weight` and `CorrHistCont4Weight` (100
  each, a third each), rounded half away from zero, inside CorrHistMax and
  then inside the mate band. The update and the table field read the raw
  evaluation, as before."
- The "absent, search" row: "the non-pawn and continuation correction
  tables" becomes "the non-pawn correction tables (S110, reverted on its
  H0)".
- INV-5's row: the blend's sign symmetry with the continuation reads is held
  by `tests/test_search.cpp` "the blend weighs each continuation read at its
  own weight"; no mirror case of S111's own (deviation 11).

## 2026-10-21: phase 2's goldens, and DEC-303

The coordinator ran phase 2's fixture, sweep and floors on the rebased tree
(`.tuning/coord/S111/p2/`): fixture A `4529b79`; `sweep.sh` 108 rows; the
floors by `floors.sh 163`. **DEC-303** (the coordinator's): `EXACT_FLOOR`
stays 163 in the tree -- lowering a mate floor weakens a mate-safety test,
which the owner's rules forbid without the owner, and DEC-299 calls a lowered
floor a regression signal -- so **S111 is not committed before the owner
rules**, and its SPRT plays the working tree instead (fastchess.sh's dirty-tree
mode, S256).

### The goldens inside DEC-302's guard, by their scripts (DEC-142)

| golden | before | now | script, and the reading's file |
|---|---|---|---|
| `tests/test_mate_carry.cpp` `short_line_ceiling` | 2, 5, 4, 0, 1, 2 | **unchanged** | `adocs/data/S203_case_sweep.sh --ceilings` over S202's, S261's, S286's and S111's grids answers 2, 5, 4, 0, 1, 2 (`adocs/data/S111_sweep_s170.txt`, 108 cells, 1421 mate lines and 15 short; alone 2, 2, 0, 0, 1, 2); the re-derive command and the grid table gain S111's grid |
| `adocs/data/S170_cases.tsv` budgets | D 500000, F 300000 | **D 1500000, F 100000**; A 1000000, B 100000, C 1200000, E 300000 unmoved | the same grid, the rule applied by `.tuning/coord/S099/p2/pick_budgets.sh` (`p2/picks.txt`): D 1 line, F 1 at its stride 2, none short; an S111 paragraph in the file's note, S286's form |
| `tests/test_engine.cpp` `MATE_IN_THREE_FLOOR` | 11 | **11**, holds | `S154_floor_margin_sweep.py floor` and `red`: 11 of 24 shipping, 6 weakened, the gate at `RfpMinPly` 1 red at 6 >= 11 (`adocs/data/S111_floor_sweep.log`) |
| `tests/test_engine.cpp` `first_mate_depth`, `mate_in` | 9, 9; 5 | **9, 9; 5**, unmoved | `S188_repair_goldens.py first-mate` (`adocs/data/S111_first_mate.txt`) |
| `tests/test_search.cpp` "ordering keeps the tree small" | 13446 | **13446**, inside [7270, 32512] | `S192_node_budget.py` (the same file) |
| `tests/test_mate_breadth.cpp` `EXACT_FLOOR` | 163 | **163, red** -- the tree reads 162 | `S156_mined_floor_sweep.py --floor 163`: 162 shipping, 159 weakened, so 163 no longer separates (`adocs/data/S111_mined_floor.log`); outside the guard, below |
| `tests/test_search_params.cpp` `golden_defaults` | 75 | 77 | the X-list is the derivation |
| bench | 6042933 | **7343608** | `chesso bench` |

DEV_MANUAL's `short_line_ceiling` row and its re-derive command gain S111's
grid; `adocs/data/README.md` gains rows for the four reading files.

### Waiting for the owner (DEC-303)

- **`tests/test_mate_breadth.cpp` `EXACT_FLOOR` and its comment**: 163 stays;
  the tree reads 162 exact (159 weakened). The fast suite is red there, in
  both builds, and only there. Not touched.
- **MANUAL.md's `RfpMinPly` row**: "164 of its 318 mates ... against a floor
  of 163" is S286's reading; on S111's tree it reads 162. Not touched.
- **The commit**: none before the ruling. The SPRT plays the uncommitted
  tree; on H1 the commit lands after the ruling, with the verdict's block
  and the run's `candidate.diff` beside the log (DEC-254).

### The fast check's fixes

- `tools/mutants/S111_continuation_correction.py`'s docstring: **CC13 does
  move the search at the seeds** -- it halves both continuation weights in
  the numerator -- and the engine-level cases kill it beside the blend case
  (red-first's table above shows them); only CC10 and CC11 change no number at
  100 and 100. The sentence says so.
- **Two more mutants for the UCI layer's other two clears**, both observed red
  by hand (applied to `src/chesso.cpp` alone, `test_engine` rebuilt, the
  cases run, the file restored from the index; bench 7343608 after):
  - **CC24_test_command_keeps_continuation** -- `test` clears the table and
    the pawn table before each position and not the continuation tables.
    Killed by S271's `tests/test_engine.cpp` "a repeated test reads the same
    total" (second total != first), which now names it.
  - **CC25_uci_init_keeps_continuation** -- uci_init() leaves the tables
    warm. Killed by the new `tests/test_engine.cpp` "uci_init clears the
    continuation correction tables": it dirties the tables itself with a
    search, shuts the engine down (which clears no table), requires both
    tables still warm, calls uci_init() and asserts both empty -- relying on
    no case before it.
  242 mutants in `tools/mutants/`, every anchor once; `mutants.sh` runs
  CC01-CC25, CH01-CH15 and G04, 41 mutants, about 81 min.

### `adocs/data/S111_sprt.sh` in working-tree mode

REF pinned to `03fa3ec`; CAND refused if set; the script runs from
`/home/max/ws/chesso-s111r`, `OUT=/home/max/ws/chesso/.tuning/sprt_s111_<stamp>`.
Its guards: the worktree's HEAD is REF; the tree is dirty (`git status
--porcelain` non-empty); build/ was built from the tree -- `cmake --build
build --target chesso` first, then no file under src/, cmake/ or a
CMakeLists.txt newer than `build/src/chesso`; fastchess.sh still reads
`SPRT_BOUNDS` and `SPRT_ROUNDS`. The banner it must print: `candidate
03fa3ec ... + uncommitted changes`, a `diff ... candidate.diff blob <hex>`
line and the engine name `cand-03fa3ec+<hex>`. The closing commit carries
the diff as `adocs/data/S111_candidate.diff` beside the log (DEC-254). The
pre-registration is otherwise unchanged; the coordinator's bracketed marker
stands, now the open-findings sentence for the day it starts.

### After the changes (2026-10-21, `-j12`, `CLANG_FORMAT_MAJOR=22`)

Release, tune and Debug rebuilt without a warning; the fast suite **49 of 50
in both builds, `test_mate_breadth`'s `EXACT_FLOOR` (162 >= 163) the only
red**; format clean; `anchors.sh` 242 mutants, every anchor once; `bench`
7343608 and `bench 12` 2460543, unchanged -- src/ did not move.

### Phase 2's remainder

1. `fixture.sh fixB "S111 phase 2 fixture B"` and `gate.sh gate2` (expect
   `EXACT_FLOOR` red): 5 min.
2. `mutants.sh`: 41 mutants, about 81 min.
3. `selfplay.sh`: 10 min.
4. `gate_extra.sh`: 17 min (its fast stage will show `EXACT_FLOOR` red).
5. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`,
   `--gate`: seconds.
6. Fill `adocs/data/S111_sprt.sh`'s bracketed sentence, check the A/A rule
   (DEC-143), then `cd /home/max/ws/chesso-s111r && nohup
   adocs/data/S111_sprt.sh > /home/max/ws/chesso/.tuning/s111_sprt.log 2>&1
   &`: cap 5250 rounds, 19.7 h, watcher ceiling 144000 s. Leave the
   worktree untouched until it ends.
7. On the verdict, and after the owner's ruling on `EXACT_FLOOR`: the commit
   (H1) or the record of zero (H0), `commit_s111.txt` and the stamp.

## 2026-10-22: the verdict, H0

The run (`adocs/data/S111_sprt.sh` as pinned in the worktree, committed with
the record), 2026-10-22 00:11 to 10:22, 10 h 11 m, 538.5 games an hour, 12 of
12 cores, after an owner shutdown and one refused launch (no book in the
worktree); the banner printed what the coordinator's sentence says it must --
`candidate 03fa3ec 2026-10-21 + uncommitted changes`, the diff line with blob
`f6cb46046bc0`, engine names `cand-03fa3ec+f6cb46046bc0` and `ref-03fa3ec`,
the candidate's `id name` carrying `-dirty`, `Started game 1 of 10500`:

    SPRT | cand 03fa3ec+f6cb46046bc0 vs ref 03fa3ec, 32+0.32, Hash=64, noob_3moves.epd, {0, 10} nElo
    Elo | -1.01 +/- 6.15, nElo -1.52 +/- 9.20
    LLR | -2.96 (-2.94, 2.94) -> H0
    Games | N: 5484 W: 1276 L: 1292 D: 2916, Ptnml [150, 659, 1115, 693, 125]
    Wall | 10 h 11 m, 538.5 games/h, forfeits 0
    Log | adocs/data/S111_sprt.log

0 forfeits either side (`tools/forfeit_report.py`), no crash or disconnect;
`Incomplete mating PV` 4 lines from 2 searches on the candidate's side and 1
on the reference's, an observation; the abort rule never fired. **The
pre-registered reading: not shown to gain at 32+0.32.** The interval,
[-10.72, +7.68] nElo, does not reach the upper bound. A true gain under about
5 nElo reads H0 at `{0, 10}` more often than at `{0, 5}` -- the price of the
bounded run DEC-297 booked, recorded here as the pre-registration asks.
**The H0 row executes**: S111 is not committed and is recorded as zero, the
played diff kept as the record; the pawn table stays (S286). `EXACT_FLOOR`
was never lowered, so DEC-303's owner ruling is not needed and the mutation
pass it held back is not owed. No follow-up run (DEC-297).

What the verdict measured is the pair as DEC-300 blended it: the continuation
conditioning together with the pawn read's share cut to a third at the seeds,
R 0.808 of the parent's applied correction on the blend-gain games
(`adocs/data/S111_blend_gain.txt`), where S110 measured 0.787. As DEC-302
reads it, one verdict cannot separate the two; the family's blend vector,
and with it any continuation weight, is S127's to fit. With S110's H0 the
correction family's two extensions both read zero at 32+0.32; the pawn table
is the family that ships.

The worktrees `../chesso-s111` (the blind original) and `../chesso-s111r`
(the played tree) stay until the owner prunes them; the diff in
`adocs/data/` is the record either way.
