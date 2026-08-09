# Decisions

Append only, newest last. Every entry has a stable id, topic tags, and its
rejected options. A reversal marks the old entry `VOID`, dated, with a pointer
to the superseding entry; it never deletes.

Entry format:

```
## DEC-001  YYYY-MM-DD  short title
Tags:         topic, topic
Context:      what forced a choice
Decision:     what was chosen, and by whom
Rejected:     options not taken, each with the reason
Consequences: what this now constrains
```

---

## DEC-001  2026-08-09  README stays human-written; moltke's README role becomes DEV_MANUAL.md
Tags:         workflow, docs, moltke
Context:      AGENTS.md sections 2 and 7 make `README.md` agent-owned, rewritten
              in place, and require it to carry the exact build and test
              commands. This repository has the opposite rule, set deliberately:
              `README.md` is written by hand by the owner and agents do not
              write in it. moltke additionally enforces that every step
              completion stamp records a README and MANUAL check, so the two
              rules meet at every step.
Decision:     By the owner. `README.md` stays human-only and is never written by
              an agent; the stamp records "checked, human-owned, no change
              needed", which is a valid outcome of a check. The developer-facing
              document AGENTS.md calls README becomes `DEV_MANUAL.md`, which is
              agent-owned. `MANUAL.md` is agent-owned and behaves exactly as
              AGENTS.md describes. The override is written into AGENTS.md itself
              so a later moltke upgrade cannot quietly erase it.
Rejected:     Letting agents write README.md -- simplest against the stock
              ruleset, but it overrides a rule the owner set on purpose.
              Dropping MANUAL.md and setting surface_guard to none -- loses the
              golden surface test and the known-bugs discipline for an engine
              whose whole product surface is a protocol.
Consequences: Three developer-facing documents with distinct owners: README.md
              (human), DEV_MANUAL.md (agent, developer), MANUAL.md (agent, end
              user). CLAUDE.md and TOOLCHAIN.md keep their existing roles. Any
              moltke upgrade that rewrites AGENTS.md must have this override
              re-applied, which is why it is stated in the file and not only
              here.

## DEC-002  2026-08-09  No code from other engines, and no Stockfish-derived NNUE data
Tags:         licensing, nnue, provenance
Context:      Every published chess-engine technique is written up in C++ and
              most reference implementations are GPL. The fastest route to a
              strong engine is to copy them. The owner does not want GPL
              questions anywhere in this codebase, now or when a network lands.
Decision:     By the owner, recorded here from the working notes that predate
              this file. Published ideas and techniques are used freely; source
              is never copied. The piece-square tables in `eval_tables.hpp` are
              hand-written rather than taken from a published set for this
              reason. NNUE training data will come from self-play only.
Rejected:     Taking the PeSTO tables, which are published and tuned and would
              have made S010 an afternoon -- provenance. Training on
              Stockfish-derived data, which is the shortest path to a working
              network -- licence provenance of the network.
Consequences: S010's tables are untuned, which is why S028 exists and is worth
              more than several S027 terms. S029 cannot start until self-play
              data exists. Running the Stockfish *binary* as a tool -- perft
              oracle, game analysis, calibration opponent -- creates no
              derivative work and stays fully allowed.

## DEC-003  2026-08-09  Evaluation is side-to-move relative
Tags:         evaluation, conventions
Context:      `evaluate()` was White-relative and every call site multiplied by
              `(active_color == WHITE) ? +1 : -1`. A sign applied at the call
              site is a sign that can be forgotten at the next call site.
Decision:     Proposed by the agent, accepted by the owner. Positive means the
              side to move is better. The sign is applied once, at the end of
              `evaluate()`, and callers apply none.
Rejected:     Keeping White-relative and auditing the call sites -- the audit
              has to be repeated for every future term.
Consequences: INV-5. Four test contracts were rewritten with the change rather
              than relaxed, including colour symmetry, which now expects the
              mirrored score to agree rather than negate because `mirror_fen()`
              swaps the side to move along with the colours.

## DEC-004  2026-08-09  Published Elo figures from other engines are direction, never prediction
Tags:         measurement, planning
Context:      Three estimates taken from published reports have failed here.
              Staged move generation was quoted at 30-50 Elo and measured 0
              (S006). SEE pruning in quiescence measured 0 (S015). Move ordering
              by good and bad captures is reported around 150 Elo and measured
              *slower*, three separate ways (S025). The 30-50 miss is the
              largest wrong estimate in the project.
Decision:     Proposed by the agent from the measurements, accepted by the
              owner. Numbers quoted from other engines are labelled "reported"
              and used to order work, never to predict a result. The only number
              that counts is the one this engine measures on this hardware
              against its own previous commit.
Rejected:     Trusting the figures, which is what produced the three misses.
Consequences: Every step whose acceptance is Elo says "returns a verdict"
              rather than naming a target. A verdict of zero is recorded as zero
              and the feature may still be kept, with the reason stated -- S006
              and S015 both were. It is also why S018 sits ahead of S019: this
              engine's own error distribution is the only reliable guide to
              which evaluation term is worth writing.

## DEC-005  2026-08-09  SPRT results are attributable to a commit, and the binary is snapshotted
Tags:         measurement, tooling
Context:      An SPRT reported +301 Elo and meant nothing: `build/` was rebuilt
              while the match was running, and fastchess spawns the engine
              process per game, so later games played a different engine than
              earlier ones. The number looked plausible, which is what made it
              dangerous.
Decision:     Proposed by the agent after causing the contamination. The
              reference engine is built from a git ref into `.ref-builds/`, and
              `fastchess.sh` snapshots the candidate binary with `mktemp` and a
              `trap` before the match starts.
Rejected:     Remembering not to rebuild during a match -- the failure mode is
              silent and produces a believable number.
Consequences: Every recorded verdict is attributable to a commit range.
              `.ref-builds/` holds git worktrees and is gitignored. `REF=<sha>
              ./fastchess.sh` picks the baseline.

## DEC-006  2026-08-09  Table-size work on the attack tables is not worth doing
Tags:         movegen, performance
Context:      Fancy and black magics shrink the attack tables from 2307 KB to
              roughly 860 KB and are a standard optimisation.
Decision:     Proposed by the agent from the corrected profile, accepted by the
              owner. Not done. The profile does not implicate table size
              anywhere and `is_attacked` is 0.1 % of the workload. The 2.5 MB
              problem was ergonomic -- it sat inside every `game_t` -- and S007
              solved that by sharing one copy.
Rejected:     Fancy magics, black magics -- measure a cache-miss counter before
              spending a day on either. Callback enumeration instead of a move
              list, the single largest item in every published perft record --
              unavailable to a search, which must materialise moves to score and
              order them; adopting it optimises `bench_movegen` and pessimises
              the engine. Bulk counting at depth 1 in perft -- multiplies the
              headline number by about 6 and changes nothing about the engine.
Consequences: `bench_movegen`'s headline number is not comparable with published
              perft records, on purpose. S032 keeps magics as the fallback path.

## DEC-007  2026-08-09  Bad-capture ordering is deferred until capture history exists
Tags:         search, move-ordering, measurement
Context:      Splitting captures into good and bad and searching the bad ones
              after the quiets is where other engines report their SEE gain. It
              was implemented three ways here and every one was slower: exact
              `see()` in `score_move` +17.6 %, `see_ge` in `score_move` +13 %,
              lazily in the picker +3 %. Nodes fell where it fired, 2288701 to
              1870912 on one position, but the calls cost more than the ordering
              saved.
Decision:     Proposed by the agent, accepted by the owner. Reverted, and the
              rebuild instructions kept in S025 so the next attempt does not
              re-derive them. Retry only after S023 and S024, which order
              captures without calling SEE at all.
Rejected:     Keeping it at +3 % on the argument that the Elo would show up in
              games -- this project does not keep measured regressions on an
              argument. Deleting the knowledge with the code -- the trap below
              would then be rediscovered the hard way.
Consequences: S025 carries the trap that made the first attempt a silent no-op:
              the guard was `*score >= ORDER_CAPTURE`, but a losing capture
              scores `ORDER_CAPTURE` plus a negative victim term, so a queen
              taking a pawn sits at 999200 and the guard rejected exactly the
              moves it existed to catch. Identical node counts exposed it; the
              timings did not.

## DEC-008  2026-08-09  Chess judgement comes from a tool, never from the agent
Tags:         workflow, analysis
Context:      A drawn game was analysed by reading the move list. The analysis
              claimed the evaluation was two pawns too optimistic before move
              62. Stockfish put the position at +196 against the engine's +1.95
              -- agreeing to within five centipawns. The real defect was the
              opposite one, in the ten moves after that trade, and the same
              analysis missed the four other moves that each cost more than a
              pawn, including the largest error in the game.
Decision:     By the owner, after the failure. No agent assesses a position,
              move, line or result from its own reasoning. Getting a position
              onto a board is itself a tool job. The rule is a top-level section
              in CLAUDE.md and covers whether a position is winning, whether a
              move is a blunder, whether an ending is theoretically won,
              material balance after a sequence, and opening soundness.
Rejected:     Trusting the agent's reading with a caveat -- the failure mode is
              not uncertainty, it is confident and specific error. Reading a
              game finds the move you were already looking for.
Consequences: S016 built the tooling that makes the rule practical:
              `pgn_to_positions` for the board, `analyse_game.py` for the cost
              per move. What stays the agent's own is explicitly carved out:
              code, measurement, profiles, search behaviour and test design. The
              engine's own reported evaluation is data to quote, not a premise
              to reason from.

## DEC-009  2026-08-09  surface_guard is cli, and the golden UCI test is a plan step
Tags:         workflow, testing, moltke
Context:      Chesso's entire product surface is the UCI protocol. AGENTS.md
              section 7 makes a golden test over that surface mandatory when
              `surface_guard` names one. Nothing of the kind exists:
              `test_uci.sh` is three lines running `fastchess --compliance` and
              is not in ctest. `test_engine` has "uci reports the options the
              GUI needs", which is close but does not fail on an addition.
Decision:     Proposed by the agent, accepted by the owner. `surface_guard` is
              `cli`. The missing test becomes S017 and is the first pending step
              rather than a waiver.
Rejected:     Setting `surface_guard` to `none` -- valid only for a project with
              no checkable surface, which is the opposite of a UCI engine.
Consequences: Until S017 lands, every completion stamp asserts a check that
              nothing performs. S021 and anything else that adds a UCI option
              sits behind it.

## DEC-010  2026-08-09  test_command is the fast suite, not the full one
Tags:         workflow, testing, moltke
Context:      moltke runs `test_command` at every step completion and refuses on
              failure, with a 600 s timeout. This project's full verification is
              much larger than that: deep perft under `ctest -L slow` runs for
              minutes, the debug build re-runs the generator with `squares[]`
              assertions on every node, and anything that alters play needs an
              SPRT that costs about an hour.
Decision:     Proposed by the agent, accepted by the owner. The gate is
              `cmake --build build -j8 && ctest --test-dir build -L fast
              --output-on-failure && ./clang-format.sh --check`, measured at
              11.2 s with 6 of 6 green. The heavier verification is named in
              each step's `accepts:` field and run by hand.
Rejected:     Including `-L slow` -- risks the 600 s timeout, which refuses
              completion as though the suite had failed. Leaving `test_command`
              unset -- moltke then says out loud that nothing ran the suite,
              which is honest but leaves the gate on the agent alone.
Consequences: A green gate is necessary and not sufficient. A step whose
              `accepts:` names an SPRT or deep perft is not complete because the
              gate passed.

## DEC-011  2026-08-09  C++20 is the language, and it is not reopened
Tags:         language, architecture
Context:      The owner asked whether C, Rust or Zig would be a better absolute
              choice for the strongest possible engine.
Decision:     By the owner after the analysis. C++, targeting `gnu++20`.
Rejected:     C -- no compile-time specialisation, so the templating in S002 and
              S004, worth 12.1 % and 11.7 %, would be hand-written duplicates.
              Rust -- SIMD intrinsics are less mature and the entire published
              body of chess-engine technique would need translating. Zig -- same
              translation cost against a smaller ecosystem.
Consequences: The decision rests partly on SIMD intrinsics, which is a bet that
              pays at S029 and not before. Do not reopen without being asked.

## DEC-012  2026-08-09  The old plan documents are reelaborated into the moltke plan, not kept alongside it
Tags:         workflow, docs, moltke
Context:      `TMP_PLAN.md` (511 lines) and `EVAL_PLAN.md` (349 lines) held both
              the roadmap and the measurement record -- every SPRT verdict,
              every rejected change and the reason. Keeping them next to
              `adocs/plan.md` would leave two documents claiming to be the plan.
Decision:     By the owner. The roadmap becomes `adocs/plan.md` and 32 step
              files; the measurements move into the step files they belong to,
              where a completed step records what it measured; the reasoning
              that outlives any one step becomes the entries above. The two
              files are then deleted, and git history keeps them.
Rejected:     Keeping them as retitled records -- two sources for the same
              numbers, and the step files would point outward instead of being
              readable on their own. Folding everything into this file -- it is
              append-only and would grow by roughly 800 lines of measurement
              narrative that can never be tidied.
Consequences: A completed step file is the primary record of what that change
              cost and what it bought. `TOOLCHAIN.md` is unaffected: it is
              tooling reference, not a plan, and stays where it is.
