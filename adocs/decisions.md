# Decisions

Append only, newest last. Every entry has a stable id, topic tags, and its
rejected options. A reversal marks the old entry `VOID`, dated, with a pointer
to the superseding entry; it never deletes.

**Start with DEC-013.** It is what this project is; the entries before it are
how it is worked on. The whole file was rewritten on 2026-08-09 when the owner
stated the project's definition; the ids were kept because `plan_done/` cites
them and completed history is immutable, but no wording survived.

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

## DEC-001  2026-08-09  The work is documented in the repository, and README.md stays the owner's
Tags:         workflow, docs, moltke
Context:      An agentic project has a specific failure mode: everything that
              matters lives in a session transcript that the next agent cannot
              read. The owner needs to be able to open the repository months
              later and find what was decided, what was measured, and why the
              next step is the next step -- without asking anyone.
Decision:     By the owner. The moltke workflow is adopted: `adocs/specs.md` for
              what must be true, `adocs/plan.md` and the step directories for
              what is being done and in what order, this file for why, and
              `adocs/testing.md` for what proves it. Nothing that matters is
              allowed to exist only in an agent's memory.
              `README.md` is written by hand by the owner and no agent writes in
              it. The developer-facing document the stock ruleset calls README is
              `DEV_MANUAL.md` here; `MANUAL.md` is the end-user document and
              behaves as the ruleset describes. The override is written into
              `AGENTS.md` section 0 so a later upgrade cannot silently erase it.
Rejected:     Keeping the project in hand-written plan documents, which is what
              was there before -- they interleaved the roadmap with the
              measurement record and could not answer "what is next" without
              being read end to end. Letting agents write `README.md` -- it is
              the one document that is the owner's voice.
Consequences: Three developer-facing documents with distinct owners:
              `README.md` (the owner), `DEV_MANUAL.md` (agent, developer),
              `MANUAL.md` (agent, end user). At step completion, "checked
              README.md, owner-written, no change needed" is the expected
              outcome and a valid one.

## DEC-002  2026-08-09  The agent never copies code or tables, and never trains on another engine's output
Tags:         provenance, licensing, nnue, tables
Context:      This is the rule the whole `achesso` branch exists to test. The
              owner's earlier work followed published sources closely enough
              that it stopped being interesting -- see DEC-013 -- and the point
              of building with an agent is to find out whether it can produce
              something rather than reproduce something. There is also a plain
              licensing reason: most reference engines are GPL, and the owner
              wants no GPL question anywhere in this codebase or in a future
              network.
Decision:     By the owner. Published ideas, articles and techniques are used
              freely and are in fact the plan (DEC-014). **Source is never
              copied. Tables are never copied.** No NNUE training data derived
              from another engine's evaluation or search, ever.
Rejected:     Taking the PeSTO piece-square tables, which are published and
              tuned and would have made S010 an afternoon instead of a
              hand-written guess -- provenance, and it is exactly the copying
              that made the earlier branches feel pointless. Training on
              Stockfish-labelled positions, the shortest path to a working
              network -- licence provenance of the network, and the same
              objection.
Consequences: The tables in `eval_tables.hpp` are hand-written and untuned,
              which is why S028 exists and is worth more than several S027 terms
              put together. S029 cannot start until self-play data exists.
              Running another engine's *binary* as a tool -- perft oracle, game
              analysis, calibration opponent -- creates no derivative work and
              stays fully allowed and encouraged; see DEC-008 and DEC-015 for
              where that line sits.

## DEC-003  2026-08-09  Evaluation is side-to-move relative
Tags:         evaluation, conventions
Context:      `evaluate()` was White-relative and every call site multiplied by
              `(active_color == WHITE) ? +1 : -1`. A sign applied at the call
              site is a sign that can be forgotten at the next call site, and
              the symptom is an engine that plays one colour worse than the
              other.
Decision:     Proposed by the agent, accepted by the owner. Positive means the
              side to move is better. The sign is applied once, at the end of
              `evaluate()`, and callers apply none.
Rejected:     Keeping White-relative and auditing the call sites -- the audit
              has to be repeated for every future term, and there will be many.
Consequences: INV-5. Four test contracts were rewritten with the change rather
              than relaxed, including colour symmetry, which now expects the
              mirrored score to agree rather than negate, because `mirror_fen()`
              swaps the side to move along with the colours.

## DEC-004  2026-08-09  Published Elo figures are direction, never prediction
Tags:         measurement, planning
Context:      The plan is to follow the documented state of the art (DEC-014),
              and the documented state of the art comes with numbers attached.
              Three of them have now been taken at face value here and none
              survived contact with this engine. Staged move generation was
              quoted at 30-50 Elo and measured **0** (S006). SEE pruning in
              quiescence measured **0** (S015). Ordering captures into good and
              bad is reported around 150 Elo and measured **slower**, three
              separate ways (S025).
Decision:     Proposed by the agent from the measurements, accepted by the
              owner. Figures quoted from other engines are labelled "reported"
              and used to decide what to try and in what order. They are never
              used to predict a result and never used to conclude one. The only
              number that counts is the one this engine measures on this
              hardware against its own previous commit.
Rejected:     Trusting the figures, which is what produced all three misses. The
              common cause is that a technique's value depends on the search
              around it: staged generation pays when the quiet moves above it
              are being pruned, and at the time this engine pruned nothing.
Consequences: Every step whose acceptance is Elo says "returns a verdict" rather
              than naming a target. A verdict of zero is recorded as zero and
              the feature may still be kept with the reason stated -- S005, S006
              and S015 all were. It is also why S018 sits ahead of S019: this
              engine's own error distribution is a better guide to the next
              evaluation term than any published figure.

## DEC-005  2026-08-09  Every SPRT verdict is attributable to a commit, and the binary is snapshotted
Tags:         measurement, tooling, sprt
Context:      An SPRT reported +301 Elo and meant nothing. `build/` was rebuilt
              while the match was running, and fastchess spawns the engine
              process per game, so later games played a different engine than
              earlier ones. The number was plausible, which is what made it
              dangerous. Before that, the reference was a binary in
              `~/.local/bin` that drifted out of date silently, so every result
              measured an unknown amount of unrelated work.
Decision:     Proposed by the agent after causing the contamination, accepted by
              the owner. The reference is built from a git ref into a worktree
              under `.ref-builds/`, and `fastchess.sh` snapshots the candidate
              binary with `mktemp` and a `trap` before the first game.
Rejected:     Remembering not to rebuild during a match -- the failure mode is
              silent and produces a believable number, which is the worst
              possible combination.
Consequences: Every recorded verdict is attributable to a commit range.
              `REF=<sha> ./fastchess.sh` picks the baseline. This is what makes
              SPRT the deciding instrument it is in DEC-014 rather than a number
              that happens to be printed.

## DEC-006  2026-08-09  Shrinking the attack tables is not worth doing
Tags:         movegen, performance
Context:      Fancy and black magics shrink the attack tables from 2307 KB to
              roughly 860 KB and are a standard, well documented optimisation --
              exactly the kind of thing DEC-014 says to follow.
Decision:     Proposed by the agent from the corrected profile, accepted by the
              owner. Not done. The profile does not implicate table size
              anywhere and `is_attacked` is 0.1 % of the workload. The real
              2.5 MB problem was ergonomic -- the tables sat inside every
              `game_t` -- and S007 solved that by sharing one copy, taking
              `game_t` from 2515 KB to 87 KB.
Rejected:     Fancy magics, black magics -- measure a cache-miss counter before
              spending a day on either. Callback enumeration instead of a move
              list, the single largest item in every published perft record --
              unavailable to a search, which must materialise moves to score and
              order them; adopting it optimises `bench_movegen` and pessimises
              the engine. Bulk counting at depth 1 in perft -- multiplies the
              headline number by about 6 and changes nothing about the engine.
Consequences: `bench_movegen`'s headline number is deliberately not comparable
              with published perft records. This is the clearest case of DEC-014
              in practice: the documented technique was read, understood, and
              measured against this engine's own profile rather than adopted
              because it is standard.

## DEC-007  2026-08-09  Bad-capture ordering is deferred until capture history exists
Tags:         search, move-ordering, measurement
Context:      Splitting captures into good and bad and searching the bad ones
              after the quiets is where other engines report their SEE gain. It
              was implemented three ways here and every one was slower: exact
              `see()` in `score_move` +17.6 %, `see_ge` in `score_move` +13 %,
              lazily in the picker +3 %. Nodes fell where it fired, 2288701 to
              1870912 on one position, but the exchange calls cost more than the
              ordering saved.
Decision:     Proposed by the agent, accepted by the owner. Reverted. The
              rebuild instructions are kept in S025 so the next attempt does not
              re-derive them, and the retry waits for S023 and S024, which order
              captures without calling SEE at all.
Rejected:     Keeping it at +3 % on the argument that the Elo would show up in
              games -- this project does not keep a measured regression on an
              argument. Deleting the knowledge along with the code -- the trap
              below would then be rediscovered the hard way.
Consequences: S025 carries the trap that made the first attempt a silent no-op:
              the guard was `*score >= ORDER_CAPTURE`, but a losing capture
              scores `ORDER_CAPTURE` plus a *negative* victim term, so a queen
              taking a pawn sits at 999200 and the guard rejected exactly the
              moves it existed to catch. It looked like it worked and changed
              nothing. **Identical node counts exposed it; the timings did not.**

## DEC-008  2026-08-09  Chess judgement comes from a tool, never from the agent
Tags:         workflow, analysis, tooling
Context:      A drawn game was analysed by an agent reading the move list. The
              analysis claimed the evaluation was two pawns too optimistic
              before move 62. Stockfish put the position at +196 against the
              engine's +1.95 -- agreeing to within five centipawns. The real
              defect was the opposite one, in the ten moves *after* that trade,
              and the same reading missed the four other moves that each cost
              more than a pawn, including the largest error in the game.
Decision:     By the owner, after the failure. No agent assesses a position,
              move, line or result from its own reasoning. This covers whether a
              position is winning, whether a move is a blunder and what should
              have been played, whether an ending is theoretically won, the
              material balance after a sequence, and whether an opening line is
              sound. Getting a position onto a board is itself a tool job.
Rejected:     Trusting the agent's reading with a caveat attached -- the failure
              mode is not uncertainty, it is confident and specific error.
              Reading a game finds the move you were already looking for.
Consequences: S016 built the tooling that makes the rule practical:
              `pgn_to_positions` puts the position on a board using the engine's
              own parser, `analyse_game.py` reports the cost of every move from
              Stockfish. What remains the agent's own is carved out explicitly:
              code, measurement, profiles, search behaviour and test design. The
              engine's own reported evaluation is data to quote, never a premise
              to reason from. This is the positive half of DEC-015: tools for
              everything except training and tuning.

## DEC-009  2026-08-09  The UCI surface is guarded, and the golden test is a plan step
Tags:         workflow, testing, uci
Context:      Chesso's entire product surface is the UCI protocol, so
              `surface_guard` is `cli` and a golden test over that surface is
              required. Nothing of the kind exists: `test_uci.sh` is three lines
              running `fastchess --compliance` and is not in ctest.
              `test_engine` has "uci reports the options the GUI needs", which is
              close but does not fail when a command or option is added.
Decision:     Proposed by the agent, accepted by the owner. `surface_guard`
              stays `cli`. The missing test becomes S017 and is the first
              pending step rather than a waiver.
Rejected:     Setting `surface_guard` to `none`, which is valid only for a
              project with no checkable surface -- the opposite of a UCI engine
              whose every user reaches it through that protocol.
Consequences: Until S017 lands, every completion stamp asserts a check that
              nothing performs. S021 and anything else that adds a UCI option
              sits behind it. `MANUAL.md` is what the test holds to account.

## DEC-010  2026-08-09  The automatic gate is the fast suite; the real verification is per step
Tags:         workflow, testing, sprt
Context:      The workflow runs one command at every step completion and refuses
              on failure, with a 600 s timeout. This project's actual
              verification is far larger: deep perft runs for minutes, the debug
              build re-runs the generator with `squares[]` and accumulator
              assertions on every node, and anything that alters play needs an
              SPRT costing about an hour.
Decision:     Proposed by the agent, accepted by the owner. The gate is
              `cmake --build build -j8 && ctest --test-dir build -L fast
              --output-on-failure && ./clang-format.sh --check`, measured at
              11.2 s with 6 of 6 green. Everything heavier is named in the
              step's `accepts:` field and run deliberately.
Rejected:     Including `-L slow` -- risks the timeout, which refuses completion
              as though the suite had failed, teaching everyone to distrust the
              gate. Leaving the gate unset -- honest, but it puts the whole
              green-suite requirement back on the agent alone.
Consequences: **A green gate is necessary and never sufficient.** A step whose
              `accepts:` names an SPRT or deep perft is not complete because the
              gate passed. This is the one place where the workflow could give a
              false sense of verification, so it is stated here and in
              `DEV_MANUAL.md`.

## DEC-011  2026-08-09  C++20 is the language, and it is not reopened
Tags:         language, architecture
Context:      The owner asked whether C, Rust or Zig would be a better absolute
              choice for the strongest possible engine. The question is fair:
              the `bitboard` branch this work is founded on followed a series
              written in C.
Decision:     By the owner after the analysis. C++, targeting `gnu++20`.
Rejected:     C -- no compile-time specialisation, so the templating in S002 and
              S004, worth 12.1 % and 11.7 % respectively, would be hand-written
              duplicates that drift. Rust -- SIMD intrinsics less mature, and the
              entire published body of chess-engine technique would need
              translating before it could be read, which fights DEC-014
              directly. Zig -- the same translation cost against a smaller
              ecosystem.
Consequences: The choice rests partly on SIMD intrinsics, a bet that pays at
              S029 and not before. Do not reopen without being asked.

## DEC-012  2026-08-09  The hand-written plan documents were reelaborated into the plan directories
Tags:         workflow, docs
Context:      `TMP_PLAN.md` (511 lines) and `EVAL_PLAN.md` (349 lines) held both
              the roadmap and the measurement record -- every SPRT verdict,
              every rejected change and the reason. Keeping them alongside
              `adocs/plan.md` would leave two documents each claiming to be the
              plan.
Decision:     By the owner. The roadmap became `adocs/plan.md` and 32 step
              files; the measurements moved into the step files they belong to,
              so a completed step is the record of what that change cost and
              bought; the reasoning that outlives any one step became the
              entries in this file. The two documents were then deleted, and git
              history keeps them.
Rejected:     Keeping them as retitled records -- two sources for the same
              numbers, and the step files would point outward instead of being
              readable on their own. Folding all of it into this file -- it is
              append-only and would have grown by roughly 800 lines of
              measurement narrative that could never be tidied.
Consequences: `adocs/plan_done/S0nn_*.md` is the primary record of what a
              completed change measured. `TOOLCHAIN.md` was unaffected: it is
              tooling reference, not a plan.

## DEC-013  2026-08-09  What chesso is, and what the achesso branch is for
Tags:         project, identity, scope
Context:      Chesso began as the owner's way to learn chess engine
              development, following published material including the Chess
              Programming Wiki. That produced a hand-written mailbox engine with
              no AI involvement, which is what `master` holds. The next stage
              moved to a bitboard board representation, following Maxim's
              "Bitboard chess engine in C" video series on the
              `@chessprogramming591` channel; the code is the owner's and AI was
              used only for debugging. That is the `bitboard` branch. Working
              that way, the owner concluded they were **reproducing most of what
              already exists**, which is a good way to learn and a poor way to
              arrive anywhere new.
Decision:     By the owner. A third branch, `achesso` -- *agentic chesso* -- to
              find out what AI-driven development can build. It starts from the
              owner's bitboard implementation, its test framework, and the
              fastchess SPRT scripts, and nothing else is inherited. **The goal
              is the strongest open-source chess engine in the world.**
Rejected:     Continuing on the `bitboard` branch by hand -- it works, and it
              was heading toward a competent reimplementation of published
              material rather than toward a strong engine. Starting from
              nothing -- the move generator, the test suite and the SPRT harness
              are the assets that make every later change measurable, and
              discarding them would keep the liability and throw away the asset.
Consequences: `master` and `bitboard` are the lineage and are not maintained
              here. The fixed chesso build in `~/.local/bin` is a mailbox build
              kept as an independent implementation to cross-check perft against
              and as a never-changing rung to measure absolute progress, since
              chained SPRTs give relative gains that need not add up. Everything
              in this branch is subject to DEC-002: the point is not to
              re-derive the published engine faster.

## DEC-014  2026-08-09  Follow the documented state of the art first; experiment second
Tags:         project, planning, strategy
Context:      Two ways to aim for the strongest engine. Start experimenting
              immediately, or first reach the level the published literature
              already describes and experiment from there. The published
              techniques are documented, they are known to work, and a strong
              engine is the only platform on which a novel idea can be
              *measured* -- an experiment against a weak baseline tells you
              nothing about whether it would help a strong one.
Decision:     By the owner. Phase one: build a genuinely strong engine by
              following documented online sources and articles, reading them for
              the idea and implementing it here (DEC-002 -- never by copying).
              Phase two: experiment, and look for ways to be strong that are not
              in the literature.
Rejected:     Experimenting first -- novel ideas evaluated against a weak engine
              produce numbers that do not transfer, which is the same failure as
              DEC-004 in the other direction. Following the literature forever --
              it caps the project at "a good implementation of what exists",
              which is precisely what DEC-013 was a reaction to.
Consequences: `adocs/plan.md` is phase one, and it is long: S017 to S029 are all
              documented technique. Phase two has no steps yet and should not
              get any until the engine is strong enough for an experiment to
              mean something. The transition is a decision to be recorded here
              when it happens, not a drift.

## DEC-015  2026-08-09  Tools everywhere, except training and tuning, which the owner runs
Tags:         project, tooling, nnue, tables, boundaries
Context:      The second foundation of this effort, alongside DEC-002, is
              rigorous measurement -- and measurement means external tools. But
              two activities are different in kind from analysis: training a
              network, and fitting the evaluation constants. Both consume large
              amounts of data and produce the artefact that *is* the engine's
              judgement, and the owner wants to run and own them.
Decision:     By the owner. The agent uses Stockfish and any other tool freely
              for position evaluation, game analysis, debugging, perft oracles,
              labelling, calibration and anything else that helps development.
              **The agent does not run NNUE training or evaluation-table fine
              tuning.** It builds the tooling, prepares the data, states what
              the run should be, and hands it over. The owner runs it when the
              time comes.
Rejected:     Letting the agent run tuning end to end -- it is the step where a
              provenance mistake becomes permanent and invisible, baked into
              numbers nobody can audit by reading. Forbidding the agent from
              touching tuning code at all -- it still has to build the tuner and
              the data pipeline, or S028 and S029 never start.
Consequences: S028 and S029 are split in practice: the agent delivers the
              tuner, the self-play data generation and the training program; the
              owner executes the run and the result comes back as constants or a
              network to be measured by SPRT like any other change. The line is
              *running* the training, not writing it. DEC-008 is the same
              boundary seen from the other side: tools for judgement, never the
              agent's own judgement, and never the agent's own training run.
