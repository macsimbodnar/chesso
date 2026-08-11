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

---

# Superseded on 2026-08-09 — the project was redefined

Everything above this line is **VOID**. It was written before the owner stated
what this project is, and it framed chesso as a chess engine that happens to be
worked on with an agent. That is backwards: `achesso` is an experiment in
AI-driven development that happens to produce a chess engine (DEC-013).

The entries are retained because this file is append-only and because
`plan_done/` cites their ids, and completed history is immutable. Each is listed
below with the entry that replaces it. A citation to a VOID id is not broken —
follow the pointer.

| VOID | superseded by | subject |
|---|---|---|
| DEC-001 | DEC-017 | the work is documented in the repository; `README.md` is the owner's |
| DEC-002 | DEC-016 | nothing is copied |
| DEC-003 | DEC-018 | evaluation is side-to-move relative |
| DEC-004 | DEC-019 | published Elo figures are direction, never prediction |
| DEC-005 | DEC-020 | every SPRT verdict is attributable to a commit |
| DEC-006 | DEC-021 | shrinking the attack tables is not worth doing |
| DEC-007 | DEC-022 | bad-capture ordering waits for capture history |
| DEC-008 | DEC-023 | chess judgement comes from a tool |
| DEC-009 | DEC-024 | the UCI surface is guarded |
| DEC-010 | DEC-025 | the automatic gate is the fast suite |
| DEC-011 | DEC-026 | C++20 is the language |
| DEC-012 | DEC-027 | the hand-written plans were reelaborated |

Nothing measured was lost in the supersession: every number in the VOID entries
is restated in the entry that replaces it, or in the step file it belongs to.

---

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
              on this branch is subject to DEC-016: the point is not to re-derive
              the published engine faster. This entry is why the whole file above
              it is VOID.

## DEC-014  2026-08-09  Follow the documented state of the art first; experiment second
Tags:         project, planning, strategy
Context:      Two ways to aim for the strongest engine. Start experimenting
              immediately, or first reach the level the published literature
              already describes and experiment from there. The published
              techniques are documented and known to work, and a strong engine
              is the only platform on which a novel idea can be *measured* -- an
              experiment against a weak baseline says nothing about whether it
              would help a strong one.
Decision:     By the owner. Phase one: build a genuinely strong engine by
              following documented online sources and articles, reading them for
              the idea and implementing it here -- never by copying (DEC-016).
              Phase two: experiment, and look for ways to be strong that are not
              in the literature.
Rejected:     Experimenting first -- novel ideas evaluated against a weak engine
              produce numbers that do not transfer, which is DEC-019's failure in
              the other direction. Following the literature forever -- it caps
              the project at "a good implementation of what exists", which is
              precisely what DEC-013 was a reaction to.
Consequences: `adocs/plan.md` is phase one, and it is long: S017 to S029 are all
              documented technique. Phase two has no steps yet and should not get
              any until the engine is strong enough for an experiment to mean
              something. The transition between phases is a decision to be
              recorded here when it happens, not a drift.

## DEC-015  2026-08-09  Tools everywhere, except training and tuning, which the owner runs
Tags:         project, tooling, nnue, tables, boundaries
Context:      The second foundation of this effort, alongside DEC-016, is
              rigorous measurement -- and measurement means external tools. But
              two activities differ in kind from analysis: training a network,
              and fitting the evaluation constants. Both consume large amounts of
              data and produce the artefact that *is* the engine's judgement, and
              the owner wants to run and own them.
Decision:     By the owner. The agent uses Stockfish and any other tool freely
              for position evaluation, game analysis, debugging, perft oracles,
              labelling, calibration and anything else that helps development.
              **The agent does not run NNUE training or evaluation-table fine
              tuning.** It builds the tooling, generates and prepares the data,
              states exactly what the run should be, and hands it over. The owner
              runs it when the time comes.
Rejected:     Letting the agent run tuning end to end -- it is the step where a
              provenance mistake becomes permanent and invisible, baked into
              numbers nobody can audit by reading. Forbidding the agent from
              touching tuning code at all -- it still has to build the tuner and
              the data pipeline, or S028 and S029 never start.
Consequences: S028 and S029 are split in practice: the agent delivers the tuner,
              the self-play data generation and the training program; the owner
              executes the run, and the result comes back as constants or a
              network to be measured by SPRT like any other change. The line is
              *running* the training, not writing it. DEC-023 is the same
              boundary seen from the other side: tools for judgement, never the
              agent's own judgement.

## DEC-016  2026-08-09  The agent never copies code or tables, and never trains on another engine's output
Tags:         provenance, licensing, nnue, tables
Supersedes:   DEC-002, which framed this as a licensing precaution
Context:      This is the rule the `achesso` branch exists to test. The owner's
              earlier work followed published sources closely enough that it
              stopped being interesting (DEC-013), and the point of building with
              an agent is to find out whether it can produce something rather
              than reproduce something. There is a plain licensing reason on top:
              most reference engines are GPL, and the owner wants no GPL question
              anywhere in this codebase or in a future network.
Decision:     By the owner. Published ideas, articles and techniques are used
              freely and are in fact the plan (DEC-014). **Source is never
              copied. Tables are never copied.** No NNUE training data derived
              from another engine's evaluation or search, ever.
Rejected:     Taking the PeSTO piece-square tables, which are published and tuned
              and would have made S010 an afternoon instead of a hand-written
              guess -- provenance, and it is exactly the copying that made the
              earlier branches feel pointless. Training on another engine's
              labelled positions, the shortest path to a working network -- the
              same objection, permanently baked into the artefact.
Consequences: The tables in `eval_tables.hpp` are hand-written and untuned, which
              is why S028 exists and is worth more than several S027 terms put
              together. S029 cannot start until self-play data exists. Running
              another engine's *binary* as a tool -- perft oracle, game analysis,
              calibration opponent -- creates no derivative work and is
              encouraged; DEC-023 and DEC-015 say where that line sits.

## DEC-017  2026-08-09  The work is documented in the repository, and README.md stays the owner's
Tags:         workflow, docs, moltke
Supersedes:   DEC-001
Context:      An agentic project has a specific failure mode: everything that
              matters lives in a session transcript the next agent cannot read.
              The owner must be able to open the repository months later and find
              what was decided, what was measured, and why the next step is the
              next step, without asking anyone.
Decision:     By the owner. The moltke workflow is adopted: `adocs/specs.md` for
              what must be true, `adocs/plan.md` and the step directories for
              what is being done and in what order, this file for why, and
              `adocs/testing.md` for what proves it. Nothing that matters may
              exist only in an agent's memory. `README.md` is written by hand by
              the owner and no agent writes in it; the developer-facing document
              the stock ruleset calls README is `DEV_MANUAL.md` here, and
              `MANUAL.md` is the end-user document. The override is written into
              `AGENTS.md` section 0 so an upgrade cannot silently erase it.
Rejected:     Keeping the project in hand-written plan documents -- they
              interleaved roadmap with measurement and could not answer "what is
              next" without being read end to end. Letting agents write
              `README.md` -- it is the one document that is the owner's voice.
Consequences: Three developer-facing documents with distinct owners: `README.md`
              (the owner), `DEV_MANUAL.md` (agent, developer), `MANUAL.md`
              (agent, end user). At step completion, "checked README.md,
              owner-written, no change needed" is expected and valid.

## DEC-018  2026-08-09  Evaluation is side-to-move relative
Tags:         evaluation, conventions
Supersedes:   DEC-003
Context:      `evaluate()` was White-relative and every call site multiplied by
              `(active_color == WHITE) ? +1 : -1`. A sign applied at the call
              site is a sign that can be forgotten at the next call site, and the
              symptom is an engine that plays one colour worse than the other.
Decision:     Proposed by the agent, accepted by the owner. Positive means the
              side to move is better. The sign is applied once, at the end of
              `evaluate()`, and callers apply none.
Rejected:     Keeping White-relative and auditing the call sites -- the audit has
              to be repeated for every future term, and there will be many.
Consequences: INV-5. Four test contracts were rewritten with the change rather
              than relaxed, including colour symmetry, which now expects the
              mirrored score to agree rather than negate, because `mirror_fen()`
              swaps the side to move along with the colours.

## DEC-019  2026-08-09  Published Elo figures are direction, never prediction
Tags:         measurement, planning
Supersedes:   DEC-004
Context:      The plan is to follow the documented state of the art (DEC-014),
              and it comes with numbers attached. Three have been taken at face
              value here and none survived contact with this engine. Staged move
              generation was quoted at 30-50 Elo and measured **0** (S006). SEE
              pruning in quiescence measured **0** (S015). Ordering captures into
              good and bad is reported around 150 Elo and measured **slower**,
              three separate ways (S025).
Decision:     Proposed by the agent from the measurements, accepted by the owner.
              Figures quoted from other engines are labelled "reported" and used
              to decide what to try and in what order. They are never used to
              predict a result and never to conclude one. The only number that
              counts is the one this engine measures on this hardware against its
              own previous commit.
Rejected:     Trusting the figures, which produced all three misses. The common
              cause is that a technique's value depends on the search around it:
              staged generation pays when the quiet moves above it are being
              pruned, and at the time this engine pruned nothing.
Consequences: Every step whose acceptance is Elo says "returns a verdict" rather
              than naming a target. A verdict of zero is recorded as zero and the
              feature may still be kept with the reason stated -- S005, S006 and
              S015 all were. It is also why S018 sits ahead of S019: this engine's
              own error distribution is a better guide to the next evaluation term
              than any published figure.

## DEC-020  2026-08-09  Every SPRT verdict is attributable to a commit, and the binary is snapshotted
Tags:         measurement, tooling, sprt
Supersedes:   DEC-005
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
              silent and produces a believable number, the worst combination.
Consequences: Every recorded verdict is attributable to a commit range.
              `REF=<sha> ./fastchess.sh` picks the baseline. This is what makes
              SPRT the deciding instrument of DEC-014 rather than a number that
              happens to be printed.

## DEC-021  2026-08-09  Shrinking the attack tables is not worth doing
Tags:         movegen, performance
Supersedes:   DEC-006
Context:      Fancy and black magics shrink the attack tables from 2307 KB to
              roughly 860 KB and are a standard, well documented optimisation --
              exactly the kind of thing DEC-014 says to follow.
Decision:     Proposed by the agent from the corrected profile, accepted by the
              owner. Not done. The profile does not implicate table size anywhere
              and `is_attacked` is 0.1 % of the workload. The real 2.5 MB problem
              was ergonomic -- the tables sat inside every `game_t` -- and S007
              solved that by sharing one copy, 2515 KB to 87 KB.
Rejected:     Fancy magics, black magics -- measure a cache-miss counter before
              spending a day on either. Callback enumeration instead of a move
              list, the single largest item in every published perft record --
              unavailable to a search, which must materialise moves to score and
              order them; adopting it optimises `bench_movegen` and pessimises the
              engine. Bulk counting at depth 1 in perft -- multiplies the headline
              number by about 6 and changes nothing about the engine.
Consequences: `bench_movegen`'s headline number is deliberately not comparable
              with published perft records. This is the clearest case of DEC-014
              in practice: the documented technique was read, understood, and
              measured against this engine's own profile rather than adopted
              because it is standard.

## DEC-022  2026-08-09  Bad-capture ordering is deferred until capture history exists
Tags:         search, move-ordering, measurement
Supersedes:   DEC-007
Context:      Splitting captures into good and bad and searching the bad ones
              after the quiets is where other engines report their SEE gain. It
              was implemented three ways here and every one was slower: exact
              `see()` in `score_move` +17.6 %, `see_ge` in `score_move` +13 %,
              lazily in the picker +3 %. Nodes fell where it fired, 2288701 to
              1870912 on one position, but the exchange calls cost more than the
              ordering saved.
Decision:     Proposed by the agent, accepted by the owner. Reverted. The rebuild
              instructions are kept in S025 so the next attempt does not re-derive
              them, and the retry waits for S023 and S024, which order captures
              without calling SEE at all.
Rejected:     Keeping it at +3 % on the argument that the Elo would show up in
              games -- this project does not keep a measured regression on an
              argument. Deleting the knowledge along with the code -- the trap
              below would then be rediscovered the hard way.
Consequences: S025 carries the trap that made the first attempt a silent no-op:
              the guard was `*score >= ORDER_CAPTURE`, but a losing capture scores
              `ORDER_CAPTURE` plus a *negative* victim term, so a queen taking a
              pawn sits at 999200 and the guard rejected exactly the moves it
              existed to catch. It looked like it worked and changed nothing.
              **Identical node counts exposed it; the timings did not.**

## DEC-023  2026-08-09  Chess judgement comes from a tool, never from the agent
Tags:         workflow, analysis, tooling
Supersedes:   DEC-008
Context:      A drawn game was analysed by an agent reading the move list. The
              analysis claimed the evaluation was two pawns too optimistic before
              move 62. Stockfish put the position at +196 against the engine's
              +1.95 -- agreeing to within five centipawns. The real defect was the
              opposite one, in the ten moves *after* that trade, and the same
              reading missed the four other moves that each cost more than a pawn,
              including the largest error in the game.
Decision:     By the owner, after the failure. No agent assesses a position, move,
              line or result from its own reasoning. This covers whether a position
              is winning, whether a move is a blunder and what should have been
              played, whether an ending is theoretically won, the material balance
              after a sequence, and whether an opening line is sound. Getting a
              position onto a board is itself a tool job.
Rejected:     Trusting the agent's reading with a caveat attached -- the failure
              mode is not uncertainty, it is confident and specific error. Reading
              a game finds the move you were already looking for.
Consequences: S016 built the tooling that makes the rule practical:
              `pgn_to_positions` puts the position on a board using the engine's
              own parser, `analyse_game.py` reports the cost of every move from
              Stockfish. What remains the agent's own is carved out explicitly:
              code, measurement, profiles, search behaviour and test design. The
              engine's own reported evaluation is data to quote, never a premise to
              reason from. This is the positive half of DEC-015: tools for
              everything except training and tuning.

## DEC-024  2026-08-09  The UCI surface is guarded, and the golden test is a plan step
Tags:         workflow, testing, uci
Supersedes:   DEC-009
Context:      Chesso's entire product surface is the UCI protocol, so
              `surface_guard` is `cli` and a golden test over that surface is
              required. Nothing of the kind exists: `test_uci.sh` is three lines
              running `fastchess --compliance` and is not in ctest. `test_engine`
              has "uci reports the options the GUI needs", which is close but does
              not fail when a command or option is added.
Decision:     Proposed by the agent, accepted by the owner. `surface_guard` stays
              `cli`. The missing test becomes S017 and is the first pending step
              rather than a waiver.
Rejected:     Setting `surface_guard` to `none`, valid only for a project with no
              checkable surface -- the opposite of a UCI engine whose every user
              reaches it through that protocol.
Consequences: Until S017 lands, every completion stamp asserts a check that
              nothing performs. S021 and anything else that adds a UCI option sits
              behind it. `MANUAL.md` is what the test holds to account.

## DEC-025  2026-08-09  The automatic gate is the fast suite; the real verification is per step
Tags:         workflow, testing, sprt
Supersedes:   DEC-010
Context:      The workflow runs one command at every step completion and refuses
              on failure, with a 600 s timeout. This project's actual verification
              is far larger: deep perft runs for minutes, the debug build re-runs
              the generator with `squares[]` and accumulator assertions on every
              node, and anything that alters play needs an SPRT costing about an
              hour.
Decision:     Proposed by the agent, accepted by the owner. The gate is
              `cmake --build build -j8 && ctest --test-dir build -L fast
              --output-on-failure && ./clang-format.sh --check`, measured at 11.2 s
              with 6 of 6 green. Everything heavier is named in the step's
              `accepts:` field and run deliberately.
Rejected:     Including `-L slow` -- risks the timeout, which refuses completion as
              though the suite had failed, teaching everyone to distrust the gate.
              Leaving the gate unset -- honest, but it puts the whole green-suite
              requirement back on the agent alone.
Consequences: **A green gate is necessary and never sufficient.** A step whose
              `accepts:` names an SPRT or deep perft is not complete because the
              gate passed. This is the one place the workflow could give a false
              sense of verification, so it is stated here and in `DEV_MANUAL.md`.

## DEC-026  2026-08-09  C++20 is the language, and it is not reopened
Tags:         language, architecture
Supersedes:   DEC-011
Context:      The owner asked whether C, Rust or Zig would be a better absolute
              choice for the strongest possible engine. The question is fair: the
              `bitboard` branch this work is founded on followed a series written
              in C.
Decision:     By the owner after the analysis. C++, targeting `gnu++20`.
Rejected:     C -- no compile-time specialisation, so the templating in S002 and
              S004, worth 12.1 % and 11.7 % respectively, would be hand-written
              duplicates that drift. Rust -- SIMD intrinsics less mature, and the
              entire published body of chess-engine technique would need
              translating before it could be read, which fights DEC-014 directly.
              Zig -- the same translation cost against a smaller ecosystem.
Consequences: The choice rests partly on SIMD intrinsics, a bet that pays at S029
              and not before. Do not reopen without being asked.

## DEC-027  2026-08-09  The hand-written plan documents were reelaborated into the plan directories
Tags:         workflow, docs
Supersedes:   DEC-012
Context:      `TMP_PLAN.md` (511 lines) and `EVAL_PLAN.md` (349 lines) held both
              the roadmap and the measurement record -- every SPRT verdict, every
              rejected change and the reason. Keeping them alongside
              `adocs/plan.md` would leave two documents each claiming to be the
              plan.
Decision:     By the owner. The roadmap became `adocs/plan.md` and 32 step files;
              the measurements moved into the step files they belong to, so a
              completed step is the record of what that change cost and bought;
              the reasoning that outlives any one step became the entries in this
              file. The two documents were then deleted, and git history keeps
              them.
Rejected:     Keeping them as retitled records -- two sources for the same numbers,
              and the step files would point outward instead of being readable on
              their own. Folding all of it into this file -- it is append-only and
              would have grown by roughly 800 lines of measurement narrative that
              could never be tidied.
Consequences: `adocs/plan_done/S0nn_*.md` is the primary record of what a completed
              change measured. `TOOLCHAIN.md` was unaffected: it is tooling
              reference, not a plan.

## DEC-028  2026-08-09  The surface guard enumerates commands and options, and pins go and position arguments by hand
Tags:         testing, uci, surface
Context:      S017 had to make the UCI surface checkable (DEC-024). Two parts of
              that surface are enumerable at runtime and two are not. The command
              set is an `unordered_map` in `chesso.cpp` that `uci_process_line`
              dispatches through, and the option declarations are the lines the
              `uci` reply prints, so a test can read both out of the running
              engine and a name that is added, renamed or removed cannot pass
              unnoticed. The `go` and `position` arguments are if-else chains
              inside their handlers -- there is no table to read, and an
              unrecognised token is silently ignored, so a newly added one has no
              observable effect a test can catch.
Decision:     Proposed by the agent, accepted by the owner. `tests/test_uci_surface.cpp`
              reads the commands from the new `uci_command_names()` and the option
              lines from the `uci` reply, and compares each against a golden list
              and against `MANUAL.md`. The `go` and `position` argument lists in
              that file are maintained by hand: a rename or a removal fails, both
              through the documentation check and through a behavioural probe, and
              a brand-new argument does not fail. `command_help` was changed to
              print `uci_command_names()`, which also makes its output sorted
              rather than in bucket order.
Rejected:     Rewriting `command_go` and `command_position` into dispatch tables so
              their arguments enumerate like the commands do. `uci_search_options_t`
              holds `int`, `uint64_t` and `bool` members with per-token ranges, so
              the table needs a variant or a member-pointer union, and three of the
              twelve tokens exist only to be ignored. That is a rewrite of live
              parsing code for a guard that is not the step's stated acceptance,
              and it is a drive-by refactor of the exact kind the house rules
              forbid. Grepping `chesso.cpp` from the test for `token == "..."`
              string literals -- exact today, and broken by the first reformat.
              Leaving the arguments out of the test entirely -- a rename would then
              silently break every GUI that sends it.
Consequences: Adding a `go` or `position` argument does not fail the suite. It has
              to be added to `expected_go_tokens` or `expected_position_tokens` and
              to `MANUAL.md` by whoever adds it, and the comment at the top of
              `tests/test_uci_surface.cpp` says so. If that turns out to be a real
              source of drift, the dispatch-table rewrite is the fix and it gets
              its own step. `uci_command_names()` is now part of the engine's
              public header and `help` output is sorted.

## DEC-029  2026-08-10  S018 profiles games against sgambetto, not the mailbox build
Tags:         measurement, s018, opponent
Context:      S017's successor step names "the fixed mailbox build in
              ~/.local/bin" as the opponent whose games are profiled. Measured,
              that build loses 6-0 to the current engine over a smoke match, and
              `~/.local/bin/chesso` advertises only `Use Book`, confirming it is
              the pre-bitboard engine. An error profile taken from those games
              is the error distribution of an already-won position, which is not
              the distribution that decides games. sgambetto, an engine written
              by a friend and available in the same directory, scored 8.5/20
              against the current build under the same conditions.
Decision:     Proposed by the agent, chosen by the owner. sgambetto is the
              opponent for the S018 run. 210 games were played at 10+0.2 and the
              current engine scored 39.3 % (+61 =43 -106), so the two are close
              enough that the games are fought rather than decided.
Rejected:     The mailbox build, for the reason above -- the step file's letter
              against its purpose. Self-play, which is balanced by construction
              and closest to what SPRT measures, but shares every blind spot
              between the two sides, so a weakness both hold is never punished
              and never appears as cost. Nodes-limited Stockfish, which is
              tunable and would give a lasting strength anchor, but is the same
              program as the judge, so the opponent and the referee would share
              an evaluation function. It stays on the table as a second opponent
              if the distribution needs a cross-check.
Consequences: The step file's `accepts` field is not satisfied literally and this
              entry is the reason. The finding this run produces has been shown
              on one opponent only; if S019 turns out to rest on the phase
              ranking, a second opponent is the check that it transfers, and
              DEC-019 is the history that says such checks are not optional.

## DEC-030  2026-08-10  The reference search is limited by nodes, never by depth
Tags:         measurement, tooling, stockfish
Context:      S016 analysed one game at a fixed depth of 18 and reported 159
              positions in about 47 seconds, which reads as 0.30 s per position.
              That figure does not generalise. Over 12 positions sampled at
              random from real games, depth 18 ran a median of 0.71 s, a mean of
              78.80 s and a maximum of 925.90 s: one position in twelve took a
              quarter of an hour. A budget built on the mean is meaningless and a
              budget built on the median is wrong by two orders of magnitude on
              the positions that matter.
Decision:     Proposed by the agent, accepted by the owner. Bulk analysis is
              limited by nodes. Measured over 20 sampled positions: 500000 nodes
              is 0.83 s mean, 1.08 s worst, median depth 18; 1000000 nodes is
              1.63 s mean, 2.11 s worst, median depth 21; 3000000 nodes is 4.84 s
              mean, 6.21 s worst, median depth 24. The S018 run uses 3000000.
              `--depth` remains available and is documented as unbudgetable.
Rejected:     Keeping fixed depth for consistency with S016 -- consistency with a
              measurement whose cost estimate was wrong by 100x is not worth
              having. A movetime limit, which bounds cost but makes the result
              depend on machine load and on how many workers are running, so two
              runs of the same file would disagree.
Consequences: Reference scores in this project are quoted with a node count, not
              a depth. The S016 figures were taken at depth 18 and are not
              directly comparable; the closest equivalent is 500000 nodes, which
              reaches median depth 18. Node limits are reproducible across
              machines, which fixed depth also is and movetime is not.

## DEC-031  2026-08-10  Profiling matches are adjudicated loosely, strength matches are not
Tags:         measurement, fastchess, adjudication
Context:      `fastchess.sh` adjudicates at `-resign movecount=3 score=400
              -draw movenumber=40 movecount=8 score=10`, which is right for an
              SPRT: it stops spending time on games whose result is already
              known. Reusing it for S018 was measured and produced 48 opening,
              61 early middlegame, 6 late middlegame and **0 endgame** moves out
              of 115. S019 exists because of an endgame evaluation defect, so the
              profiling run would have been blind to the phase it was for.
Decision:     Proposed by the agent, chosen by the owner. Profiling matches use
              `-draw movenumber=80 movecount=10 score=5 -resign movecount=8
              score=900 -maxmoves 200`. Measured over 10 games, this moved the
              median game from 50 plies to 166 and produced 238 late middlegame,
              289 endgame and 9 pawn endgame moves.
Rejected:     No adjudication at all, which is the most faithful and spends the
              machine on dead-drawn endings that contribute nothing. Keeping the
              SPRT settings, for the reason above.
Consequences: The two match kinds are not comparable to each other. Under the
              SPRT settings the engine scored 57.5 % against sgambetto over 20
              games; under the loose settings it scored 39.3 % over 210 with the
              same binary, `c7ea34b5`. Games played to completion are a different
              measurement, and any future comparison has to state which settings
              produced it. Whether that gap is the endgame defect or the small
              sample is exactly what the S018 profile is being run to find out,
              and it is not to be asserted before the numbers land.

## DEC-032  2026-08-10  Evaluation work is aimed by the measured error profile, not by the anecdote
Tags:         evaluation, measurement, s018, s019
Context:      S019 was written from one game. Stockfish at depth 18 found chesso
              holding +1.5 to +1.9 for ten consecutive moves in a king and pawn
              endgame it valued at +0.25, and the step that followed was called
              "endgame evaluation". S019's own text required that finding to be
              confirmed against a distribution before anything was written,
              because one game is an anecdote. S018 measured the distribution:
              13522 moves by chesso over 210 games against sgambetto, every
              position scored by Stockfish at 3000000 nodes.
Decision:     By the owner, on the agent's measurement. The profile is the basis
              for choosing evaluation work from here. What it says, over 407740
              centipawns given away:

              | phase | cp/move | share |
              |---|---|---|
              | opening 22-24 | 39.9 | 18.9 % |
              | early middlegame 14-21 | 44.1 | 36.0 % |
              | late middlegame 7-13 | 28.4 | 24.3 % |
              | endgame 1-6 | 18.3 | 20.5 % |
              | pawn endgame 0 | 6.8 | 0.3 % |

              The early middlegame is the largest pot on both measures. The
              endgame is the cheapest phase per move outside pawn endgames.
              Separately, chesso's own score against Stockfish's for the same
              position is optimistic in every phase, worst in the late
              middlegame at +100.2 mean, +55 median, +392 p90.
Rejected:     Keeping the endgame as the target because a step file already said
              so. That is the failure mode this project has recorded three times
              under DEC-019 -- acting on a number that was never shown to
              transfer. Treating the S016 game as refuted: it is not. It measured
              bias in one position class and the endgame p90 bias of +357 says
              such positions exist. It simply does not say where the centipawns
              go.
Consequences: S019 as written is aimed at the wrong phase and its content is
              reopened; what it becomes is a separate decision and is not taken
              here. The ranking survives removing the 1347 mate-touching moves
              and again removing every clamped reference score, so it is not an
              artefact of the +/-1000 clamp. It has been measured against one
              opponent, which DEC-019 says is exactly the condition under which
              a figure has previously failed to transfer; a second opponent is
              the outstanding check. Raw per-move records are kept, so any later
              question can be asked with `--from-raw` without replaying the 8.8 h
              run.

## DEC-033  2026-08-10  Evaluation is the binding constraint, so tuning comes before search
Tags:         evaluation, search, measurement, plan-order, s019, s027, s028, s033
Context:      DEC-032 established where the centipawns go by phase but not what
              kind of defect gives them away. A move can cost 200 cp because the
              engine did not see far enough, or because it saw far enough and
              liked the wrong position. The plan's order assumed the first: S019
              to S026 are search and ordering work, placed ahead of the
              evaluation terms in S027 on the argument that a better score at the
              leaves is worth less when the tree above them is the wrong shape.
              That argument was never measured.

              It is now. 160 positions were sampled from the S018 raw records
              where chesso gave away 100 cp or more with the game still
              undecided (reference score inside +/-300, no mate). Each was
              re-asked of chesso at 4000000 nodes, which is about its budget per
              move at 10+0.2, and at 64000000 nodes, which is 16 times that and
              more effective depth than any pruning package buys. Both answers
              were costed by the same reference the profile used: Stockfish
              dev-20260803-762dd1da at 3000000 nodes, DEC-030.

              | | cp/move |
              |---|---|
              | as played in the game | 204.7 |
              | at 4000000 nodes | 170.1 |
              | at 64000000 nodes | 129.1 |

              Sixteen times the search removes 24.1 % of the error, 10.3 cp per
              doubling. **95 of the 160 moves are unchanged at 16x**: the engine
              is not missing a refutation, it believes the move. Where the move
              does change, 65 of 160, cost falls from 186.0 to 84.9, so depth
              works where the horizon is the defect and is not the defect three
              times in five. 97 of 160 still cost 100 cp or more at 16x.

              Removed by 16x, by phase: opening 26 %, early middlegame 30 %,
              late middlegame 15 %, endgame 3 % (n=11, too few to lean on).

              Two supporting cuts of the same raw file: quiet moves carry 82.8 %
              of all loss and 1031 of the 1235 errors of 100 cp or more; the
              coupling between evaluation bias and move cost is weak, 18.2
              cp/move at bias under 25 against 40.7 at bias 300 or more, twelve
              times the bias for 2.2 times the cost.

              What the engine evaluates with, meanwhile, is material plus a
              tapered piece-square table and nothing else, with knight and
              bishop both at 300, no bishop pair, and tables that eval_tables.hpp
              itself calls a starting point and nothing more. None of it has ever
              been fitted to anything.
Decision:     By the owner, on the agent's measurement. The plan order changes.
              S028, fitting the evaluation constants that already exist to
              self-play outcomes, is promoted ahead of S019 to S026 and becomes
              the next step. S027's terms follow it and are fitted in the same
              way rather than hand-picked. The cheap search items stay in the
              plan and stay worth doing at about 10 cp per effective doubling,
              but they are bounded by that number and no longer lead.

              S019 is dropped. Its premise died twice: DEC-032 showed the endgame
              is the cheapest phase per move, and the measurement above shows
              endgame errors are the least depth-fixable but on 11 positions. Its
              content was always going to be S027's, and keeping two steps for
              one body of work is how the same term gets written twice. The step
              file is removed from plan_todo and its line from plan.md; the id is
              retired and never reused.

              S033 is created for reverse futility pruning, which the plan did
              not contain anywhere. S026 is "futility and razoring" and its goal
              line describes forward futility; reverse futility, also called
              static null move pruning, is a different technique and is the
              largest single search gain in the one public per-feature log
              found (Blunder, +57.1 +/- 16.9 self-play).
Rejected:     Keeping the search-first order. It rests on an argument that the
              measurement contradicts for this engine at this strength: the tree
              shape is worth about 10 cp per doubling and the leaf value is worth
              the 129 cp that four doublings did not touch. DEC-019 is three
              cases of exactly this, a plausible ordering that did not survive
              contact with a number.

              Going straight to S029. A network needs training data from an
              engine that already plays reasonably, and a tuned hand-crafted
              evaluation is that floor. It is also the only evaluation the owner
              can inspect when the network later disagrees with it.

              Writing S027's terms before tuning. An untuned term is measured
              against untuned tables, so its SPRT answers a question about two
              unfitted things at once, and a term that measures zero cannot be
              told from a term whose weight is wrong. Tuning what exists first is
              one change and makes every later term's verdict mean something.

              Retargeting S019 at the middlegame instead of dropping it. That
              makes it a duplicate of S027 under a different id.
Consequences: The next step is S028 and it ends in a handoff, not in a verdict:
              the agent builds the tuner, generates and prepares the self-play
              data and states the run; the owner executes the fit; the returned
              constants are then measured by SPRT. DEC-015 is unchanged and this
              is the first step it actually binds.

              The evidence behind DEC-032 and this entry now lives in
              adocs/data/ instead of a temporary directory. It was found in a
              session scratchpad under /private/tmp, which macOS purges: 8.8
              hours of reference search was one cleanup away from gone.
              tools/depth_vs_eval.py is the probe that produced the numbers
              above and re-runs them from the raw records.

              The measurement inherits every caveat of the profile it samples.
              One opponent, sgambetto, DEC-029. It samples only errors of 100 cp
              or more, which are 61 % of the total loss and not all of it. The
              4000000-node baseline starts from an empty transposition table
              where the game move had a warm one, so the in-game figure and the
              4M figure are not the same measurement; the 4M against 64M
              comparison, which is what the decision rests on, is internally
              consistent.

## DEC-034  2026-08-11  The owner delegated the S028 fit to the agent for one run
Tags:         tuning, workflow, dec-015

Context:      DEC-015 draws the line at *running* the fit: the agent builds the
              tuner, generates the data and states the run, and the owner
              executes it. On the night of 2026-08-11 the owner had roughly
              eight hours of idle machine and no intention of staying awake for
              it. The self-play data was already generated and the tuner already
              built and tested, so the only thing standing between the data and
              an SPRT verdict was a single command nobody was awake to type.

Decision:     The owner, asked which of three overnight plans to run, chose the
              one that starts with the fit, and then said "everything else is on
              you". The agent takes that as authorisation to execute this
              specific run: the exact command written in
              plan_current/S028_texel_tuning.md, on .tuning/selfplay_v1.tsv,
              with the stated hyperparameters and thread count unchanged.

              This is a one-run delegation, not an amendment. DEC-015 stands
              unchanged for every later fit and for the S029 network training,
              which is the case it was really written for. The agent states the
              crossing out loud rather than performing it quietly, and the run
              is reversible: the constants are worthless until an SPRT says
              otherwise, and the owner can discard them and re-run the fit
              themselves at no cost but the hours.

Rejected:     Waiting for the owner to type the command. Correct by the letter
              of DEC-015 and it spends the night doing nothing, which is the
              opposite of what the owner asked for.

              Treating the delegation as a general amendment to DEC-015. The
              owner authorised a night, not a policy. The reason DEC-015 exists
              -- that the owner wants the training runs of an engine bearing
              their name to be theirs -- is untouched by one Texel fit over
              constants the agent could equally have hand-written.

              Running with more threads than the step file states, to finish
              sooner and leave more of the night for the SPRT. Changing the
              stated run makes the recorded run and the executed run two
              different things, and float summation order is thread-dependent.
              The 25 % is not worth the discrepancy.

Consequences: S028's fit is executed by the agent this once and the fact is
              recorded here rather than left in a transcript. The constants that
              come back are measured by SPRT exactly as the step file requires,
              against e0c338e, and a verdict of zero is recorded as zero. If the
              owner reads this in the morning and disagrees, the constants are
              discarded and the fit is re-run by them; nothing downstream has
              been built on it by then.

## DEC-035  2026-08-11  The plan order survives the tuning, re-measured rather than assumed
Tags:         evaluation, measurement, plan-order, s027

Context:      DEC-032 and DEC-033 set the order of the entire plan. They put the
              evaluation ahead of the search on two findings: the early
              middlegame costs most per move, and sixteen times the search
              removed only 24.1 % of the error, so the engine believes the move
              it plays rather than failing to see the refutation. Both were
              measured on the hand-written constants that S028 has just
              replaced, and S028 was worth +188.74 Elo. A conclusion about
              where the error lives, drawn from an engine that no longer
              exists, is not evidence about the engine that does.

              So the same measurement was run again on the fitted evaluation:
              same opponent, same time control, same loose adjudication, same
              reference at the same 3000000-node limit. 98 games, 5582 profiled
              moves, against S018's 210 games and 13522 moves. The probe that
              produced DEC-033 was re-run on the new profile with the same
              criteria and the same sample size.

Decision:     The plan order stands. S027 is next, the search block follows it,
              and nothing moves. The agent ran the measurement and proposed
              this reading; the owner was asleep and has not yet seen it.

              What the numbers say. The phase ranking is unchanged: early
              middlegame first at 35.2 cp/move and 38.8 % of the loss, against
              44.1 and 36.0 % before. Per-move cost fell in every phase except
              the endgame, which is flat at 18.6 against 18.3 -- the tuning
              bought most of its Elo where most of the loss already was, and
              the endgame is the phase it moved least.

              The engine is still evaluation-limited. Sixteen times the search
              now removes 29.8 % of the error against 24.1 % before, and 78 of
              160 moves are unchanged against 95. So the balance did shift
              toward the search, but seventy per cent of the error still
              survives a sixteen-fold search, and the price of buying it is
              unchanged at 10.4 cp per effective doubling against 10.3. Neither
              figure is close to reversing the order.

              The systematic optimism is gone. The engine used to score every
              phase above the reference: +39, +77, +100, +42, +30. It now runs
              -1.5, -24.3, -40.3, -3.5 and +204.3. Medians are 6, 0, -4, -1 and
              227, so four of the five are tail effects rather than a uniform
              shift, and the p90 spread of 111 to 423 is still wide. The pawn
              endgame is the exception and it is 22 moves, which decides
              nothing on its own but is the one place the old warning survives
              and the one place it got worse.

Rejected:     Assuming DEC-033 still held because the change was an
              improvement. It is exactly the assumption this project has been
              wrong about often enough to have a rule against it, and the
              measurement cost four hours of an idle machine.

              Reordering the plan to put the search block first on the strength
              of 24.1 % becoming 29.8 %. The direction is real and the size is
              not decisive; 10.4 cp per doubling is what the search block is
              playing for either way, and S027 now has a tuner to fit its terms
              with, which is the whole reason it was put after S028.

              Profiling all 120 games. Cut at 98 for time. The lost 22 games
              would narrow the buckets and cannot plausibly move a ranking this
              wide, and the full PGN is kept so the run can be finished.

Consequences: The optimism entry in MANUAL.md is rewritten against these
              numbers instead of the S018 ones. specs.md carries both profiles
              rather than replacing one with the other, because the pair is the
              measurement of what the tuning did.

              S027 keeps its term list and its aim at the middlegame. The
              pawn-endgame bias is the one signal that argues for an endgame
              term, and 22 moves is not enough to act on: if it matters it will
              show up in the next profile, which is cheap now that the pipeline
              exists.

              The same caveats as DEC-032 apply and are not lessened by
              repetition: one opponent, one time control, one machine. DEC-019.

## DEC-036  2026-08-11  Recomputed mobility costs a third of the search, measured
Tags:         evaluation, performance, inv-4, s027, mobility

Context:      S027's first term is mobility and its step file says every term
              must go through the S014 accumulators. Mobility cannot. Material
              and the piece-square tables are sums over pieces of a function of
              one piece and one square, which is why eval_add_piece is four
              table lookups and why make_move can apply an O(1) delta. Mobility
              is a function of occupancy: move any piece and it changes for
              every slider whose ray crosses the from or the to square. There
              is no delta for add_piece, remove_piece or move_piece to apply.

              The step file's escape hatch does not exist either. It says the
              generator already produces the attack sets cheaply, but
              evaluate() is the first statement of every quiescence node,
              search.cpp:123, before is_check and before any generation, and a
              node that stands pat returns at search.cpp:135 without generating
              a move at all. There is nothing to reuse at the point the score
              is wanted.

              The literature does not accumulate mobility either. The
              Chess Programming Wiki calls only *safe* mobility expensive, and
              only "unless a program already keeps incrementally updated attack
              tables"; OliThink's whole evaluation is material plus mobility.
              What the literature does instead is reduce how often the
              evaluation runs: lazy evaluation with the cheap accumulated terms
              as the first stage, and caching the static score -- Arasan keeps a
              separate evaluation cache for exactly this rather than putting
              q-nodes in the transposition table.

Decision:     Measure the cost rather than argue about it, which is what the
              owner asked for. A probe was written -- mobility recomputed in
              evaluate() over knights, bishops, rooks and queens, own pieces
              excluded, tapered, placeholder weights -- measured, and reverted.
              Nothing from it is in the tree and its weights were never
              intended for play.

              tests/bench_eval was written to take the measurement and is kept.
              No existing tool could: a term that changes the score changes the
              tree, so search_bench at fixed depth prices the term and the
              search it caused together. bench_eval calls evaluate() and
              nothing else over ten positions from a full board to bare kings,
              and reports its own resolution the way bench_movegen does.

              The numbers, three interleaved passes, resolution 0.1 %:

                evaluate() today          1.31 ns per call, 762 M per second
                with recomputed mobility 15.93 ns per call,  62.8 M per second

              12.2 times the cost per call. In a real search at depth 12,
              nodes per second fell 32.8 % on kiwipete, 45.5 % on the midgame
              position and 23.7 % on the tactical one, and wall time to depth 12
              over the three rose 33 %. That is worse than the 25 % S014
              removed, and 0.58 of an effective doubling against the 10.4 cp per
              doubling DEC-035 measured the same day.

Rejected:     Shipping it and letting an SPRT decide the net. Defensible, and it
              would confound a term's value with a third of the machine; if it
              measured zero nobody could say which half was responsible. One
              change at a time.

              Concluding from the number that mobility is not worth having.
              Engines carry it while paying this cost, and 33 % of nps is a
              price, not a verdict. What the number settles is that the price
              must be seen before the term is chosen, not that the term is bad.

Consequences: The open question for S027 is no longer "can mobility be
              accumulated" -- it cannot -- but which of three ways to pay for
              it, and that is the owner's call:

                1. Ship it recomputed and SPRT the net, accepting the 33 %.
                2. Take the parked machinery first -- a static evaluation in the
                   transposition table entry and a quiescence eval cache -- so
                   evaluate() runs less often, then add the term. Standard, on
                   the parked list in status.md already, and it needs a decision
                   to become a step.
                3. Lazy evaluation: the accumulated terms as stage one, mobility
                   and king safety behind a margin. Cheapest in nps and the one
                   with a documented failure mode in sharp endgames.

              INV-4 is narrower than it reads. What S014 proved is that
              rebuilding material and the tables from the bitboards cost 25 % of
              nps. It did not prove that any recomputed term costs that, and
              until today no measurement here had priced one. bench_eval is the
              instrument for the five S027 terms that follow.

## DEC-037  2026-08-11  Recomputed mobility measured -14.93 Elo and is not kept
Tags:         evaluation, mobility, sprt, s027, negative-result

Context:      DEC-036 priced recomputed mobility at 12.2 times the cost per
              evaluate() call and a third of the nodes per second, and left
              three ways to pay for it as the owner's choice. The owner's answer
              was that nodes per second is not Elo and Elo is the only verdict:
              ship it recomputed and let an SPRT decide the net, fast bounds
              first and full bounds if that was not conclusive.

Decision:     It was conclusive. Not kept.

              REF=c8fe860 ./fastchess.sh --fast, 10+0.2, concurrency 3, the term
              uncommitted in the working tree against the same commit without
              it. **-14.93 +/- 16.44 Elo over 1164 games**, LLR -2.23 against a
              bound of -2.20, H0 accepted, LOS 3.72 %. 373 wins, 423 losses, 368
              draws, Ptnml [77, 129, 203, 113, 60]. Three and a half hours.

              The probe is reverted and nothing from it is in the tree. What is
              kept is the number, this entry, and tests/bench_eval, which was
              built to take the DEC-036 measurement and stays.

              Extending to the full bounds was considered during the run and
              rejected on arithmetic: for a negative true effect the log
              likelihood ratio drifts toward H0 faster the further away elo1 is,
              and the fast bound is +/-2.20 against the full run's +/-2.94. The
              fast configuration was already the quicker route to this verdict.

Rejected:     Reading this as "mobility is worthless". It is not what was
              measured. Two things were bundled into one candidate: four
              hand-picked weights, and a third of the machine. The weights were
              never fitted -- the tuner fits piece_value and the two tables and
              nothing else -- so a term with the wrong weight and a real cost is
              exactly what a -15 looks like. Engines carry mobility while paying
              this price.

              Stopping the run early once the trend was clear. It was 87 % of
              the way to the bound with a settled point estimate and it would
              have joined S013's LMR run in the parked list as a number nobody
              could cite. It was allowed to finish.

Consequences: S027's first term is unresolved rather than closed, and the two
              experiments that would resolve it are separable:

                1. Fit the weights. The model stays linear -- mobility counts
                   are a property of the position, not of the parameters, so
                   they enter as features. PARAM_COUNT 773 to 781, four mg and
                   four eg weights, one int16 count difference per piece type
                   per position, about 12 MB over the existing 1.49 M rows.
                   tools/tuner already links chesso_engine, so the magic tables
                   are there and no build change is needed. test_eval_model
                   must learn the term in the same change: it failed during
                   this probe, correctly, because the model did not know about
                   mobility.
                2. Make evaluate() run less often first -- a static evaluation
                   in the transposition table entry and a quiescence eval cache,
                   both already on the parked list in status.md. That discounts
                   the 33 % for mobility and for the five S027 terms after it,
                   and it is the answer the literature gives to this exact
                   problem rather than one specific to mobility.

              Order matters and is not settled here: fitting the weights against
              a baseline that still pays full price measures the weights, while
              the cache changes what every later measurement costs. The owner
              chooses.

              A verdict of zero, or worse, is recorded as zero. S005, S006 and
              S015 are the precedent and two of those features were kept anyway
              with the reason stated. This one is not kept.

## DEC-038  2026-08-11  Cut the cost of calling evaluate() before adding terms to it
Tags:         evaluation, search, plan-order, s027, s034, transposition-table

Context:      DEC-037 rejected mobility at -14.93 Elo while it was paying a
              third of the nodes per second, and the verdict bundled two causes:
              weights that were never fitted, and the cost. Two experiments
              separate them, and they were put to the owner. Fitting the weights
              measures the weights against a baseline still paying full price.
              Making evaluate() run less often changes what every S027 term
              costs, not just this one.

              evaluate() is the first statement of every quiescence node and a
              node that stands pat returns having done nothing else, so the
              evaluation is computed at the highest-traffic point in the search
              and the result is discarded every time. Quiescence never probes or
              stores the transposition table. Both a static evaluation in the
              table entry and a quiescence evaluation cache have sat on the
              parked list in status.md with no decision behind them.

Decision:     The owner chose the cost work first, and it becomes S034, inserted
              between S028 and S027. The agent proposed both options and
              recommended this order on the grounds that it discounts the price
              for six terms rather than one.

              **The step opens with a measurement that is allowed to end it.**
              The literature's premise is that calling the evaluation is
              expensive -- the Chess Programming Wiki says so, and Arasan keeps
              a separate evaluation cache for exactly that reason. Chesso's
              evaluate() is 1.36 ns, measured by bench_eval at resolution 0.1 %.
              A probe into a table big enough to be useful is a memory access,
              and one that misses cache is plausibly slower than 1.36 ns of
              arithmetic on values already in registers. If that holds, the
              cache is a pessimisation today and its value is conditional on an
              expensive term existing, which is circular: the expensive term was
              rejected for being expensive.

              That reasoning is an argument, and arguments have been wrong here
              three times (DEC-019), so it is settled with a number before any
              cache is built. A probe measured slower than the evaluation ends
              S034 with a recorded figure and moves the answer to lazy
              evaluation, which skips the expensive work rather than
              remembering it.

Rejected:     Fitting the mobility weights first. It measures the weights
              honestly but against a baseline paying full price, so a second
              negative would again bundle two causes.

              Building both the table-entry static evaluation and the
              quiescence cache as one change. Two caches at once and neither
              number is attributable. They are separate changes inside the step,
              each proving neutrality by node count before any timing.

              Putting quiescence nodes in the main transposition table. There
              are far more of them than useful entries and they would evict what
              the main search needs; Arasan's separate cache exists for that
              reason.

Consequences: S034 is created and started. S027 moves behind it and its term
              list is unchanged. Mobility stays unresolved rather than closed,
              and DEC-037 records the two experiments that would resolve it.

              A widened transposition entry is a behaviour change hiding in a
              speed change: the table is sized in entries rounded down to a
              power of two, so a wider entry means fewer entries for the same
              megabytes. Neutrality is proved by node count and best move before
              any timing is believed, INV-6.

              If S034 dies on its own opening measurement, that is a success
              costing an hour rather than a failure, and lazy evaluation gets a
              step of its own with the number as its justification.
