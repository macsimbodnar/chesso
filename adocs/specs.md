# Specs

What the software must do. Precedence: specs beat plan beat status. Code that
disagrees with this file is a bug or an unrecorded decision.

## What is being built

The strongest open-source chess engine in the world, built on the `achesso`
branch — *agentic chesso* — to find out what AI-driven development can produce.
It is founded on the owner's own bitboard engine, its test framework and its
fastchess SPRT scripts, and on nothing else. DEC-013.

Phase one is to reach the level the published literature describes, by reading
documented technique and implementing it here. Phase two is to experiment.
`adocs/plan.md` is phase one. DEC-014.

**Measured strength, for the first time, 2026-08-18: chesso is approximately
2570 on the CCRL Blitz scale, 95 % ±25, and the figure is soft.** Soft because
the reference set disagrees internally by 83 Elo, above the 30 the procedure
allows; approximate because the games were played at `10+0.2` rather than the
list's time control. 2672 games against four rated engines, solved with `ordo`
anchored on each in turn. `adocs/data/rating_2026-08-18_ccrl_blitz.md` is the
record and `./rating.sh` re-derives it. S087, DEC-067 to DEC-069. Before this
every strength figure in the project was a delta against an earlier chesso and
the distance to the goal above was unknown.

**The target is at least 3000 on that scale, and it is pursued without a
network.** DEC-071, 2026-08-18, the owner's decision. It is a goal and not an
invariant: nothing fails a test for being below it. Reachability was checked
against the same list, single-CPU entries read 2026-08-18 -- Stockfish 11 at
3565, Komodo 14.1 at 3482, Xiphos 0.6 at 3356, Ethereal 11.75 at 3346 and six
more above 3130, every one a hand-crafted evaluation one version below that
engine's first network -- and against the Leorik 2.x line on this machine, which
carries no network file at any tag and reaches 2917. DEC-054 stands: S029 is
parked and NNUE is not reopened. `./rating.sh` is re-run at milestones, after
any landed step an SPRT credits with 20 Elo or more, because every other figure
here is self-play against an earlier chesso and the factor between the two
scales is not yet measured.

## Prime directive

Chesso never plays or accepts an illegal move and never corrupts its own board
state; a reproduced correctness defect outranks every strength, speed and
feature item on the plan.

## Invariants

Numbered, testable properties. Referenced by number from code comments, test
names and commit messages. Each one has a row in `testing.md`.

- **INV-1 Move generation is legal-only and exact.** Perft counts match an
  independent oracle at every tested position and depth. A generator that loses
  moves is faster and wrong, so node counts are checked before any timing is
  printed.
- **INV-2 `make_move` and `unmake_move` are exact inverses.** Bitboards,
  `squares[64]`, hash, castling rights, en passant and the evaluation
  accumulators after an unmake equal their values before the matching make.
- **INV-3 `generate_captures` and `generate_quiets` partition
  `generate_moves` exactly.** Same multiset, no overlap, no quiet move in the
  capture list.
- **INV-4 The incremental evaluation accumulators equal a full recomputation.**
  `material`, `psqt_mg`, `psqt_eg` and `phase` are maintained by `make_move`;
  they must agree with rebuilding them from the bitboards, at every node.
- **INV-5 `evaluate()` is side-to-move relative.** Positive means the side to
  move is better. Callers apply no sign, and mirroring a position mirrors the
  side to move, so the score agrees rather than negates.
- **INV-6 A change is retained only against a measurement.** A change claimed
  behaviour-neutral proves it with identical node counts and identical best
  moves from `tools/search_bench.py`. A change that alters play is retained only
  with an SPRT verdict against a named commit, and a verdict of zero is recorded
  as zero. (2026-08-13, S037: the count the tool reads is the whole search's,
  cumulative over every iteration, so the identity covers the whole tree.
  Before S037 `info nodes` was the current iteration's own count and the tool
  compared the final iteration alone — about half the search — so a change that
  altered depths 1..n-1 could pass. Every node figure recorded from the tool
  before that date is a sum of last iterations.)

## Behaviour

Chesso is a UCI engine. The protocol surface is the product surface, which is
why `surface_guard` is `cli`; `MANUAL.md` documents it and S017 makes it
checkable.

There is one build that is not the product. `-DCHESSO_TUNE=ON` turns the ten
parameters in `src/search_params.hpp` from constants the compiler folds into
variables settable over UCI, and adds one spin option per parameter. **No
strength number is ever taken on it**: a constant that folds is not the same
code as a variable that must be loaded, and the difference is a timing rather
than a node count. The release build's option surface is the three lines it has
always had, the two builds' defaults are held equal member by member by
`test_search_params`, and `tools/search_bench.py` reports the same counts on
both with no `setoption` sent. (2026-08-16, S073.)

Engine state as of 2026-08-09, at commit `b6ef5c4`:

| area | state |
|---|---|
| move generation | legal-only, templated `<Color, Constrained, Type>`, ~90 Mnps perft |
| `generate_captures` / `generate_quiets` | partition `generate_moves` exactly (INV-3) |
| board | `board_t` 216 B: bitboards, `squares[64]`, evaluation accumulators |
| make/unmake | 16-byte history record, undo by xor, no board copy |
| evaluation | material, tapered piece-square tables, mobility, king safety, passed pawns and pawn structure. The first two are maintained incrementally (INV-4). Mobility and king safety are recomputed behind a lazy shortcut that skips them when the cheap score is a margin clear of the window, and their sum is clamped to that margin so the bound holds by construction (2026-08-11, S034; 2026-08-12, S027). The three pawn terms are recomputed in the cheap stage instead, sharing four bitboard fills, because the clamp would truncate them on the positions they exist for (2026-08-12, S027). **All 817 shipped constants fitted to chesso's own self-play outcomes**, each term fitted with every other constant frozen so its SPRT measured one change. The corpus they are fitted to is **deduplicated by the engine's own zobrist key**, one row per distinct position: 207998 of 11003693 rows, 1.8903 %, were repeats carrying repeated weight, and the refit without them measured `Elo: 26.68 +/- 16.40`, H1 accepted over 1044 games against `elo0=-5 elo1=5` -- the pre-registered claim being not a regression of 5 or more with the sign positive at `LOS: 99.93 %`, since an early stop biases the point estimate upward (2026-08-17, S076; DEC-065) |
| search | alpha-beta, transposition table, quiescence, PVS, aspiration windows, null move pruning, late move reduction, reverse futility pruning, staged generation, killers, history, countermoves, insufficient-material draws. Reverse futility returns a static lower bound instead of searching and therefore cannot see a mate; it is bounded to depth 6 and to ply 3 and below, and five guards that were meant to make it mate-safe measured red or inert (2026-08-16, S033). Its margin is **75 centipawns per remaining ply**, cut from the 100 S033 shipped unfitted: 14.5 % fewer nodes at depth 9 and, pooled over 11348 games, a small positive effect of about 3 to 5 Elo and not a regression of 5 or more (2026-08-17, S068; the bounds lesson is DEC-063). Aspiration windows search the root of each iteration from depth 5 in a band **50 centipawns** either side of the previous iteration's score, doubling the failing side alone and going to the full window past 400; the schedule was chosen on node counts over 300 positions in three samples, where it costs 0.9248 of what the feature switched off costs and nothing else swept is below 0.9439, and one sample alone would have chosen a worse one. **H1 accepted over 824 games** against `elo0=-5 elo1=5`, `Elo: 35.12 +/- 19.06` -- the point estimate is biased upward by the early stop, so what the run establishes is the pre-registered claim, not a regression of 5 Elo or more, with the sign positive at `LOS: 99.99 %` (2026-08-17, S021) |
| exchange evaluation | `see()` exact, `see_ge()` fast; quiescence declines losing captures |
| absent, evaluation | bishop pair, rook on an open file, rook on a half-open file, rook on the seventh, tempo — all five present in the code and **shipped at zero weight because they measured zero** (2026-08-13, S027). At zero the compiler deletes them, so they cost nothing; the tuner still carries their 10 parameters, making 827 fitted and 817 shipped. Mobility arrived at S034, king safety, passed pawns and pawn structure at S027 |
| absent, search | forward futility, razoring, late move pruning, extensions of any kind, delta pruning, capture history, continuation history, correction history. Reverse futility left this row at S033 (2026-08-16) and aspiration windows at S021 (2026-08-17) |
| absent, machinery | quiescence never probes or stores the transposition table; no static evaluation in a table entry; no `improving` flag; history is `[piece][to]` with a bonus and no malus and no ageing, and is zeroed on every `go` rather than carried through the game; quiescence is capped at 8 plies |

Both absent rows hold strength. **The evaluation row holds more of it**, which
is measured rather than assumed — see below and DEC-033. The machinery row is
what the audit of the search found while measuring that and has no steps yet.

### Where the centipawns actually go

Measured, not assumed. 13522 moves by chesso over 210 games against sgambetto at
10+0.2, every position scored by Stockfish `dev-20260803-762dd1da` at 3000000
nodes. S018, DEC-032. Phase is the engine's own `game_phase()`, so it names the
quantity the tapered evaluation tapers on.

| phase | moves | cp/move | share of 407740 cp | own score minus reference, mean |
|---|---|---|---|---|
| opening 22-24 | 1934 | 39.9 | 18.9 % | +39.2 |
| early middlegame 14-21 | 3323 | **44.1** | **36.0 %** | +77.4 |
| late middlegame 7-13 | 3493 | 28.4 | 24.3 % | **+100.2** |
| endgame 1-6 | 4570 | 18.3 | 20.5 % | +41.8 |
| pawn endgame 0 | 202 | 6.8 | 0.3 % | +29.5 |

Half the loss sits in moves costing 100 to 400 cp. Blunders above 400 cp are
0.5 % of moves and 9.3 % of the loss; moves under 25 cp are 73 % of moves and
7.7 % of the loss.

Two things followed. **The early middlegame is where the centipawns go**, not
the endgame, which is the cheapest phase per move outside pawn endgames. **The
evaluation is optimistic in every phase**, worst in the late middlegame. The
ranking is stable after removing mate-touching moves and after removing clamped
reference scores. It has been measured against one opponent only, and DEC-019
is the reason that matters.

#### The same profile after the constants were fitted

Added 2026-08-11 with DEC-035. S028 replaced every constant in the evaluation
and was worth +188.74 Elo, so the table above describes an engine that no longer
exists. Re-run identically -- same opponent, time control, adjudication,
reference and 3000000-node limit -- over 98 games and 5582 moves. Evidence in
`adocs/data/S028_raw.tsv`.

| phase | moves | cp/move | share of 154039 cp | own score minus reference, mean | median |
|---|---|---|---|---|---|
| opening 22-24 | 1098 | 31.6 | 22.5 % | -1.5 | 6 |
| early middlegame 14-21 | 1697 | **35.2** | **38.8 %** | -24.3 | 0 |
| late middlegame 7-13 | 1289 | 24.8 | 20.8 % | -40.3 | -4 |
| endgame 1-6 | 1448 | 18.6 | 17.5 % | -3.5 | -1 |
| pawn endgame 0 | 50 | 13.2 | 0.4 % | **+204.3** | 227 |

**The ranking is unchanged and the optimism is gone.** Per-move cost fell in
every phase except the endgame, which is flat at 18.6 against 18.3: the tuning
bought most where most of the loss already was, and moved the endgame least.
The bias reversed sign in four phases of five, but the medians say that is a
tail effect rather than a uniform shift. The exception is the pawn endgame,
which is the one place the old warning survives and the one place it got worse
-- on 22 moves, which decides nothing by itself.

### Search error or evaluation error

Added 2026-08-10 with DEC-033. The profile above says how much was given away
and where; it does not say why. 160 moves that cost 100 cp or more while the
game was still undecided were re-asked of chesso at 4000000 nodes, about its
budget per move at 10+0.2, and at 64000000 nodes, and both answers were costed
by the same reference. `tools/depth_vs_eval.py`, evidence in
`adocs/data/DEC033_depth_vs_eval.tsv`.

| | cp/move |
|---|---|
| as played in the game | 204.7 |
| at 4000000 nodes | 170.1 |
| at 64000000 nodes | 129.1 |

Sixteen times the search removes **24.1 %** of the error, 10.3 cp per doubling,
and **95 of the 160 moves are unchanged**. Where the move does change, cost
falls from 186.0 to 84.9. Quiet moves carry 82.8 % of all loss in the profile
and 1031 of its 1235 errors of 100 cp or more.

**Chesso is evaluation-limited, not depth-limited, on the errors that decide
games.** That is what put S028 next and reordered everything after it. The
figure bounds the search block too: about 10 cp per effective doubling is what
those steps are playing for.

Re-run on the fitted evaluation, 2026-08-11, DEC-035, same criteria and sample
size, evidence in `adocs/data/S028_depth_vs_eval.tsv`: as played 182.1, at 4M
139.7, at 64M 98.0. Sixteen times the search now removes **29.8 %** and **78 of
160 moves are unchanged**. The balance shifted toward the search and did not
turn over -- seventy per cent of the error still survives a sixteen-fold search
-- and the price is the same, **10.4 cp per effective doubling** against 10.3.
The order stands.

## Non-goals

- **Nothing is copied.** No source from another engine, no tables from another
  engine, no NNUE training data derived from another engine's evaluation or
  search. Ideas and published articles are used freely; that is the plan.
  Running another engine's binary as a tool creates no derivative work and is
  encouraged. DEC-016.
- **The agent does not run the NNUE network training.** It builds the trainer,
  prepares the data and states what the run should be; the owner executes that
  one run. Fits, measurements and evaluation tuning are the agent's to run.
  DEC-015 as amended by DEC-041. (2026-08-13: narrowed from "training or table
  tuning" — DEC-041 superseded DEC-015 for evaluation tuning and every kind of
  measurement, and S028's fit was agent-run.)
- **Published Elo figures are not targets.** They have failed to transfer three
  times here. They decide what to try, never what to conclude. DEC-019.
- **Phase-two experiments are not started early.** An idea measured against a
  weak engine produces a number that does not transfer. DEC-014.
- **`README.md` is not an agent-writable file.** DEC-017.
- **`master` and `bitboard` are lineage, not maintained here.** DEC-013.

## Open items

- The S015 quiescence SEE pruning measured 0 Elo when `see()` cost 12.1 % more
  than it does now. The rerun has not happened; it is folded into S022.
- The S013 LMR SPRT was killed at 96 % LLR, +129.2 +/- 33.8 over 183 games. It
  was never formally concluded.
- Measurement capacity is the binding constraint on the whole plan: an SPRT
  verdict costs three to four and a half hours at the DEC-048/DEC-050 settings
  (all 12 threads of the DEC-049 machine), and the opening book is only
  `8moves_v3.pgn`. (2026-08-13: rewritten for the DEC-049 move -- the old text
  priced an hour on three Apple cores with `opendirectoryd` overhead and called
  for an x86-64 box, which S032 and S029 now have.)
- Phase two has no steps and should not get any until the engine is strong
  enough for an experiment to mean something. The transition gets a decision
  entry when it happens.
- ~~The "absent, machinery" row above has no plan steps behind it.~~
  **Discharged 2026-08-18 by DEC-071**, which is the decision that was missing.
  Late move pruning is S090, check extensions S096, singular extensions S097,
  the quiescence transposition probe and the static evaluation in the entry
  S094, the `improving` flag S092, history malus and ageing S093, correction
  history S099; SEE pruning in the main search is S091, internal iterative
  reduction S095 and reduction refinement S098. Time management, which no row
  here listed at all, is S089.
