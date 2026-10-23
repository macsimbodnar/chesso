id:         S292
goal:       close the defect S289's guard closed -- six roots proved mated in two misreported because a null move fails high against the table's mated bound -- with a narrower form that does not cost what S289's run measured, or show by measurement that there is none
accepts:    over positions from S289's run's own games (`.tuning/sprt_s289_20261022_155029/games.pgn`, archived as `chesso_S289_sprt_2026-10-22.tar.xz`), from the engine's own output (a counter or trace in a scratch build, never shipped), at S289's guard: how often it fires per node searched, how many of the bounds it fires on were stored with moves unsearched (a shallow-depth rule skipped a move) against bounds from a search of every legal move, and the node cost of each kind; if a narrower condition, taken from a published description (DEC-221, source named and quoted), reports `mate -2` at all 31 of `tests/test_engine.cpp`'s roots mated in two that S289's case drives, it ships as its own change on the reverted tree: guard tests observed red first, a mutant they kill (DEC-141), Debug self-play command written for the coordinator, gate_extra, every moved golden by its script inside DEC-299's guard, `tools/search_bench.py` d9/d12, and a `{-5, 0}` SPRT pre-registered in `adocs/data/S292_sprt.sh` in `S289_sprt.sh`'s shape; if none does, the figures and a proposed decision in DEC-158's form for the owner (DEC-309 (3)); the fast suite green in both builds
touches:    adocs/data/S292_*; this file; src/search.cpp negamax_at (the null-move block's `whole_search_found_mated` guard and the guard list's pointer to it, the copy of the entry's byte, `move_skipped` set at the two skip sites, negamax's store passing it); src/data_structures.hpp (`tt_entry_t::no_move_skipped`, the padding byte); src/transposition_table.hpp and src/transposition_table.cpp (`tt_store_entry()`'s `no_move_skipped`, defaulted false); tests/test_search.cpp (two cases), tests/test_engine.cpp (one case, `<fstream>`), tests/CMakeLists.txt (`CHESSO_SOURCE_DIR` for test_engine); tests/test_mate_carry.cpp (the ceilings' command gains S292's grid, comments only); adocs/data/S170_cases.tsv (C's budget by the rule, and its header); tools/mutants/S292_null_move_full_search_guard.py (NF01-NF09) and three anchors re-pointed in tools/mutants/S268_gives_check.py (GC05), S099_correction_history.py (CH07) and S234_tt_estimate_margins.py (G04); adocs/data/README.md (S292's rows); DEV_MANUAL.md (the defender set's paragraph, the guard's paragraph, the goldens rows); MANUAL.md (the Known bugs entry for the closed defect, removed)
excludes:   the full verified null-move search (DEC-087 (e), the owner's); late move reduction; S083's constants; any lowering of a mate floor; another run of S289's guard as it was
decisions:  DEC-309, DEC-306, DEC-171, DEC-158, DEC-087, DEC-221, DEC-142, DEC-141, DEC-299, DEC-063
closes:
blocks:     S289
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-23 10:45 -- H1 at {-5, 0} nElo, 8+0.08, Hash 16, CAND e27d3e5 vs REF 10f1e42: Elo +0.35 +/- 3.08, nElo +0.50 +/- 4.41, LLR 2.96, 23802 games, 11 h 07 m, 0 forfeits (adocs/data/S292_sprt.log, archived as chesso_S292_sprt_2026-10-23.tar.xz); the banner as pinned (a393ba5), both binaries at bench 6042933, quarters -1.9, 0.0, +0.7, +2.6, so the narrowed null-move guard stays (DEC-310): the six roots proved mated in two report `mate -2` (31 of 31), S289 completes on it, S083 re-lands on this tree. Implemented by one Opus 5.5 subagent (e27d3e5: red first, NF01-NF09 killed, gate_extra 5/5, Debug self-play clean, both builds green), fast-checked (two stale comments and five notes, fixed); run, read and stamped by the coordinator.

## Why this step exists (the coordinator, 2026-10-22, DEC-309)

S289's guard -- no null move at a node whose entry is an upper bound or an
exact score at or below `-MATE_MIN` -- read H0 at `{-5, 0}`: Elo -7.42 +/-
5.07 over 8716 games against a predicted zero (`adocs/data/S289_sprt.log`).
The run stands, and its pre-registered row reverted the guard whole, so the
defect it closed is open again: six of the 31 roots
`adocs/data/S165_defender_set.tsv` proves mated in two report `cp 0` or about
-670 through 12 iterations at the shipped weights (`adocs/data/S289_reach.txt`).
That moves a reported score, so the BUGS rule binds (DEC-171 (c)).

The guard's own comment named a way it could cost more than predicted: at a
non-PV node with alpha outside the mated band the shallow-depth rules skip
moves without touching `best_so_far`, so a mated-band upper bound can be stored
with moves unsearched, and a guard trusting such a bound removes passes where
nothing was proved. That is a hypothesis, not a reading. This step measures
where the cost came from before anything is changed, and tries the narrower
form the pre-registration offered before the owner rules between the BUGS
rule and the measured cost.

## 2026-10-22: where S289's guard cost, the narrowed guard, and its proof (the implementer)

One Opus 5.5 subagent, in `../chesso-s292` (branch `s292` at `10f1e42`), with
one scratch worktree, `../chesso-s292x`: first the census build (`10f1e42` plus
`adocs/data/S292_census_patch.py`, never shipped), then the mutation fixture.
Nothing committed; the fixture is a commit object on no branch, `b1c8923`
(`10f1e42` plus the whole working tree, made through a temporary index; no ref
moved). Positions come from python-chess and the engine's own `fen`, mates from
S165's proved set and S145's enumeration; nothing was read off a board.

### 1. Where the cost came from (step 1)

`adocs/data/S292_cost.txt`, from `adocs/data/S292_census.py` driving the census
build: one binary, the guard picked by `$S292_GUARD` -- 0 none (bench 6042933,
the reverted tree), 1 S289's on every bound (6042928, `cd50178`), 2 only on a
bound stored by a node at which no shallow-depth rule skipped a move (kind P),
3 only on one stored by a node that skipped a move (kind S). The provenance is
a side table keyed by the full hash, written at negamax's store and matched
against the entry's score, depth and type where the guard's condition holds;
`negamax_at` is wrapped so every exit of such a node is seen with its value and
subtree.

**The sample.** S289's run's PGN, read by python-chess: 541980 positions the
candidate moved in, over 8716 games. `random.Random(292)`: 3000 positions
(2451 games) searched from `ucinewgame` at depth 14 (the run's median reported
depth), and 200 whole games, their 12408 candidate moves replayed in one
process each with the table carried, at depth 14 and at the node count the run
recorded for each move. The engine's `fen` agrees with python-chess on 3000 of
3000.

**The figures** (positions / replays at depth 14 / replays at the run's nodes):

- **How often it fires.** The condition holds at 470, 1337 and 1181 nodes --
  2.6, 2.2 and 1.5 per 100000 null-move-eligible nodes, in 7.7, 5.8 and 4.9 %
  of searches.
- **On what.** Kind S: 466, 1313, 1162; kind P: 4, 24, 19; kind U (no matching
  record): 0, 0, 0 on the reverted tree; 0, 0 and 1 with S289's guard in. With
  S289's guard in, 99.1, 97.6 and 98.1 % of its firings are on S. An S bound's node searched 3.2 to 3.7 moves and skipped 18 to 22;
  the rules that skipped (several per bound): futility 213, history 229, quiet
  SEE 204, capture SEE 156, late move pruning 13 (positions). A P bound's node
  searched 4.6 to 9.7 moves.
- **Proof or not.** Where S289's guard stopped the pass on an S bound, the
  node's own search at the guard's depth failed high or came back unmated at
  353 of 452, 988 of 1270 and 895 of 1111 (78, 78, 81 %), and re-proved the mate
  over every move at 0, 8 and 4. The pass at those nodes, without the guard,
  failed high at 20 of 466, 32 of 1313 and 44 of 1161.
- **Node cost** (each mode against mode 0; 95 % by game, 2000 resamples):
  P x1.000003 [1.0000, 1.0000] and x0.999944 [0.9998, 1.0000]; S x1.000857
  [0.9991, 1.0032] and x1.010637 [0.9978, 1.0328], the replay figure carried by
  one game (7076, 22.5 M nodes -> 87.5 M at one move); at the run's node budgets
  the mean depth reached is 14.9329 without a guard, 14.9331 with P's, 14.9453
  with S's. The firing nodes' own subtrees: 51463 nodes over 452 S firings
  against 54423 over 466 passes (positions). No tree-size cost resolved for
  either kind.
- **Root answers moved.** P: 0 of 3000, 0 and 10 of 12408 best moves (1, 2 and
  32 scores). S: 6, 670 and 747 best moves -- the same rows as S289's guard, in
  all three readings. Within a game the rows move together once a firing changes
  the table: 0 of the replay rows before a game's first such node moved, 2556 of
  the 7839 from it on did.
- **The defect.** Kind P alone reports `mate -2` at all 31 S165 roots mated in
  two (`adocs/data/S289_rows.py`, depth 12): 31 of 31 against the reverted
  tree's 25, S289's guard 31, kind S 25; the 104-row set reads 83 (m2 31/31, m3
  2/15, m4 0/8) against 77, S289's fix's own reading. Mode 2 benches 6042933,
  mode 3 S289's 6042928.

**Reading.** S289's guard was S289's in name only: 98 to 99 % of its firings read
the best of about three moves at a node that skipped about twenty, which the
node's own search refutes four times in five, and those firings carry every
root move it changed. They carry no node cost this census resolves, so what
S289's H0 priced is the moves they changed (or noise), not search effort; that
is the run's to say, not a node count's. Kind P is rare, moves no root best
move at fixed depth, and is the whole of the defect.

### 2. The narrowed form and its source (step 2)

**The rule.** No null move at a node whose table entry is an upper bound or an
exact score at or below `-MATE_MIN` **written by a node that searched every
legal move**: `whole_search_found_mated` in the null-move block, beside the
depth floor, reading the entry's new `no_move_skipped` byte from the copies
taken before anything recursed. negamax's store passes `!move_skipped`, set at
the two sites a shallow-depth rule skips a move (before `make_move` in the
engine's node, after it in the probed one); every other store takes the
default, false. For an upper bound or an exact score the node's loop ran to
its end, so the byte there means every legal move was searched. It is the
struct's padding byte: `tt_entry_t` stays 20 bytes, three to a line.

**The source, DEC-221.** Omid David-Tabibi and Nathan S. Netanyahu, "Verified
Null-Move Pruning", ICGA Journal 25(3), 2002 (arXiv:0808.1125), section 2: "This
value can be treated as a lower bound on the position, since the value of the
best (legal) move has to be better than that obtained from the null move."
Section 3: "If the null-move search indicates a cutoff, but the search shows
that the best value is smaller than β, this implies that the position is a
zugzwang, as the value from the null move is greater than or equal to the value
from the best move." The comparison is with the value of the best move; a node
that skipped a move stored the best of the moves it searched, which is not
that value, so only a whole search's bound is read. No constant, no option, no
engine's code read; implemented from the paper's description.

**It reports `mate -2` at all 31** of `tests/test_engine.cpp`'s roots, and its
node cost on step 1's sample is mode 2's: the frozen build reproduces mode 2
exactly -- best move, score, depth and nodes identical on 3000 of 3000 positions
and 12408 of 12408 replayed moves.

**Alternatives, and why not.** S289's guard as it was (DEC-309, excluded). A
node that skipped moves storing or returning `alpha` instead of its mated best:
changes every reader of those entries and every parent, not only this guard,
with no census behind it. A depth or ply condition in place of provenance: no
published basis, and P and S bounds share depths 5 to 11. The full verified
null-move search: DEC-087 (e), the owner's.

### 3. The change (step 3)

**Tests, observed red first** (`adocs/data/S292_red.log`). On `10f1e42` with the
byte, its store and the three cases but no guard (bench 6042933):
`tests/test_search.cpp`'s `a node a whole search found mated makes no null
move` red on its two guarded legs, 104 rows each, the five premise legs clean;
`tests/test_engine.cpp`'s `a root mated in two reports the mate even where its
node could pass` red on the six roots (four `cp 0`, `cp -676`, `cp -672`), each
`mate -2` from iteration 7 with the guard (`adocs/data/S292_reach.txt`, every
iteration of both builds);
`tests/test_search.cpp`'s `a node that skips a move says so in its entry`
green there, its red being NF03, NF05 and NF06's below. All three green in
both builds after the last edit.

**Mutants** (`tools/mutants/S292_null_move_full_search_guard.py`, NF a new
prefix; `adocs/data/S292_mutation.tsv`, fixture `b1c8923`): NF01 (the guard
dropped) by the guard case and the defender-root case; **NF02** (the guard
reading a pruned search's bound too -- S289's guard) by the pruned-bound leg;
NF03 (the store marks every node) by the store case; NF04 (it marks none) by
the defender-root case and the store case; NF05 and NF06 (one skip site
unmarked each) by the store case; NF07 (a lower bound), NF08 (the band dropped,
also `test_mate_carry`) and NF09 (the exact disjunct) by the guard case. 9 of 9.
Three existing mutants anchored on lines this change moved were re-pointed --
GC05 (the gives-check skip), CH07 and G04 (negamax's store) -- and still die;
S289's sixteen whose cases the guard could mask (M01-M04, M06a, N01, N02, E21,
B04, J01, J02, R02, C02, C05, LQ02, LQ03) all die. **28 of 28**, 3064 s; the
whole catalogue validates, 226 mutants.

**Bench 6042933 -> 6042933**, all eight replies identical: the bench positions
never meet the condition on a whole search's bound. It is a functional change
all the same (the census moves rows), so the commit's line is `Bench:
6042933`, not `No functional change`. `tools/search_bench.py` at 9 and 12 node-
and move-identical to `.ref-builds/4a3a0e5`. S289's census shape, 11503 S219 A/A
positions at depth 10: 1 search moved by 4 nodes, 0 best moves, 0 scores
(`adocs/data/S292_census_s210.txt`; S289's guard 145, 4, 11).

**Goldens** (`adocs/data/S292_goldens.txt`, each by its script on fixture
`b1c8923`, DEC-299's guard): `EXACT_FLOOR` 164 shipping / 160 weakened (163
stays), `MATE_IN_THREE_FLOOR` 12 / 7 (11 stays), first-mate 9 and 9 at mate 5,
S192's 13446, the four table-less mined rows identical but for the milliseconds
column, ceilings 2, 5, 4, 0, 1, 2 over S202's, S261's, S286's and S292's grids
-- all unmoved. **The S170 budget for C moves by the rule**, 1200000 -> 500000
(2 lines, none short), applied by `.tuning/coord/S099/p2/pick_budgets.sh`'s
output with a header paragraph; the parent's grid, taken the same way, is
S286's cell for cell and reproduces the shipped budgets. The grid is
`adocs/data/S292_sweep_s170.txt` (108 cells, 1465 mate lines, 18 short), added
to the ceilings' command. **C's case is thinner for it**: the rule's answer,
500000, reports 2 mate lines where 1200000 reported 23 on the parent and
reports 19 on this tree, and the cell between, 1000000, reports none
(`adocs/data/S292_sweep_s170.txt` lines 44 to 46) -- the cheapest cell with a
mate line, as the rule says, not a hand edit; the case still guards C, on two
lines instead of nineteen. Instrument 4's defender sweep: 82 of 104, m2 31/31,
m3 1/15, against 77, 25/31, 2/15 -- S289's fix's figures exactly; per row with
a fresh table (`adocs/data/S292_reach.txt`) m3 reads 2/15 in both builds.

**Gates.** The fast suite **50 of 50 in both builds**, format clean
(`CLANG_FORMAT_MAJOR=22`, DEC-146), both builds benching 6042933, after the
last code, test and budget edit (`.tuning/coord/S292/gate2.log`).
`tools/gate_extra.sh` **five stages green in 994 s** (prose, citations, the
Debug binaries' fast suite 379 s, the sanitizer build 563 s, deep perft 52 s;
`.tuning/coord/S292/gate_extra/`). `plan_prose_check.py` `--prose`,
`--citations`, `--touches`, `--params` and `--gate` flag nothing. Debug
self-play is the coordinator's (below).

**Debug self-play** (DEC-141 (1)), for the coordinator on the committed tree:
`.tuning/coord/S292/debug_selfplay.sh` -- S289's script with this step's paths:
`build-debug`, `fastchess` four rounds `-repeat` at 4+0.04, Hash 16,
`noob_3moves.epd`, trace log with engine stderr, then `grep -c Assertion` and
`grep -c disconnect`; prints `DEBUG-SELFPLAY-DONE` last.

**The SPRT**: `adocs/data/S292_sprt.sh`, S289_sprt.sh's shape, refusing to start
while the coordinator's bracketed lines stand: CAND S292's commit, REF its
parent, 8+0.08, Hash 16, `noob_3moves.epd`, `{-5, 0}` nElo at alpha = beta =
0.05; 25591 games (12.1 h) with a true zero, 41861 (19.8 h) at the midpoint,
cap 20950 rounds, abort at 40 h, the forfeit and crash rules. H1 or no verdict:
the guard stays, S289 completes on it, S083 re-lands. H0: inspect the run; if
it stands, revert whole and the owner rules with both runs' figures (DEC-309
(3)). Its guards check that CAND changes `src/` in the four files and nowhere
else.

### 4. Proposed, for the coordinator

- **specs.md**, the search row, replacing the S289-reverted sentence: "**A null
  move is not made at a node a whole search found mated, since S292**: where
  the node's entry is an upper bound or an exact score at or below
  `-MATE_MIN` written by a node at which no shallow-depth rule skipped a move
  (the entry's `no_move_skipped`) -- a shallower search of every one of the
  node's moves found the side to move mated -- the pass is not tried: a pass
  that fails high there is the comparison verified null-move pruning reads as a
  zugzwang (David-Tabibi and Netanyahu 2002). A bound from a node that skipped a
  move is the best of the moves it searched and is not read; S289's guard,
  which read those too, read H0 (DEC-309), 98 to 99 % of its firings on them.
  It closes the case S289 found: six of the 31 roots
  `adocs/data/S165_defender_set.tsv` proves mated in two reported `cp 0` or
  about -670 through 12 iterations; all 31 report `mate -2` from iteration 7
  (`adocs/data/S292_reach.txt`). Its price there: instrument 4's sweep, one
  table for the whole set, reads mates in three 1 of 15 with it against 2
  without (`adocs/data/S292_goldens.txt`). Held by
  `tests/test_search.cpp`'s `a node a whole search found mated makes no null
  move` and `a node that skips a move says so in its entry` and
  `tests/test_engine.cpp`'s `a root mated in two reports the mate even where
  its node could pass`. The pass where no whole search has found the node mated
  stays exposed, DEC-087 (e)." And the machinery row: an entry carries, since
  S292, one byte saying no shallow-depth rule skipped a move at the node that
  wrote it, in the padding the struct already had (20 bytes, three a line).
- **DEC text** (the owner's or the coordinator's): "DEC-310 (proposed) S292's
  narrowed null-move guard ships as a correctness fix decided by a `{-5, 0}`
  SPRT. Context: S289's guard read H0 (DEC-309); S292's census of that run's
  games puts 98 to 99 % of its firings on mated bounds stored by nodes that
  skipped moves, refuted by the node's own search four times in five, and every
  root move it changed on them; the six hidden mates in two are all on bounds of
  whole searches. Decision: the guard reads a mated upper bound or exact score
  only where the entry's `no_move_skipped` says the node that wrote it searched
  every legal move (David-Tabibi and Netanyahu 2002, the value of the best
  move); the byte is the entry's padding; `{-5, 0}` at 8+0.08, H0 returning the
  defect to the owner with both runs' figures. It gains the six mates in two
  (31 of 31) and costs instrument 4's sweep one mate in three (2 of 15 -> 1 of
  15; per row with a fresh table 2 of 15 both ways). Rejected: S289's guard again
  (DEC-309); storing `alpha` at a node that skipped moves (changes every reader,
  unmeasured); a depth condition (no source, and the kinds share depths); the
  full verified search (DEC-087 (e)). Consequences: S289 completes on H1 or no
  verdict; S083 re-lands on this tree; the entry's byte has one reader; S170's
  C case runs at the rule's 500000, 2 mate lines against 19 at 1200000 on this
  tree -- thinner coverage the rule chose, not a hand edit."
- **DEV_MANUAL.md**: written in this change (the defender set's paragraph, the
  guard's paragraph, the goldens rows). **MANUAL.md**: the Known bugs entry for
  the six mates in two removed -- the tree no longer has the defect; an H0
  revert puts it back. No option, default or output moves. **README.md**: not
  touched (the owner's).

### 5. Deviations from the brief

- `build/tools/pgn_to_positions` replays from the default position only and
  every game of the run starts from a book FEN, so positions came from
  python-chess (S024's and S210's precedent), each cross-checked by the
  engine's own `position ... moves` and `fen`: 3000 of 3000 agree.
- `touches:` widened past `src/search.cpp`: the bound's provenance has to live
  in the entry, so `src/data_structures.hpp` and `src/transposition_table.*`
  carry one byte and one defaulted parameter. Three existing mutant files had
  anchors on lines the change moved and were re-pointed (GC05, CH07, G04), and
  `adocs/data/S170_cases.tsv` and `tests/test_mate_carry.cpp` took the budget
  and the grid the rule answered.
- The bench does not move, but the change is functional; the run is owed
  (DEC-242's discharge fails on its own terms: root scores move in the census,
  and the change removes passes rather than adding exact cuts).
- Disk: `../chesso-s292` 1.7 GB (Release, tune, and gate_extra's Debug and
  sanitizer builds), `../chesso-s292x` 323 MB, `.tuning/coord/S292/` 29 MB --
  about 2.1 GB; the S154 and S156 scripts' own temporary worktrees under `/tmp`
  were removed by them. 11 GB free after.

## 2026-10-23: the SPRT read H1; the guard stays (the coordinator)

**The verdict.** `adocs/data/S292_sprt.sh`'s pair, `e27d3e5` (the narrowed
guard) against its parent `10f1e42`, from 2026-10-22 23:27 for 11 h 07 m:

```
SPRT | cand e27d3e5 vs ref 10f1e42, 8+0.08, Hash=16, noob_3moves.epd, {-5, 0} nElo
Elo | 0.35 +/- 3.08, nElo 0.50 +/- 4.41
LLR | 2.96 (-2.94, 2.94) -> H1
Games | N: 23802 W: 6597 L: 6573 D: 10632, Ptnml [756, 2803, 4726, 2893, 723]
Wall | 11 h 7 m, 2140.9 games/h, forfeits 0
Log | adocs/data/S292_sprt.log
```

**The inspection.** The banner matches the pin (`a393ba5`): both shas, 8+0.08,
Hash 16, 12 of 12 cores, `noob_3moves.epd`, `{-5, 0}` at alpha = beta = 0.05,
seed 20261022232728. Both binaries the run played reproduce bench 6042933.
No crash or disconnect, 0 forfeits in 23804 PGN games, no incomplete mating
PV; the console's only warnings are fastchess's 8 PV-past-threefold blocks
(5 the reference's, 3 the candidate's). The four quarters in game order read
-1.9, 0.0, +0.7 and +2.6 Elo, each +/- 6.6. The kernel journal is empty at
warning level over the run. The run stands.

**By the pre-registered H1 row**: H1's 0 is preferred to H0's -5 at
alpha = beta = 0.05, so the guard is accepted as not costing 5 nElo at
8+0.08 and **stays**. The stopping run's Elo is biased and is not the effect
size (DEC-063). The defect S289 found is closed on the tree: all 31 roots
`adocs/data/S165_defender_set.tsv` proves mated in two report `mate -2` from
iteration 7 (`adocs/data/S292_reach.txt`). S289 completes on it (its
`paused_by` clears), and S083 re-lands on this tree (DEC-307, DEC-309).

**Archive.** The untrimmed console and the run directory (`games.pgn`,
`fastchess.log`) are `chesso_S292_sprt_2026-10-23.tar.xz` in the owner's
archive, sha256 `4354aaec...45d4aa`, built under `.tuning/` and moved in:
written there in place, Synckeeper split it into conflict copies while tar
was still writing (`.moltke.local.md` has the note).
