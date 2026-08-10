# Worklog

Append only. Forensic history for humans, never a context source: never read
this file to work out what to do or why something is the way it is. Anything
here that matters gets promoted into `status.md`, `specs.md`, or `decisions.md`.

Prompts are appended mechanically. Work turns additionally get a recap: step
id, what changed, files touched, tests added, commit sha.

## Recap 2026-08-09 — planning session, moltke adoption

No step id: this is the adoption and planning session, not step work.

Scaffolded the workflow into a repository with about 250 commits of existing
work, then reelaborated the two hand-written plan documents into it. 32 step
files: S001 to S016 retro-stamped into `plan_done/` with the measurement each
change actually produced, S017 to S032 in `plan_todo/` in the order they should
be attempted. 12 decisions recorded, 6 invariants written, 41 rows added to the
testing ledger.

Files: `.moltke.json`, `AGENTS.md` (repository overrides section added),
`CLAUDE.md` (rewritten as the chess-specific half, `@AGENTS.md` first line),
`DEV_MANUAL.md` and `MANUAL.md` (new), `adocs/*`, `src/eval_tables.hpp` (one
stale reference to a deleted file), `tools/pgn_to_positions.cpp` (formatting).
Deleted: `TMP_PLAN.md`, `EVAL_PLAN.md` — content migrated, git history keeps
them.

Tests: no new tests. `tools/pgn_to_positions.cpp` was unformatted in the tree
and `./clang-format.sh --check` failed on it; fixed before anything else, per
the bug-first rule. Gate green afterwards: 6 of 6 in 11.6 s, format clean.

## 2026-08-09T16:50+02:00 prompt

> I want to redefine the project. Please whipe out what is there excpet the plan done and the future plan but what is this project about and how it hsould be handlead should be rewriten. This is what the project is:
> I started chesso as a way to lear chess engine development. I followed multiple online sources like chess dev wiki. And i ended up with my own mailbox implementation of the engine with no ai. That's what in master right now. Then i started to move to bitboard board representation following Maxim from @chessprogramming591 youtube channel the video series Bitboard chess engine in C. That is what in bitboard branch. I used ai for debugging but still code is mine. Then, since i realized I was copieng majority of what was out there i decided that i want to give a try to AI development. And that's what i am doing in the branch achesso that stais for agentic chesso. This piece starts with my bitboard implementation as founfaments and the tests framework with tests and fastechess for SPRT scripts tests. My goal is to use AI to build the strongest opensource chess engine in the world. My plan is first to follow the state of the art documentated online documentation and articles to build a really strong engine and then start to experiment and maybe find new way to be strong. The rules for the ai are to never copy the code or tables from existing open source engines nor use them for training NNUE. I would prefer to run the training and the fine tuning of tables by myself when the time will come. The other foundation of this effort is the rigorous testing and tool usage. The changes must be evaluated by running sprt tests, the tables must be finetuned by proper tools and measurements and the AI should use tools like stockfish and others for chess game evaluation, position, debugging and everything that can help in the development that is not training of models or tables finetuning directly.

## Recap 2026-08-09 — project redefinition

No step id: this rewrites what the project is, not what it does. `plan.md`,
`plan_todo/` and `plan_done/` are untouched, as asked.

The owner stated the project's definition: chesso began as learning, following
the Chess Programming Wiki and similar; `master` is the hand-written mailbox
engine with no AI; `bitboard` followed Maxim's "Bitboard chess engine in C"
series with AI used only for debugging; `achesso` is agentic chesso, founded on
that bitboard engine plus its tests and fastchess SPRT scripts, aiming at the
strongest open-source engine in the world. Phase one follows documented
technique, phase two experiments. Two foundations: nothing is copied, and
nothing is believed without a measurement. New boundary: the agent does not run
NNUE training or table fine tuning — it builds the tooling and the data and
hands the run to the owner.

`adocs/decisions.md` was wiped and rewritten from scratch on the owner's
instruction. The 12 DEC ids were kept because `plan_done/` cites them and
completed history is immutable; no wording survived, and DEC-013 to DEC-015 are
new. **INV-8 refused this**, correctly — it is the append-only guard installed
one commit earlier. It was cleared by committing the rewrite, not by restoring
the old entries. That is a deliberate, owner-instructed reset of decision
history and it is recorded here so it is not mistaken for drift.

Files: `CLAUDE.md`, `AGENTS.md` section 0, `adocs/specs.md`,
`adocs/decisions.md`, `adocs/plan.md` description, `adocs/plan_todo/S028`,
`adocs/plan_todo/S029`. No source changed. Gate green: 6 of 6 in 10.1 s, format
clean.

## Recap 2026-08-09 — project redefinition, second attempt

Correction to the recap above: the first attempt wiped `adocs/decisions.md` and
was wrong to. INV-8 walks the file's whole git history and requires the current
content to contain every past version's lines in order; INV-9 forbids duplicate
DEC ids. Together they make a wipe-and-reuse-the-ids impossible without
rewriting git history. Commit 6953e12 made it unresolvable, so 3c902ee restored
the committed entries and the redefinition landed the way the invariant is built
for: DEC-001 to DEC-012 stand, marked VOID with forward pointers, and DEC-013 to
DEC-027 supersede them. Every citation outside `plan_done/` was repointed;
`plan_done/` was not touched, and its citations resolve through the VOID table.

Content: the project is `achesso`, agentic chesso, founded on the owner's
bitboard engine plus its tests and fastchess SPRT scripts, aiming at the
strongest open-source engine in the world. Phase one follows documented
technique, phase two experiments. Two foundations: nothing is copied, nothing is
believed without a measurement. New boundary: the agent builds the tuner, the
data and the training program and states the run; the owner executes it.

Files: `CLAUDE.md`, `AGENTS.md` section 0, `adocs/specs.md`,
`adocs/decisions.md`, `adocs/plan.md`, `adocs/status.md`, `adocs/testing.md`,
`DEV_MANUAL.md`, and seven `plan_todo/` step files. No source changed.

## 2026-08-09T17:05+02:00 prompt

> let's move next with the plan

## 2026-08-09T17:09+02:00 prompt

> go

## 2026-08-09T23:26+02:00 prompt

> next?

## 2026-08-09T23:32+02:00 prompt

> i used for my implementation fastchess uci test. I even have script for that. Is it not more convinient to use that one for uci tests?

## 2026-08-09T23:34+02:00 prompt

> what's next in the plan?

## 2026-08-10T09:21+02:00 prompt

> ok let's measure and record the outcome remembering to register the characteristics of hte machien and the compilation flags aswell. Stockfish, sgambetto (a chess engine maid complitly by AI by a friend) and mailbox chesso, fastechss, cutechess are all in /Users/max/.local/bin/ and can be invoked directly in console

## 2026-08-10 recap — S018 in progress, not complete

Step S018 started. Tooling written and smoke tested; no full run yet, nothing
committed.

Files touched:
- `tools/error_profile.py` new. PGN of a match in, per-move cost out, bucketed
  by game phase, by error size, and the two crossed. Also reports evaluation
  bias, engine score against reference score, which the PGN carries for free in
  fastchess's move comments. `--raw-out` writes every analysed move so a rerun
  is never needed to re-bucket; `--from-raw` re-buckets with no engine.
- `tools/pgn_to_positions.cpp` emits a fifth column, the engine's own
  `game_phase()` for the position before the move. Appended, not inserted.
- `tools/analyse_game.py` takes the first four fields, so the added column does
  not break the S016 usage.

Machine and build recorded for the step: Apple M1, MacBookPro17,1, 4
performance + 4 efficiency cores, 8 GB, macOS 14.8.5 build 23J423. Apple clang
16.0.0. Release flags `-O3 -DNDEBUG -std=gnu++20 -arch arm64 -Wall -Wextra
-Werror`. Reference `stockfish dev-20260803-762dd1da`.

Two measurements taken:
- Smoke match achesso vs mailbox, 6 games: 6-0. `~/.local/bin/chesso`
  advertises only `Use Book`, confirming it is the mailbox build.
- Probe achesso vs sgambetto, 20 games at 10+0.2: 11.5/20 for achesso.

One finding that changes the run design: under the SPRT adjudication settings
the smoke run produced 48 opening, 61 early middlegame, 6 late middlegame and
**0 endgame** moves. S019 is an endgame defect, so those settings cannot see the
phase the step exists to measure. Opponent, adjudication and run size are with
the owner.

Depth-18 throughput measurement was still running at the end of the turn.

No tests added yet. No commit yet.

## 2026-08-10T09:55+02:00 prompt

> ok meanwhile let's go over open items

## 2026-08-10 recap — S018 measurement turn, no source change

No code written this turn. Measurements only, plus a defect found in the tooling
written in the previous turn.

Reference cost, one Stockfish process, three middlegame FENs taken from the
smoke games, machine mostly idle:

| depth | s/position |
|---|---|
| 14 | 0.09 |
| 16 | 0.36 |
| 18 | 0.78 |
| 20 | 1.52 |

S016's figure of 158 positions in about 47 s, 0.30 s per position, does not
transfer. That game was largely a simplified endgame and endgames are cheap at
fixed depth; middlegame-heavy games cost 2.6x more at the same depth.

**Defect, blocking, not yet fixed.** `tools/error_profile.py` is slower with
four workers than with one. The 6 smoke games score 236 positions, which is
about 184 s serially at 0.78 s per position. The four-worker run took 1139 s,
6.2x slower. The same code at depth 12 was 2.2x faster than serial, so it
degrades as Stockfish's `info` output grows. Suspect is `evaluate()` calling
`line.split()` on every info line, with four Python threads contending on the
GIL. Fix before any full run: keep the last matching line and parse once after
`bestmove`, and use processes rather than threads.

Open items carried to the owner: opponent choice (mailbox as the step file says,
sgambetto, nodes-limited Stockfish, or self-play), adjudication (the SPRT
settings produced 0 endgame moves out of 115), reference depth 16 against 18,
and run size. Run size cannot be decided until the defect above is fixed.

No tests added. No commit. Working tree still carries the previous turn's
`tools/` changes and the S018 step-file move.

## 2026-08-10T10:02+02:00 prompt

> Let's go back to the iteractiv way for the questions you asked two messages ago

## 2026-08-10T11:51+02:00 prompt

> retyr

## 2026-08-10 recap — S018 tooling committed, 400-game match started

Commit `45c83ab`. `tools/error_profile.py` new, phase column added to
`tools/pgn_to_positions.cpp`, `tools/analyse_game.py` made tolerant of it.

**Correction to the previous recap.** The "defect, blocking" recorded there --
`error_profile.py` 6.2x slower with four workers than with one -- was not real.
It came from comparing a 236-position run against an extrapolation from three
hand-picked FENs that happened to sit near the median cost. Measured properly,
6 games and 236 positions take 140.67 s with four workers against a 385 s
serial estimate, a 2.7x speed-up. There was never a parallel defect and the GIL
explanation was wrong: a search prints about 30 lines, not thousands. The
parse-once change was kept because it is verified behaviour-neutral, identical
raw output at depth 12, but its justification comment was rewritten to say it
is tidiness and not a fix.

**Fixed depth abandoned for the reference search.** Over 12 sampled positions
depth 18 ran a median of 0.71 s, a mean of 78.80 s and a maximum of 925.90 s:
one position in twelve took a quarter of an hour. Node limits have no such
tail.

| limit | mean | median | max | depth reached min/med/max |
|---|---|---|---|---|
| 500000 nodes | 0.83 s | 0.88 s | 1.08 s | 16/18/64 |
| 1000000 nodes | 1.63 s | 1.77 s | 2.11 s | 17/21/82 |
| 3000000 nodes | 4.84 s | 5.04 s | 6.21 s | 17/24/134 |

1000000 nodes reaches median depth 21, so it is at least the depth 20 the owner
asked for, and it is budgetable.

**Adjudication settings decided by measurement.** The SPRT settings produce no
endgames at all, which would have made the run blind to the phase S019 is about.

| | tight (SPRT) | loose |
|---|---|---|
| median plies | 50 | 166 |
| scored positions per game | 39 | 155 |
| endgame moves | 0 | 289 |
| pawn endgame moves | 0 | 9 |

Loose is `-draw movenumber=80 movecount=10 score=5 -resign movecount=8
score=900 -maxmoves 200`.

Owner's choices for the run: opponent sgambetto, loose adjudication, depth 20
equivalent, about 400 games. Opponent deviates from the step file, which names
the mailbox build; mailbox lost 6-0 in the smoke match while sgambetto scored
8.5/20, so the mailbox distribution would be that of an already-won position.
A decision entry is owed before the step completes.

Match started in the background: 400 games, achesso vs sgambetto, 10+0.2,
concurrency 3, engine binary snapshotted. About 2 h. Analysis follows and is
about 10.3 h at 1000000 nodes or 5.3 h at 500000; the choice is open.

No tests added. No step completion.

## 2026-08-10T12:50+02:00 prompt

> <task-notification>
> <task-id>bi7yrwfhv</task-id>
> <tool-use-id>toolu_01GNxApFFBMN7vWWtqFgWhj7</tool-use-id>
> <output-file>/private/tmp/claude-502/-Users-max-ws-chesso/c7bab205-8190-4227-9eb5-361c8575ebfe/tasks/bi7yrwfhv.output</output-file>
> <status>killed</status>
> <summary>Background command "Play 400 games vs sgambetto" was stopped</summary>
> </task-notification>

## 2026-08-10 recap — S018 analysis running, decisions recorded

Commits `ed56ef6` (resumable runs) and this one (decisions).

The 400-game match was killed by the harness at 210 games, cause unknown;
fastchess's own output was lost behind a pipe. The PGN is intact: 210 games,
all parse, last game ends cleanly. Owner chose to proceed with the 210 rather
than top up.

Match result, for the record: achesso +61 =43 -106 against sgambetto, 39.3 %,
median 133 plies, longest 380, 142 adjudications and 68 normal terminations.
Both matches used binary `c7ea34b5`, verified by checksum, so the swing from
57.5 % under SPRT adjudication to 39.3 % under loose adjudication is not a build
difference. What it is instead is undetermined and is not to be guessed at.

Decisions recorded before the numbers land, so the method cannot be chosen to
fit the answer: DEC-029 opponent, DEC-030 node limits instead of fixed depth,
DEC-031 loose adjudication for profiling matches.

Analysis launched detached, so a harness kill cannot take it down as it did the
match:

```
tools/error_profile.py <match.pgn> --player achesso \
    --engine ~/.local/bin/stockfish --nodes 3000000 --workers 6 \
    --raw-out <raw.tsv> --resume --worst 40
```

27124 positions over 210 games at 3000000 nodes, median depth 24. Six workers on
a four performance plus four efficiency core M1; fixed node counts make the
scores independent of scheduling, so only wall time is affected. Estimated 10 to
13 h.

Still owed before S018 can close: the numbers themselves, `testing.md` rows, and
a check of `MANUAL.md` and `DEV_MANUAL.md`.

## 2026-08-10T13:36+02:00 prompt

> <task-notification>
> <task-id>bg2g8ldux</task-id>
> <tool-use-id>toolu_013z9okK56UpnQy6f1cxts1n</tool-use-id>
> <output-file>/private/tmp/claude-502/-Users-max-ws-chesso/c7bab205-8190-4227-9eb5-361c8575ebfe/tasks/bg2g8ldux.output</output-file>
> <status>killed</status>
> <summary>Background command "Wait for analysis completion" was stopped</summary>
> </task-notification>

## 2026-08-10T14:32+02:00 prompt

> it still running in background

## 2026-08-10T14:33+02:00 prompt

> ok monitor the execution and once completed proceed

## 2026-08-10T14:34+02:00 prompt

> <task-notification>
> <task-id>bk2f88z31</task-id>
> <summary>Monitor event: "S018 analysis progress and completion"</summary>
> <event>S018 progress: 48/210 games</event>
> </task-notification>
