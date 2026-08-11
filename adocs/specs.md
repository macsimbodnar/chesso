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
  as zero.

## Behaviour

Chesso is a UCI engine. The protocol surface is the product surface, which is
why `surface_guard` is `cli`; `MANUAL.md` documents it and S017 makes it
checkable.

Engine state as of 2026-08-09, at commit `b6ef5c4`:

| area | state |
|---|---|
| move generation | legal-only, templated `<Color, Constrained, Type>`, ~90 Mnps perft |
| `generate_captures` / `generate_quiets` | partition `generate_moves` exactly (INV-3) |
| board | `board_t` 216 B: bitboards, `squares[64]`, evaluation accumulators |
| make/unmake | 16-byte history record, undo by xor, no board copy |
| evaluation | material plus tapered piece-square tables, maintained incrementally (INV-4), **all 773 constants fitted to chesso's own self-play outcomes** (2026-08-11, S028) |
| search | alpha-beta, transposition table, quiescence, PVS, null move pruning, late move reduction, staged generation, killers, history, countermoves, insufficient-material draws |
| exchange evaluation | `see()` exact, `see_ge()` fast; quiescence declines losing captures |
| absent, evaluation | any term beyond material and the tables: mobility, king safety, pawn structure, passed pawns, bishop pair, tempo. The constants that do exist are fitted (2026-08-11, S028); there is nothing to fit them into beyond the two tables |
| absent, search | aspiration windows, reverse futility, forward futility, razoring, late move pruning, extensions of any kind, delta pruning, capture history, continuation history, correction history |
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

Two things follow and are held to. **The early middlegame is where the
centipawns go**, not the endgame, which is the cheapest phase per move outside
pawn endgames. **The evaluation is optimistic in every phase**, worst in the
late middlegame. The ranking is stable after removing mate-touching moves and
after removing clamped reference scores. It has been measured against one
opponent only, and DEC-019 is the reason that matters.

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

## Non-goals

- **Nothing is copied.** No source from another engine, no tables from another
  engine, no NNUE training data derived from another engine's evaluation or
  search. Ideas and published articles are used freely; that is the plan.
  Running another engine's binary as a tool creates no derivative work and is
  encouraged. DEC-016.
- **The agent does not run training or table tuning.** It builds the tuner, the
  self-play data generation and the training program, and states what the run
  should be. The owner executes the run. DEC-015.
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
- Measurement capacity is the binding constraint on the whole plan: at 10+0.2
  with three usable cores an SPRT verdict costs about an hour, the opening book
  is only `8moves_v3.pgn`, and this machine has a habit of running
  `opendirectoryd` at half a core. An x86-64 Linux box resolves this and is
  needed for S032 and S029 regardless.
- Phase two has no steps and should not get any until the engine is strong
  enough for an experiment to mean something. The transition gets a decision
  entry when it happens.
- The "absent, machinery" row above has no plan steps behind it. Late move
  pruning, check and singular extensions, a quiescence transposition probe, a
  static evaluation in the table entry, an `improving` flag, history malus and
  ageing, and correction history are all standard and all missing. They were
  found while measuring DEC-033 and are parked rather than planned, because a
  step is created by a decision and no decision has been taken on them.
