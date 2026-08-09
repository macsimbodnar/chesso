@AGENTS.md

# Chesso — what this is, and how it is built

`AGENTS.md` above is the workflow: plans, steps, decisions, tests. This file is
what the project is and the rules that are specific to it. Read both.

`README.md` is written by hand by the repository owner. **Do not write in it.**
The developer-facing document is `DEV_MANUAL.md`.

## What this is

Chesso began as the owner's way to learn chess engine development, following
published material including the Chess Programming Wiki. Three branches, in
order:

| branch | what it is |
|---|---|
| `master` | a hand-written mailbox engine. No AI involved at any point |
| `bitboard` | the move to bitboards, following Maxim's "Bitboard chess engine in C" series on the `@chessprogramming591` channel. The code is the owner's; AI was used for debugging only |
| `achesso` | **this branch. Agentic chesso** |

Working on `bitboard`, the owner concluded they were reproducing most of what
already exists. That is a good way to learn and a poor way to arrive anywhere
new. `achesso` is the answer to a different question: **what can AI-driven
development actually build?**

**The goal is the strongest open-source chess engine in the world.**

This branch inherits exactly three things from `bitboard`: the bitboard engine
as its foundation, the test framework and its tests, and the fastchess SPRT
scripts. Those are what make every later change measurable. Everything else is
built here.

## The plan, in two phases

**Phase one, now.** Reach the level the published literature already describes.
Read the documented state of the art — articles, the wiki, published results —
understand the idea, and implement it here. `adocs/plan.md` is phase one and it
is long.

**Phase two, later.** Experiment. Look for ways to be strong that are not in the
literature.

The order is deliberate and it is not timidity. A novel idea measured against a
weak engine produces a number that does not transfer. A strong engine is the
only platform on which an experiment means anything. DEC-014 — and the
transition between phases is a decision to be recorded, not a drift.

## The two foundations

Everything below follows from these. They are not preferences.

### 1. Nothing is copied

**Never copy code from another engine. Never copy tables. Never train on another
engine's output.**

Ideas, techniques and published articles are used freely — that is the entire
plan. Source is not. Tables are not. NNUE training data derived from another
engine's evaluation or search is not, ever.

This is the rule the branch exists to test. Copying is exactly what made the
earlier work stop being interesting, and there is a licensing reason on top: the
owner wants no GPL question anywhere in this codebase or in a future network.

Consequences you will meet: the piece-square tables in `eval_tables.hpp` are
hand-written and untuned, because the tuned published ones were available and
refused. Running another engine's *binary* as a tool creates no derivative work
and is encouraged. DEC-016.

### 2. Nothing is believed without a measurement

The project's history is a list of confident predictions that were wrong. Nearly
every rule here exists because a specific number came out the other way.

**1. Node counts before timings.** `bench_movegen` verifies perft counts before
printing anything. A generator that loses moves is faster and wrong.

**2. A change meant to be behaviour-neutral must prove it.** Identical node
counts and identical best moves from `tools/search_bench.py`. If they match, an
SPRT is pointless — the engines play identical games. INV-6.

**3. A change that alters play is decided by SPRT.** `./fastchess.sh --fast`, or
`REF=<sha> ./fastchess.sh`. Not by argument, not by fixed-depth timing, not by
how sound it sounds. DEC-020 is the contamination that made attribution
necessary: one run reported +301 Elo and meant nothing.

**4. Know the noise floor before believing a number.** `bench_movegen` reports
its own resolution — the disagreement between the two halves of the run. It has
printed 0.1 % on an idle machine and 2.0 % on a busy one. Anything smaller than
that resolution has not been shown to exist. Check `ps aux | sort -rnk3 | head`
first; this machine runs `opendirectoryd` at half a core often enough to matter.

**5. Under 3 % is noise** unless `hyperfine` says otherwise with a tight sigma
over interleaved runs.

**6. One change at a time.** Two at once and neither number means anything.

**7. Published Elo figures do not transfer.** Three times now: staged move
generation quoted at 30–50 Elo and measured **0**; SEE pruning in quiescence
measured **0**; capture ordering reported around 150 Elo and measured
**slower**. The common cause is that a technique's value depends on the search
around it. Reported figures decide what to try, never what to conclude. DEC-019.

**8. Record negative results.** A verdict of zero is recorded as zero, in the
step file, and the feature may still be kept with the reason stated. S005, S006
and S015 all were.

## Chess judgement is not yours to make

**Never assess a chess position, move, line or result from your own reasoning.
Use a tool.** You are not a strong chess player and the failure mode is not
uncertainty, it is confident and specific error.

This covers every claim of the form:

- is this position winning, equal or lost, and by how much
- is this move good, is it a blunder, what should have been played instead
- is this ending theoretically won or drawn
- what is the material balance after this sequence
- is this opening line sound

What to use instead:

| question | tool |
|---|---|
| what did each move in this game cost | `tools/analyse_game.py` |
| what is this position worth | `stockfish` on the FEN, `go depth 20` |
| is this endgame won | a tablebase, or Stockfish, never a rule you remember |
| what are the legal moves here | the engine, not a mental board |

Getting a position onto a board is itself a tool job:
`build/tools/pgn_to_positions` turns SAN into FENs using the engine's own
parser. Do not track a position in your head across a move list.

**What is still yours:** reasoning about code, measurement, profiles, search
behaviour and test design. The engine's own reported evaluation is data — quote
it. Deriving a chess conclusion from it is not.

The rule exists because of a specific failure, recorded as DEC-023. A drawn game
was analysed by reading the move list. The analysis claimed the evaluation was
two pawns too optimistic before move 62; Stockfish put the position at +196
against the engine's +1.95, agreeing to within five centipawns. The real defect
was the opposite one, in the ten moves *after* that trade. The same analysis
missed the four other moves that each cost more than a pawn, including the
largest error in the game. Reading a game finds the move you were already
looking for.

## Where the agent stops: training and tuning

Use every tool, for everything — evaluation, analysis, debugging, perft oracles,
labelling, calibration.

**Except two things. Do not run NNUE training. Do not run evaluation-table fine
tuning.** Build the tooling, generate and prepare the data, state exactly what
the run should be, and hand it to the owner. The result comes back as constants
or as a network, and is then measured by SPRT like any other change.

The line is *running* the training, not writing it. S028 and S029 are split that
way in practice. DEC-015.

## Known hazards and one-way doors

- **The move-ordering bands clear each other by 100 points.** A king capturing a
  pawn scores `1000000 + 100 - 100000 = 900100`, against 900000 for a killer.
  Tuning `piece_values_abs` can invert that silently, and the symptom is a
  strength regression rather than a wrong node count. S023 walks straight into
  this.
- **Every evaluation term must be accumulated, not recomputed.** `evaluate()`
  runs at every quiescence node. It was 25 % of nodes per second when the tables
  were rebuilt from the bitboards; S014 removed that and INV-4 keeps it removed.
  A term added the recomputed way puts the 25 % back.
- **`make_move` is the NNUE hook.** Every piece change goes through `add_piece`,
  `remove_piece` and `move_piece`, which is the alphabet an accumulator is
  updated from. `unmake_move` deliberately does not use them: an accumulator is
  kept per ply in the search stack and popped, never reverse-updated.
- **Pruning that hides a mate is the recurring bug.** Null move pruning hid a
  mate in 2 by reducing to depth 0; late move reduction reduced the mating move
  at the root. Both were caught by a mate test in the fast suite, not by a
  benchmark. Any new pruning gets the same treatment before it is called done.
- **A bug that has been found gets fixed before anything else starts.** A known
  defect in the tree contaminates every measurement taken after it. One SEE bug
  shipped and pruned quiescence on wrong values for two commits.
- `.ref-builds/` holds git worktrees created by `fastchess.sh`. Gitignored.

## Where everything lives

| what | where |
|---|---|
| what the engine must do, invariants, current state | `adocs/specs.md` |
| the roadmap and its order | `adocs/plan.md` |
| what a completed change cost and bought | `adocs/plan_done/S0nn_*.md` |
| why something is the way it is | `adocs/decisions.md` |
| build, test, benchmark and SPRT commands | `DEV_MANUAL.md` |
| tool setup and the traps in each one | `TOOLCHAIN.md` |
| the UCI surface and known bugs | `MANUAL.md` |
