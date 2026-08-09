@AGENTS.md

# Chesso — what this project is, and how it is worked on

`AGENTS.md` above is the workflow: how plans, steps, decisions and tests are
kept. This file is the part that is specific to a chess engine. Read both.

`README.md` is written by hand by the repository owner. **Do not write in it.**
The developer-facing document is `DEV_MANUAL.md`.

## What this is

A UCI chess engine in C++20, bitboard based. The goal is a genuinely strong
open-source engine, worked on over a long period, with every change justified
by measurement rather than by argument.

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

Getting a position onto a board is itself a tool job: `build/tools/pgn_to_positions`
turns SAN into FENs using the engine's own parser. Do not track a position in
your head across a move list.

**What is still yours:** reasoning about code, measurement, profiles, search
behaviour and test design. The engine's own reported evaluation is data - quote
it. Deriving a chess conclusion from it is not.

The rule exists because of a specific failure, recorded in full as DEC-008. A
drawn game was analysed by reading the move list. The analysis claimed the
evaluation was two pawns too optimistic before move 62; Stockfish put the
position at +196 against the engine's +1.95, agreeing to within five
centipawns. The real defect was the opposite one, in the ten moves *after* that
trade. The same analysis missed the four other moves that each cost more than a
pawn, including the largest error in the game. Reading a game finds the move you
were already looking for.

## How to measure — this is the part that matters

The project's history is a list of confident predictions that were wrong.
Follow the procedure rather than the intuition. `DEV_MANUAL.md` has the exact
commands; these are the rules that govern them.

**1. Node counts before timings.** `bench_movegen` verifies perft counts before
printing anything. A generator that loses moves is faster and wrong.

**2. A change meant to be behaviour-neutral must prove it.** Same node counts
and same best moves from `tools/search_bench.py`. If they match, an SPRT is
pointless - the engines play identical games. Prove it with a determinism check
rather than burning an hour on a match. This is INV-6.

**3. A change that alters play needs games.** `./fastchess.sh --fast`, or
`REF=<sha> ./fastchess.sh` to pick the baseline. The reference is built from a
git ref into `.ref-builds/`, so a result is always attributable to a commit
range. DEC-005 is the contamination that made this necessary.

**4. Know the noise floor before believing a number.** `bench_movegen` reports
its own resolution - the disagreement between the two halves of the run. It has
printed 0.1 % on an idle machine and 2.0 % on a busy one. Anything smaller than
that resolution has not been shown to exist. Check `ps aux | sort -rnk3 | head`
first; this machine has a habit of running `opendirectoryd` at half a core.

**5. Under 3 % is noise** unless `hyperfine` says otherwise with a tight sigma
over interleaved runs. On a loaded machine use the benchmark's own best-of-N,
which rejects interference by construction, rather than hyperfine's mean.

**6. One change at a time.** Two at once and neither number means anything.

**7. Published Elo figures from other engines do not transfer.** Three times
now: staged move generation quoted at 30-50 Elo and measured 0, SEE pruning in
quiescence measured 0, capture ordering reported around 150 Elo and measured
slower. Treat external numbers as direction, never as prediction. DEC-004.

**8. Record negative results.** A verdict of zero is recorded as zero, in the
step file, and the feature may still be kept with the reason stated. S005, S006
and S015 all were.

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
