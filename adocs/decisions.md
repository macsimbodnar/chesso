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

## DEC-039  2026-08-11  Skip the expensive evaluation work rather than remember it
Tags:         evaluation, search, lazy-evaluation, s034, s027

Context:      S034 was created to make calling evaluate() cheaper, by keeping a
              static score in the transposition entry and a separate cache for
              quiescence. Its own opening measurement retired that plan. A probe
              is cheaper than the evaluation -- 0.36 ns into a 256 KB cache and
              0.80 to 1.08 ns into the 12 MB table, against 1.36 ns to recompute
              -- so the concern that killed it was not the price. It was the
              prize: a node costs about 116 ns, so the whole saving is under 1 %
              against a 3 % noise floor and no SPRT could resolve it.

              The same measurement found that tools/bench_eval understates an
              occupancy term by 3.9 times. Kiwipete searched 9095066 nodes in
              one build and 8860613 in the other, close enough to compare
              per-node cost directly: 115.7 ns against 172.1, a difference of
              56.4 ns per node where bench_eval reported 14.6 per call. The
              isolated loop keeps a slice of the magic tables hot; a real search
              walks 2 MB of rook attacks at random. Mobility was never costing
              14.6 ns. DEC-036 and DEC-037 both understated it.

Decision:     Presented with the options in plain terms, the owner chose to skip
              the expensive work rather than remember it. S034 is retargeted
              from caching to lazy evaluation and keeps its id: evaluate()
              splits into a cheap stage the accumulators already provide and an
              expensive stage that only runs when the cheap score is close
              enough to the window to matter.

              The owner also set the discipline for the terms that follow: one
              at a time, a fast SPRT after each, the full-bounds run when that
              is inconclusive, and the weights fitted rather than guessed. That
              is now what S027's accepts field means in practice.

              Lazy evaluation alone has nothing to skip, so it lands with
              mobility behind it, using the same hand-picked weights DEC-037
              measured at -14.93. The only difference between the two candidates
              is the staging, so the comparison attributes to the staging and to
              nothing else. Fitting the weights is the experiment after, kept
              separate so that a term, its price and its weights are never three
              unknowns in one number.

Rejected:     Keeping the 16-bit static evaluation in the transposition entry as
              an enabler for S033 and an improving flag. It is free in bytes and
              the agent recommended it, but the owner did not take it and
              nothing needs it yet. S033 can add it when S033 needs it, and a
              field added early is a field whose reason has to be remembered.

              Building lazy evaluation with nothing behind it. The staging costs
              a comparison and a branch and saves nothing while every term is
              cheap, so it would measure a small negative and teach nothing.

              Adding all six S027 terms and fitting them together. Fastest to a
              number and the number would be unattributable. The owner rejected
              this explicitly.

Consequences: The step's verdict is not INV-6 behaviour-neutral. Returning a
              stage-one score where the full score would have differed changes
              what the engine plays, on purpose, so node counts move and the
              identical-nodes route does not apply. It is judged by SPRT against
              HEAD and read against the -14.93.

              The margin is the risk and it is the familiar one: pruning that
              hides a mate is this project's recurring bug and both earlier
              instances were caught by the mate tests in the fast suite rather
              than by a benchmark. The eleven mate tests are the gate before any
              match is played.

              tools/bench_eval keeps its blind spot in writing. Any future term
              built on occupancy is priced in a real search, not in that loop.

## DEC-040  2026-08-11  Lazy evaluation is kept at a measured zero, as the platform for the weight fit
Tags:         evaluation, lazy-evaluation, mobility, s034, s027, negative-result

Context:      S034 built lazy evaluation and measured it with mobility behind
              it, on the same hand-picked weights DEC-037 measured at -14.93,
              so that the only difference between the two candidates was the
              staging.

              The staging works. Kiwipete at depth 12, with node counts close
              enough to compare directly, went 116.6 ns per node at the baseline
              against 172.1 with mobility always computed and 138.4 with it
              behind the shortcut: about 61 % of the term's cost removed,
              -32.8 % nodes per second becoming -14.8 %.

              The strength run does not resolve, and it will not. 498 games:
              +4.19 +/- 24.11 Elo, LLR +0.01 of +/-2.20, LOS 63.3 %, Ptnml
              [21, 61, 86, 53, 28]. An SPRT separates two hypotheses and this
              truth sits between elo0=0 and elo1=10, so the ratio random-walks.
              2500 games remained. Extending to the full bounds is slower, not
              faster, for an effect this close to zero.

Decision:     The run is stopped and recorded as a partial: **consistent with
              zero over 498 games, +/-24 Elo**. It is not a verdict and is not
              written down as one. S013's LMR run is the precedent for saying so
              out loud rather than letting a stopped run be read as a pass.

              The code is kept and committed, at a measured zero, for a stated
              reason: it is the platform the next experiment needs. Mobility on
              hand-picked weights is worth about nothing once it is no longer
              being paid for, and the only remaining unknown is the weights.
              Fitting them and measuring the fitted version against this commit
              isolates the weights and nothing else, which is the attribution
              the owner asked for. S005, S006 and S015 are the precedent for
              keeping a feature that measured zero with the reason stated.

              **If the fitted weights do not measure positive, this commit is
              reverted with them.** Lazy evaluation has no independent
              justification: it is machinery for making an expensive term
              affordable, and with no expensive term worth having it is a branch
              and a margin for nothing.

Rejected:     Running to 3000 games. Four to five hours for the same shrug, on
              the one machine that can measure anything.

              Leaving the work uncommitted while the fit runs. Hours of work in
              a dirty tree, and the fit needs the model changes that come with
              it.

              Committing only the tuner-side changes, which are behaviour
              neutral, and holding the engine changes back. The tests were
              re-anchored for mobility, so the two halves are not separable
              without a red suite.

Consequences: The engine now plays with mobility in its evaluation and a lazy
              shortcut in quiescence, on a measurement that says neither helps
              nor hurts. That is an unusual thing to ship and it is temporary by
              this entry's own terms.

              The margin is 150 and it is a guarantee, not an observation: the
              term is clamped to it, because the corpus contains a nine-bishop
              promotion position where the correction reached 155 and the new
              test caught it before it shipped. Any later term added behind the
              shortcut has to fit inside the same bound or move it, and
              test_evaluation "the lazy shortcut cannot change a decision" is
              what forces that to be deliberate.

              DEC-015 is unchanged: the fit that follows is the owner's to run.
              DEC-034 was one night and said so.

## DEC-041  2026-08-11  The agent runs tests and evaluation tuning without asking
Tags:         workflow, tuning, dec-015, delegation

Context:      DEC-015 stopped the agent short of running a fit: it built the
              tuner, generated the data, stated the run, and the owner executed
              it. DEC-034 crossed that line once, for one night, and said
              explicitly that it was not an amendment.

              In practice the rule cost more than it bought. Every fit and every
              measurement became a handoff, and the last one sat unstarted for
              an hour while the machine was idle and the agent had nothing to do
              but wait for a command that takes thirty seconds to type.

Decision:     The owner grants a standing delegation: the agent runs tests,
              measurements and evaluation tuning itself, without asking. No
              handoff, no waiting.

              Two conditions came with it. The machine has caffeinate running
              permanently on the owner's side, so the agent does not need to
              wrap anything or worry about sleep. And a job of several hours is
              scheduled for the night if there is better work to do in the
              meantime -- the machine is the binding constraint on the whole
              plan and daylight hours are worth more spent on work that needs a
              person awake.

              This supersedes DEC-015 for evaluation tuning and for every kind
              of measurement. **It is not extended to the S029 network training
              on the agent's own initiative.** DEC-015's stated reason was that
              the owner wanted the training runs of an engine bearing their name
              to be theirs, and a Texel fit over constants the agent could have
              hand-written is not the case that reason was written for. Training
              a network is. That one is asked again when S029 arrives.

Rejected:     Reading the delegation as covering everything DEC-015 covered.
              The owner said "tests or tuning"; the network is neither, it is
              the case DEC-015 exists for, and assuming otherwise would be the
              agent quietly widening its own permissions.

              Leaving DEC-015 in force and treating this as another one-off.
              It is the second time the rule has been suspended in a day, which
              is what a rule that does not fit looks like.

Consequences: CLAUDE.md and AGENTS.md both state the old rule in their own
              words and both are amended in the same commit as this entry, with
              a pointer here. DEC-015 is not deleted -- it is superseded in
              part, and the part about the network still stands.

              Long runs move to the night by default. The agent says what it is
              scheduling and why, rather than silently occupying the machine.

## DEC-042  2026-08-11  Matches use every performance core, and no efficiency core
Tags:         measurement, sprt, throughput, fastchess

Context:      fastchess.sh played on the performance cores minus one, leaving a
              core free for the operating system on the argument that a timed
              match on a busy machine measures the load. On this machine that is
              three of four, so a third of the throughput was being spent on
              quiet.

              It did not buy the quiet. The script's own load check fires above
              60 % of a core and it has fired on nearly every run today, at 39,
              69, 96 and 153 %, from opendirectoryd, the terminal, this session
              and a post-hibernation wake storm. The core was being left free
              for a machine that was busy anyway.

Decision:     The owner asked for all cores. The default becomes every
              performance core, four here, and CONCURRENCY still overrides it
              when a run has to share the machine with something else.

              **Efficiency cores stay out**, and that is the agent's reading of
              "all cores" rather than the literal one. They run at a different
              speed, so a game landing on one is played at the wrong speed, and
              which of the two engines gets hit is luck. That is not more data,
              it is data with a bias neither engine controls and nothing
              corrects. The distinction was stated to the owner rather than
              taken silently.

Rejected:     Using all eight cores. Above.

              Keeping one core free. The argument is sound in principle and
              false in practice on this machine; the measured load says the
              quiet was never there to protect.

Consequences: A verdict costs about a third less wall time, which matters
              because measurement capacity is the binding constraint on the
              whole plan and has been all day.

              Contention rises, and both engines pay it equally on an
              interleaved match, so it inflates variance rather than biasing
              the result. If runs start needing more games to reach a bound,
              that is the cost showing up and CONCURRENCY is the dial.

              The change applies from the next run. The fitted-weights SPRT in
              flight when this was decided keeps concurrency 3; restarting it to
              gain a third of the speed would have cost more than the third was
              worth with forty minutes left.

## DEC-043  2026-08-11  S027 adds its terms in the order the literature reports
Tags:         evaluation, s027, ordering, measurement

Context:      S027's step file listed king safety, passed pawns, pawn structure,
              bishop pair and tempo in the order engines generally report value
              in, and then argued against its own list. The argument for
              inverting it was that the cheap terms -- tempo, bishop pair, the
              rook file terms -- cost near nothing, so each SPRT would measure
              the term rather than the term minus a speed penalty, and the
              record of what a term is worth here would be built before anything
              expensive was paid for.

              Mobility is the evidence for that argument. On hand-picked weights
              and at full price it measured -14.93 Elo and looked like a
              failure; behind the lazy shortcut with fitted weights it measured
              +28.46. DEC-040.

              The step file left the choice open and said so: "The owner chooses
              when the step starts."

Decision:     The owner chose the literature order. King safety first, tempo
              last. The agent laid out three orders with the cost and the risk
              of each and recommended the cheap-first inversion; the owner took
              the other one.

Rejected:     Cheap terms first, then the pawn terms, king safety last. The
              agent's recommendation. Its case is above and it was not refuted,
              only outweighed: the largest expected gain lands first and
              resolves fastest under SPRT, and measurement capacity is the
              binding constraint on the whole plan.

              Pawn terms first, on the argument that the pawn hash is the one
              piece of new infrastructure the step needs and two term families
              would amortise it.

Consequences: The first term is the expensive one, so its verdict mixes the
              term's value with its speed cost. That is answered by measuring
              the cost separately and removing it before the SPRT rather than by
              reordering: king safety at zero weights measured 7.7 % against the
              build without it, and fusing its piece loop with mobility's
              brought that to 0.35 %, inside the noise floor.

              The remaining terms keep the order of the list. Each still gets
              its weights fitted before any verdict is believed.

## DEC-044  2026-08-11  King safety is linear in its weights, not a lookup curve
Tags:         evaluation, s027, king-safety, tuning, texel

Context:      The king safety the literature describes is not linear. The
              standard form accumulates a weighted count of attackers on the
              king zone and uses that count to index a table whose values grow
              faster than linearly, so two attackers are worth much more than
              twice one attacker.

              chesso fits its evaluation constants with the Texel tuner built at
              S028. That tuner exists because the evaluation is a linear
              function of its own constants, which is what makes a full pass
              over 1.49 M positions cost milliseconds instead of a million
              searches, and what makes the gradient closed form. An
              attack_table[weighted_count] term is not a linear function of the
              weights that build its index. It cannot be fitted by that
              machinery at all.

              Every term in S027 must have its weights fitted before its SPRT is
              believed -- DEC-033, and DEC-040 is what happens when they are
              guessed instead.

Decision:     King safety ships as nine count features per side, each with a
              middlegame and an endgame weight, tapered like every other term.
              Attackers by piece type, zone attack incidences, near and far pawn
              shield, open and half-open files by the king. Eighteen parameters,
              linear, fittable.

              Proposed by the agent, which supplied the constraint and the
              options. The non-linear form is not rejected on its merits; it is
              deferred until there is a fitter that can handle it.

Rejected:     The indexed attack curve with hand-picked weights and a
              hand-picked table. It is the form that is reported to work, and it
              would have gone into an SPRT unfitted. That is exactly the
              sequence that produced -14.93 for mobility and nearly closed the
              feature as a failure.

              Extending the tuner to fit a non-linear model. A gradient through
              a table index is a different optimiser and a different tool, and
              building it before knowing whether the linear form is worth
              anything is work on a guess.

Consequences: The individual weight signs are not interpretable and must not be
              read as chess statements. The attacker counts and the zone
              incidence count are collinear by construction -- a piece that adds
              one to a count adds several to the incidence total -- so the fit
              splits one effect across several parameters, exactly the way the
              piece values and the piece-square tables are degenerate by five
              dimensions. What is fitted is the sum.

              If the linear form measures at or near zero, that is a result
              about the linear form and not about king safety. Recording it as
              "king safety is worth nothing here" would be wrong, and the
              non-linear version becomes a candidate for phase two with a fitter
              to match.

## DEC-045  2026-08-12  evaluate_lazy returns the guaranteed bound, not the cheap score
Tags:         evaluation, search, lazy-eval, correctness, s027, s034

Context:      S034's lazy shortcut skips the expensive evaluation terms when the
              cheap score is already LAZY_EVAL_MARGIN clear of the window, and
              returned the cheap score when it did. The soundness argument
              written at the time was about the caller's decision: if
              cheap - 150 >= beta then the true score is above beta too, so the
              node fails high either way and the expensive stage would change
              nothing the caller does.

              That argument is correct and it is not the whole contract.
              quiesce() is fail soft. It returns stand_pat to its parent, which
              reads that number as a lower bound on the true score and may store
              it in the transposition table as one. The guarantee is only that
              the true score lies within a margin of cheap, so cheap can be up
              to 150 centipawns *better* than the truth. A lower bound that is
              not a lower bound.

              Found while applying S027's fitted king safety weights, when
              test_search "the table never changes the answer" began failing:
              the same position answered 110 or 59 depending on what was cached,
              because the shortcut makes the returned score depend on the window
              and the table changes windows. The defect predates king safety
              entirely. Mobility's corrections were too small to flip a score on
              those positions; the fitted king safety weights were not.

Decision:     The shortcut returns cheap - LAZY_EVAL_MARGIN at beta and
              cheap + LAZY_EVAL_MARGIN at alpha. Both are still on the caller's
              side of the window, so no cutoff changes, and both are true
              statements about the real score.

              It ships and is measured alone, before the king safety weights it
              was found under. A known defect in the tree contaminates every
              measurement taken after it, and two changes in one SPRT means
              neither number means anything.

              The owner was asked how to sequence this and was asleep; the agent
              took the option it had recommended and recorded the choice here.

Rejected:     Keeping the cheap score and narrowing the table invariant test to
              nodes the shortcut cannot reach. It would have shipped king safety
              tonight instead of tomorrow, and it accepts a wrong bound
              propagating through the tree in exchange. The strongest test over
              the transposition table would have lost coverage permanently.

              Shipping the fix and the king safety weights in one commit and one
              SPRT. Half the machine time and no attribution: a verdict of +5
              could be king safety at +15 and this at -10, or the reverse.

Consequences: The shortcut still makes the search score window dependent, and
              that is inherent to lazy evaluation rather than to this bug. "The
              table never changes the answer" is green again but it is now a
              weaker statement than it reads as -- it says this corpus at depths
              2 and 3 does not expose the impurity, not that there is none. A
              comment in the test says exactly that, and a future failure there
              should be read as "one of these two changed", not "the table is
              broken".

              Over the 2696 test positions the old form claimed a bound it did
              not have on 2613 of them, 1473 at beta and 1140 at alpha. That is
              the size of what was wrong, not a count of games lost -- what it
              cost in play is what the SPRT measures.

## DEC-046  2026-08-12  No pawn hash: it was built, measured, and recovers nothing
Tags:         evaluation, performance, cache, s027, passed-pawns, dec-039

Context:      S027's passed pawn term goes in stage one, where it is paid at
              every node. Written as a loop over each pawn with a table lookup
              per pawn it cost 10.8 % of a depth 12 search. Rewritten set-wise
              -- the enemy pawn set widened one file each way, Kogge-Stone
              filled, passers falling out as own_pawns & ~span -- it costs 4.5 %.

              4.5 % is above this project's 3 % noise bar with sigmas of 0.7 to
              1.0 %, so the standard answer was the standard one: cache the term
              on a pawn-only Zobrist key, which pawn structure would then share.

Decision:     The pawn hash was built in full -- key folded in add_piece,
              remove_piece and move_piece, restored by unmake through the same
              history entry the main key uses, 1024 entries of 16 bytes to stay
              in L1, whole 64-bit key stored, xored with the payload so a torn
              read degrades to a miss, and a debug assertion comparing every hit
              against a fresh computation.

              It is discarded. It recovers nothing: 1.892 s against the fill's
              1.897 s over 40 hyperfine runs each at sigma 0.023 to 0.026, and
              the sign of the difference flips between two passes run in
              opposite order.

              The term ships as the set-wise fill, at 4.5 %, uncached.

Rejected:     Tuning the table size until it wins. At an 89.8 % hit rate over
              15291002 probes the entire remaining headroom is a tenth of a 3 %
              term, and the probe already costs about what the fill costs. A
              size that won would be a size fitted to one benchmark.

              Keeping it anyway because it is standard. A cache is state: it can
              go stale, it is one more thing that has to be right for the
              evaluation to be right, and it is measured here to buy nothing.

Consequences: **DEC-039 repeated itself exactly and the tools said opposite
              things again.** bench_eval's ten-position loop says the cache
              recovers 61 % of the term -- 27.48 ns per evaluate() down to
              26.71, against 26.22 with no term -- because ten pawn structures
              sit in the table at a 100 % hit rate. The real search says it
              recovers nothing. An isolated benchmark cannot price anything that
              touches memory in this engine, and that is now the second time.

              The cache cost 2.2 % *with the term switched off*, which is the one
              difference in the whole exercise that was consistent across 40 runs
              a side. At zero weights clang folds the fill to nothing and
              evaluate_cheap() compiles to 19 instructions; it cannot fold a
              mutable global's probe, and the key maintenance in make_move is
              paid whatever the evaluation does.

              The pawn structure term that follows loses the cache it was
              expected to share, so its own cost is unpaid for and has to be
              measured on its own terms.

## DEC-047  2026-08-12  A term shipped at zero weights is deleted, not measured
Tags:         measurement, methodology, staging, s027, s034, compiler

Context:      S034 arrived at a staging discipline that S027 repeated: ship a new
              evaluation term with its weights at zero, prove that commit
              behaviour-neutral by identical node counts and best moves, then fit
              the weights and let the SPRT measure the fitted term rather than a
              guess about it. It is a good discipline and it produced +20.87 Elo
              for king safety.

              It carries a hole that went unnoticed for two terms. The weights
              are `const` with constant initialisers in the same translation
              unit, so the compiler folds every read of them to zero and deletes
              the arithmetic and the loops that feed it. evaluate_cheap()
              compiles to nineteen instructions with no loop in them whether or
              not the passed pawn term is present, and times identically.

              Every "the term is free" measurement taken at zero weights is a
              measurement of a build that does not contain the term.

Decision:     A term's cost is measured with its weights **forced non-zero**, in
              a throwaway build, and the numbers are recorded in the comment next
              to the weights so the next person does not re-derive them.

              Where two builds must be compared on wall time, they have to walk
              the identical tree. Nodes per second between builds searching
              different trees varies by +/- 8 % with tree size for the same
              binary, which is larger than every effect being measured here. The
              method that works is a cost-only build: compute the term at
              non-zero weights and discard the result behind an asm volatile
              barrier, so the search is node-for-node identical and wall time is
              the cost and nothing else.

Rejected:     Shipping terms with hand-picked non-zero weights so the compiler
              cannot fold them. That is what DEC-040 exists to stop -- mobility
              measured -14.93 Elo the one time a term was judged on guessed
              weights at full price, and +28.46 once they were fitted.

              Marking the weights volatile or otherwise defeating the fold in the
              shipping build. It would make the neutral commit non-neutral and
              cost speed in every build to serve a measurement.

Consequences: The behaviour-neutral commit is still worth making and INV-6 is
              still discharged by it -- identical node counts prove the term
              changes no decision. What it does not prove, and what it was
              casually read as proving, is that the term is cheap.

              King safety's recorded "0.35 % after fusion" was measured at zero
              weights and understates its cost. The +20.87 Elo verdict stands,
              because that was measured with the weights fitted and includes
              whatever the term really costs; only the intermediate figure was
              weaker than it read as.

## DEC-048  2026-08-13  Matches use every core, efficiency cores included
Tags:         measurement, sprt, throughput, fastchess, dec-042

Context:      DEC-042 set matches to every performance core and deliberately
              excluded the efficiency cores. The agent proposed that exclusion
              and stated the reason rather than taking it silently: a timed game
              landing on an efficiency core is played at roughly half speed, the
              scheduler picks which games those are, and that is not more data
              but data carrying a distortion neither engine controls.

              S027 then spent about twenty hours of machine time on six verdicts,
              13462 games. Measurement capacity has been the binding constraint
              on the plan all along and that step is what it costs in practice.
              Four terms remain unbuilt in the search block and every one of them
              needs at least one run.

Decision:     The owner asked for maximum cores and, asked directly whether that
              included the efficiency cores, said yes. `fastchess.sh` now
              defaults to every physical core -- eight here rather than four.

              This supersedes DEC-042 on the core count. Everything else DEC-042
              decided still stands, including that CONCURRENCY overrides the
              default and that the one free core it used to leave for the
              operating system bought quiet the machine never delivered.

Rejected:     Keeping performance cores only. The agent recommended it and the
              argument is unchanged and was not refuted: a game at the wrong
              speed measures something other than the change. It was outweighed
              by throughput, knowingly, and the tradeoff was stated before the
              choice was made.

Consequences: Verdicts get faster and noisier. The distortion falls on both
              engines on average because the assignment is the scheduler's, so
              it inflates variance rather than biasing the result -- a run needs
              more games to reach the same bound, and how many more is not known
              yet. Watch the games-to-verdict figures against S027's, which were
              1300 to 3000 at four cores, and record what actually happens
              rather than assuming this was free.

              **A verdict measured at eight cores is not directly comparable to
              one measured at four**, so the S027 numbers keep their conditions
              attached wherever they are quoted.

              If a result ever needs to be as clean as this setup can make it,
              `CONCURRENCY=4` is the way back and the reason should be recorded
              with the run.

## DEC-049  2026-08-13  GCC is the reference compiler on the Linux machine
Tags:         toolchain, measurement, portability, build, dec-048

Context:      Work moved to a Linux machine -- Pop!_OS 24.04, i7-8700K, 6 cores
              and 12 threads. Apple clang, the compiler every recorded figure
              before this point was built with, does not exist here.

              The tree did not compile at all with the distribution default.
              `g++ 13.3` under `-Werror` rejected four sites: an enum mixed with
              `uint8_t` in a ternary (`src/bitboard.cpp`), a zero-length
              `printf` format (`tests/bench_eval.cpp`), and two range-for loops
              binding `const std::string&` to a temporary built from `const
              char*` (`tests/test_engine.cpp`, `tests/test_movegen.cpp`). All
              four are portability defects, not gcc pedantry, and none of them
              was visible under Apple clang.

              TOOLCHAIN.md carries a standing warning that pointing CMake at a
              different compiler makes every recorded benchmark incomparable.
              The agent raised it and did not decide it.

Decision:     The owner chose the distribution default, `g++ 13.3`, as the
              reference compiler on this machine.

              The owner's reasoning, recorded because it narrows the TOOLCHAIN.md
              warning rather than overriding it: comparability is a property of
              a (machine, compiler) pair, not of a compiler. A comparison made
              on one machine with one compiler is sound. The warning bites only
              when a figure is carried across machines, or across compilers on
              one machine, and neither is done here.

              The owner also counted the switch as a positive: a different front
              end reports defects the previous one did not. The four `-Werror`
              sites are that argument's evidence, found on day one.

Rejected:     Clang 22 as the reference. It is installed and is the closer
              relative of Apple clang, so it would have kept the old numbers
              nominally comparable -- which is worth nothing once numbers are
              not carried across machines at all. It also cannot link as
              installed (`cannot find -lstdc++`; it resolves gcc-14's directory
              while only gcc-13's libstdc++ is present). Kept as a second front
              end for `clang-tidy-22` and for a second opinion on warnings, not
              as the build compiler.

Consequences: Every figure recorded before this entry was measured on Apple
              silicon under Apple clang and is not comparable to anything
              measured here. Those figures keep their conditions attached
              wherever they are quoted -- the same discipline DEC-048 applies to
              core counts, for the same reason.

              The baseline restarts here. `bench_movegen`, `bench_eval` and
              every Elo verdict get re-established on this machine before any of
              them is compared against.

              Changing compiler on this machine again is a decision and an entry
              here, never a build flag, because it restarts the baseline a
              second time.

              TOOLCHAIN.md documents the macOS toolchain throughout -- Homebrew
              paths, `dsymutil`, `xcrun --show-sdk-path`, `-mcpu=apple-m1` -- and
              is stale on this machine until rewritten.

## DEC-050  2026-08-13  Every parallel tool uses all 12 hardware threads here
Tags:         measurement, concurrency, toolchain, dec-048, dec-049

Context:      DEC-048 set match concurrency at "every core the machine has" on
              hardware whose extra cores were efficiency cores. This machine's
              extra cores are not a second core type: `nproc` reports 12 for 6
              physical cores with SMT, so two games land on one physical core
              rather than one game landing on a slower core.
              `.moltke.local.md` recorded the policy for it as undecided, and
              `fastchess.sh` had been defaulting to 12 without one.

              The rest of the toolchain still carried numbers written for the
              Apple machine: `-j8` in the step gate, in `DEV_MANUAL.md` and in
              `TOOLCHAIN.md`, and a compiled-in `--threads 3` default in both
              `tools/datagen.cpp` and `tools/tuner.cpp`.

Decision:     The owner: on this machine fastchess and every other tool that
              multitasks uses all 12 hardware threads. This closes the open
              question in `.moltke.local.md` in favour of DEC-048's reasoning --
              measurement capacity is the binding constraint on the plan --
              extended from a second core type to SMT siblings.

              `datagen` and `tuner` now default to
              `std::thread::hardware_concurrency()` rather than to a literal,
              so the policy holds on the next machine without another edit.
              `fastchess.sh` already reached 12 through its `|| nproc` fallback
              and needed no change; its comment did.

Rejected:     Six, one game per physical core. It is the cleaner measurement --
              no two games sharing an execution unit -- and it is what DEC-042
              would have chosen. Rejected for DEC-048's reason and available as
              `CONCURRENCY=6` when a single result has to be as clean as this
              machine can make it.

              A literal 12 in the two tools. Same defect class as the
              `mktemp -t` and `sysctl -n hw.logicalcpu` lines S035 had just
              removed from `fastchess.sh`: a machine's number written into a
              file that travels.

Consequences: Two games share a physical core throughout a match, so a game's
              speed depends on what its sibling thread is doing. As with
              DEC-048 the scheduler hits both engines about equally, so this
              inflates variance rather than biasing the result, and a run may
              need more games to reach its bound. Nothing corrects it.

              A tuner fit at 12 threads is not bit-identical to one at 3: the
              per-thread gradient partials are summed in a different order.
              Weights differ in the last digits, which is noise against what a
              fit is read for, but a fit is not reproducible across a thread
              count.

              The S027 verdict costs -- 3 to 4.5 hours, measured at four cores
              on the Apple machine -- carry to nothing here. What a verdict
              costs on this machine is unknown until the first runs report it,
              and DEC-049 already says the same about every other figure.


## DEC-051  2026-08-13  Tool config is tracked when it describes the project
Tags:         git, tooling, agents, repo-layout

Context:      `.cursor/rules/moltke.mdc` was tracked and `.claude` was ignored
              wholesale, which read as an inconsistency and prompted the
              question of which one was the leftover. Neither was. The two
              directories arrived by different routes: `.cursor/rules/moltke.mdc`
              was scaffolded deliberately in `b6ef5c4` when the moltke workflow
              was adopted, while the `.claude` line in `.gitignore` predates
              moltke entirely -- it was added in `bf80449`, a tapered-evaluation
              commit, to keep one machine-local permissions file out.

              The apparent asymmetry was an artefact of where each tool reads
              its pointer. Both agent pointers are eight lines aimed at
              `AGENTS.md` and both were already tracked: Claude Code reads its
              one from the repository root as `CLAUDE.md`, and Cursor requires
              its one under `.cursor/rules/*.mdc`. Nothing was missing.

              What was wrong was the width of the ignore rule. `git check-ignore`
              confirmed `.gitignore:8:.claude` swallowed `.claude/settings.json`
              and `.claude/skills/` along with the local file it was aimed at, so
              shared Claude configuration added later would have been invisible
              to git with no warning -- the same silent-omission class as the
              stale figures DEC-049 had to go back and mark.

Decision:     Tool configuration is tracked when it describes the project and
              ignored when it describes a machine. The owner removed the
              `.claude` line; the agent supplied the analysis and the narrow
              replacement, `.claude/settings.local.json`.

              Tracked: `AGENTS.md`, `CLAUDE.md`, `.moltke.json`,
              `.cursor/rules/moltke.mdc`, `.vscode/`. Ignored:
              `.claude/settings.local.json`, which holds this machine's
              permission grants as absolute `/tmp/claude-1000/` paths carrying a
              session UUID and is meaningless anywhere else.

              The narrow line is stated in the repository's own `.gitignore`
              rather than left to the global one at `~/.config/git/ignore`, which
              is where it was actually being caught once the blanket line went.
              A rule that only holds on one developer's machine is not a rule the
              repository has.

Rejected:     Ignore `.cursor` as well, for symmetry. It is project
              configuration: a Cursor session with native `AGENTS.md` reading
              turned off gets no ruleset at all without it, and on this branch
              the ruleset is load-bearing rather than decorative. The same
              argument already keeps `.vscode/` tracked.

              Leave `.gitignore` bare and rely on `~/.config/git/ignore`. It
              works here and only here; a second machine or a contributor would
              see the local file as untracked and could commit it. DEC-049 is
              the standing reminder that this repository now spans more than one
              machine.

              Add a shared `.claude/settings.json` while the directory is being
              sorted out. Not taken and not proposed as a step: a checked-in
              permission allowlist pre-approves tool calls for anyone who clones,
              which is a trust decision and not a tidiness one. The directory is
              now visible to git if that is ever wanted.

Consequences: `.claude/settings.json`, `.claude/skills/`, `.claude/commands/` and
              `.claude/agents/` are now visible to git, so adding one is a
              deliberate act rather than a silent omission. `DEV_MANUAL.md`'s
              layout table states the rule, so the next tool directory --
              whichever assistant it belongs to -- is classified by what is in it
              rather than by which tool wrote it.


## DEC-052  2026-08-13  VS Code's CMake Tools is kept out of the measured build directory
Tags:         tooling, measurement, build, vscode, contamination

Context:      Found while running the step gate for S036, twice, with a green
              run in between: `ctest -L fast` took 126 s instead of 18 and
              `test_movegen` and `test_search` blew the 60 s timeout every
              `fast` target carries. Neither is a slow test. `build/` had been
              reconfigured underneath the session.

              `build/CMakeCache.txt` was written at 17:23:22 and the `code`
              process started at 17:23. `~/.local/share/CMakeTools/log.txt`
              carries the command: `-DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc-14
              --no-warn-unused-cli -S /home/max/ws/chesso -B
              /home/max/ws/chesso/build -G Ninja`. The extension defaults
              `cmake.buildDirectory` to `${workspaceFolder}/build` and
              `cmake.configureOnOpen` to true, so opening the project points its
              own Debug configuration at the one directory `DEV_MANUAL.md` calls
              "Release, the one that gets measured".

              Two separate contaminations, not one. The build type went to Debug,
              which is what the timeouts were. The C compiler went to `gcc-14`,
              against the `g++ 13.3` DEC-049 pins as the reference compiler on
              this machine -- and DEC-049 records that gcc-14 cannot even link
              here (`cannot find -lstdc++`). `DEV_MANUAL.md` states the stake
              directly: different compiler, different codegen, every recorded
              benchmark incomparable.

              Nothing announces either. An unoptimised binary produces numbers
              that look like numbers, and the only signal is two timeouts that
              read as test failures rather than as a build directory that
              changed underneath you. Foundation 2 of `CLAUDE.md` is that
              nothing is believed without a measurement; this silently
              invalidates the measurement instead of the belief.

Decision:     `.vscode/settings.json`, which is tracked, pins
              `cmake.buildDirectory` to `${workspaceFolder}/build-debug` and sets
              `cmake.configureOnOpen` and `cmake.configureOnEdit` to false. The
              extension keeps working and lands its Debug configuration in the
              directory this project already reserves for asserts-on builds. The
              agent found it and proposed the fix; the owner had asked for the
              tool-config question to be settled.

              `build/` was deleted and reconfigured from the command
              `DEV_MANUAL.md` documents, so the injected `gcc-14` is gone:
              Release, `/usr/bin/c++` at g++ 13.3.0, `/usr/bin/cc`. The
              generator went from Ninja to Unix Makefiles as a consequence --
              the documented command names no generator and the Ninja came from
              the extension. Same compiler and same flags, so nothing measured
              is affected.

Rejected:     Leave it and remember to check. It was not noticed for a full gate
              run and the second occurrence was 25 minutes after the first. A
              hazard whose only symptom is two timeouts is not one a habit
              catches.

              Have CMake refuse to configure `build/` as anything but Release.
              It would also refuse the debug and profiling directories unless
              the check knew their names, which puts the directory layout into
              `CMakeLists.txt` where it does not belong.

              Uninstall or disable the extension. It is the owner's editor and
              the C++ tooling around it is wanted; the defect is where it builds,
              not that it builds.

Consequences: A contributor opening this repository in VS Code no longer has
              their editor rewrite the measured build directory, because the
              setting is tracked rather than machine-local -- which is DEC-051's
              rule applied the same day it was written. `build-debug` is now
              shared between the extension and anyone configuring it by hand, so
              its build type is whatever was set last; it carries no measurements
              and `DEV_MANUAL.md` already calls it the asserts-on directory.

              Any measurement taken from `build/` between 17:23 and this entry
              was on an unoptimised binary. None was: S036 is a correctness step
              and the only numbers it recorded are node counts, depths and the
              suite's own wall clock, all of them re-taken green afterwards.


## DEC-053  2026-08-13  The tuner-model guard's slack is raised to the truncation bound, not earned back
Tags:         evaluation, tuning, testing, measurement, truncation

Context:      `tests/test_eval_model.cpp` asserted that `tools/eval_model.hpp`
              reproduces `evaluate()` to within 2.0 cp, justified by a comment
              saying `evaluate()` truncates "twice, once for the tables and once
              for mobility". The 2026-08-13 adversarial audit (F04) measured
              3446 of 200000 real corpus positions past that bound, worst
              exactly 2.875. The test passed only because its corpus was 26
              hand-picked FENs, every one of which happens to divide evenly
              enough to agree within 2.

              The comment was wrong and so was the audit's correction of it.
              `evaluate()` divides by `GAME_PHASE_MAX` **four** times, not two
              and not the three F04 counted: `src/evaluation.cpp:640` the
              piece-square pair plus the pawn terms, `:668` tempo, `:951`
              mobility, `:953` king safety. F04 missed tempo, which
              `src/evaluation.cpp:647-670` documents at length as a deliberate
              second truncation. Three of the four can round today because
              tempo ships at `tempo_mg == tempo_eg == 0`
              (`src/evaluation.cpp:582-583`), so its division truncates 0 / 24
              exactly. That puts the bound at 3 x 23/24 = 2.875, which is why
              F04's measured worst case was exactly 2.875 and not 3.833: the
              arithmetic and the measurement agree, on a count of divisions
              neither document had right.

              This guard is the only thing stopping the model drifting from
              `evaluate()`, and a fit is only as good as that correspondence.
              Two routes existed, both of which make the guard honest.

Decision:     Raise the slack to 3, correct the comment to name all four
              divisions with their lines and to state which one is inert and
              why, and pin the four FENs F04 recorded into the test corpus so it
              stops being 26 positions chosen for feature coverage. The
              one-division merge becomes S055, measured on its own SPRT, placed
              after S042 and before S029 -- the tighter bound protects fits of
              the hand-crafted evaluation and S029 is where the network takes
              over from the HCE as the thing being fitted.

              **Chosen by the owner** from the two routes and their costs as
              supplied by the agent (AGENTS.md section 8). The agent also
              re-derived the division count, which is how the audit's three
              became four.

              A new case, "the pinned positions reach the truncation bound",
              asserts that each pinned position still disagrees by more than the
              old 2.0 and that one of them still reaches 2.8, so the corpus
              cannot quietly stop exercising the bound the tolerance claims. It
              also asserts the tempo weights are zero, because that premise is
              what makes the bound 2.875 rather than 3.833.

Rejected:     Merge the taperings now, in this step. It spends a three to four
              and a half hour SPRT slot on a change whose expected verdict is 0
              -- pure rounding of at most 1 cp per position -- and measurement
              capacity is the binding constraint on the whole plan. It also
              bundles a behaviour change into a guard-honesty fix, against
              one change at a time: with both in one commit, neither the verdict
              nor the tolerance means anything on its own.

              Raise the slack to 3 and never revisit. It concedes the saved
              integer division and, more importantly, the tighter bound,
              permanently. The bound is the part worth having.

              Set the slack to 4, the general bound over all four divisions. It
              is a centipawn of slack that today's code cannot produce, since
              tempo's division is provably inert at zero weights, and a guard
              is worth what it refuses.

Consequences: Until S055 lands the guard cannot separate rounding from a real
              model error of up to about 2.8 cp per position, which over a
              1.49 M position fit is a systematic bias the fit would absorb
              silently. That is the cost of the route taken and it is why S055
              exists rather than being dropped.

              The pinned corpus removes the test's dependence on gitignored
              `.tuning/`, which does not exist on the DEC-049 machine at all --
              F04's own reproduction cannot be re-run here, and the four FENs
              are now the record of it inside the suite.

              Fitting tempo to anything non-zero raises the bound to
              4 x 23/24 = 3.833 and requires the tolerance to go to 4. The test
              asserts the premise rather than trusting the comment, so that
              change fails loudly with the arithmetic in the failure message.

              Two comments in `src/evaluation.cpp` (`:638-639` "one-centipawn
              slack", `:662` "test_eval_model allows two") are now false and
              were left alone: S038's `excludes:` forbade touching the file so
              that the evaluation's output stayed bit-identical. S055 owns them.


## DEC-054  2026-08-13  NNUE is deferred; strength comes from search and the hand-crafted evaluation
Tags:         nnue, evaluation, search, planning, s029, dec-015, dec-041, dec-033

Context:      `plan.md` positions S029 as the endpoint of the evaluation work:
              a perspective network trained on chesso's own self-play, with the
              tuned hand-crafted evaluation as "the floor that generates
              training data". It is also the one step the agent does not
              finish -- DEC-015 as amended by DEC-041 leaves running the
              network training with the owner, and S029's own `excludes:` says
              so in those words.

              The whole plan list is phase one, reaching the level the
              published literature already describes (DEC-014), and in that
              reading the network is where the hand-crafted evaluation stops
              being the thing that is fitted.

Decision:     **By the owner**: "no NNEU Training. Focus on building a better
              and stronger engine without NNUE for now." NNUE is deferred and
              effort goes to search and to the hand-crafted evaluation.

              S029 is **parked, not retired**. Its step file stays in
              `plan_todo/`, its id is not reused, and its list entry moves to
              the end of the pending order carrying the word parked, so nothing
              derives it as the next step. Resuming it is a decision, not a
              drift.

Rejected:     Retire S029 outright, the way S019 was retired. The work may
              resume, and DEC-014's phase ordering still argues that a network
              is where a tuned hand-crafted evaluation leads; retiring it would
              throw away the architecture, the accumulator plan and the data
              pipeline already written down in the file.

              Leave it in the order unmarked. The next step is derived from
              `plan.md` order and from nothing else (AGENTS.md section 1), so
              an agent would eventually start it, and DEC-015 as amended by
              DEC-041 reserves the training run to the owner -- the step would
              stall mid-flight, with the data generated and no one able to run
              the thing it was generated for.

Consequences: The parked "standard search machinery with no step behind it"
              list is now where the next steps have to come from rather than a
              sideline. `status.md`'s Parked block and `specs.md`'s open item
              both carry it: late move pruning, check and singular extensions,
              a quiescence transposition probe, a static evaluation in the
              table entry, an `improving` flag, history malus and ageing,
              correction history. Parked still means a step is created by a
              decision, and this entry creates none of them.

              S055 keeps its value and loses its deadline. Its ordering
              argument was that the tighter model-guard bound is worth more
              before S029, because S029 is where the network takes over from
              the hand-crafted evaluation as the thing being fitted. With S029
              parked the hand-crafted evaluation stays the thing being fitted
              indefinitely, so the bound matters for longer rather than less.

              DEC-033's finding is now answered by hand or not at all. 160
              expensive moves re-asked at 16 times the search removed 24.1 % of
              the error and left 95 of 160 moves unchanged, so the engine is
              evaluation-limited on the errors that decide games -- and the
              answer to that is hand-crafted terms and fits, not a network.

              Phase one loses its stated endpoint for the evaluation work.
              Nothing else moves: the delegation boundary in `specs.md`'s
              non-goals is unaffected, because it says who runs a training run
              and not whether one happens.

## DEC-055  2026-08-14  Regenerate the tuning corpus tonight, with the tactical filter clause loosened
Tags:         tuning, datagen, corpus, evaluation, planning, s065, s066, s033,
              dec-016, dec-019, dec-041

Context:      There is no tuning corpus on this machine. Audit finding
              `2026-08-13_plan_review.2-F02` established it: `.tuning/` is
              gitignored (`.gitignore:11`), the work moved machines at DEC-049
              and `.tuning/selfplay_v1.tsv` did not come along, and it is not
              regenerable identically because S027 and S028 changed the engine
              that generates it. Every fit the plan still wants needs a corpus
              that does not exist here, so a fit is impossible without one.

              A 2026-08-14 literature survey of hand-crafted evaluation and its
              tuning ranked corpus regeneration second of twenty candidates and
              reported that the largest published Texel methodology gains came
              from filtering *less*: Österlund's biggest single step was
              removing an exclusion, Grant filters only mate scores, Blunder
              converged on check, mate and a quiescence disagreement. chesso
              filtered harder than all three.

              The night held one alternative: S033's SPRT, which is next in
              `plan.md` order.

Decision:     **By the owner**, from options the agent supplied and measured:
              regenerate the corpus tonight, ahead of `plan.md` order, as S065.
              Eight hours of `datagen` at 12 threads (DEC-050), 120000 games at
              100000 nodes a move, about 11.3 M positions from an engine that
              is seven steps newer than the one that produced the last corpus.

              **One filter clause is loosened, and it is the one the engine's
              own code contradicts.** `tools/datagen.cpp` excluded a position
              whose search-chosen move was a capture or a promotion, justified
              in its header by the claim that `evaluate()` is only asked about
              positions quiescence has already resolved. `src/search.cpp:125`
              calls `evaluate_lazy()` at the top of every quiescence node,
              before `generate_captures` at `:152` runs, and returns on that
              stand-pat score at `:136-137`. The static evaluation is asked at
              exactly the positions the clause excluded. It becomes
              `--allow-tactical`, default 0, so the S028 behaviour stays
              reachable and the change is a run parameter; the run passes 1.
              Measured cost of the clause: 4814 positions of 27839 considered
              over 240 games, +27.4 % on top of what was recorded.

              **The other three clauses stay**, on measurement rather than
              taste. In check: kept, all three surveyed engines keep it and the
              stand-pat score is not a bound while in check
              (`src/search.cpp:135`). Mate: kept, and it is shadowed rather
              than redundant — 0 marginal rejections under the score cap, 649
              without it. Score past `--quiet-limit` 1000: kept, because
              uncapping admits 8.93 % more rows carrying **0.206 %** of the
              corpus's gradient mass, the tuner weighting a row by
              `sig * (1 - sig)` (`tools/tuner.cpp:380-384`), which is 5.28 % of
              an equal position's at a score of 1000 and falling. One clause
              moves, so one thing is attributable.

              **This produces candidate weights, not a verdict.** 827 constants
              move at once. Nothing is kept without an SPRT against the
              constants shipping today, and a verdict of zero is recorded as
              zero. Held-out error cannot rank it: a different corpus is a
              different objective, and K is refitted per corpus — 0.7472 here
              against S028's 1.1141.

Rejected:     **Run S033's SPRT tonight instead.** It is the next step in
              `plan.md` order and it stays there, one place behind. A verdict
              costs three to four and a half hours and would leave half the
              night idle, while generation scales with every hour it is given;
              and S033 is a pruning change measured against an evaluation that
              may be about to move under it, so its number is worth more taken
              after the corpus question is settled than before.

              **Loosen more than one clause.** Admitting the score cap as well
              was measured at 0.206 % of the gradient mass, which buys almost
              nothing and costs the attribution: two clauses behind one SPRT
              and neither number means anything.

              **Regenerate with the filter unchanged.** Free, and it would
              answer only "is this engine's self-play better data than the old
              engine's" while leaving the exclusion the code contradicts in
              place for the next corpus too.

              **Edit the filter rather than flag it.** The old behaviour would
              stop being reachable and `selfplay_v1.tsv`'s generation settings
              would stop being expressible, which is what makes a corpus
              comparison possible at all.

              **Grant's PV-resolution construction** (the survey's C16), which
              plays out the principal variation to reach a quiet position
              instead of filtering for one. It is the most expensive candidate
              in the survey — one deep search per sampled position — and it
              tests the same hypothesis this does for a fraction of the
              machine time. Behind this, not instead of it.

Consequences: `--allow-tactical` exists in `tools/datagen.cpp` and is
              documented in `DEV_MANUAL.md`. Every datagen run now also prints
              what each filter clause cost, so the next corpus is sized from a
              measurement rather than from a comment. The default is inert:
              240 games at seed 20260814 give byte-identical output through the
              pre-change and post-change binaries.

              S065 carries the run and the SPRT. S066 goes immediately ahead of
              it: the tuner shuffles rows and holds out 10 %, `datagen` writes
              about 94 rows per game consecutively, and 99.5 to 99.8 % of games
              land on both sides of the split, so S065's own held-out gate is
              read through an instrument known to be optimistic until that is
              fixed. It changes a diagnostic and no game.

              A second corpus variant is now a step and not a drift. The survey
              lists three separable changes here and this takes one of them;
              the others cost an SPRT each and are decided the same way.

              The datagen filter is no longer a rule with a rationale in a
              comment. It is four independent clauses, each with a measured
              marginal cost, and changing one is a run parameter.


## DEC-056  2026-08-14  The tuner holds out whole games, and the boundary is reconstructed from the FEN
Tags:         tuning, tuner, validation, datagen, corpus, s066, s065, dec-019,
              dec-055

Context:      `tools/tuner.cpp` shuffled a **row** index and held out the last
              `--validation` of it. `tools/datagen.cpp` writes a game's rows
              inside one `lock_guard` on `output_lock`, about 92 of them, and
              gives every one the same label -- the game's result. Measured over
              `.tuning/selfplay_v2.tsv`, 11003693 rows from 120000 games:
              **119360 of 119999 games, 99.47 %, had rows on both sides of the
              cut**. A held-out row whose game is in the training set measures
              memorisation, so the held-out error was optimistic by an unknown
              amount.

              This decides no verdict -- every S027 and S028 verdict is an SPRT
              and this touches no game. It decides an instrument the project
              uses to **choose** what to run, and S027's carried-forward lesson
              is that held-out error already predicts Elo poorly in either
              direction (DEC-019). A split that shares games makes a weak
              instrument weaker.

              `S066` framed the choice as two shapes and left it to the step: a
              game id as a fifth `datagen` column, or reconstruct the boundary
              from the contiguity the file already has.

Decision:     **Reconstruct**, taken by the agent under the delegation in the
              step's own text ("Decide it in the step, and record it") with the
              analysis and the measurements below supplied by the agent.

              The boundary is the **ply**, `2 * (fullmove - 1) + (black to
              move)`, in `tools/tuner_split.hpp`. `make_move` only ever advances
              it (`src/bitboard.cpp:883`), so a row whose ply does not advance
              on its predecessor cannot belong to the same game. The blocks are
              shuffled, whole blocks are held out until `--validation` of the
              rows are, and `--seed` seeds that shuffle.

              The predicate is one-sided and the direction is the point. It
              **never cuts a game in half**, because it only fires where a
              within-game invariant is violated -- so zero games straddle the
              split by construction rather than by a measurement that came out
              well. It **can miss a boundary**, and a missed boundary merges two
              games into one block that still lands wholly on one side: the cost
              is granularity, not contamination. Measured: 119998 blocks against
              120000 games, **at most 2 missed boundaries in 120000**, 0.0017 %.

Rejected:     **A game id as a fifth `datagen` column.** The explicit format and
              the right one long term. Rejected for this step because the corpus
              that exists cost seven hours of generation and 11003693 rows, and
              a column cannot be added to it retroactively; because it would
              change what every reader of the four-column format sees,
              `tools/eval_spread` included; and because the reconstruction is
              exact where it matters, missing at most 2 boundaries in 120000 and
              never splitting a game. It stays available as a separate step and
              is not foreclosed.

              **The FEN's move number alone**, which is what S066 proposed and
              what was verified on the smoke corpus. Sound for the same reason
              -- the move number never decreases inside a game -- but measured
              **148 missed boundaries of 120000, 0.123 %**, sixty times the ply,
              because a game ending with Black to move and the next starting at
              the same move number is no descent. One extra bit, already parsed
              by `load()` for the tempo term, buys all of it.

              **A stronger predicate on top of the ply.** Adding "a piece
              appeared, so a capture ran backwards" finds 119998 blocks --
              exactly what the ply alone finds, 0 more. Adding "a castling right
              came back, or the label changed" finds 119999, 1 more. Neither is
              worth the code.

              **Keep the row-level split and report it as optimistic.** Free,
              and it leaves S065's held-out gate reading through an instrument
              known to be wrong.

              **Divide the last block to hit `--validation` exactly.** That is
              the defect: one game on both sides. The fraction overshoots by up
              to one block instead, measured at 1100388 rows against 1100369
              asked for at seed 1, 10.0002 % against 10 %.

Consequences: `datagen`'s row order is now load-bearing. A change that
              interleaved two games' rows would leave the reconstruction
              silently wrong, and `tests/test_tuner_split.cpp` holds the
              splitter rather than the writer, so it could not see it.
              `DEV_MANUAL.md` says so where the split is documented.

              The held-out fraction is approximate, bounded above by the target
              plus the largest block, and the run prints what it realised.

              **A block is indivisible in both directions, so the last block is
              never held out.** A corpus of one game cannot give up part of
              itself, and giving up all of it left `gradient()` dividing by a
              count of zero: a two-row corpus at `--validation 0.5` printed
              `0 train, 2 validation (100.0000%)` and then `epoch 1  train
              0.000000  validation -nan`. The failure mode came in with this
              change -- a row-level split could always cut one row off a game --
              and it is reachable, because a two-row corpus is how S040 and S041
              measured the `--only` groups. It is clamped in `split()` rather
              than refused up front, so every invocation that worked before this
              change still works, and `main()` says out loud when nothing could
              be held out rather than leaving a validation error of 0.000000 to
              be read as a perfect fit. The defect was found by this step's own
              test, before the commit.

              **The held-out figure did not move measurably.** The corpus fitted
              under both splitters at the same pinned K, seed and epoch budget
              gave 0.117352 held out at row level against 0.117380 at game
              level, a difference of 2.8e-05 -- against 3.7e-04 between the two
              held-out sets at the untuned starting constants, thirteen times as
              much. Recorded as zero. 827 parameters over 9.9 M rows have no
              capacity to memorise a game, so this is the expected result and not
              a reason to doubt the defect: the old split shared games by
              construction and the number it produced was not interpretable,
              whatever its value. The gap would widen with more parameters or
              fewer rows, which is exactly where the project is going.

              `tuner` now refuses a row whose FEN has no usable move number,
              where before it read the row and ignored the field. Every one of
              the 11003693 rows in `selfplay_v2.tsv` carries all six FEN fields.

              A held-out figure from before this change and one from after are
              not comparable: they are errors over different row sets. S028's
              0.113852 to 0.108043 is a row-level figure, and K is refitted per
              corpus regardless.

              S065's fit reads a held-out figure that is no longer shared with
              its training set. The verdict on the constants is still an SPRT
              and a verdict of zero is still recorded as zero.


## DEC-057  2026-08-14  S065 re-fits with tempo and piece_placement frozen at zero
Tags:         tuning, evaluation, tuner, testing, s065, s027, s038, dec-019,
              dec-053, dec-055

Context:      S065's first fit over `.tuning/selfplay_v2.tsv` freed all 827
              parameters. It reported K = 0.7624 and held-out error 0.122560 to
              0.117359 at epoch 4800 with no refusal warning, and every one of
              the 827 constants was applied and verified. `ctest -L fast` then
              came back 9 of 12, and each of the three failures was a guard an
              earlier step had placed:

              1. `test_eval_model`'s `CHECK(tempo_unfitted)`. Tempo fitted to
                 39 / 21, so all four of `evaluate()`'s tapered divisions can
                 truncate and the model-versus-engine bound moves from
                 3 x 23/24 = 2.875 to 4 x 23/24 = 3.833, taking the tolerance
                 from 3 to 4. DEC-053's own stated consequence.
              2. The same case's four pinned FENs. Their residuals are of the
                 old weights: 2.875 / 2.333333 / 2.25 / 2.125 became
                 2.541667 / 1.250000 / 1.125000 / 2.416667, so two fell under
                 the `> 2.0` per-position assertion and the worst under `> 2.8`.
              3. `test_evaluation`'s "removing a piece moves the score". A white
                 queen on d1 against bare kings evaluated 713 against her own
                 fitted material value of 1152 — 439 off, against
                 `POSITIONAL_ROOM` 150.

              The paste was reverted and the tree left green. Two options were
              put to the owner: accept the fit as fitted and re-target all three
              guards, or re-fit with groups held.

Decision:     **By the owner**, from options and measurements the agent
              supplied: **re-fit with `tempo` and `piece_placement` both frozen
              at zero**, and freeze them *during* the fit.

              `piece_placement` because S027 measured it at **-5.48 +/- 11.46
              Elo over 2284 games, H0 accepted**, and zeroed its weights
              specifically so the compiler would delete the term —
              `evaluate_cheap()` is the same 164 instructions either way
              (`plan_done/S027_handcrafted_eval_terms.md:191-206`). Re-applying
              it re-pays a measured 3.1-4.0 % search cost for a term measured at
              zero.

              `tempo` because holding it at zero keeps the truncation bound at
              three effective divisions, 2.875, so guard 1 cannot fire and
              `test_eval_model`'s tolerance stays 3.

              **During the fit, not after.** With a parameter held, the
              remaining 817 absorb what it would have taken. Fitting all 827 and
              then zeroing two groups leaves the other 825 fitted against values
              that are no longer there, which is a different and worse result
              than a fit that never had them.

              **Guard 2 is re-targeted, on the owner's authority**, which
              AGENTS.md section 6 permits for a deliberate behaviour change
              while forbidding relaxation. The four FENs were pinned to absolute
              residuals of the old weights; the guard's purpose is that the
              corpus contains positions that genuinely exercise the truncation
              bound, so new positions are measured under the new weights and
              pinned, and the threshold is re-derived from the arithmetic rather
              than lowered to whatever came out.

              **Guard 3 is not re-targeted and `POSITIONAL_ROOM` is not
              touched.** How much positional room the evaluation legitimately
              needs is a design question and the owner's call. If it fires, the
              agent stops and reports.

Rejected:     **Accept the fit as fitted and re-target all three guards.** The
              cheaper option and the one that keeps the better held-out number.
              Refused because two of the three thresholds would then be moved to
              accommodate weights nothing had measured: `POSITIONAL_ROOM` is a
              property rather than an anchor, and widening it is weakening it.
              The tempo tolerance alone was already decided by DEC-053, so that
              part of the option was never in question.

              **Fit everything and zero the two groups afterwards.** Free, and
              it produces a header with the same two groups at zero. Refused
              because it is not the same fit: the other 825 parameters were
              fitted against a tempo of 39 / 21 and a piece placement term that
              is then removed from under them.

              **Freeze `tempo` only.** Answers guard 1 and leaves guard 3
              untouched, and would have kept the piece placement weights the fit
              wanted. Refused on S027's measurement: the term is priced at zero
              and costs 3.1-4.0 % of the search to compute.

              **Keep the corpus and stop.** The corpus cost seven hours and its
              only product so far is a reverted paste. Refused because a re-fit
              is 35 minutes on the same data and needs no new games.

Consequences: `tools/tuner.cpp` gains `--freeze LIST`, the inverse of `--only`:
              a comma-separated subset of the same group names, held at what the
              engine ships, with everything else fitted. Default empty, which is
              the behaviour before it and was proved inert — identical epoch
              reports and identical 827 constants through the pre-change binary.
              `all` is refused and an unknown name refuses the whole list.
              `tools/tuner_groups.hpp` holds it beside `free_mask` and does not
              change the partition `test_tuner_groups` asserts, since it selects
              among the ranges `free_mask` already defines.

              `tempo_mg`, `tempo_eg` and the eight `piece_placement` weights
              stay at zero through this fit and are not evidence of anything the
              corpus says about them: they were held, not measured.

              A held-out figure from this fit cannot be ranked against the
              unfrozen one or against S028's. Different free-parameter sets over
              a corpus whose K is refitted per fit. Only the SPRT decides and a
              verdict of zero is recorded as zero. DEC-019, INV-6.

## DEC-058  2026-08-14  repair the test defects now, ahead of S065's SPRT
Tags:         testing, workflow, invariants, audit

Context:     The owner asked for an investigation of the test suite: are the
             tests correct, and are they in line with the published literature
             and what other open-source engines test. The answer is recorded as
             `adocs/audit/2026-08-14_test_review.md`, which is a review with the
             blue team's knowledge rather than a clean-context adversarial run,
             and says so.

             The suite is green and every oracle value in it that has an
             independent source is correct — 56 perft counts re-derived from
             Stockfish, every published breakdown column matching the Chess
             Programming Wiki, and 127 of 127 on `perftsuite.epd`, a suite six
             times larger than the one in the repository. Seven findings came
             out of it anyway, two of them cases that cannot fail for the reason
             they exist.

             S065 sat in `plan_current/` at the time, with an SPRT as the only
             thing left in its `accepts:` and nothing measured yet.

Decision:    The owner's, on the agent's analysis: fix them now, before that
             SPRT. AGENTS.md section 0 — "a bug that has been found gets fixed
             before anything else starts ... a known defect in the tree
             contaminates every measurement taken after it" — makes the repair
             precede S065's remaining work rather than follow it, so S067
             carries `blocks: S065` and S065 is paused. The relationship is a
             real one and not a device for getting a second step into the active
             slot: S065 cannot complete without a green suite, and two of the
             cases in that suite assert nothing while a third asserts a premise
             that is false.

             S067 closes F01 to F06. **F07 is accepted, not planned.** The
             built-in `test` command prints seven expected best moves and a node
             total and checks none of them; turning it into something that fails
             is a UCI surface change covered by `test_uci_surface` and
             `MANUAL.md`, and it is the same work as giving the engine a real
             `bench` signature. That belongs in its own step with its own
             surface rows, not inside a test repair.

Rejected:    **Record the findings and carry them into S065.** Refused on the
             rule above: the next thing S065 does is take a measurement.

             **Fix only F01 and F02, the two vacuous cases.** Refused because
             F03 is a `TIMEOUT` that the build it is written for cannot meet —
             `ctest --test-dir build-debug -L fast` reports Timeout on
             `test_movegen` and `test_search`, which run 165.33 s and 215.91 s
             and pass when run directly — and that is what makes INV-2's and
             INV-4's asserts unreachable through the normal command.

             **Add the missing machinery in the same step: a bench signature, a
             mate suite, sanitizers, CI.** Refused as scope. Those are the four
             gaps against Stockfish's `tests/`, `TerjeKir/EngineTests` and
             OpenBench that the review names, and every one of them is a new
             instrument rather than a repair. They are steps of their own and
             none has been decided.

             **File the review as a plan review rather than an audit.** Refused:
             it re-measured the code, not the documents, which is what section
             10 means by an audit running against the code.

Consequences: Two of the six invariants keep their current enforcement — INV-2's
             and INV-4's asserts still live only in the debug build, and F03's
             fix makes that build runnable under `ctest` without putting it in
             the gate. Whether it joins the gate is undecided and unrecorded
             either way.

             The comparison against other engines is kept in the report rather
             than promoted into `specs.md`: it describes what other projects do,
             not what chesso must do, and nothing in it is an invariant until a
             step is taken on it.

             A test position added to this suite from here carries a legality
             precondition. F01 and F02 were both illegal positions and both
             passed for years, so the class is the finding rather than the two
             instances.

## DEC-059  2026-08-15  S065's queen is re-anchored through the degeneracy the tuner documents
Tags:         tuning, evaluation, testing, s065, dec-019, dec-055, dec-057

Context:      DEC-057's frozen re-fit cleared two of the three guards the first
              fit fired and stopped on the third. `test_evaluation`'s "removing
              a piece moves the score" asserts that taking a piece off the board
              moves the score by its own material value to within
              `POSITIONAL_ROOM` = 150 (`tests/test_evaluation.cpp:174,184`). The
              refitted white queen on d1 against bare kings evaluates **716**
              against her own fitted `QUEEN` of **1148** — a residual of
              **432**, dominated by `psqt_eg[queen][d1]` = -485 at endgame
              weight 20/24.

              What constrains that square, measured over all 11003693 rows of
              `.tuning/selfplay_v2.tsv`: a queen at `phase <= 4` appears on
              **9786 rows, 0.0889 %**, and `phase <= 4` with a queen is one
              queen and no other piece.

              S065 stopped there rather than acting, because `POSITIONAL_ROOM`
              is a property and not an anchor: what fires it is a real property
              of these constants, and how much positional room the evaluation
              legitimately needs is a design question.

              Measured and put to the owner as data. `tools/tuner.cpp:26-30`
              documents the parameterisation as degenerate: adding a constant to
              every square of `psqt_mg[t]` and `psqt_eg[t]` and subtracting it
              from `piece_value[t]` is the same evaluation. Measured on this
              weight set, with 432 added to all 128 queen squares and subtracted
              from `QUEEN`: **six of the seven pinned anchors are identical** and
              the queen on d1 moves by **1**, 716 to 715, because that
              position's tapered numerator crosses zero and truncation towards
              zero is not translation-invariant across it. The guard's residual
              becomes **1** against 150. In the opposite direction the same
              shift takes the residual to 864 with all seven anchors unchanged,
              which is what says the direction is the free parameter and not the
              evaluation.

Decision:     **By the owner**, from options and measurements the agent
              supplied: **re-anchor the queen**. Subtract 432 from `QUEEN` and
              add 432 to all 64 squares of `psqt_mg[queen]` and all 64 of
              `psqt_eg[queen]`. `POSITIONAL_ROOM` stays 150 and
              `tests/test_evaluation.cpp` is not edited.

              The shift is applied to the tuner's emitted header **before** the
              paste, never to the source after it, so `verify_fit.py` still
              compares the engine against a single file and "827 of 827" keeps
              meaning what it says.

              **This amends S065's `excludes:`**, which forbids "any change to
              what `evaluate()` computes". The re-anchor changes it by one
              centipawn on one of the seven pinned positions and by nothing on
              the other six. That clause exists to stop two changes arriving
              behind one verdict; a shift the tuner documents as
              evaluation-preserving, measured here at one centipawn, is not a
              second change, and the SPRT that decides the paste decides this
              with it.

Rejected:     **Widen `POSITIONAL_ROOM` to 432 or more.** One line, and the
              guard is weaker for every fit after this one. It asserts a piece
              is worth its material to within what one square's positional terms
              can say; the residual it fired on is real, and a threshold raised
              to accommodate the weights that failed it stops being evidence.

              **Reject the fit and keep the constants shipping today.** Would
              close S065 on a negative verdict without ever putting
              0.122560 -> 0.118460 in front of a match, and would spend the
              corpus night for nothing. The held-out figure ranks nothing and
              only the SPRT decides — which is the reason to run the SPRT, not
              the reason to skip it.

              **Re-fit a third time with the queen's endgame table regularised,
              or with that square constrained.** A tuner change and another
              night, against a residual the documented degeneracy already
              answers for one centipawn.

              **Skip or disable the failing case.** Not an option: section 11.

Consequences: The queen's fitted material value now reads **716** in
              `src/eval_tables.hpp` and is **not comparable** with the 1067
              shipping today, with the unfrozen fit's 1152, or with any earlier
              figure. A fitted material value is only meaningful together with
              its own tables. This entry is the reason, and the same applies to
              any future re-anchor of any piece.

              `piece_values_abs[]` (`src/evaluation.cpp:41`) is built from the
              `MVV_*` constants and is untouched, so the 100-point
              capture-to-killer band clearance is unaffected. Checked before the
              paste rather than after, because that hazard is silent — it shows
              up as a strength regression and never as a wrong node count.

              `.tuning/s065_guard2_retarget.patch` becomes committable. It
              re-targets `test_eval_model`'s four pinned FENs and was pinned to
              constants that were not in the tree, which is why S065 could not
              commit it either.

              The transformation is recorded in the step file as well as here,
              because `.tuning/` is gitignored and the emitted header does not
              survive in git.

## DEC-060  2026-08-16  Reverse futility is bounded by ply, because no guard makes it mate-safe
Tags:         search, pruning, testing, mate, s033, dec-019, dec-033, inv-6

Context:      S033's own hazard section named the trap it expected: "a node
              holding a forced mate for the opponent can still have a static
              score above beta and be pruned. Guard `beta` against the mate band
              exactly as the null move guard does." That guard was written, and
              it is not what fires.

              The rule as first built -- non-PV, not in check, `ply > 0`,
              `depth <= 6`, `beta` outside the mate band, `evaluate() - 100 *
              depth >= beta` -- turned two existing cases red on the first
              `ctest` run, both on `MATE_IN_2_B_POS` at fixed depth 3. A trace
              of every prune found the node:

                RFP ply=1 depth=2 alpha=-965 beta=-964 eval=-764 ret=-964
                fen=4K3/q7/4k3/8/8/8/8/8 w - - 1 2

              The side to move is a bare king losing by a queen. It fails high
              because `beta` is **-964**: the parent is a null-window scout
              hunting a mate score, and "I am only 764 behind" clears that bound
              by exactly zero. `beta` is nowhere near the mate band, so the
              guard the step prescribed does nothing. What the rule returns is a
              lower bound on the node, and a forced mate is the one thing a
              static bound cannot respect.

              Two sweeps, 56 builds, each run against the fast search suite with
              `tools/search_bench.py` at depth 9 for what the setting buys.
              Baseline `c56ab41`, built from a worktree: **3752725 nodes**.

              Constants first, margin x lower depth bound x upper depth bound.
              Green arrives at margin 150 and it is arithmetic, not safety: the
              harmful prune needs `-764 - 2 * margin >= -964`, true at 100 and
              false at 150. A second position built to test exactly that --
              `MATE_IN_2_B_POS` with White material added until White leads by
              500, keeping a mate in two no checking move also forces, verified
              by `python-chess` exhaustively and by `stockfish` at depth 18 --
              is **red at margin 150**, where the original cases pass.

              Then five guards, all at margin 100, depth 1..6: none; the side to
              move is not losing on the static score; the parent's bound is not
              a losing one; the side to move still has a piece; `game_phase() >=
              6`. **All five red** on that second position. The last two do not
              change the bench node count by a single node, which is the
              measurement saying they never fire on a real position.

              This is a property of the technique. A static evaluation is never
              a mate score, and in this engine it provably cannot approach one:
              `evaluate_expensive()` clamps the whole king-safety and mobility
              correction to `+/-LAZY_EVAL_MARGIN`, 150 cp. The term is fitted and
              non-zero since S065 and still bounded two orders of magnitude below
              a mate.

              What does work is not a guard but a bound. The rule already exempts
              the root, because the root's answer is the one that gets played.
              The trace says that exemption is one ply too narrow. Exempting the
              first plies instead, at margin 100, depth 1..6:

              | first ply pruned | nodes | cost | suite |
              |---|---|---|---|
              | 1 | 1398911 | -- | RED |
              | 2 | 1408517 | +0.7 % | green |
              | 3 | 1422053 | +1.7 % | green |
              | 4 | 1781672 | +27.4 % | green |
              | 5 | 1895553 | +35.5 % | green |

Decision:     By the agent inside the step, on the measurements above, and open
              to the owner's revision: **ship reverse futility bounded to ply 3
              and below, margin 100, depth 6**, and record that the technique is
              not mate-safe rather than pretending a guard made it so.

              1422053 nodes against 3752725, **62.1 % fewer**, same three best
              moves on the bench, 0.226 s against 0.470 s at fixed depth 9.
              Nodes per second fall, 8283 to 6145 knps: the rule adds an
              `evaluate()` at interior nodes and pays for it many times over in
              nodes not visited. Under iterative deepening from a cold table both
              mate positions report `mate 2` at depth 3, identical to `c56ab41`
              at every iteration from 1 to 6.

              The ply bound is set at 3 by the cliff and not by the greenness:
              ply 2 is green as well and two plies of protection cost 1.7 %,
              where a third costs 27 %.

Rejected:     Dropping the feature and recording it as a negative result. The
              precedent exists -- S005, S006 and S015 -- but it is for a
              technique that measured zero, and this one has not been measured in
              games yet at all. 62.1 % of the tree is not a zero to record.

              Relaxing the mate cases to depths 4 to 6. Under the unbounded rule
              both positions still report the mate under iterative deepening, one
              iteration late, so the case for it was arguable: the fixed-depth
              cold call the suite makes is stricter than the path the engine
              plays through. It was rejected because the ply bound costs 1.7 %
              and keeps the assertion, and a test that is relaxed once has been
              relaxed. Section 6 permits re-targeting for a deliberate behaviour
              change; it does not require accepting one that 1.7 % of the nodes
              buys back.

              Margin 150 or 200, which are green on the original two cases. They
              are green by arithmetic and the second position shows it. Margin 75
              at the same ply bound is green and saves more, 1216123 nodes, and
              is **not** shipped: one change at a time, and the margin is a
              tuning question for a later step with its own verdict.

              A king-danger guard built on the king safety feature counts. It
              would fire on both positions -- a queen bearing on the king zone is
              exactly what both have -- but reading those counts in the search
              needs a second instantiation of the collecting evaluation in the
              hot path, which is the documented 21 % regression from S027. It is
              a step of its own if anyone wants it, not a line in this one.

Consequences: `specs.md` moves reverse futility from the absent row to the search
              row with the bound and the blindness stated inline.

              The suite now holds a position for the case the step file did not
              predict: the mated side **ahead** on material, with a quiet-only
              mating key. It is red at settings the two `MATE_IN_2_B_POS` cases
              call green, which is why it exists.

              A mate that first becomes visible below ply 3 can still be missed
              for an iteration and nothing in the suite covers that. Stated, not
              fixed.

              The margin is unfitted and known to be improvable. Whatever the
              SPRT returns, 75 is the first thing a follow-up step should try.

              `DEV_MANUAL.md`'s depth-9 node figure was stale before this step --
              3136397, from before S065 refitted the constants, against 3752725
              actually measured at `c56ab41`. It now carries the commit each
              figure belongs to, because a baseline quoted without one is how
              this step nearly reported a 55 % saving against the wrong number.

## DEC-061  2026-08-16  A watcher exits on the run's own last line, not on a turn boundary
Tags:         tooling, monitor, workflow, agents-md, dec-048

Context:      A `Monitor` armed with `persistent: true` was found running with
              nothing under it. `tail -f
              .../e6b3ca05-.../scratchpad/rfp_sweep.log | grep --line-buffered
              -E "margin=|BUILD_FAIL|error"`, pid 844036, elapsed **2 h 02 m**.
              The sweep it watched -- S033's RFP constant sweep, 56 builds --
              wrote its last line at 13:17, nine minutes after the watcher was
              armed. The machine showed no engine process, which is how the
              owner noticed: a watcher with no load behind it.

              Two failures stacked. The command is unbounded: `tail -f` has no
              exit condition, so it holds whether or not anything will ever be
              written again. And `persistent: true` outlives `/clear` -- the
              context carrying the task id is discarded, the process is not --
              so `TaskStop` was no longer reachable and the only way to end it
              was `kill 844036`.

              Nothing was lost. The sweep's results are in S033, committed as
              `fca9522`. The cost was an idle process and the two hours in which
              the session believed a run was still being watched.

Decision:     Owner, on finding the process; analysis by the agent. A detached
              run prints a terminal marker as its last action, and the watcher
              is a command that **exits on that marker** -- not a bare `tail
              -f`. Failure signatures go in the same alternation as the progress
              lines, so a crash produces an event instead of silence. `TaskStop`
              once the result is read stays the explicit close; the self-exit is
              what covers the case where the reading turn never comes.

              `AGENTS.md` carries this as a house rule next to the detach rule
              it completes.

Rejected:     Leave it and rely on `TaskStop`. It is exactly what failed: the
              id lives in context and `/clear` outlives nothing.

              Drop `persistent: true` and let `timeout_ms` expire. Caps the
              orphan at an hour but also kills live watchers on real SPRTs,
              which run three to four and a half hours -- the rule in DEC-048's
              neighbourhood exists because those runs must be watched to the
              end.

              `tail -f log | grep -m 1 DONE`. Does not work: with the log gone
              quiet after the match, `tail` never writes again, never receives
              SIGPIPE, and the pipeline hangs anyway.

Consequences: Sweep and match scripts under `tools/` and in scratchpads end with
              an explicit `DONE` line so a watcher has something to exit on.

              A watcher without an exit condition is a defect in the watcher,
              found by `ps -eo pid,ppid,etime,cmd | grep 'tail -f'` when a
              session is unsure what it left armed.

              This says nothing about how long a run may take. Long runs are
              still detached and still watched; only the watcher is now
              required to end by itself.

## DEC-062  2026-08-16  The tuning strategy document is adopted as a plan input, and what it buys is nine steps
Tags:         tuning, evaluation, search, spsa, datagen, planning, nnue,
              book-learning, dec-014, dec-019, dec-054, s073, s075, s076, s077,
              s082, s083, s084, s085, s086

Context:      `adocs/eval_tuning_strategy.md` arrived as an untracked 529-line
              summary of published technique in evaluation and search-parameter
              tuning: Texel-style gradient fitting, NNUE, SPSA, TD and
              TreeStrap, book learning, and the statistical gate around all of
              them. It named itself an "input document for a development plan"
              and nothing in the plan referenced it.

              The 2026-08-16 plan_review audit recorded it as
              `2026-08-16_plan_review-F08`: a plan input that is neither tracked
              nor in the file map is the memory-outside-the-repository AGENTS.md
              section 12 forbids, and its executive summary is a table of
              published Elo figures presented as "Typical gain", which is the
              exact shape DEC-019 exists to stop being read as a target.

              Mapping it against the tree found most of section 2 already
              built. The Texel fit, Adam, the fitted K, the held-out split by
              game and the linear treatment of king safety are S027, S028,
              S065, S066 and DEC-044. The pentanomial gate the document asks for
              is already what `fastchess.sh` runs -- `model=normalized`, and
              every recorded verdict carries an nElo figure. What is missing is
              specific and is what the steps below are.

Decision:     **By the owner, on the agent's analysis and options.** Track the
              document under a file-map row, mark its Elo column as reported
              figures per DEC-019, and create nine steps from it. Ordering, by
              the owner's answer to three questions:

              1. **Instrumentation first, then the cheap fits.** S073 exposes
                 the search constants as one addressable set, settable in a tune
                 build and unchanged in the shipping one, because the two
                 pending hand-tunes S068 and S039 currently cost a source edit
                 and a rebuild per point -- and `2026-08-16_plan_review-F04`
                 found the sweep method no longer compiles at all. Then S075,
                 S076 and S077, each a fit and at most one verdict. The search
                 and ordering block keeps its place after them.
              2. **Both compute-heavy items become steps, ordered last.** S083,
                 a corpus past 50 M positions, and S085, the first SPSA run.
                 Written down with their real cost stated; nothing derives them
                 as next.
              3. **Book learning is built, not rejected.** S086, behind a UCI
                 option that ships off, last in the order.

              S082 -- labelling the quiescence leaf rather than the root -- is
              the document's own "critical design decision" and is placed with
              the expensive band because it needs a regenerated corpus, ahead of
              S083 so that what is being generated is settled before five nights
              are spent generating it.

Rejected:     **Online TD and TreeStrap weight updates** (document sections 5.1
              and 5.2). The document deprioritises them itself: TreeStrap
              converges to a fixed point of the engine's own search and is
              sample-inefficient against fitting millions of stored positions
              with known outcomes, and it is the right choice only "when no
              dataset exists and none can be generated". A dataset exists,
              `.tuning/selfplay_v2.tsv`, 11003693 positions.

              **Any scheme that mutates shipped weights without an SPRT gate.**
              This is INV-6 and is not negotiable in either direction.

              **Reopening NNUE.** The document calls it the ceiling, +400 to
              +700 over a tuned hand-crafted evaluation. DEC-054 is the owner's
              decision that it is deferred, and a literature summary is not a
              reason to reverse an owner's decision. S029 stays parked with its
              file whole; the document's section 3 is a reference for it if it
              resumes, and its Phase B data pipeline is shared work that S082,
              S083 and S076 advance anyway.

              **Deleting the document instead of tracking it.** It is the only
              record of why these nine steps exist and of the figures that were
              deliberately not taken as targets.

Consequences: `adocs/eval_tuning_strategy.md` is tracked and gets a row in
              AGENTS.md section 2's file map as research input. Its executive
              summary carries a note that the Elo column is reported figures
              which decide what to try and never what to conclude, DEC-019.
              `2026-08-16_plan_review-F08` is resolved by this entry.

              Nine new steps: S073, S075, S076, S077, S082, S083, S084, S085,
              S086. Three of them owe no verdict at all -- S073 is neutral by
              construction and proves it with identical node counts, S077 and
              S084 touch nothing under `src/`. The rest owe one SPRT each, and
              each of those steps says so in its own `accepts:` with the verdict
              recorded whatever it is.

              The document's multiple-comparison warning is now a standing
              constraint on the tuning steps: a sweep is decided on held-out
              error and exactly one candidate goes to a match. S075 and S083
              both carry it in their gates. This is not a new project rule and
              does not touch INV-6; it is how the three fit steps are written.

              Measurement capacity stays the binding constraint. S083 is
              several nights of generation and S085 is a night plus a
              verification match, and they compete for the same nights as every
              SPRT in the search block above them. Both are last in the order
              for that reason, and starting either is a deliberate call rather
              than a derivation.

## DEC-063  2026-08-17  SPRT bounds straddle the expected effect, they do not bracket it from one side
Tags:         sprt, measurement, bounds, s068, s021, dec-019, dec-041, dec-048

Context:      S068 measured one constant -- `RFP_MARGIN` 75 against the shipping
              100 -- twice, with the same two Release binaries, the same book,
              the same 10+0.2 time control, the same 12-way concurrency and the
              same adjudication. The only difference between the two runs was the
              hypothesis pair.

              | run | bounds | games | wall | outcome |
              |---|---|---|---|---|
              | 1 | `elo0=0 elo1=5` | 9036 scored | 6 h 36 m | no verdict, terminated |
              | 2 | `elo0=-5 elo1=5` | 2312 | 1 h 41 m | **H1 accepted** |

              Run 1 is `fastchess.sh`'s default (`fastchess.sh:58`). It did not
              fail to reach a bound by bad luck. With the effect near +3 Elo the
              truth lies **between** H0 and H1, so neither hypothesis is ever
              the more likely one for long, and the log-likelihood ratio random
              walks instead of climbing: it drifted 0.86 to 0.59 over the run's
              final ten minutes, against a +/-2.94 bound, after six and a half
              hours. At the measured 1371 games/h the remaining 30934 games to
              the 40000-game cap were a further 22.5 h for an answer that could
              not structurally arrive.

              Run 2's bounds can terminate because H0 and H1 sit on opposite
              sides of the observed effect. Same change, same binaries, a
              quarter of the games, four times faster, and a verdict.

              **The warning was already in the repository and was not applied.**
              `adocs/plan_todo/S021_aspiration_windows.md:12-17`, written before
              S068 ran, in full:

                ## Expected

                Small. One engine reported +9 with an error bar of +/-17, which is a reported
                figure and therefore direction only (DEC-019). `elo0=0 elo1=10` cannot resolve
                an effect this size -- use `elo0=-5 elo1=5` or similar or the run random-walks,
                as S006's did for 340 games.

              So this failure mode has now cost the project three runs -- S006's
              340 games, S068's 9036 -- and had been correctly diagnosed and
              prescribed against in a step file nobody reads until that step
              comes up. That is why it is recorded here: a session about to spend
              six hours greps `decisions.md`, not the pending step files of other
              work.

Decision:     By the owner's standing instruction to run measurements unasked
              (DEC-041), on analysis supplied by the orchestrating agent inside
              S068 (AGENTS.md section 8): **bounds are chosen to straddle the
              effect the change is expected to have, and the expectation is
              stated before the run.**

              - A change expected to be small, which is most of them, gets
                `elo0=-5 elo1=5`. H1 accepted then means "not a regression of 5
                Elo or more", which is a real answer and is what a cheap
                node-count saving needs.
              - `elo0=0 elo1=5`, the script's default, is for a change expected
                to clear 5 Elo. It asks "is this worth at least 5", and it can
                only answer when the truth is not sitting inside the interval it
                leaves undefended.
              - The interpretation of each outcome is written down before the
                run starts. S068's second run carries it in the script header
                (`adocs/data/S068_sprt_run2.sh`), which is what makes "H1
                accepted, therefore ship 75" a pre-registration rather than a
                reading chosen after seeing +12.
              - An SPRT that stops early has an upward-biased point estimate, so
                the stopping run's Elo is not the effect size. S068 pools both
                runs for the estimate and says which half carries the bias.

Rejected:     Letting run 1 reach the 40000-game cap. 22.5 h more of the
              binding constraint on the whole plan (`specs.md:199-204`) for an
              answer the bounds cannot produce. The cost was not the objection
              on its own -- a verdict is worth a night -- the objection is that
              no number of games fixes a hypothesis pair that excludes the
              truth.

              `./fastchess.sh --fast`, `elo0=0 elo1=10` (`fastchess.sh:53`). The
              same one-sided defect and a wider undefended interval: at +3 Elo
              it random-walks for the same reason, and its looser alpha and beta
              buy speed only on effects it can already resolve.

              A third run at other bounds, or with the two pooled into a fresh
              SPRT. Run 2's pre-registered text forbids it without a decision,
              and re-running until a bound is hit is how an alpha of 0.05 stops
              meaning 0.05.

              Making `elo0=-5 elo1=5` the script's default in `fastchess.sh`.
              Tempting and out of S068's scope: it changes the harness every
              past verdict was taken with, and `--fast` and the full run would
              then differ in more than width. It is a step of its own if anyone
              wants it, and until then the bounds are named per run.

Consequences: Every pending step whose expected effect is small now sizes its
              bounds from that expectation rather than from the script default.
              That is S021 explicitly, which prescribed this fix itself and now
              cites this entry; S023 and S024, both move-ordering refinements
              where DEC-019 has already measured two related techniques at zero
              or worse; and S026, whose own literature figures are single
              digits. None of them is expected to clear 5 Elo, so none of them
              should be measured against a bound that assumes it.

              DEC-019 gains a companion clause. "Published figures decide what
              to try, never what to conclude" now also means **a published
              figure sizes the bounds**: it is the only prior available before
              the run, it is the number that says whether 5 Elo is a floor or a
              ceiling, and it is still not evidence about the outcome.

              A run that lowers `CONCURRENCY` already has to say why (AGENTS.md
              section 0). A run that departs from `fastchess.sh`'s bounds now
              says why as well, in the script that runs it, before it runs.

              `fastchess.sh` is unchanged. Its default is still `elo0=0 elo1=5`,
              so a step wanting straddling bounds writes its own invocation as
              S068 did rather than editing the shared harness.

## DEC-064  2026-08-17  the blended target's K is fitted on the outcome, and the game result alone selects lambda
Tags:         tuning, tuner, evaluation, measurement, s075, s082, s083, dec-019,
              dec-041, dec-057

Context:      S075 adds `--lambda F` to `tools/tuner`, fitting against

                target = lambda * sigma(K * score) + (1 - lambda) * result

              which is `adocs/eval_tuning_strategy.md` section 2.7. The step
              file fixes the shape of the run -- a sweep decided on held-out
              error, exactly one candidate to an SPRT -- and leaves two
              questions open that the arithmetic forces an answer to. Both were
              found while writing the code and neither is a matter of taste.

              **1. K appears on both sides.** `fit_k()` fits the scale that maps
              a centipawn to a win probability by minimising error against the
              target. With a blended target, that target is itself built from K.
              A K fitted against it is fitting to its own output, and two runs
              at different lambdas would come back with different scales, so
              their errors would not be comparable numbers.

              **2. Every lambda trains against a different target, so training
              error cannot rank them.** Lower error at lambda 1 means the fit
              found the 2026-08-14 engine's search easier to predict than the
              outcome of a game, which is true and says nothing about strength.
              Measured on a 1 M-row slice at a deliberately wrong K of 1.5:
              lambda 0.7 reports held-out 0.033419 against lambda 0's 0.120249
              on the same rows and the same split -- a factor of 3.6 that is
              entirely the target changing under the metric.

Decision:     **By the agent, and recorded because S082, S083 and S085 inherit
              it.** Two rules, both in `tools/tuner_target.hpp` beside the code
              they constrain:

              **K is fitted against the game result alone, whatever `--lambda`
              says, and then held.** It belongs to the data rather than to the
              objective. At lambda 0 this is what the tuner already did, so the
              rule costs that path nothing.

              **Held-out error against the game result selects lambda -- never
              the training objective.** The tuner prints it as a separate `wdl`
              column at every report, stamps it into the emitted header with the
              figure at the constants the run started from, and the refusal
              warning is tested against it. Applied inside a run as well as
              across runs: the kept checkpoint and the early stop are decided on
              it, so the vector a run emits is the best outcome predictor its
              trajectory passed through, and a lambda is never rejected because
              its own target's optimum sat a few hundred epochs away from the
              game result's.

              At lambda 0 both metrics are the same function. That is what keeps
              the pre-S075 fit reproducible bit for bit -- proved against the
              pre-change binary on a 1 M-row corpus, identical 50-report logs
              and identical sha256 over the emitted constants,
              `9bbf81126aa44f93c90dcc2bb08888183306c9e2717562e980bb5646a7dbdbaa`
              -- and it is why S065's figures still stand.

Rejected:     **Fit K against the blended target.** The scale would then be the
              lambda's rather than the corpus's, and the sweep would be
              comparing six different units. Refused on that alone.

              **Rank lambdas by the training objective.** Free, and it is what a
              tuner log invites a reader to do. Refused: the numbers measure
              different things, and the 3.6x above is what reading them side by
              side would have concluded.

              **Keep the checkpoint by the training objective and report the
              game-result error only at the end.** Purer as an experiment --
              each run is then faithfully "descend this target" -- and it was
              the first design. Refused because it decides the shipping question
              on a vector nobody chose: the emitted constants could sit past the
              point where outcome prediction started degrading, and a lambda
              would then lose for a bookkeeping reason rather than for what it
              is.

              **Two verdicts, one against the incumbent and one against a
              lambda 0 refit at the same budget.** The clean attribution, and it
              is what would separate the blend from the extra epochs. Refused on
              cost: a verdict is three to four and a half hours and the step
              allows one. The limit is recorded in the step file instead, and
              the lambda 0 refit's held-out figure carries the attribution as
              loss rather than as Elo.

Consequences: `--lambda 0` remains the default, so nothing changes for a run
              that does not ask for the blend. The emitted header gains a
              `lambda` line always and a `wdl error` line at a non-zero lambda,
              which is what makes a fitted table say what it was fitted toward.

              S082 and S083 both regenerate the corpus with the engine being
              fitted, which is when the one thing keeping lambda 1 off
              `eval_tuning_strategy.md` section 1's fixed point expires: the
              scores in `selfplay_v2.tsv` are the 2026-08-14 engine's, from
              before S065 moved 827 constants. Whoever runs a fit at a non-zero
              lambda states the corpus's generating commit beside the lambda.

              S085 fits search parameters by SPSA against playing strength, not
              against a corpus, so the second rule reaches it as the general
              form it already obeys: loss ranks candidates, an SPRT decides one.
              DEC-019.

## DEC-065  2026-08-17  the first row of a repeated position survives, and no label is averaged
Tags:         tuning, corpus, dedupe, datagen, tuner, s076, dec-056, dec-019

Context:      `adocs/eval_tuning_strategy.md` section 2.6 lists "deduplicate by
              Zobrist key; heavily repeated positions bias the fit" among the
              filters that matter, and S076 is that pass. The accepts fixes the
              shape -- one row per distinct position, keyed on the engine's own
              key -- and leaves two things the arithmetic has to decide.

              **Which row survives.** `tools/datagen.cpp:337-341` gives every
              row of a game the same label, the game's result, so two rows with
              the same position can carry different labels: the same position
              reached in two games that ended differently, and the same position
              repeated inside one game on the way to a repetition draw.

              **Whether the surviving label is one game's or the mean of them.**
              A position seen n times with labels y_1..y_n contributes, in a
              squared-error objective, n times the error against their mean plus
              a constant. So the corpus as written already fits the mean, n times
              over; dropping to one sample replaces that mean with one draw from
              it, and averaging keeps it while dropping the repetition.

Decision:     **Keep the first occurrence in file order and write the row
              through unchanged.** Taken by the agent, with the analysis and the
              options here, under the step's own delegation: its accepts names
              the shape and not the tie-break.

              The identity test is 64-bit key equality and nothing else. Over
              11.0 M rows the birthday exposure is about 3e-06, so a collision
              would drop one distinct position; the tool carries a `--verify`
              mode that holds the four hashed FEN fields per distinct key and
              counts key-equal rows that disagree on them, so the number is
              measured on the real corpus rather than argued from the bound.

Rejected:     **Average the labels over the duplicates.** The better estimator,
              and it is what the objective is already computing. Refused for
              three reasons, none of them statistical: it decouples the surviving
              row's `result` from its own `score` and `phase`, which are the
              first occurrence's and which S082's relabelling reads; it writes a
              `result` no `datagen` run can produce, so the corpus stops being
              readable as a record of games played; and it is a second change in
              the same step, which is the one-change-at-a-time rule (`CLAUDE.md`
              foundation 2, rule 6) -- a verdict would then price the dedupe and
              the relabelling together. It is worth its own step if this one
              measures a drop rate large enough for the difference to exist.

              **Keep the last occurrence, or a random one.** Same information as
              keep-first and none of it better. Keep-last costs a second pass or
              a map holding the row text; a random one costs the determinism the
              accepts asks for.

              **Dedupe on the four hashed FEN fields as text.** Exact by
              construction, no collision exposure at all, and refused by the step
              itself: the FEN is written by `generate_FEN` from a board whose en
              passant square is set whether or not a capture is available
              (`2026-08-13_adversarial-F08`, S042 pending), so a text key
              inherits the defect the plan is already fixing elsewhere and the
              engine's key is the definition the rest of the project uses.

Consequences: The dedupe is a pure row filter: every surviving line is a byte
              copy of a line `datagen` wrote, so a deduplicated corpus stays
              loadable by anything that reads the four-column format, and the
              pass is deterministic without any tie-break state.

              A position dropped here takes its game's label with it. The
              surviving label is unbiased for that position and noisier than the
              mean it replaces, which is the trade this decision makes and the
              reason the paragraph above says the averaging variant is worth a
              step of its own rather than being wrong.

              `tools/tuner_split.hpp` reconstructs game boundaries from the ply,
              and dedupe can only remove rows, never reorder them, so no game
              gains a boundary and none is split. It can lose one: a game whose
              first surviving row now stands at a later ply than the previous
              game's last merges into that block. A merged block still lands
              wholly on one side of the cut, so the cost is a coarser split and
              not a contaminated one -- S066's own asymmetry, and S076 measures
              the block count before and after.

## DEC-066  2026-08-17  the corpus stamp is SHA-256 written out here, and the commit is stamped at build time
Tags:         tuning, tuner, provenance, corpus, build, s077, dec-041, dec-049

Context:      `adocs/eval_tuning_strategy.md` section 10 asks that a weight
              vector name the engine commit and the dataset hash that produced
              it. S077's accepts fixes what the header must carry and leaves
              two implementation questions that decide whether the stamp is
              worth carrying at all.

              **Which hash.** Anything deterministic satisfies "hashing the same
              file twice gives the same value".

              **When the commit is read.** A sha is available at configure time,
              at build time or at run time, and the three name different things.

Decision:     **SHA-256, implemented in `tools/corpus_hash.hpp` from FIPS 180-4,
              over the file's bytes exactly as they sit on disk**, and **the
              commit stamped at build time** by `cmake/build_info.cmake` through
              a custom target that rewrites its generated header only when the
              value changes. Taken by the agent, with the analysis here, under
              the step's own delegation and DEC-041.

              The reason for SHA-256 specifically is that `sha256sum` is
              everywhere and knows nothing about this project, so the value in
              an emitted table is checkable by something that is not us.
              Verified rather than asserted: the tuner printed
              `ffdcd801ab59c58390503bfee468a5caeb71a6bb38c012904ec87675382533de`
              for a 200000-row fixture and `sha256sum` printed the same.

Rejected:     **A 64-bit hash of our own** -- FNV-1a or similar, ten lines
              instead of a hundred and fast enough to be free. Refused because
              its value is reproducible only by the binary that emitted it,
              which is a strictly weaker claim than the one the step exists to
              make, and because 64 bits over a directory of corpora is a
              birthday argument nobody should have to make.

              **Shell out to `sha256sum`.** No implementation to get wrong.
              Refused: a fit that dies because a shell tool is missing or named
              differently is a fit lost to provenance, and the failure is
              silent-ish -- it appears at the end of a run that has already
              spent its epochs.

              **A crypto library.** `Never add a dependency on your own`
              (`CLAUDE.md`), and this needs one function.

              **The commit at configure time**, `-DCHESSO_GIT_SHA=` from
              `CMakeLists.txt`. One line and it is what most projects do.
              Refused because it is wrong in the direction that matters: `cmake
              -S . -B build` runs once and the sha it captures goes stale on the
              next commit, so an emitted table would name a commit that is not
              the one its binary was built from. A wrong provenance line is
              worse than none, because a reader trusts it.

              **The commit at run time**, `git rev-parse` from inside the tuner.
              Refused for the same reason from the other side: it names the
              working tree at the moment of the fit, which is not what the
              binary was compiled from -- exactly the case a rebuilt tree makes
              wrong.

Consequences: The tuner takes one pass over the corpus before it loads it:
              **2.70 s over the 706 MB deduplicated corpus**, against 55 s to
              load it and 0.4 s an epoch, so the stamp costs under a fifth of
              one percent of a real fit. It is refused rather than skipped when
              the file cannot be read.

              `sha256sum` does the same file in 1.29 s, 2.1x faster, because
              coreutils uses the CPU's SHA extensions and this does not. That
              gap is the whole price of the decision and it buys the check:
              both print
              `0a6b59b6af9f50c4360cac7c87c33250318e712250f2653eb09bcb9f0b64b69f`,
              which is also the value S076's dedupe log recorded for that
              corpus.

              An emitted header gains three lines: `engine`, `corpus` and the
              row and byte counts. `.tuning/apply_fit.py`, `verify_fit.py` and
              `diff_fit.py` parse definitions and ignore comments, and were
              re-run against a stamped header to confirm it.

              Every table emitted before this step -- `.tuning/tuned_v2*.hpp`,
              `adocs/data/S075_fits/`, `adocs/data/S076_fits/` -- carries no
              stamp and is not regenerated. They are evidence and are byte for
              byte what the tool wrote at the time; their provenance stays in
              the step files and the run logs beside them.

              `-dirty` follows the run scripts' convention, `git diff --quiet`
              over tracked files. A fit run from a tree with uncommitted source
              changes says so, which is the case S065's and S076's pastes both
              passed through.

## DEC-067  2026-08-17  chesso gets an absolute rating on the CCRL Blitz scale, from a gauntlet against engines the public lists rate
Tags:         measurement, rating, gauntlet, ordo, s087, dec-016, dec-048, dec-050, dec-041

Context:      Every strength number this project has ever recorded is a delta
              against an earlier chesso. `fastchess.sh` plays the working tree
              against a `.ref-builds/` worktree and runs an SPRT on the result,
              which answers "is B stronger than A" and cannot answer "how
              strong". The stated goal is the strongest open-source engine in
              the world (`CLAUDE.md`, DEC-013) and there is no instrument in the
              tree that measures the distance to it. Self-play cannot supply
              one at any game count.

              The owner supplied a specification for closing that gap: a
              gauntlet against reference engines carrying published CCRL Blitz
              ratings, solved into an absolute rating by `ordo`, with a
              bracketing check before the expensive run and an anchor-sensitivity
              check after it. The document is sound on method. Four of its
              specifics are wrong for this repository or this machine.

              Verified on this machine before deciding: `ordo 1.2.6` is already
              installed at `/usr/local/bin/ordo`; `fastchess alpha 1.8.1`
              supports `-tournament gauntlet -seeds N`; `cargo 1.95.0`,
              `g++ 13.3` and `go1.22.2` cover the proposed starting set;
              `dotnet` is **absent**, so Leorik cannot be built without adding a
              dependency; `https://github.com/nescitus/sungorus` returns **404**;
              Rustic's home is Codeberg, not GitHub, and is already cloned at
              `/home/max/ws/rustic` with its alpha tags; the document's
              `run_test.sh` does not exist here and the script it means is
              `fastchess.sh`.

Decision:     **Adopt the procedure as S087 and execute it next, ahead of
              S060.** Analysis, the machine survey above and the options were
              the agent's; the four settings below were the owner's answers and
              the specification was the owner's.

              **1. The reference binaries live in `/usr/games`, beside
              `stockfish` and `fastchess`, and nothing third-party enters this
              repository.** Sources are cloned under `/home/max/ws/engines/`.
              What is tracked here is a manifest naming each engine and the
              exact version installed -- enough to attribute a result to a
              specific opponent build, and no more. The CCRL rating used as each
              anchor and the date it was read go in the per-run results file,
              since they change between runs and the installed binary does not.

              **2. Concurrency stays at 12, DEC-050 unchanged.** The forfeit
              bias below is accepted and **detected** rather than avoided: the
              run script counts time forfeits in the PGN and a single one
              invalidates the run.

              **3. Bracket cheap, then decide.** A short run at the repository's
              own `10+0.2` establishes that the reference set brackets chesso
              and that nothing forfeits, before any long run is booked. The
              rated run's time control is chosen after that, with a score in
              hand.

              **4. The book is `books/8moves_v3.pgn`**, the book behind every
              measurement on record here.

              **5. The owner compiles the reference engines and installs
              them.** The step carries the exact clone, checkout, build and
              install commands; the owner runs them; the agent verifies each
              installed binary answers `uci` and records its version in the
              manifest. This is narrower than DEC-041, which hands the agent
              measurements and tuning, and it is narrower by the owner's choice
              rather than by a rule.

Rejected:     **Concurrency 5 or 6, the document's own rule.** DEC-050's
              justification is that oversubscription hits both sides about
              equally, so it inflates variance rather than biasing the result.
              That argument holds for chesso against chesso and does not
              obviously hold against a foreign engine whose time management
              differs: the side with the thinner safety margin forfeits more,
              and a forfeit is a whole point rather than noise. Rejected anyway,
              by the owner, for throughput -- measurement capacity is the
              binding constraint -- with the forfeit count as the detector. If
              the bracketing run shows any forfeit, the rated run drops to 6 and
              this entry is superseded rather than reinterpreted.

              **Running at CCRL Blitz `2+1` straight away.** It is the honest
              time control for importing a CCRL anchor, and at 1000 games it is
              roughly 8 h at concurrency 12 against roughly 1 h at `10+0.2`.
              Refused as the *first* run only: a bracketing failure would spend
              that night on a number the document itself calls unreliable.

              **A UHO book**, as the document names. Refused for the first run:
              it is a new variable in a run already introducing foreign engines,
              and no chesso result on record used it.

              **Leorik in the starting set.** `dotnet` is absent and
              `CLAUDE.md` forbids the agent adding a dependency on its own. It
              stays available as the upward expansion if bracketing fails high,
              as an owner decision.

              **`bayeselo` instead of `ordo`.** The document prefers Ordo for
              cleaner anchoring and Ordo is already installed.

              **Deriving an absolute figure from the self-play history.**
              Self-play Elo measures progress within one version lineage and is
              inflated; no game count converts it to a scale.

Consequences: The project gains a second measurement instrument with a different
              job from `fastchess.sh`, and `fastchess.sh` stays the per-change
              gate. The two are not comparable: Ordo's intervals are trinomial
              and the SPRT verdicts here run `model=normalized`, so an Ordo
              interval is never quoted against an nElo figure.

              A rated run is bookable after any milestone and re-derivable from
              one script, which is what makes "chesso gained N Elo since S0nn"
              expressible on a public scale for the first time.

              The `/usr/games` binaries become a machine-local asset this
              repository depends on and does not contain. It is the same class
              of loss `status.md` already parks for `.tuning/`: the manifest is
              the record that lets it be rebuilt, and it is tracked for that
              reason. An opponent binary is a measurement instrument, so a
              result is only attributable while the manifest names the version
              it was played against -- which is what the manifest is for.

              Anchors are read from `https://computerchess.org.uk/ccrl/404/`,
              which 302s to `https://computerchess.org.uk/404/`, so the fetch
              follows redirects. A rating is copied into the results file with
              the date it was read; nothing hardcodes one.

## DEC-068  2026-08-17  the reference set is Leorik 1.0, Blunder 5.0.0 and a Rustic Alpha 3, anchored at CCRL Blitz read on the day
Tags:         measurement, rating, gauntlet, ccrl, anchors, s087, dec-067

Context:      DEC-067 adopted the procedure with a provisional reference set --
              Rustic `alpha-1` and `alpha-2`, Blunder `v3.0.0` and `v4.0.0` --
              chosen because those were the toolchains present and Leorik could
              not be built without `dotnet`. The owner then built and installed
              a different and better set, which makes DEC-067's set and its
              Leorik rejection both obsolete before either was used.

              Installed, each identified by asking the binary rather than by
              its filename:

                  /usr/games/Leorik-1.0  -> id name Leorik 1.0
                  /usr/games/blunder     -> id name Blunder 5.0.0
                  /usr/games/rustic      -> id name engine 3.99.36

              Sources at `/home/max/ws/Leorik`, `/home/max/ws/blunder` and
              `/home/max/ws/rustic`, each a detached checkout of its tag.

              CCRL Blitz, read from `rating_list_all.html` on 2026-08-17:
              **Leorik 1.0 = 2102** +20/-20, **Blunder 5.0.0 = 2017** +20/-20,
              **Rustic Alpha 3.0.0 = 1792** +16/-16. Also on the list and
              relevant as expansions: Blunder 7.1.0 = 2389, Leorik 2.0.2 = 2538,
              Rustic Alpha 2 = 1719, Blunder 4.0.0 = 1615, Rustic Alpha 1 = 1549.

Decision:     **The reference set is Leorik 1.0, Blunder 5.0.0 and a Rustic
              Alpha 3**, spanning 1792 to 2102 on the CCRL Blitz scale, 310 Elo.
              The owner supplied the binaries; the agent identified them, read
              the anchors and recorded this entry.

              **Anchors are read at run time and copied into the results file
              with the date.** The values above are planning figures, quoted to
              show the set brackets; nothing hardcodes them.

              **`option.Threads=1` is not sent.** None of the three references
              exposes `Threads` -- all are single-threaded by construction, and
              Rustic prints `Threads: 1 (unused, always 1)` -- while chesso
              exposes it as `min 1 max 1`. The specification's single-thread
              condition holds without the option, and sending an option an
              engine does not have is a way to lose a game to a protocol error
              rather than to strength.

              **`option.Hash=64`**, which is inside every engine's maximum. The
              binding one is Blunder at 256 MB; Leorik allows 2047, Rustic
              65535, chesso 4096.

Rejected:     **Anchoring on the installed `/usr/games/rustic`.** It reports
              `engine 3.99.36` and is `md5` identical to
              `/home/max/ws/rustic/target/release/rustic`, the workspace
              development build -- not the tag build, which at `alpha-3.0.6` is
              named `rustic-alpha` and reports `Rustic Alpha 3.0.6`. A
              development build has no published rating, so anchoring it at 1792
              would attach a number to a binary CCRL has never played. Refused;
              the tag binary is installed instead.

              **Blunder 6.1.0 at 2107 as a third or fourth engine.** It sits
              five points from Leorik 1.0 and adds no reach to the bracket.

              **Treating the 3.0.6-versus-3.0.0 gap as immaterial.** Six patch
              releases separate the installed binary from the rated one and
              nothing here measures them. It is either closed by building
              `alpha-3.0.0` or recorded as an approximate low anchor; both are
              honest and the choice is the owner's. Quietly using 1792 for 3.0.6
              is not.

Consequences: DEC-067's provisional set is superseded by this entry, and its
              `Rejected` note that Leorik is out for want of `dotnet` no longer
              describes the machine. DEC-067 is otherwise unchanged: the
              procedure, the concurrency setting, the bracket-then-decide shape
              and the book all stand.

              The bracketing run needs no anchors and is unblocked by neither
              open question. The rated run needs the tag binary installed and
              the 3.0.x question settled.

              A three-family set is a better spread than the two-family set
              DEC-067 planned and is still three. The anchor-sensitivity sweep
              shows the references disagreeing with each other; it cannot show
              them jointly wrong. A fourth family is owed before the figure is
              quoted outside this repository.

## DEC-069  2026-08-18  the agent builds the reference engines, they live in /home/max/ws/engines, and Leorik comes as a release binary
Tags:         measurement, rating, gauntlet, references, toolchain, s087, dec-067, dec-068

Context:      DEC-067 decision 5 made the reference builds the owner's, by the
              owner's choice rather than by a rule. After bracketing run 1
              failed -- the whole set was too weak, chesso scored 90.4 % against
              the strongest of it -- the owner granted the builds to the agent:
              "you can build the engines and at the version that you prefer".

              Two facts on this machine constrain how that is done, both
              measured rather than assumed:

              **`sudo` requires a password**, so the agent cannot install into
              `/usr/games` where DEC-067 decision 1 put the binaries.

              **There is no .NET SDK.** `~/.dotnet` exists but holds only
              `corefx`; there is no `dotnet` on `PATH` or anywhere under a
              four-level search from `/`. Leorik is C#. `CLAUDE.md` forbids the
              agent adding a dependency on its own, and an SDK is a dependency.

Decision:     **The agent builds and places the reference engines under
              `/home/max/ws/engines/`, and `references.tsv` points there.**
              Granted by the owner; the constraints and this entry are the
              agent's. DEC-067 decision 5 is superseded and its decision 1 is
              narrowed to its principle -- nothing third-party in this
              repository -- rather than to the `/usr/games` path.

              **Blunder is built from source**, in `git worktree` checkouts off
              `/home/max/ws/blunder` so that clone stays at `v5.0.0` and the
              record of what bracketing run 1 played does not move under it.

              **Leorik is the project's official `linux-x64` release binary**,
              downloaded from its GitHub releases. The specification prefers
              source "where practical" because a release may be built for
              another microarchitecture; a .NET self-contained publish targets a
              baseline rather than the host, so that concern is materially
              weaker here than it would be for a C++ engine, and the alternative
              is an SDK the agent may not install.

              **The set is Blunder 7.1.0 (2389), Leorik 2.1 (2568), Blunder
              8.5.5 (2664) and Leorik 2.4 (2829)**, CCRL Blitz, read 2026-08-17.
              Four rungs, two families, spanning 440 Elo, chosen to bracket from
              both sides an engine that beat 2102 at 90.4 %.

Rejected:     **Installing into `/usr/games`.** It needs a password the agent
              does not have. Not a preference -- the owner's own placement from
              DEC-067 stands for binaries the owner installs.

              **Installing the .NET SDK to build Leorik from source.**
              `CLAUDE.md`: never add a dependency on your own. The release
              binary answers `id name Leorik 2.1` and `Leorik 2.4`, which is
              what the anchor needs.

              **Keeping any of bracketing run 1's engines.** Leorik 1.0 at 2102
              is the strongest of them and chesso scored 90.4 % against it, so
              it is at the edge of the usable range and would spend games in the
              tail. Rustic leaves at any tag: its strongest rated build is
              Alpha 3.0.0 at 1792.

              **Blunder 6.1.0 at 2107 and Blunder 7.4.0 at 2521.** The first
              adds nothing over Leorik 1.0; the second sits between two rungs
              already present and buys resolution the interval does not need.

Consequences: The reference set is now four engines the agent can rebuild from
              one command each, and `references.tsv` records the tag, the source
              and the `md5` of every one. `/usr/games` still holds the owner's
              original three; nothing removes them and nothing in this
              repository points at them any more.

              If bracketing run 2 also fails high, the next rungs are Leorik 2.5
              at 2917 and Leorik 3.0 at 3266, both release binaries, both one
              download.

              Found while doing this and fixed in the same commit: `rating.sh`
              probed each engine with `timeout 15`, and **Blunder 8.5.5 does not
              exit on `quit`**. `timeout` then returned 124 for a probe that had
              already printed the name, and under `set -euo pipefail` that 124
              propagated out of the command substitution and killed the script
              with no message and a zero-byte log. The probe now tolerates a
              non-exiting engine and the caller checks for an empty name
              instead, which is a real check rather than a swallowed error.

## DEC-070  2026-08-18  a watcher that only reports the terminal marker is not a watcher that ends, and one leaked for ten hours
Tags:         process, watcher, monitoring, dec-061, s087, incident

Context:      DEC-061 requires two independent ways for a watcher to end: the
              detached run prints a terminal marker as its last action and the
              watcher is a command that **exits on that marker** -- the braces --
              with `TaskStop` when the result is read as the belt. `tail -f`
              never exits on its own and is named in that entry as the thing not
              to use.

              S087 ran four detached matches. `rating.sh` was written to print
              `RATING-RUN-DONE <mode> <OK|INVALID> <outdir>` as its last line,
              deliberately, for this rule. Every one of the four watchers was
              then armed as:

                  tail -f <log> | grep -E --line-buffered "RATING-RUN-DONE|..."

              That form *reports* the marker as an event. It does not exit on
              it. So all four ran with the belt only, and the belt held three
              times: `TaskStop` after bracket 1, bracket 2 and rated run 2.

              After rated run 1 the marker fired, the transcript read the log
              directly with `tail -25` to get the anchor sweep, and the next
              action was launching rated run 2. `TaskStop` was skipped, because
              having read the result felt like having closed the loop.

              The watcher survived **10 h 46 m**, through the rest of the step,
              its completion and its commit. It was found only when the owner
              asked what was being monitored, having noticed the CPU was idle,
              and it was killed by pid.

Decision:     **A watcher for a run in this repository exits on the run's
              terminal marker. `TaskStop` is the belt and never the only
              mechanism.** Taken by the agent after the incident above; the
              rule it restores is DEC-061's and was not new.

              **`DEV_MANUAL.md` gains a ready-made self-exiting watcher for
              `rating.sh`**, so the correct form is a copy rather than a
              reconstruction. The generic pattern was already there and was not
              reached for.

              **Reading a run's result does not close its watcher.** The two
              are separate actions and the first is the one that feels like
              finishing.

              **`ps -eo pid,etime,cmd | grep '[t]ail -f'` is run when a run's
              result is taken**, not only when something looks wrong. It is
              already in `DEV_MANUAL.md` as a diagnostic; it is now part of
              taking a result.

Rejected:     **Treating it as a one-off slip and not recording it.** The wrong
              watcher form was used four times out of four, and three of those
              were rescued by hand. That is a systematic error with a manual
              workaround, which is exactly the shape that eventually fails.

              **Blaming the missing `TaskStop`.** It is the proximate cause and
              the least useful one. Had the braces been in place, forgetting the
              belt would have cost nothing.

              **Adding a cleanup sweep at step completion instead.** It would
              have caught this one and would not fix the class: a watcher armed
              after the last completion, or in a session that is then cleared,
              is still unreachable. `/clear` discards the task id while the
              process lives, so `TaskStop` stops being available at all and
              `kill <pid>` is the only exit.

Consequences: No measurement was affected and none needed re-running. `tail -f`
              blocked on a file descriptor costs no CPU: load average was 0.29
              with the leak still live, and rated run 2 and the combined solve
              had the machine to themselves. The cost was one leaked process
              and the risk that a `/clear` would have made it killable only by
              pid, which is the state DEC-061 exists to prevent.

              The general lesson, and the reason this is an entry rather than a
              worklog line: **building a mechanism is not using it.** The
              terminal marker was implemented in `rating.sh` for this rule, in
              the same session, by the same agent that then armed four watchers
              which could not act on it.

## DEC-071  2026-08-18  the target is 3000 CCRL Blitz without a network, and the plan is restructured around what is missing
Tags:         planning, rating, search, evaluation, nnue, dec-054, dec-019, s087

Context:      S087 gave the project its first absolute figure -- **2570 CCRL
              Blitz, 95 % +/-25, soft**. The owner then set the next goal: at
              least **3000**, and asked whether the published literature says
              that is reachable without NNUE, which DEC-054 defers.

              **It is, with about 500 Elo of headroom.** From the same list
              chesso is rated on, single-CPU entries, read 2026-08-18:
              Stockfish 11 **3565**, Komodo 14.1 3482, Xiphos 0.6 3356,
              Ethereal 11.75 3346, rofChade 2.3 3322, Laser 1.7 3294,
              Defenchess 2.2 3281, Booot 6.3.1 3266, Texel 1.07 3130. Each is a
              hand-crafted evaluation and each is one version below that
              engine's first network release.

              The nearest proof needed no web claim and is on this machine.
              `/home/max/ws/Leorik` has **zero** files matching
              `nnue|network|neural|\.nn` at every 2.x tag and four at 3.0, and
              CCRL rates that line 2.0.2 = 2538, 2.1 = 2568, 2.2 = 2689,
              2.4 = 2829, **2.5 = 2917**. One author, hand-crafted throughout,
              +379 Elo from a point 32 Elo *below* where chesso is now. Blunder
              is the same story to 2664 with no network file at any tag. Only
              filenames were listed; no source was read (DEC-016).

              So the ceiling is not the constraint. The plan is.

              `src/search.cpp` at HEAD has alpha-beta, transposition table,
              quiescence, PVS, aspiration windows, null move pruning, late move
              reduction, reverse futility pruning, staged generation, killers,
              history, countermoves, and `see_ge` in quiescence alone. Missing
              entirely, with **no step behind any of it**: late move pruning,
              extensions of any kind, SEE pruning in the main search, internal
              iterative reduction, an `improving` flag, history malus and
              ageing and persistence across `go`, a quiescence transposition
              probe, a static evaluation in the table entry, correction history,
              and any refinement of the reduction itself. `specs.md` calls that
              list parked, and parked meant nothing could derive a step from it.

              **Time management was not even parked.** `chesso.cpp:405-410`
              allocates `remaining_ms / movestogo + increment_ms / 2` with
              `DEFAULT_MOVES_TO_GO 20` (`uci.hpp:13`) and a soft-limit
              percentage that decides whether to start another iteration.
              Nothing scales the budget by best-move stability or by a score
              that is falling.

              Speed is not the problem. **5.95 Mnps single-thread, depth 16
              from the start position in 3 s**, measured at HEAD on 2026-08-18,
              which is the same order as the engines listed above.

              Five standard evaluation terms -- bishop pair, rook on an open
              file, rook on a half-open file, rook on the seventh, tempo --
              ship at zero weight because they measured zero at S027. Every
              engine in the table above carries all five.

Decision:     **By the owner**: the target is **at least 3000 on the CCRL Blitz
              scale, pursued without NNUE**. DEC-054 stands unamended and S029
              stays parked. The agent supplied the feasibility evidence, the
              gap inventory and the step list; the goal and the restructure are
              the owner's.

              Three parts, agreed as one plan:

              **1. The instrument is fixed before the target is read off it.**
              2570 is soft -- 83.1 Elo of anchor spread against the 30 the
              procedure allows, isolated to Leorik 2.1. A finish line cannot be
              read from an instrument that loose. A fourth engine family joins
              the reference set and the rating is re-solved. **S088**, and it
              goes first.

              **2. The parked machinery gets steps, one technique each.**
              **S089 to S099**: time management, late move pruning, SEE pruning
              in the main search, the `improving` flag, history malus and
              ageing and persistence, the quiescence transposition probe with a
              static evaluation in the entry, internal iterative reduction,
              check extensions, singular extensions, reduction refinement,
              correction history. Three evaluation steps follow, **S100 to
              S102**: the five zero-weight terms re-examined at the current
              fit, threat terms, and outposts with space.

              **3. Time management moves ahead of the movegen micro-steps.**
              S030 to S032 are worth 1-3 % each by their own files -- under 1 %
              for S031 -- and sat at positions 77 to 79 while the flat
              allocation above had no step at all.

              And one rule rather than a step: **the rating is re-run at
              milestones**, with `./rating.sh`, so the factor between a
              self-play SPRT and a point on the public scale becomes measured
              rather than assumed. Every Elo figure this project holds is
              self-play against an earlier chesso; 2570 is external. The two
              diverge and nothing here knows by how much. The trigger is any
              landed step that an SPRT credited with 20 Elo or more, and the
              cost is about an hour.

Rejected:     **Reopening NNUE.** It is the highest-ceiling technique on record
              -- `eval_tuning_strategy.md` reports +400 to +700 over a tuned
              hand-crafted evaluation, and Leorik's own 3.0 to 3.2 line at 3493
              is that gain measured on the engine quoted above. It is also not
              the question the owner asked, DEC-054 is the owner's decision, and
              the evidence above says the target does not need it.

              **Taking the reported per-technique figures as a budget.** They
              sum to roughly the size of the gap at their midpoints, which is
              the only reason to believe the list is long enough. They are not a
              forecast: this engine has taken three of them at face value and
              measured 0, 0 and *slower*. DEC-019 is unchanged and each of the
              fifteen steps owes its own verdict.

              **Banding the missing techniques into three or four steps to save
              SPRTs.** It is the obvious way to buy back machine time and it
              destroys attribution -- two changes at once and neither number
              means anything. The cost is accepted instead.

              **Starting on the features and fixing the instrument later.** The
              anchor spread is 83 Elo; a run that lands 40 Elo of real strength
              cannot be distinguished from one that lands nothing on an
              instrument that wide, and the reference set would then have to be
              changed *during* the climb, invalidating comparison across it.

              **Reordering the whole pending list by expected Elo.** The
              existing order encodes reasons -- corrections sit ahead of the
              step each corrects, S087 was an instrument, S065 was a corpus. The
              new block is inserted; the list is not re-derived.

Consequences: **Fifteen new steps, S088 to S102**, all in `plan_todo/`. Fourteen
              of them alter play and owe an SPRT each; S088 owes a rating run
              instead and touches nothing under `src/`.

              **The cost is the whole measurement budget for months.** Roughly
              40 to 60 verdicts once failures and retunes are counted, at three
              to four and a half hours each, is 150 to 250 machine hours -- six
              to ten weeks of nights at DEC-048/DEC-050. Measurement capacity is
              the binding constraint on the plan (`specs.md`) and this decision
              spends all of it.

              **The "absent, machinery" parked item is discharged.** It appears
              in `status.md` and in `specs.md`'s Open items as a list with no
              steps behind it, on the rule that a step is created by a decision.
              This is that decision.

              **`specs.md` gains the target**, dated, beside the measured 2570
              it is a distance from. The 3000 figure is a goal and not an
              invariant: nothing fails a test for being below it.

              **Eval retuning is expected to follow the search block, not to
              precede it.** `eval_tuning_strategy.md` section 0 states that eval
              parameters are only optimal relative to the search that uses them
              and that any material change to pruning, reductions or quiescence
              invalidates the tuning. Eleven such changes are now queued, so
              S100 to S102 sit after them and a refit after the block is normal
              rather than a discovery.

## DEC-072  2026-08-18  the third family is Stash 21.0, and the rated run is played fresh rather than spliced onto S087's
Tags:         measurement, rating, gauntlet, references, ccrl, s088, dec-067, dec-069, dec-071

Context:      S087 returned **2570 +/-25, SOFT**. The interval criterion was met;
              the anchor-stability one was not. The spread across anchors was
              **83.1 Elo** against the ~30 the procedure allows, and it was
              isolated rather than diffuse: Blunder 7.1.0, Blunder 8.5.5 and
              Leorik 2.4 reproduce each other's CCRL ratings to within 16 Elo
              across a 440-point span, while Leorik 2.1 comes out about 82 Elo
              above its listed rating under every other anchor -- six standard
              errors at 668 games per pairing. **Two families cannot arbitrate a
              disagreement between two families.**

              DEC-071 then set the target at 3000, a 430 Elo climb. A finish
              line cannot be read off an instrument 83 Elo wide, and changing
              the reference set *during* the climb would make every reading
              across it incomparable. So the set is fixed now and not later.

              Candidates, all read from the CCRL Blitz list on 2026-08-18 (list
              computed 2026-08-15) rather than from memory: **Stash 21.0 2713
              +/-14**, Stash 25.0 2932 +/-18, Weiss 0.10 2847 +/-17, Weiss 1.0
              2896 +/-18, Zurichess Neuchatel 2920 +/-9, Monolith 2 3011 +/-15.

              One fact measured before deciding anything: `src/`, `tests/`,
              `CMakeLists.txt` and `cmake/` are **byte-identical between
              `a9f2b33`** -- S087's rated commit -- **and `c2f1c43`**, so S088
              rates the same engine S087 rated and the two figures are directly
              comparable rather than approximately so.

Decision:     **The fifth rung and third family is Stash 21.0**, CCRL Blitz 2713
              +/-14. Built by the agent from `gitlab.com/mhouppin/stash-bot` at
              tag `v21.0` (`6dc8c9cd`) with the project's own
              `utils/unix_build.sh` at `ARCH=x86-64-bmi2`, installed as
              `/home/max/ws/engines/stash-21.0.bin`, recorded in
              `references.tsv` with tag, source and md5 per DEC-069. Third
              author, third language, third evaluation, so it votes on Leorik
              2.1 independently of both incumbents.

              **The rated run is a fresh 3340-game gauntlet** -- 334 rounds per
              pairing, 668 games each, five pairings -- and `rating.sh`'s rated
              default moves from 167 rounds to 334 to match. 668 is measured,
              not chosen: S087 got +/-34 at 334 games per pairing and +/-24 to
              +/-28 at 668. In a gauntlet only chesso plays everybody, so the
              graph is a star and chesso's rating under a given anchor is fixed
              by that one pairing alone; adding a rung adds an independent
              estimate and narrows no existing one.

              Taken by the agent, with the analysis here, under S088's own
              delegation of the choice ("the choice is recorded in the results
              file with the reason"), DEC-069's grant of the builds, and DEC-041.

Rejected:     **Weiss 0.10 (2847), Weiss 1.0 (2896), Zurichess Neuchatel (2920),
              Stash 25.0 (2932) and Monolith 2 (3011).** Every one sits at or
              above 2847, at least 277 Elo above chesso's measured 2570, so
              chesso's score against it would sit in the tail of the logistic
              curve where the estimate is dominated by the curve rather than by
              the games. That is the exact condition S087's bracketing run 1 was
              rejected for, in the other direction. Zurichess is additionally
              Go, the same language as Blunder, so it buys less independence per
              rung than a third language does.

              **Reusing S087's 2672 rated games and playing chesso against Stash
              alone.** It costs about 30 minutes instead of two and a half
              hours, the binary is provably the same one, and S087 itself set
              the precedent by combining two PGNs. Refused because the run's
              whole purpose is to arbitrate a disagreement *between references*,
              and splicing one pairing played by itself at concurrency 12 onto
              four pairings played interleaved introduces a per-pairing
              difference in machine load precisely where that disagreement is
              being measured. Buying two hours by contaminating the measurement
              being taken is a bad trade, and `rating.sh` would have needed a
              partial-run mode the step's `touches:` does not permit.

              **Lowering `CONCURRENCY` below 12.** DEC-048 and DEC-050 stand.
              S087 looked for forfeits in 3148 games across four runs and found
              none; S088's bracketing run found none in 340 more. Detection
              rather than avoidance, and one forfeit still invalidates a run.

              **Adding two engines to reach four families.** One new family
              satisfies the accepts -- at least three families, at least five
              rungs -- and every further rung costs another 668 games, about
              half an hour, on this run and on every milestone re-run after it.

Consequences: The reference set is five engines, three families, three authors
              and three languages, spanning 2389 to 2829 with chesso near the
              middle. A rated run is now **3340 games and about two and a half
              hours**, and `plan.md`'s rule that `./rating.sh` is re-run at
              milestones costs that each time.

              **Stash's md5 is an identity record and not a rebuild check.**
              `utils/unix_build.sh` is a two-pass PGO build whose profile
              counters vary run to run, so a rebuild from the same tag will not
              reproduce the hash. Every other row's md5 does. `references.tsv`
              says so at the row.

              **Stash v21.0 carries a startup deadlock that cannot reach a
              match, and it is recorded rather than assumed away.**
              `engine_thread` parks on an unconditional `pthread_cond_wait` with
              no predicate re-check, so a `go` arriving before the thread parks
              loses the broadcast and the search never starts. Measured: 5/5
              hangs from a shell `printf | engine`, 15/25 from
              spawn-then-write-with-no-wait, **0/200 and 0/240 from anything
              that waits for `uciok`** -- the last being 240 full GUI-paced
              cycles at 12-way concurrency, which is the match condition.
              fastchess waits, and `rating.sh`'s `identify()` sends only `uci`
              and `quit`. If it ever did fire, the symptom would be a hung
              engine scored as a time loss, which the forfeit check invalidates
              the whole run on rather than averaging in.

              **`uci_name` and `ccrl_name` differ for the first time**: the
              binary answers `id name Stash v21.0` and the CCRL entry is `Stash
              21.0`. Both columns already existed; this is the first row that
              needs them apart, and the identity guard would refuse the run on
              the wrong one.

## DEC-073  2026-08-18  a rated run drops to concurrency 6, because oversubscription is not neutral across foreign engines

**VOID as to its Decision, 2026-08-18 — superseded by DEC-075.** The owner
reversed the concurrency drop: rating runs saturate the machine at 12. The
*evidence* below is not void and is not restated elsewhere — the three forfeits,
their millisecond overshoots, the 2.4 Elo measured impact of removing them, and
the exact reproduction of S087's solve — so follow the pointer for the decision
and read the numbers here. **One inference below is also wrong and DEC-075
corrects it**: this entry blames the anchor shift on concurrency-12
oversubscription, and S087 ran at concurrency 12 as well, so concurrency cannot
explain a difference between the two runs.
Tags:         measurement, rating, gauntlet, concurrency, forfeit, s088, dec-067, dec-048, dec-050, dec-072

Context:      S088's rated run -- 3340 games, five engines, `10+0.2`,
              concurrency 12, 2 h 27 m 28 s -- returned **RATING-RUN-INVALID**.
              2829 `adjudication`, 508 `normal` and **3 `time forfeit`**, all
              three Stash v21.0, losing by **1118 ms, 1309 ms and 149 ms** at
              rounds 619, 774 and 884. All three games went to chesso.

              **Not the startup deadlock** DEC-072 records: that hangs forever
              and would fire at the first `go`, and these are millisecond
              overshoots spread through the middle of the run.

              The forfeits are the visible part. The rest is not visible and is
              worse. Against the **same binaries and the same time control**,
              chesso's solved rating moved from S087:

                  anchor              S087      S088 @12    shift
                  Blunder 7.1.0       2559.3    2546.4      -12.9
                  Leorik 2.1          2492.5    2485.3       -7.2
                  Blunder 8.5.5       2574.6    2623.7      +49.1
                  Leorik 2.4          2575.6    2606.9      +31.3

              The solve is not the difference: re-solving S087's own combined
              PGN with the identical `ordo` command reproduces 2559.3 / 2492.5 /
              2574.6 / 2575.6 and their intervals **exactly**. It is in the
              games -- chesso scored **44.3 % against Blunder 8.5.5 against
              S087's 37.6 %**, 6.7 points on 668 games each side, about 2.5
              standard errors, while Leorik 2.1 and Blunder 7.1.0 moved under
              1.5 points.

              DEC-067 accepted concurrency 12 against foreign engines on the
              argument that DEC-050's "oversubscription hits both sides equally"
              may not hold when time management differs, and chose **detection
              instead of avoidance**. It was detected.

Decision:     **A rated `rating.sh` run drops to `CONCURRENCY=6`** -- one game
              per physical core on the DEC-049 machine. DEC-067's "12 stands,
              with detection instead of avoidance" is **superseded for
              `rating.sh`**, exactly as DEC-067 itself instructed: "the rated run
              drops to CONCURRENCY=6 and DEC-067 is superseded rather than
              reinterpreted". The bracketing run showed no forfeit and the rated
              run did; the remedy is the one that was pre-authorised.

              **DEC-048 and DEC-050 stand unchanged for `fastchess.sh`.** There
              both sides are the same binary, so oversubscription really does
              hit them equally and inflates variance rather than biasing the
              result. That argument is exactly what fails here.

Rejected:     **Keeping concurrency 12 and dropping the three forfeit games.**
              Measured rather than argued: removing them moves only Stash's
              solve, 2590.7 -> 2588.3, **2.4 Elo**, and leaves the other four
              anchors identical to a decimal. Refused because the forfeits are
              evidence *about the other 3337 games*, not a defect confined to 3.
              An engine that overshoots by 1.3 s three times has been late in
              many more without crossing the line, and that damage is invisible,
              unequal across engines, and lands on the anchors. The rule is zero
              and one invalidates; this is the case it was written for.

              **Raising Stash's `Move Overhead` from its default 20 ms.** It
              targets the forfeits directly and would keep the 2 h 30 m
              throughput. Refused on two grounds: it changes a reference
              engine's configuration away from the default its CCRL rating was
              earned at, and it treats the symptom -- the +49.1 on Blunder 8.5.5
              is not a forfeit and would survive it untouched.

              **Dropping Stash and choosing a different third family.** The
              forfeits are Stash's, so removing Stash removes them. Refused
              because the evidence says the load is the problem and Stash is
              only where it broke the surface: the anchor shift is on Blunder
              8.5.5 and Leorik 2.4, not on Stash, and an 83 Elo spread was
              already present in S087 with no Stash in the set at all.

              **A cheap bracketing run at concurrency 6 as insurance first.**
              Refused on arithmetic. 3 forfeits in 668 Stash games is 0.45 %; a
              340-game bracket contains 68 Stash games and expects **0.3**
              forfeits, so a clean bracket would have proved essentially nothing
              and cost 31 minutes. Cheap insurance that cannot fail informatively
              is not insurance.

              **Slowing the time control instead.** S088 excludes it: it would
              make the figure incomparable with S087's, which is the comparison
              the step exists to make.

Consequences: **A rated run now costs about 5 hours instead of 2 h 30 m.**
              `plan.md`'s rule that `./rating.sh` is re-run at milestones -- after
              any landed step an SPRT credits with 20 Elo or more -- now costs a
              night rather than an afternoon, against a plan expecting 40 to 60
              SPRT verdicts. Measurement capacity was already the binding
              constraint and this tightens it.

              **S087's 2570 was measured at concurrency 12 and inherits this
              doubt.** It is not withdrawn here and nothing is edited in
              `plan_done/`: S088's valid run is what replaces it. But the two
              were not taken with the same instrument, and any comparison
              between them is between two instruments as well as two runs.

              **`rating.sh`'s concurrency default is NOT changed in the same
              turn as this entry, and the reason is a hazard worth recording:
              bash reads a script lazily from disk while executing it**, so
              editing `rating.sh` during the 5-hour run this decision launched
              could corrupt it at any point. The edit is deferred to after the
              run. This amends S088's `touches:`, which permitted a `rating.sh`
              edit only where the set size is wired in; the concurrency default
              is not set size, and the amendment is recorded here rather than
              taken silently.

              The invalid run's evidence is kept, not deleted --
              `adocs/data/S088_rated_c12_INVALID_*`. It is the only measurement
              this project has of what concurrency does to a foreign-engine
              gauntlet, and the concurrency-6 run is its controlled comparison:
              same five binaries, same book, same time control, same game count,
              one variable.

## DEC-074  2026-08-18  the rating gauntlet is re-run on substantial work, not on a 20 Elo threshold
Tags:         measurement, rating, gauntlet, cadence, planning, s088, dec-071, dec-073, dec-067

Context:      `plan.md` and `specs.md` both carried the same rule since DEC-071:
              `./rating.sh` is re-run at milestones, **after any landed step an
              SPRT credits with 20 Elo or more**, because every other figure this
              project holds is self-play against an earlier chesso and the factor
              between that scale and the CCRL one is unmeasured.

              DEC-073 then doubled what that rule costs. A rated run went from
              2 h 30 m to about 5 hours when concurrency dropped to 6, against a
              plan expecting 40 to 60 SPRT verdicts. A 20 Elo threshold is not a
              rare event in a 430 Elo climb, so the rule as written would have
              spent a night on an absolute figure repeatedly, out of the same
              budget the SPRTs come from.

Decision:     **The gauntlet is re-run when substantial work has been done to
              the engine, not on a numeric Elo threshold.** Per-change decisions
              stay with the SPRT, which is what `fastchess.sh` is for and what
              every step in the plan is gated on. **The owner's decision**,
              given in this session: "the ELO re-check does not need to run that
              often. We can use normal SPRT tests and run the ELO re-evaluation
              only when substantial work was done to the engine."

              **The trigger is deliberately not derivable and is not an agent's
              to assume.** "Substantial" is the owner's judgement. An agent does
              not book a five-hour gauntlet because it has totted up enough
              landed steps; it proposes one, or runs one when asked. This is a
              narrowing of DEC-041, which grants the agent measurements without
              asking, and the narrowing is the point: the cost is now large
              enough that the owner sets the cadence.

Rejected:     **Keeping the 20 Elo threshold and paying the 5 hours.** It is the
              rule DEC-071 wrote and it has an argument -- the two scales are
              still unrelated by any measurement. Refused by the owner on cost:
              the SPRT already decides every individual change, and the absolute
              figure answers a different and less frequent question.

              **Replacing 20 Elo with a larger threshold, say 50 or 100.** It
              keeps the rule derivable, which the workflow generally prefers.
              Not taken: any threshold sums self-play deltas that DEC-019 has
              three times shown do not transfer, so the number would look
              precise and rest on the quantity least trusted in this repository.

              **Dropping the re-run rule entirely.** Refused: the whole point of
              S087 and S088 is that the distance to 3000 cannot be read from
              self-play, so the instrument has to be re-read sometimes. The
              change is to the cadence, not to the instrument.

Consequences: `plan.md` and `specs.md` lose the 20 Elo clause in the same commit
              as this entry. DEC-073's consequence paragraph -- that a milestone
              re-run "now costs a night rather than an afternoon" -- stands as
              written and is **no longer a recurring cost**, because the
              milestones are rarer. Nothing in DEC-073 is edited; it is
              append-only and this entry is where the cadence changed.

              The measurement budget goes back to the SPRTs, which is where the
              plan's 40 to 60 verdicts and 150 to 250 machine hours were always
              going to be spent.

## DEC-075  2026-08-18  rating runs saturate the machine at concurrency 12, and a low time-forfeit rate is tolerated instead of voiding the run
Tags:         measurement, rating, gauntlet, concurrency, forfeit, s088, dec-073, dec-067, dec-048, dec-050

Context:      DEC-073 dropped rated `rating.sh` runs to concurrency 6 after three
              Stash time forfeits voided a 3340-game run. The owner rejected the
              premise: **"I do not understand why the cores are loaded only half.
              We should always aim to saturate them, so on this machine you
              should run at concurrency 12."**

              Measured on the concurrency-6 run while it was in flight, rather
              than argued: **6.2 of 12 threads busy**, `10.6 games/min` against
              concurrency 12's `22.6`. A game runs two engine processes but only
              the side to move thinks, so concurrency 6 leaves half the hardware
              threads idle and costs **2.1x** throughput, not the ~1.3x an
              SMT-only penalty would suggest. The owner is right on the load and
              the size of it was understated.

              **DEC-073 also contains a wrong inference, and it is the one the
              decision leaned on.** It reads the +49.1 Elo shift on Blunder 8.5.5
              between S087 and S088 as oversubscription damaging foreign engines
              unequally. **S087 ran at concurrency 12 too.** Concurrency was
              identical in both runs and cannot explain a difference between
              them. What the evidence supports is narrower: the forfeits show
              Stash is time-stressed at 12, and the anchor shift is
              **unexplained**, its candidates being the set composition changing
              from four engines to five and ordinary run-to-run variance at about
              2.7 standard errors. The agent stated the stronger claim; this
              entry withdraws it.

Decision:     **1. Rating runs saturate the machine: `CONCURRENCY` returns to
              `nproc`, 12 here.** DEC-073's drop to 6 is reversed and marked VOID
              as to its decision. DEC-048 and DEC-050 apply to `rating.sh` as
              they do to `fastchess.sh`. **The owner's decision.**

              **2. A low time-forfeit rate is tolerated rather than voiding the
              run.** The zero-forfeit rule stands as the thing that caught this
              at all, but a forfeit is now weighed instead of being fatal.
              **The owner's decision.**

              **3. The concurrency-6 run in flight finishes, judged under the old
              rule; the new rule applies from the next run.**
              *(AMENDED 2026-08-18 by DEC-076: the tolerance applies to this run
              too. This decision now concerns concurrency only.)* The owner's
              instruction verbatim: "finish this run. You can apply this rule
              from the next run." So S088's figure comes from a concurrency-6
              run, and every run after it is at 12 -- which the results file
              must state, because it makes S088's number and its successors
              readings from two different instrument settings.

              **4. The threshold and its denominator are the agent's proposal,
              not the owner's**, and are stated here to be overridden rather than
              discovered later in a script: a run is void above **1 % of an
              individual engine's own games**, and any *non-forfeit* unexpected
              termination -- crash, disconnect, illegal move, stall -- still
              voids at **zero**.

Rejected:     **A whole-run forfeit rate.** 3 forfeits in 3340 games is 0.09 %
              overall but **0.45 % of Stash's 668**, so a 0.5 % whole-run
              threshold would tolerate 16 forfeits concentrated on one engine
              while reporting a comfortable number. The denominator has to be the
              engine's own games or the rate hides exactly the case it is for.

              **Treating a crash like a forfeit.** A time forfeit is a real game
              result under time pressure and the loser earned it. A crash or a
              disconnect is a broken instrument, and no rate of it is acceptable.
              The two were one check; they are now two.

              **Raising Stash's `Move Overhead`, dropping Stash, or excluding
              forfeited games from the solve.** All three were offered and none
              was chosen. Recorded because each remains available if the tolerated
              rate is exceeded rather than approached.

Consequences: A rated run costs about **2 h 30 m** again, and DEC-074's cadence
              -- re-run on substantial work, the owner's judgement -- stands
              unchanged on top of that. The two together undo DEC-073's
              "a night rather than an afternoon" entirely.

              **`rating.sh` must report what it now tolerates.** Counting
              forfeits is no longer enough: the run has to print the per-engine
              forfeit rate against the threshold, which side won the forfeited
              games, and the solved rating with and without them, so a tolerated
              forfeit is visible as a bias rather than absorbed as an average.
              On the voided run that bias was measured at 2.4 Elo on one anchor;
              nothing guarantees the next one is that small.

              **The edit waits for the running match.** Bash reads a script
              lazily from disk while executing it, so `rating.sh` is not touched
              until the concurrency-6 run finishes. DEC-073 recorded that hazard
              and it still binds.

              **S088's `accepts:` is amended in the same commit.** It requires
              "zero time forfeits, one invalidating the run", which is now false
              as a rule. It stays true as the gate applied to S088's own figure,
              per decision 3, and the step file states both.

## DEC-076  2026-08-18  the forfeit tolerance applies to the run in flight, and a hang counts as a forfeit
Tags:         measurement, rating, gauntlet, forfeit, hang, s088, dec-075, dec-072

Context:      DEC-075 decision 3 said the concurrency-6 run in flight would be
              judged under the **old** zero-forfeit rule, from the owner's
              "finish this run. You can apply this rule from the next run." At
              1269 of 3340 games that run took a forfeit, which under that
              reading voided it and left about three hours still to play for
              nothing -- and `rating.sh` exits on `RATING-RUN-INVALID` *before*
              the anchor sweep, so it would have produced no solve at all.

              **The forfeit is not what DEC-073 or DEC-075 assumed either.**
              Round 624, Stash v21.0 as White, **274 plies**, 1 m 35 s of normal
              play at about 0.2 s a move, and then at move 138 -- in a repetition
              dance, `135. Ke4 Rh4+ 136. Ke3 Rh3+ 137. Ke4 Rh4+`, both engines
              reporting `0.00` at depth 39 and 47 -- Stash **hung for 25360 ms**.
              Not the DEC-072 startup deadlock, which fires at the first `go` and
              never plays a move. Not a thin `Move Overhead` either: the three
              concurrency-12 forfeits overran by 149 ms, 1118 ms and 1309 ms, and
              this is twenty times the largest of them.

              **And concurrency was never the cause.** Stash forfeited 3 of 668
              games at concurrency 12, **0.45 %**, and 1 of 254 at concurrency 6,
              **0.39 %**. Indistinguishable. DEC-073 dropped the concurrency to
              fix this and it did not move.

Decision:     **1. The tolerance of DEC-075 applies to the run in flight as
              well.** It finishes and is judged by the 1 %-per-engine rate, not
              by zero forfeits. DEC-075 decision 3 is amended to concern
              concurrency only: this run stays at 6, every later run is at 12.
              **The owner's decision**, asked with the three hours and the lost
              anchor sweep on the table.

              **2. A hang counts as a forfeit and is tolerated under the rate
              threshold.** It is reported as a time loss and the game result is
              real. DEC-075's carve-out -- crash, disconnect, illegal move and
              stall void at zero -- is **not** widened to cover a hang that
              fastchess scores as a time forfeit. **The owner's decision**,
              against the agent's offered alternative of voiding on any overrun
              past about 2 s.

              **3. The results file must state that Stash's forfeits are hangs
              rather than margin overruns**, so a reader of the rate does not
              take it for ordinary time pressure.

              **4. `rating.sh` exits before the sweep on an invalid run, so
              S088's anchors are solved by hand** with the same
              `ordo 1.2.6 -s 1000 -F 95 -n 12 -W -D` command the script issues.
              The command is recorded in the results file so the figure is
              reproducible without the script.

Rejected:     **Killing the run and relaunching at concurrency 12**, which the
              agent recommended: concurrency 6 bought nothing measurable, and a
              valid figure was about 2 h 30 m away against roughly 3 h more for
              this one. Not taken -- the owner kept the 1 h 40 m already spent.

              **Dropping Stash for Weiss 0.10 (2847).** Stash hangs about 4 games
              in 1000 from a defect in its own search, and a reference engine
              that hangs is a poor instrument. Not taken: the rate is inside the
              threshold and the third family is what S088 exists to add.

              **Capping the tolerated overrun at about 2 s.** It would have kept
              Stash usable for margin overruns while still catching a hang, and
              it is the agent's proposal. Not taken; the rate is the only gate.

Consequences: S088's `accepts:` is amended in this step's commit. It required
              "zero time forfeits, one invalidating the run" and that is now
              false for this step's own figure, which is exactly the case
              AGENTS.md means by amending a plan rather than deviating from it
              silently.

              **S088's figure comes from concurrency 6 and every later one from
              concurrency 12**, so the results file records the setting beside
              the number. On the evidence above the two settings are not expected
              to differ, but "not expected to" is not a measurement and the next
              run is the first that could show it.

              **Stash stays in the set with a known defect, recorded.** If its
              rate ever crosses 1 % of its own games, DEC-075's threshold voids
              the run and the rejected options above come back.

## DEC-077  2026-08-18  the quoted figure is the five-anchor mean, and the third family did not close the spread
Tags:         measurement, rating, gauntlet, anchors, ccrl, s088, dec-072, dec-071

Context:      S088 added a third family to arbitrate S087's 83.1 Elo anchor
              spread, on the premise that "two families cannot arbitrate a
              disagreement between two families; a third can". The valid
              concurrency-6 run, 3340 games, every interval inside the +/-30 the
              procedure requires:

                  anchor          CCRL    chesso    95 %
                  Blunder 7.1.0   2389    2533.8    +/-24.8
                  Leorik 2.1      2568    2476.7    +/-24.0
                  Blunder 8.5.5   2664    2586.0    +/-23.7
                  Stash v21.0     2713    2597.6    +/-24.4
                  Leorik 2.4      2829    2598.5    +/-27.7

              **The premise was wrong and the step's own goal is not met.**
              Spread across five anchors is **121.8 Elo** against the ~30
              allowed. Leorik 2.1 is still the low outlier and dropping it leaves
              **64.7**, still more than twice the allowance. There is no single
              dissenter to name.

              **What the third family bought instead is the shape of the
              disagreement.** With Leorik 2.1 set aside the solved rating rises
              monotonically with the anchor's own rating -- 2389 -> 2533.8,
              2664 -> 2586.0, 2713 -> 2597.6, 2829 -> 2598.5 -- and flattens at
              the top. That is compression, not scatter: across a CCRL span of
              440 Elo the measured differences span **375.3**, a ratio of
              **0.853**. S087 argued a scale artifact was excluded because its
              two extreme anchors agreed to 16 Elo; at 668 games a pairing with
              a fifth rung, they disagree by 64.7 and that argument no longer
              holds.

Decision:     **The quoted figure is the five-anchor mean, `chesso ~= 2559 CCRL
              Blitz, 95 % +/-25, SOFT`**, with the full spread of 121.8 Elo
              printed beside it and the 64.7 ex-Leorik-2.1 figure of 2579
              reported but not quoted.

              The rule this follows is S087's own: dropping an inconvenient
              reference is how a measurement gets talked into a nicer answer. The
              quoted number is therefore the **lower** of the two and carries the
              **larger** spread, which is what the step's `accepts` means by "a
              recorded decision rather than the smaller number".

              Taken by the agent under S088's delegation and DEC-041. It is the
              project's headline strength figure and the owner may prefer 2579 or
              a strength-matched anchor; this entry is written to be overridden.

Rejected:     **Quoting 2579, the four-anchor mean without Leorik 2.1.** Leorik
              2.1 really is anomalous -- CCRL rates it on 990 games against
              2437 to 2502 for the two that agree, and it has now dissented in
              two independent runs. But 64.7 Elo of residual spread means the
              remaining four do not agree either, so dropping it buys a nicer
              number rather than a sound one.

              **Quoting 2594, the mean of the three anchors nearest chesso's own
              strength.** It has the best argument on paper -- least
              extrapolation across a compressed scale, and those three agree to
              12.5 Elo. It is also the highest of the three candidates, chosen
              after seeing the numbers, on a rule invented for this occasion.
              Refused for exactly that reason.

              **Reporting no figure at all until the spread is inside 30.** The
              procedure calls a wide spread SOFT rather than fatal, and a figure
              labelled soft with its spread printed is more useful than silence
              to a plan whose target is 3000.

Consequences: **chesso has not measurably moved.** `src/` is byte-identical to
              S087's rated commit, and S087's 2570 sits between this run's 2559
              and 2579. The instrument changed; the engine did not. That is the
              right outcome for a step that added a reference engine.

              **The spread is now a property of the scale, not of one engine, and
              no reference set fixes it.** The largest uncounted term is the time
              control: CCRL Blitz is "equivalent to 2'+1" on an i7-4770K" and
              this runs at `10+0.2`. A step that wants the spread inside 30 has
              to attack that, not add a sixth rung -- which is a decision for
              whoever writes it, not this one.

              **An open risk with numbers behind it, for the next run.** chesso
              scored 37.6 % against Blunder 8.5.5 in S087 (4 engines,
              concurrency 12), **44.3 %** in S088's voided run (5 engines,
              concurrency 12) and 39.1 % in the valid one (5 engines, concurrency
              6). The concurrency-12 five-engine run is the outlier of the three.
              Stash is a PGO C engine at 3.76 Mnps sharing a pool with a .NET and
              two Go engines, so more contention at 12 is a plausible mechanism
              and single runs cannot establish it. DEC-075 puts every later run
              at concurrency 12, so the next one is the test: if it reproduces
              44 % against Blunder 8.5.5, the pool composition is real and
              DEC-075 needs revisiting.

## DEC-078  2026-08-18  S089's accepts named the wrong test file for the S036 case
Tags:         planning, testing, s089, s036, dec-071

Context:      S089's `accepts:` requires that "`tests/test_search.cpp` keeps a
              case that a 1 ms clock returns a legal move, which is the S036
              defect". `tests/test_search.cpp` has **no UCI-level time test and
              never had one**. The case is `tests/test_engine.cpp:807`,
              `a one millisecond clock answers without a stop`, inside the
              `engine: uci layer` fixture -- which is the suite that owns
              `require_playable()`, so it is also the only place the *legality*
              half of that clause can be asserted.

              Found by the implementing agent when the step met the code, and
              reported rather than worked around.

Decision:     **The `accepts:` clause is amended to name
              `tests/test_engine.cpp`, and the test stays where it lives.** The
              S036 case was kept and strengthened in place -- `go wtime 1
              btime 1` added to `every kind of go answers with one legal
              bestmove`, so the sudden-death path is covered by an assertion of
              legality rather than of a non-null bestmove. Recorded by the agent
              under DEC-041; the plan was wrong when it met the code, and
              AGENTS.md requires an entry and an amendment rather than a silent
              deviation.

Rejected:     **Moving the test to `tests/test_search.cpp` to satisfy the text.**
              It would put a UCI-level clock test in the search unit suite, away
              from the fixture that can assert legality, to make a sentence true.

              **Leaving the clause and quietly testing elsewhere.** That is the
              silent deviation the rule exists to prevent, and the next reader
              would have gone looking in the wrong file.

Consequences: The step file is amended in the same commit as the verdict. No
              behaviour changes and no test is weakened -- the clause is
              satisfied more strongly than it asked, in the file where it was
              already satisfied.

## DEC-079  2026-08-19  S094's three commits are kept at a measured zero, and the win is elsewhere
Tags:         search, quiescence, transposition, measurement, s094, s092, s099, inv-4, inv-6, dec-071

Context:      S094 split into three commits so each change could be measured
              separately, as its `accepts:` demands. Two verdicts and one
              neutrality proof came back:

                  f1e6d24  quiescence probes and stores at TT_DEPTH_QS = -1
                           3000 games, 2 h 07 m, NO BOUND REACHED, LLR -1.32
                           Elo -0.23 +/- 9.32, nElo -0.31 +/- 12.43, 49.97 %
                  7d2da9d  entry carries int16_t eval, read by nothing
                           BEHAVIOUR-NEUTRAL, INV-6: identical node counts
                           164123 / 670488 / 84351 and identical best moves
                           c3d5 / e2a6 / d7c8q, verified independently
                  22a74f2  quiescence stands pat on the stored value
                           H0 accepted, 1954 games, 1 h 23 m, LLR -2.21
                           Elo -6.40 +/- 11.42, nElo -8.64 +/- 15.40, 49.08 %

              0 time forfeits in either match, checked from each run's own PGN.

              **The hazard the step names has no referent.** It warned that
              widening the entry changes entries-per-bucket and therefore
              replacement. The table is direct-mapped -- one entry per slot, no
              buckets -- and `tt_resize()` floors the count to a power of two,
              which absorbs any entry size from 17 to 32 bytes: 4 MB buys 131072
              entries at 20, 24 and 32 alike. `sizeof(tt_entry_t)` stayed **24**
              because `eval` landed in padding the key's alignment already
              reserved.

              **Why zero was the right answer and was predictable.** Of 2530591
              quiescence nodes reaching the probe, only **19901, 0.79 %**, found
              an entry: depth-preferred replacement evicts a `TT_DEPTH_QS` entry
              whenever any main-search store lands on the slot. Of those, 11882
              carried an evaluation, and the stored value differed from a fresh
              `evaluate_lazy()` on **163 of 310197 nodes, 0.05 %**.

Decision:     **All three commits are kept, with the verdict recorded as zero.**
              The house rule is explicit that a verdict of zero is recorded as
              zero and the feature may still be kept with the reason stated;
              S005, S006 and S015 are the precedent. The reasons here are
              specific rather than sentimental:

              **1. `f1e6d24` is not only the probe.** It fixed two real defects.
              `de_normalize_score()` excluded `+/-MATE_MAX`, so a mate with no
              legal reply -- which quiescence is the first code to reach --
              normalised to exactly `-MATE_MAX` and never turned back into a
              distance. Four whole-search mate cases were observed red,
              `REQUIRE( 0 == 2 )` at depth 3 and `REQUIRE( 4 == 5 )` at depth 9.
              Reverting the commit would revert that. `tt_store_entry()` also
              asserted a non-zero move, which standing pat has not.

              **2. `7d2da9d` is the substrate two later steps consume.** S092's
              improving flag compares a static score across plies and S099 learns
              a correction from the difference between the static score and what
              the search returned. Both want the value in the entry. It is
              behaviour-neutral and costs no table entries, so it carries no risk
              to hold.

              **3. `22a74f2` measured "not a +10 improvement", which a true zero
              satisfies.** -6.40 +/- 11.42 does not establish a regression, and
              the 0.05 % of nodes that differ says one is not there to find.

              Taken by the agent under DEC-041 and the record-a-zero rule. The
              owner may prefer to revert `22a74f2`, which is the one commit whose
              removal costs nothing downstream.

Rejected:     **Reverting `f1e6d24` because the probe is worth nothing.** It
              would take the mate-score fix with it. If the probe is ever removed
              on its own, that fix and its four red cases stay.

              **Spending a third SPRT to separate the probe from the store within
              `f1e6d24`.** Two verdicts already cost 3 h 30 m and both landed on
              zero; a third would resolve a component of a zero.

              **Declining SPRT B on the node-count evidence**, the way S075
              declined a match on a vector already measured worse. It was the
              cheaper argument and it was not taken: the machine was idle, the
              step asks for a verdict per change, and buying the verdict cost
              less than the amendment for not buying it would have.

Consequences: **The win this step found is in none of its commits.** The
              reverse-futility site already had the stored value on **105612 of
              its 1074051 `evaluate()` calls, 9.8 %, and 0 of them disagreed**
              with a fresh call. Reading it there is a pure speed-up on the full
              evaluation -- 83.35 ns a call on this machine -- and is
              dischargeable by INV-6 node counts rather than owing a verdict. It
              is one change at a time and it is not S094's. **S103 is created for
              it.**

              INV-4 still holds and was checked rather than argued: the stored
              number is `evaluate_cheap() + evaluate_expensive()`, derived from
              the accumulators `make_move` maintains, and nothing stopped
              maintaining them. The debug build, which asserts the accumulators
              against a full recomputation on every make and unmake, is green at
              all three commits.

## DEC-080  2026-08-19  S103's touches field was narrower than its own accepts, and two line-range citations had gone stale
Tags:         planning, testing, documentation, measurement, s103, s094, inv-6, dec-041, dec-079

Context:      Two things came up while S103 was executed, neither of them a
              change to what the engine does.

              **1. The `touches:` field could not satisfy the `accepts:`
              field.** It read `src/search.cpp at the reverse futility site
              only`. The `accepts:` field requires "a test that the value read
              from the entry equals what a fresh evaluate() returns for that
              position, observed red by planting a different value". Reverse
              futility fires only at a node that is not the root, not on the PV,
              at or past `RFP_MIN_PLY` and within `RFP_MAX_DEPTH`. **No call to
              `search()` can place a test on such a node**: the root is ply 0
              and is a PV node, and which of its descendants ends up non-PV at
              ply 3 is not something a caller chooses. So the test has to drive
              `negamax` directly, the way the S094 cases drive `quiescence`,
              and that needs a declaration in `src/search.hpp` and a case in
              `tests/test_search.cpp` -- neither of which the field allowed.

              `negamax` already had external linkage; only the declaration was
              missing. So the header change moves no code the compiler emits,
              which matters here because the step's whole claim is neutrality
              plus a timing.

              **2. Three documented line ranges were stale, found by the
              completion check rather than by a reader.** `DEV_MANUAL.md`'s tune
              build section printed a four-point `RfpMargin` node sweep in the
              present tense -- "213509 is what the release build reports, to the
              node" -- measured at S068's completing commit and wrong from some
              later step onward; the release build reports 164123. `plan.md`'s
              retention paragraph named moltke **0.11.0** where
              `installed_plugins.json` names 0.12.0, and located `prune_plan()`
              and `PLAN_DONE_KEPT` at 0.11.0 line numbers. `status.md`'s two
              parked items cited `plan.md:174-188` and `specs.md:199-204`; the
              paragraphs are at `plan.md:265-282` and `specs.md:236-241`, and
              both citations had already been corrected once before, on
              2026-08-17.

Decision:     **The `touches:` field is amended in place to name
              `src/search.hpp` and `tests/test_search.cpp`, with the reason in
              the field**, following DEC-078: the plan met the code and lost, so
              it is amended rather than deviated from silently.

              **The stale figures are re-measured rather than deleted, in the
              same commit as the step.** `DEV_MANUAL.md` now carries 164123 /
              223454 / 476911 / 743308 at `RfpMargin` 75 / 100 / 300 / 2000,
              re-measured at S103's commit on `build-tune`, plus a paragraph
              saying the numbers move with the search and have been stale once.
              `plan.md` names 0.12.0 and both sets of line numbers. `status.md`
              names the current ranges and records that a line range into a
              growing file is a claim with a short life.

              Taken by the agent under DEC-041. Neither item changes what the
              engine does; S103's own verdict is separate and is INV-6 on node
              counts.

Rejected:     **Testing reverse futility through `search()` on a position
              contrived so the site fires somewhere in the tree.** What it would
              assert is a whole-search score, which the change does not move --
              the node counts are identical -- so the test would pass equally on
              the code without the read. Non-vacuity is the requirement, and
              that shape cannot meet it.

              **Making `negamax` `static` and testing through a shim.** It is
              not static now, so this would be a codegen change made to satisfy
              a field, in a step measuring a 2.5 % timing.

              **Deleting the stale sweep from `DEV_MANUAL.md` instead of
              re-measuring it.** The paragraph exists to show the tune build
              honours a `setoption`, which is a real property, and it is the
              only place that is demonstrated.

              **Fixing the citations silently.** Both had been fixed once
              already without anything recording that they go stale by
              construction; the second repair is the evidence that the pattern
              is the problem, not the two numbers.

Consequences: `src/search.hpp` now declares `negamax`, with the same "tests only"
              comment `quiescence` carries. Nothing outside `src/search.cpp`
              calls either.

              **A line-range citation is now known to be the least durable claim
              in these documents** -- three of them stale at once, two of them
              for the second time. Nothing enforces them; `tools/plan_prose_check.py`
              checks step ids in prose and not line numbers. No step is created
              for a checker: it would need to know which paragraph a range meant,
              which is the thing the range fails to express. Prefer a quoted
              phrase or a grep-able heading over a range when writing one.

## DEC-081  2026-08-19  The tree shape is the binding constraint, not the evaluation; DEC-033's ordering conclusion is amended
Tags:         planning, search, evaluation, measurement
Context:      DEC-033 measured 160 expensive moves re-asked at 16 times the
              search and concluded that chesso is evaluation-limited: sixteen
              times the *nodes* removed 24.1 % of the error, 29.8 % after S028's
              fit, and the plan's whole order followed from that. The
              measurement is sound and is not withdrawn. What it does not say is
              how many *plies* sixteen times the nodes buys, and in this tree
              that is about four. Measured 2026-08-19 on one position after
              `e4 e5 Nf3 Nc6 Bb5 a6`, `go movetime 250`: chesso reaches **depth
              12 on 1448572 nodes**; Stockfish reaches **depth 15 to 16 on
              158837 nodes**. Nine times the nodes, four plies shallower. So the
              engine is not paying for depth at the market rate, and a node
              doubling is the wrong unit to have priced the plan in.
              Independently, the published record in this rating band says the
              same thing from the other side: **Leorik 2.5 is 2917 CCRL Blitz
              with less evaluation than chesso already ships** -- no king
              safety, no threats, no outposts, no bishop pair, knight mobility
              commented out -- and its single largest measured version jump,
              2.0.2 at +436 CCRL, was four search features and no evaluation
              change at all.
Decision:     By the owner, 2026-08-19, on the agent's review. The search block
              is ordered ahead of the evaluation block. DEC-033 is **not** VOID:
              its measurement stands, and its conclusion is restated with the
              period it applies to -- the evaluation *was* the binding
              constraint in 2026-08, S028 (+188.74) and S065 (+21.10) are that
              conclusion cashed, and what binds now is the shape of the tree
              those weights are searched through.
Rejected:     Keeping the evaluation-first order -- it was derived from a node
              measurement and the plies are what the opponent sees. Re-measuring
              in plies before deciding -- it costs hours to confirm a direction
              two independent lines of evidence already agree on, and the
              evaluation block is not cancelled, only sequenced second.
Consequences: `plan.md` is reordered. The evaluation steps keep their ids and
              their content and follow the search block, after the corpus work
              they depend on. Any future claim that a category is "the binding
              constraint" states the unit it was measured in.

## DEC-082  2026-08-19  A technique whose parts are inert in isolation is one plan step and one SPRT
Tags:         workflow, measurement, planning
Context:      "One change at a time -- two at once and neither number means
              anything" is a house rule and it exists because attribution is
              what makes a verdict worth having. It collides with one documented
              case. Stockfish's own removal test measures move-count pruning at
              **~0 Elo alone** and the shallow-depth pruning block it belongs to
              -- late move pruning, futility pruning, history pruning and quiet
              SEE pruning, all gated on the reduction-adjusted depth -- at
              **~204**. The parts prune overlapping sets of moves, so each one
              measured against a tree the others are absent from returns
              nothing. Four steps here would spend four verdicts to record four
              zeros and would leave the plan believing a real +60 to +120 does
              not exist.
Decision:     By the owner, 2026-08-19. Where a technique's parts are inert in
              isolation, **the parts are one plan step**, and that step is
              measured once. The rule "one step, one test" is preserved rather
              than excepted: what changes is where the step boundary is drawn,
              not how many tests a step owes. The owner further authorises
              postponing the per-part SPRT in any case of this shape. The step
              file must name its parts in `goal` and state in `accepts` that
              per-part attribution is deliberately forfeited, so the forfeit is
              on the record and not discovered later. A block that fails is then
              bisected, which is when the parts become separately measurable --
              a failing block is evidence that one part is wrong, which is
              exactly the attribution question the block form was not asked.
Rejected:     Four steps and four zeros -- honest and expensive, and it loses the
              Elo rather than measuring it. Landing the block and then
              back-filling per-part step files -- the same forfeit with extra
              paperwork and a false record of four measurements.
Consequences: Applies to the shallow-depth pruning block, and to any later
              technique that meets the same test. It is not a general licence to
              batch: the qualifying condition is *published or measured evidence
              that the parts are inert apart*, stated in the step file, not a
              wish to go faster.

## DEC-083  2026-08-19  The SPRT harness moves to the rating list's regime, and a pure speed-up is not measured in games
Tags:         measurement, workflow, tooling
Context:      Measurement capacity is the binding constraint on the whole plan
              and the harness was set three ways that spend it. **The time
              control is 10+0.2**, roughly twice the cost per game of the 8+0.08
              and 10+0.1 the engines whose figures this plan reads from test at.
              **The hash is 16 MB** in `fastchess.sh` and 64 MB in `rating.sh`,
              while CCRL Blitz runs **128 to 256 MB** -- so every verdict is
              taken in a table regime the rating list never runs, and the
              difference is not small: measured 2026-08-19 on this machine,
              16 MB against 512 MB at `go movetime 2000` from the start position
              is **36 % fewer nodes and 21 % lower nps**. **The book is
              balanced** (`8moves_v3.pgn`), which lowers the decisive-game rate
              and lengthens every run. Separately, the published sensitivity
              floor says a speed-up below about 0.7 % is invisible to an SPRT at
              long time control and 0.24 % at short -- so an SPRT is the wrong
              instrument for a change that leaves the node count identical.
Decision:     By the owner, 2026-08-19. `fastchess.sh` moves to **tc 8+0.08**,
              **Hash 128**, and an unbalanced opening book, after a time-forfeit
              check at the faster control. And **a change that is
              behaviour-neutral is accepted on an interleaved timing, never on
              an SPRT**: identical node counts and best moves from
              `tools/search_bench.py` discharge INV-6, and the strength claim is
              the measured nps difference converted at the published 1.43 Elo
              per percent at long time control, 2.10 at short, with the
              conversion named as a conversion and not as a verdict.
Rejected:     Leaving the harness alone to keep every verdict comparable with
              the existing record -- verdicts are per-change and against a named
              commit, so cross-comparability was never a property they had.
              Changing hash and book only -- fixes the regime and leaves the
              largest throughput term on the table.
Consequences: Verdicts taken after this entry are not comparable in absolute
              size with verdicts taken before it, which was already true across
              the DEC-049 machine move. ~~Roughly three times the verdicts per
              night.~~ **Measured when S105 landed, 2026-08-20: x1.67**, 23.1
              to 38.7 games a minute over two A/A runs of 1000 games in the
              same hour -- x1.41 from the control, x1.20 from shorter games.
              **And the book's stated reason does not hold at this engine's
              strength**: the pair score variance is unchanged within its error
              bar (ratio 1.022) while pairs decided by the opening rose 13.4 %
              to 19.8 %, and chesso self-plays the *balanced* book at 40.3 %
              draws -- already under Pohl's 45 % floor, where he measured
              91.6 % between engines 600 points stronger. The book is kept on
              the x1.20 it does buy. Evidence in `adocs/data/S105_calibration*`.
              This corrects a predicted magnitude, not the decision. The build
              step and the movegen micro-steps stop owing an SPRT each, which
              removes about six runs from the plan outright.

## DEC-084  2026-08-19  Published numbers are seeds and are refit; another engine's source and tables are not even seeds
Tags:         licensing, provenance, evaluation, tuning
Context:      DEC-016 forbids copying source and tables. This plan reads the
              published record heavily -- Elo figures per technique, functional
              forms, margin formulas, table shapes -- and the owner drew the
              line more precisely than DEC-016 states it, twice, while the
              review was running.
Decision:     By the owner, 2026-08-19. Two rules, and they differ by source.
              **From open literature -- papers, the wiki, articles, published
              write-ups -- a number may be used as a starting point.** **From
              another engine's source or tables, nothing is used, not even as a
              starting point.** And in both cases **no constant ships
              unfitted**: a seed is where our own tuner or SPSA run begins, and
              what ships is what our fit returned on our own self-play data. A
              fit that lands on the published value is a confirmation and a
              perfectly good outcome; what is not acceptable is a published
              number surviving into the binary because nobody refit it. The
              owner's stated reason is that there must be no room for even the
              suspicion that another person's work was taken.
Rejected:     Treating published article numbers like engine source and refusing
              them entirely -- it discards the direction the literature is read
              for, which is DEC-014's whole method, and a seed that is refit
              leaves no trace of itself. Allowing a seed to ship when the fit
              agrees -- indistinguishable in the artefact from not having fitted
              it, which is the thing being avoided.
Consequences: Every step that takes a formula from the literature states the
              seed, the source, and the fit that replaced it. This is why the
              evaluation steps below all end in a fit and why the search steps
              end in SPSA rather than in the margins their sources quote.
              DEC-016 is unchanged and this entry is narrower than it, not an
              exception to it.

## DEC-085  2026-08-19  Book learning is retired and threading stays off the plan, because the rating list forbids one and ignores the other
Tags:         planning, scope, measurement
Context:      Two items were carried on the assumption that they buy strength on
              the scale the goal is stated in. Checked against the CCRL Blitz
              testing conditions rather than assumed: the list runs **single
              CPU** on its main table, at **2 min + 1 s**, with **the engine's
              own book disabled**, **ponder off**, and **book learning and
              position learning off** -- an engine that cannot disable its own
              book is not listed at all. So S086 is not a small win, it is a
              rule violation with no upside, and Lazy SMP is worth **zero** to
              the number DEC-071 sets the target in.
Decision:     By the owner, 2026-08-19. **S086 is retired.** Its id is not
              reused. **Threading is not a phase-one step** and gets no id; it
              is reconsidered only if the goal is ever restated on a multi-CPU
              scale. The concurrency decisions DEC-048, DEC-050 and DEC-073 are
              untouched -- they are about running matches, not about the engine
              searching on more than one thread.
Rejected:     Keeping S086 behind a UCI option that ships off -- it would then be
              dead code carrying a maintenance cost for a feature the target
              scale forbids. Implementing Lazy SMP for testing throughput --
              throughput comes from concurrent games, which the harness already
              saturates, not from a parallel search.
Consequences: The list of what CCRL actually runs is now on the record and is
              the reference for any later "does this help the rating" question.
              The tablebase question is separate and stays open: the list does
              allow 4, 5 and 6 man tablebases, and the published gain for a
              hand-crafted engine is about 13 Elo, which is why Syzygy sits last
              and optional rather than retired.

## DEC-086  2026-08-19  The ten correction steps are folded into the steps they correct, and retired
Tags:         workflow, planning
Context:      Three plan_review audits produced, among other things, ten steps
              whose entire content is a correction to another step's `accepts`
              or `touches` field: S056, S057, S058, S059, S060, S061, S063,
              S079, S080 and S081. Each was placed immediately ahead of the step
              it corrects. They are right about what they correct and they cost
              zero Elo, and with the plan reordered around what the 3000 target
              needs they now sit as ten separate gates in front of work that has
              to happen. A correction to a step's acceptance criteria is not a
              unit of work; it is an edit to that step's file.
Decision:     By the owner, 2026-08-19. Each correction is **applied directly to
              the step file it corrects**, in the same commit as this entry, and
              the correcting step is retired. The ids are not reused. The
              corrected step files carry a line naming the retired id, so the
              audit finding remains followable from the step that answers it
              rather than from a step file that no longer exists.
Rejected:     Executing the ten as written -- ten commits and ten status
              regenerations to make ten edits that fit in one. Dropping the
              corrections with the steps -- the findings are correct and several
              of them are the difference between a step measuring the right
              thing and measuring nothing.
Consequences: `plan_todo/` loses ten files and the corrections survive in the
              steps that own them. The 2026-08-16 plan_review's parked re-run is
              still owed; folding a finding into the step it corrects is
              answering it, not closing it, and closure still needs a re-run
              that no longer reports it.

## DEC-087  2026-08-19  Second adversarial review: the block order holds, four techniques are demoted or retired, three cheap wins get steps
Tags:         planning, search, evaluation, measurement
Context:      The 2026-08-19 review (DEC-081 to DEC-086) was itself reviewed
              against per-patch SPRT records of engines that passed through this
              band -- Weiss, Lynx, Stash, Berserk and Ethereal commit logs and
              release notes -- and against the CCRL conditions read from the
              live pages. The four-block structure and the search-first order
              survived; Ethereal's own feature-removal ledger (history -759,
              LMR -249, quiet-pruning family -175, extensions -60, ProbCut -9)
              is a third independent confirmation of DEC-081. Nine contents did
              not survive, and two existence proofs sharpen the target: Weiss
              1.2 sat at 3055 CCRL Blitz with a 301-line evaluation, no
              capture, continuation or correction history, and the full pruning
              stack; Stash crossed 3000 at v27 and reached 3424 with no NNUE.
Decision:     By the owner, 2026-08-19, on the agent's second review.
              (a) **S096 check extensions is retired** -- Ethereal removed check
              extensions for +4.1/+4.5 and Stormphrax removed them as a
              simplification; LMR here already exempts checking moves and S097
              covers the forcing-line concern. The id is not reused.
              (b) **S023, S025, S110 and S111 move to a reserve tail** behind
              the 3000 push. Capture history failed four SPRTs at ~2600 (Lynx)
              and was STC-negative at Weiss -- below 3000 it is ordering
              overhead; its value returns as an input to reduction and pruning
              margins. Non-pawn and continuation correction history measure +3
              to +8 only above ~3100. **S099 stays**: pawn correction history
              measured +11.4 at ~2850 (Lynx), the one correction table with
              sub-3000 evidence.
              (c) **Three new steps.** S130, quiescence stand-pat takes the
              table score where the bound permits (Weiss +10.8/+12.1; S094
              already pays the probe). S131, quiescence searches non-capture
              queen promotions (this engine's own recorded TODO at the
              quiescence generation filter). S132, the soft time limit scales
              with the best root move's share of the nodes (Ethereal +9.9/+9.7,
              Lynx +3.6).
              (d) **S133 king-relative piece-square tables is added ahead of
              S126** -- Leorik 2.5 ~+88 CCRL and Berserk 4.3 ~+65 estimated are
              the largest documented evaluation item this plan had no step for.
              It rebuilds the INV-4 accumulators and the owner approved that
              cost explicitly.
              (e) **S114 drops the null-move verification search** (no evidence
              below 3000; the game_phase zugzwang guard stays) and keeps the
              eval-scaled reduction (a deeper base R alone measured +12.3 at
              Berserk). **S115 drops the volatility-based window width** (no
              band-level evidence) and keeps fail-soft plumbing, the widening
              schedule and the fail-high root reduction.
              (f) **S083's 50 M floor is retired**: the corpus size is a
              held-out-error decision under a stated datagen budget. The
              published HCE sweet spot is 4.5 to 10 M resolved positions (Stash
              retuned on 4.5 M and reports no overfit past 500 k), and 50 M at
              S082's two-to-four rows a game is 12 to 25 M games of datagen.
              (g) **S100 is diagnosis first**; its per-term fits and SPRTs move
              into the evaluation block, except an extraction bug, which is
              fixed on discovery under the house rule.
              (h) **S118 moves from the speed block into the evaluation
              block**, behind the expensive pawn terms -- caching a cheap pawn
              evaluation measured a 10 % slowdown in the published record, and
              S122 needs the shelter and storm slots it adds.
              (i) **The evaluation block is ordered by the Stash ledger**:
              mobility area (+20 class), passers with king distance (+22.3, the
              largest single evaluation gain in that ledger),
              connected/phalanx (+25.4), pawn hash, threats (+10), king safety
              (+25 cumulative and the documented failure magnet, so it follows
              the corpus work), endgame scaling (+4 to +8), outposts and space.
              (j) **S093 lands malus and gravity as one verdict** -- they are
              one published mechanism, `entry += bonus - entry*|bonus|/MAX` --
              and persistence as a second.
Rejected:     Keeping S096 and measuring it anyway -- the removal evidence is
              two engines strong and the verdict hour buys more elsewhere.
              Dropping S023/S025/S110/S111 outright -- the reserve keeps the
              files and the evidence for the 3000-plus phase they belong to.
              Splitting S109 into four steps on the Lynx small-positive
              numbers -- four verdicts near the bounds cost more nights than the
              attribution is worth (DEC-063), and a failing block is still
              bisected.
Consequences: plan.md is reordered and its cost line restated at the honest
              verdict count (~42-47 plus two SPSA nights and the datagen).
              DEC-082's "inert apart" wording is narrowed by this entry: Lynx
              measured LMP and futility at +4.7 each alone, so the parts are
              small-positive rather than inert, and S109's single verdict is
              kept on measurement-budget grounds. Mate distance pruning and
              quiet checks in quiescence were considered and left out -- ~0 and
              no band evidence respectively. The Fathom option for S129 is on
              the record: MIT-licensed, de Man's code granted unrestricted use,
              shipped by MIT-licensed Arasan -- a licence-clean library route
              the owner may take later instead of the from-scratch prober;
              S129 is unchanged until then.

## DEC-088  2026-08-19  The SPRT harness hash is 16 MB, matching table pressure rather than table size
Tags:         measurement, tooling
Context:      DEC-083 set `Hash=128` for the SPRT harness to match the rating
              list's 128 to 256. Checked against practice and against
              arithmetic, that matches the wrong invariant. Every OpenBench
              engine preset tests STC at 8 to 32 MB -- Stash's preset is
              exactly 8+0.08 with Hash=16 -- because what transfers across time
              controls is table *pressure*, not table size. At the list's
              2'+1" a game writes on the order of 660 M nodes against 5.6 to
              11 M entries, roughly 60 to 120 overwrites per entry; 16 MB at
              8+0.08 reproduces that ratio, 128 MB undershoots it about
              eightfold and would flatter every table-hungry change S119 is
              about to make.
Decision:     By the owner, 2026-08-19. `fastchess.sh` runs **Hash=16** at
              8+0.08. `rating.sh` keeps 128 or above, because the gauntlet's
              job is to reproduce the list's absolute regime. This amends the
              hash number in DEC-083; everything else in that entry stands,
              including the time control, the unbalanced book and the
              timing-not-SPRT rule for behaviour-neutral changes. S105 is
              edited before it runs.
Rejected:     Hash=128 as decided this morning -- it optimised absolute size
              where the transferable quantity is overwrites per entry.
              Hash=8 -- inside practice (Ethereal, Berserk) but further from
              the list ratio than 16 on this engine's nps.
Consequences: S119's SPRT clause changes from "at Hash 128" to "at the S105
              harness setting, with the pressure ratio stated"; verdicts stay
              comparable within the S105 regime as before.

## DEC-089  2026-08-19  The CCRL Blitz list rates configurations, not engines; the target stays the 1CPU entry
Tags:         planning, measurement, scope
Context:      DEC-085 said "the list runs single CPU on its main table". Read
              from the live conditions page instead: CCRL Blitz rates 1CPU,
              4CPU and 8CPU builds of the same engine as **separate entries**
              (Stockfish 17.1 8CPU sits at 3789 beside its 1CPU entry), ponder
              off, own books disabled, 4-to-6-man tablebases allowed, hash 128
              to 256. The premise was wrong; the conclusion survives for a
              different reason: the 2559 anchor (S088) and DEC-071's
              reachability table were both taken against **1CPU entries**, so
              the 3000 target is defined on the 1CPU scale and a parallel
              search buys zero toward it.
Decision:     By the owner, 2026-08-19. The target remains **3000 on the CCRL
              Blitz 1CPU scale**. Threading stays off phase one exactly as
              DEC-085 concluded, with this entry as the corrected premise. For
              the record: an 8-thread search is worth about +180 at LTC on the
              Stockfish measurement (~60 per doubling), so a 4CPU or 8CPU
              listing is a cheap later rating if the owner ever wants one --
              that is a phase-two decision.
Rejected:     Reopening threading now -- it moves a number the target is not
              stated in, and the machine's 12 threads are already spent running
              matches.
Consequences: DEC-085's operative outcomes (S086 retired, threading off the
              plan, concurrency decisions untouched) all stand. Any future
              citation of the list's conditions cites this entry, not DEC-085's
              context paragraph.

## DEC-090  2026-08-20  the two exactly-degenerate evaluation columns are deleted, their weights folded into the tables
Tags:         evaluation, tuning, tuner, s100, s134, s123, s133, dec-057,
              dec-059, inv-6
Context:      S100 proved two exact linear dependencies among the fitted
              parameters, from the indexing rather than from a correlation.
              Index 0 is a8 and a black piece mirrors by `^56`, so a rook or a
              pawn on its own seventh rank always occupies one of squares
              8..15. Rook-on-the-seventh's differential is therefore
              *identically* the signed sum of eight `psqt[ROOK][8..15]`
              occupancy columns; and a pawn on its own seventh is a passer by
              definition -- "ahead" is the enemy back rank and the extractor
              refuses a pawn standing there -- so passer bucket 5 is identically
              the signed sum of eight `psqt[PAWN][8..15]` columns. Measured:
              R^2 exactly 1.000000 for both, 0 violations over 1264773 and
              550880 non-zero rows of `.tuning/selfplay_v2_dedup.tsv`.

              The consequence is that neither weight is identified. Across three
              real fits on two corpora, passer buckets 0 to 4 move by at most 4
              while bucket 5 reads +22, -1 and -17 -- the ridge, not the pawns.
              A joint fit's value for either split is wherever the optimiser
              stopped, and `passed_pawn_mg[5] = -17` is not a statement about
              chess (DEC-023).
Decision:     **By the owner**, from three options the agent supplied: **delete
              both features and fold their weights into the eight piece-square
              entries each is equivalent to.** S134 is the step; it blocks S135.

              The engine-side identity is measured first. S100 proved it against
              `tools/eval_model.hpp` and the stored columns, and the engine's own
              `evaluate_pawns()` computes passers from bitboard fills whose
              agreement is only tested over the curated positions. The argument
              carries over and is not accepted in place of the number.

              **The fold is bit-exact, and that is why the step ships on node
              counts.** `evaluate_pawns()` sums all three pawn terms into
              `pawn_mg` / `pawn_eg` and `src/evaluation.cpp:696-700` adds those
              to the piece-square accumulator *before a single tapered
              division*, so moving a weight from one summand to the other
              changes no truncation. This is stronger than DEC-059's
              re-anchoring, which moved the evaluation by one centipawn on one
              of seven anchors; here the score is identical and INV-6 is
              discharged on `search_bench` node counts with no SPRT owed.

              **The passer half is not a no-op and the step says so.** S100's
              first framing claimed both terms were already deleted by the
              compiler at zero weights. True of `piece_placement`, whose four
              weights are zero; **false of passer bucket 5, which ships mg -17
              and eg +42**. Deleting it without folding would be a
              play-altering change wearing a behaviour-neutral label.
Rejected:     **Keep both and adopt a rule instead** -- refit only frozen-base,
              and phrase every ledger row on the sum. Zero code change and it
              preserves the small residual a frozen-base fit could still find on
              top of the tables. Refused because the trap stays in the tree and
              survives only as long as everyone remembers the rule, and because
              S133 re-shapes the tables without removing the degeneracy: any
              per-square table keeps it for any feature defined on one rank.

              **Keep both, pinned at zero permanently.** Simplest and costs
              nothing. Refused for the same reason plus one: it leaves two
              columns in the parameter vector that every future fit spends a
              gradient on and no future SPRT can attribute.
Consequences: `PIECE_PLACEMENT_COUNT` 4 to 3, `PASSED_PAWN_COUNT` 6 to 5,
              `PARAM_COUNT` 827 to 823, and every `eval_model.hpp` base after
              them shifts -- which is the group-boundary hazard DEV_MANUAL
              records four times, run backwards. `test_tuner_groups`'
              disjointness and union properties are the guard and the red is
              required first. S123 rebuilds the passer suite and inherits the
              obligation: a new term defined on a single rank is degenerate with
              the tables the same way, and `tools/feature_audit`'s identity
              report is where that gets checked.

## DEC-091  2026-08-20  piece placement is unfrozen and refitted as one bundle, with the bisect rule
Tags:         evaluation, tuning, sprt, s100, s135, s027, dec-057, dec-063,
              dec-082, dec-084
Context:      DEC-057 froze `piece_placement` at zero on the strength of S027's
              verdict: -5.48 +/- 11.46 Elo over 2284 games, H0 accepted. S100
              found that verdict procedural. The four features shared **one**
              SPRT at `--fast` bounds `elo0=0 elo1=10`, were then zeroed **by
              hand**, and every fit since S065 has held them there rather than
              fitted them to zero. Published figures for the parts are +8.2 for
              the bishop pair and +9.86 for a rook file retune, which those
              bounds cannot resolve (DEC-063, DEC-084).

              S100 excluded every other cause for these features: the counts are
              right over all 10795695 corpus rows, the gradient is the
              derivative of the error to 4.9e-8 over all 827 parameters, the
              pipeline recovers a planted vector end to end, coverage is 15.81 to
              31.38 % of rows, and R^2 against the tables is 0.487, 0.265 and
              0.168 -- far under redundancy. What remains open is the correlation
              form of the corpus hypothesis, which is why the refit waits for
              S082's corpus.
Decision:     **By the owner**: unfreeze all of `piece_placement` and refit it,
              **one bundled SPRT** over the group, on the corpus S082 and S083
              produce. S135 is the step, and S134 lands first.

              Two things make this bundle unlike S027's, and they are the
              conditions the decision carries. S134 removes the one member that
              was never an identified quantity, so the group is three
              independent features and a bundled verdict is attributable. And
              **a failing bundle is bisected, never zeroed by hand** -- DEC-082's
              rule, and the hand revert is precisely what turned S027's result
              into four unmeasured features.
Rejected:     **Split the group: bishop pair alone first.** The agent's
              recommendation and S027's own untested candidate. The pair is the
              free feature -- the compiler rewrites `count_bits(x) >= 2` into
              `x & (x - 1)` -- it carried the largest weight of the four, and the
              three rook features cost 3.1 to 4.0 % of a search between them, so
              a bundle prices a cheap term that may work together with expensive
              ones that may not. **The owner chose the bundle and the concern is
              recorded rather than re-argued**; the cost of the split was two
              verdicts and a new `--only` group against one verdict.

              **Leave it frozen until decided per term.** Refused: it leaves the
              diagnosis unactioned and the freeze resting on a reason S100
              voided.
Consequences: The bundle's own speed cost is inside its verdict, measured with
              the weights forced non-zero because at zero the compiler deletes
              the term (DEC-047). Bounds are stated before the run and are not
              `--fast`. If the verdict is zero it is recorded as zero and the
              term may still be kept with the reason stated.

## DEC-092  2026-08-20  tempo is unfrozen and the truncation guard is re-derived, not relaxed
Tags:         evaluation, tuning, testing, sprt, s100, s136, s027, dec-053,
              dec-057, dec-063
Context:      This is the half of `--freeze tempo,piece_placement` whose reason
              S100 did **not** void. DEC-057 froze tempo on a mechanical
              consequence DEC-053 had stated in advance rather than on a verdict:
              `evaluate()` taper-divides in integers and truncates toward zero,
              and while `tempo_mg == tempo_eg == 0` the tempo division truncates
              `0 / 24` exactly and contributes nothing. Three divisions can round,
              the model-versus-engine bound is 3 x 23/24 = 2.875, and
              `test_eval_model`'s tolerance is 3. Fit tempo and the fourth
              division rounds too: bound 3.833, tolerance 4. It has already
              happened once -- S065's first fit put tempo at 39 / 21 and
              `ctest -L fast` came back 9 of 12 on that guard and its four
              pinned FENs.

              Tempo's own verdict is **unresolved and was recorded as
              unresolved**: S027's SPRT ran the full 3000 games and reached
              neither bound, LLR -1.46 against -2.20, for -0.69 +/- 9.64. S100
              added the size of the signal: the term's whole contribution to a
              WDL label is the mean-outcome gap between White-to-move and
              Black-to-move rows, measured at 0.007878 over 10795695 rows, which
              at the corpus mean and K = 0.7624 is a 7.26 cp score difference
              between the groups and a tempo weight near 3.6 cp. Confounded, and
              not an Elo figure. Against it, S027's fit said mg 10 and S065's
              unfrozen fit said mg 39.
Decision:     **By the owner**: unfreeze tempo and pay the guard cost. S136 is
              the step.

              The tolerance moves 3 to 4 **from the arithmetic** and the four
              pinned FENs are re-measured under the new weights with
              `build/tools/truncation_scan`, DEC-057 having pre-authorised
              exactly this re-targeting. **This is a re-derivation and not a
              relaxation**, and the clause that has to survive the edit is the
              non-vacuity one: the per-position threshold exists so a pinned
              position genuinely exercises every division that can truncate, so
              it moves from `> 2.0 = 48/24` to `> 2.875 = 69/24` for the same
              reason the bound moves.

              The fit is `--only tempo` and the label is **WDL-heavy,
              `--lambda 0` stated rather than defaulted**, because a score-blend
              label cannot teach a term the current evaluator scores at zero.
              Bounds resolve single digits and are stated in advance.
Rejected:     **Keep frozen and accept unresolved permanently.** Defensible on
              the numbers -- a few centipawns of label-side signal, a run that
              reached neither bound over 3000 games, and DEC-081 saying the
              search block leads. Refused: the question has been open since S027
              and the instrument that failed to resolve it has since got 1.67x
              cheaper (S105).

              **Defer to when S082's corpus lands.** Refused as a decision
              though not as scheduling: the step sits in the evaluation block
              regardless, so deferring the decision bought nothing.
Consequences: `test_eval_model`'s tolerance, its four pinned FENs and its
              non-vacuity threshold all move in the same commit as the weights,
              and `truncation_scan` is what re-derives them. A second run that
              reaches neither bound is recorded as unresolved **again** and the
              term stays at zero; that is a legitimate outcome and S027's row is
              its template.

Proposed:     **Agent proposal, S139, 2026-08-21 -- NOT owner-approved, nothing
              above is amended by it.** The arithmetic in `Context:` and
              `Decision:` is the count as it stands today, and the plan runs
              **S055 before S136** (`adocs/plan.md:352` entry 39 against `:363`
              entry 50). S055 tapers mobility and king safety through one
              division instead of two and re-pins the guard in its own commit
              (`adocs/plan_todo/S055_taper_stage_two_once.md:3`: "the post-merge
              bound of 2 x 23/24 = 1.917 ... the tempo precondition message's
              arithmetic becomes 3 x 23/24 = 2.875 with its tolerance of 4
              becoming 3"). So at the point S136 runs, two divisions can round
              and not three, and the numbers this entry states are each one
              division high. Proposed replacements: bound **2 x 23/24 = 1.917 to
              3 x 23/24 = 2.875**, tolerance **2 to 3**, non-vacuity threshold
              **`> 1.0` to `> 2.0 = 48/24`** (two divisions cannot reach past
              46/24 = 1.9167). Taking the literals as written would raise the
              tolerance a unit above what the arithmetic supports -- the
              relaxation this entry forbids -- and set a threshold at 2.875 that
              no position can reach when the bound is 2.875.

              **The operative decision is unaffected**: unfreeze tempo, pay the
              guard cost, re-derive and never relax, `--only tempo`, `--lambda
              0`, bounds stated in advance. Only the literals move, and only if
              S055 has landed. S136's `accepts:` was rewritten by S139 to derive
              the count at its own HEAD with these numbers as the S055-landed
              case, so the step is satisfiable either way; this block exists so
              the entry and the step do not disagree in the meantime. Closes
              half of `2026-08-20_plan_review-F03`.

## DEC-093  2026-08-20  the tuner's option check is a bounds diff and a node probe, because the engine cannot report a refusal
Tags:         tuning, spsa, uci, tooling, s084, s085, s137, dec-061, dec-084
Context:      S084's `accepts` and its research section both planned the same
              guard for the SPSA driver's dry run: send the perturbed options,
              then grep the engine log for `Rejected` to prove none was refused.
              `src/chesso.cpp:1058-1062` does write that line, so the plan was
              written against real code. It is unreachable anyway. `LOG_W` is
              `if (false) std::clog` under `NDEBUG` (`src/log.hpp:35`) and
              `build-tune` is a Release build, so nothing is printed on any
              stream or into any file. `uci` is not a fallback: it re-prints each
              parameter's compiled default, not its live value, measured
              2026-08-20.

              So both ways a tuner can be wrong -- a value outside the range,
              which is refused rather than clamped, and a misspelled name, which
              is ignored like any unknown option -- are indistinguishable from
              success, and a run can spend a night against a default parameter.
Decision:     The driver's `check` mode replaces the grep with two things that
              are observable: every configured name and both its bounds against
              the binary's own `uci` listing, and a node-count probe proving
              `setoption` reaches the search at all (`RfpMargin` 75 -> 164123
              nodes, 2000 -> 743308, depth 9). The driver clamps theta and both
              perturbed vectors itself, and the clamp is in the synthetic gate.

              The engine side is **S137**, placed immediately before S085 rather
              than left to the end of the plan, so the first real run does not
              rest on the driver's clamp alone. It prints one `info string` per
              refusal; it does not add a readback command, which is a larger
              surface and a separate question.

              `MANUAL.md` and `DEV_MANUAL.md` both claimed the refusal was
              logged. Both corrected in S084's commit.
Rejected:     **Make the tune build log to a file regardless of `NDEBUG`.**
              Refused: `LOG_W`'s compiled-out form is deliberate and shared with
              the release binary, and a tuner should not need a log file to read
              a protocol answer. `info string` is legal UCI in every build.

              **Leave it to the driver's clamp alone and skip S137.** Refused:
              the defence is correct and it is not transferable -- the next tool
              to drive this binary starts from the same silence, and the failure
              is silent by construction.

              **Treat it as a bug and fix it inside S084.** Refused on S084's own
              `excludes:` -- no change under `src/` -- which is also what keeps
              INV-6 trivially discharged for that step. A step, not a drive-by.
Consequences: No tool may take an accepted `setoption` on trust until S137
              lands. S085 runs `check` before its run and records that it did.
              DEV_MANUAL's tune build section is now the place where the two
              observability gaps are stated, and its `RfpMargin` node figures are
              load-bearing rather than illustrative: the probe compares against
              them.

## DEC-094  2026-08-20  S085 tunes 12 of the 22 live search parameters, not the goal line's twenty
Tags:         tuning, spsa, s085, s089, s127, s073, dec-019, dec-084, plan
Context:      S085's goal line was written when the exposed set was ten and says
              "the twenty search parameters that exist today". The live surface
              at freeze time is **22** (`src/search_params.hpp`): 13
              search-shape parameters and the 9 `Tm*` entries S089 added after
              the goal was written. So the goal's letter named a number that was
              never the surface, and the 2026-08-20 plan_review filed it as
              F09.

              The step's own research section left the choice open for freeze
              time and set out three options: all 22, the 13 search-shape, or
              the 13 less `OrderHistoryMax`.
Decision:     **12.** The nine `Tm*` parameters are excluded: they are the
              published time-control-overfit family -- one record measures an
              SPSA-tuned time manager at +23.8 Elo at 20+0.2 and **-22.9 at
              10+0.1** -- and this run deliberately tunes at a control its
              verification does not share, which is the worst regime for them.
              They also carry the only fresh SPRT in the set behind their
              current values (S089, H1 accepted 2026-08-18), and S127 retunes
              everything at the S105 control after the search block.
              `OrderHistoryMax` is excluded because it binds only when history
              saturates (`src/search.cpp:752`), so it random-walks, and a
              meaningless endpoint would land in the shipping vector the SPRT
              judges. SPSA costs two objective evaluations per iteration
              whatever the width, so dropping it saves no games -- it removes a
              noise axis from the answer.

              The goal line is amended to say what is frozen rather than left
              to be contradicted by the stamp.
Why:          A tuned time-management value is worth Elo at the control it was
              tuned at and can be worth negative Elo elsewhere, and this run's
              control is chosen for throughput rather than for shipping.
Rejected:     All 22 -- the goal line's letter, and Kiiski co-tuned up to 35
              variables -- because it buys TM values tuned at 2+0.02 for an
              engine that plays every control, and it muddies the rejection
              analysis if the SPRT says no. The 13 including
              `OrderHistoryMax` -- rejected for the noise-axis reason above.

## DEC-095  2026-08-21  RfpMinPly's floor is the tested 2, but the tests are re-derived before it is trusted
Tags:         search, pruning, testing, rfp, s085, s142, s145, dec-016, dec-019
Context:      S085's SPSA run walked `RFP_MIN_PLY` to 0 and sat there for 72.5 %
              of its iterations. 0 cannot ship: three of the eighteen mate cases
              in `tests/test_search.cpp` go red, and 0 is byte-identically the
              same engine as 1 because `!is_pv` exempts the root and not this
              parameter. Measured floor: **2** -- all eighteen pass there, RFP
              really does fire at ply 2 (the tree changes, TRICKY 329598 against
              375687 at 3), and the ply-2 exemption the comment argues for is an
              argument no test exercises, which `src/search.cpp:517` already
              concedes.
Decision:     The floor is **2**, by the owner. But the owner's second point is
              the operative one: **those three positions were hand-picked by the
              owner for a different engine** -- the mailbox and bitboard branches
              -- and they are now the gate standing between the tuner and a value
              it pushed hard toward. A test set chosen for another engine is not
              evidence about this one, and three positions is not a sample. So
              **S145 re-derives the mate-safety test set before S142 sets the
              bound**, and S142 is paused behind it.

              `RFP_MAX_DEPTH`'s bound stays at 63. 15 passed the full mate suite,
              so there is no demonstrated defect -- only a comment that stopped
              describing its own value. The comment is corrected; the number is
              not.
Why:          A floor derived from a test set written for a different engine
              could be blocking Elo the tuner correctly found, or permitting a
              value that loses mates this engine actually reaches. Neither is
              knowable from three hand-picked positions.
Rejected:     3, the argued floor -- keeps the stated purpose but rests on an
              untested claim and forecloses a notch the tuner wanted. Setting 2
              immediately without re-deriving the tests -- it is the right number
              from the wrong evidence, and the evidence is the part in question.
              Re-running the SPSA on the narrowed bound now -- premature until
              the bound is trustworthy; it stays available afterwards.
Amended:      2026-08-21, after S145's research. Two corrections, neither
              reversing the decision.

              **The provenance premise was wrong about one of the three
              positions.** Only two are inherited -- `MATE_IN_2_W_POS` and
              `MATE_IN_2_B_POS` enter at `3ed3b11`, 2025-04-20, and that commit
              is on `master` and `bitboard` too. The material-leader position and
              its test enter at `6bd650e`, 2026-08-16, **on `achesso` only**:
              built during S033 for this hazard, by adding White material to
              `MATE_IN_2_B_POS` until White led by 500, verified by exhaustive
              enumeration and Stockfish. The objection holds for two positions,
              not three -- and the one it does not hold for is the only one of
              the three that carries the gate.

              **The sentence given for leaving `RFP_MAX_DEPTH` alone was
              incomplete.** "15 passed the full mate suite, so there is no
              demonstrated defect" is true at every admissible floor and remains
              true -- measured 2026-08-21 under iterative deepening on the
              material-leader position, the mate is found at iteration 3 at both
              `RfpMinPly` 2 and 3 with `RfpMaxDepth` at 6 or 15. What the
              sentence should not be read as is evidence that 15 is safe *on its
              own*. At `RfpMinPly` 1 the same position finds the mate at
              iteration 8 with `RfpMaxDepth` 6 and **never, to depth 14, with
              15**, playing `e5f6` instead of `e5e6`. So the two parameters are
              substitutes for one guard, the suite cannot separate them, and 15
              is untested in isolation. Published practice puts the load-bearing
              guard on depth and not on ply -- zero of fourteen surveyed engines
              has a ply floor on reverse futility -- and Stockfish measured
              removing its depth limit as passing SPRT at both STC and LTC while
              halving mate finding, 2427 to 1246 on ChestUCI at 1M nodes, which
              is why its source has carried "The depth condition is important for
              mate finding" ever since. The decision stands: the number is not
              touched. S145 measures both parameters against its own test set and
              S142 records the result.

## DEC-096  2026-08-21  the plan_review's deferred numbers close inside the steps that own them
Tags:         audit, plan, measurement, dec-083, s085
Context:      The 2026-08-20 plan_review ran documents-and-citations only,
              because S085's SPSA run held all twelve threads for the night. Its
              method re-measures every numeric claim from the tool the step
              names, so each unverifiable figure was recorded as deferred with
              the exact command that would settle it.
Decision:     Each deferred figure is re-measured by the step whose `accepts`
              depends on it, when that step runs -- not as a batch. No machine
              time is booked for the audit's numeric half.
Why:          Measurement capacity is the binding constraint on the plan, and a
              batch run would spend it on figures belonging to steps far down the
              order, some of which will be rewritten before they are reached.
Rejected:     A dedicated re-measurement run -- buys a clean close on the audit
              and costs machine time on numbers not yet needed. Accepting them
              as permanently unverified -- cheapest, but leaves figures in the
              plan that decide work and that nobody has confirmed.

## DEC-097  2026-08-21  the SOTA enrichment pass resumes, in parallel, on nights
Tags:         plan, research, process, dec-084, dec-041
Context:      The enrichment pass appends a technical-details section per pending
              step -- published form, traced records, `file:line` grounding, seeds
              per DEC-084, measurement plan per DEC-083. It was stopped at S120
              by the owner on 2026-08-20 with 28 of 48 done.
Decision:     It resumes, one agent per step, in parallel. It needs no machine, so
              it is the work that fits a night when a match or a fit holds the
              hardware. It is not a plan step and does not enter the sequential
              order -- plan steps still run one at a time.
Why:          The research is free in the constraint that actually binds, and
              S138 showed the cost of documents drifting behind the code: eight
              pending steps were sending implementers to the wrong mate test.
Rejected:     Leaving it stopped -- the enriched steps are the near ones and the
              far ones would arrive unresearched. Enriching only the next few --
              bounded, but it wastes the parallelism that makes this cheap.

## DEC-098  2026-08-21  the two killer slots stay duplicated; CPW's replacement rule measured -11 Elo here
Tags:         search, move-ordering, measurement, sprt, dec-019, s149, audit

Context:      2026-08-21_adversarial-F01 found the killer store at
              `src/search.cpp:761-762` shifting slot 0 into slot 1 with no
              distinctness guard. A quiet that fails high twice at one ply
              copies slot 0 onto itself, and `search_state_t state = {}` is
              built once per `go` (`src/chesso.cpp:647`), so killers persist
              across every iteration of iterative deepening and the repeat is
              the common case: **351422 of 532133 stores, 66.0 %**, leaving both
              slots equal on **5115505 of 11531069 negamax nodes, 44.4 %**, over
              11 positions at depths 12 to 22. `score_move` tests slot 0 first,
              so on those nodes no distinct move can reach `ORDER_KILLER_1` at
              all. The Chess Programming Wiki's *Killer Heuristic* page states
              the rule the code was missing: the replacement scheme ought to
              ensure the available slots contain different moves.

Decision:     Proposed by the agent from the measurement, taken under DEC-041's
              standing delegation of measurement; **agent-proposed and open to
              the owner's review or reversal.** The duplication is kept and the
              guard is not. S149 implemented the published rule -- two lines,
              guarding the shift -- and measured it: **H0 accepted** against
              `elo0=-5 elo1=5` in **2522 games and 1 h 05 m** against `ac4c588`,
              `LLR -2.97`, `Elo -11.02 +/- 10.53`, `nElo -14.21 +/- 13.56`,
              `LOS 2.00 %`, 48.41 %, **0 time forfeits in 2524**. The guard did
              what it claimed -- the same instrumentation reads 0 duplicated
              nodes of 11146351 after it, negamax nodes -3.34 % and total nodes
              -2.87 % -- and still lost. The two lines were reverted.

              **This is DEC-019's fourth entry**, and the strongest of the four,
              because the other three measured zero and this one measured
              negative. Staged move generation, quoted 30-50 Elo, measured 0
              (S006). SEE pruning in quiescence measured 0 (S015). Capture
              ordering, reported around 150 Elo, measured slower (S025). CPW's
              killer replacement rule measures **-11 Elo** here.

Why:          The only number that counts is the one this engine measures on
              this hardware against its own previous commit, and it said no.

Rejected:     Keeping the guard because it is the published rule and the
              mechanism is sound. That is precisely the failure mode DEC-019
              exists to stop, and S149's own pre-registered H0 clause -- written
              before a game was played -- forbids it by name.

              Re-running at other bounds hoping for a different answer. `LLR
              -2.97` against a `-2.94` boundary is a completed test, not a near
              miss, and DEC-063 makes a bounds change a recorded decision rather
              than a retry.

              Reverting the test with the code. The defect F01 named in the test
              is real and independent of the verdict: it counted slots that were
              **non-zero**, which a slot 0 copied onto itself satisfies, so it
              passed while describing a property that was false. It now counts
              **duplicated** slots, asserts the behaviour that shipped, and
              carries this number in its comment -- an agent who re-guards the
              store goes red and finds the measurement.

Consequences: `adocs/specs.md`'s ordering clause states the duplication and its
              rate as engine behaviour rather than as a defect.
              2026-08-21_adversarial-F01 closes as **accepted**, not fixed: the
              defect is real and stays in the tree by measurement.

              **What the number does not establish** is why. The unguarded shift
              discards whatever slot 1 held on every repeat, so it is also an
              ageing mechanism for the second slot, and the guard preserves a
              stale killer for the whole of one `go`. That is a hypothesis, it
              has not been measured, and it is S159 -- which, like S149, lands
              before S093 rewrites this same block or is folded into it
              deliberately.

## DEC-099  2026-08-21  S149 ran at elo0=-5 elo1=5, and the two-sided pair paid for itself again
Tags:         sprt, bounds, measurement, dec-063, dec-019, s149

Context:      `fastchess.sh`'s default is `elo0=0 elo1=5`, the gainer form for a
              change expected to gain. S149's change was not that. The mechanism
              argued positive -- a band that could never fire would start firing
              -- but DEC-019's ledger is three published figures that measured
              0, 0 and *slower* on this code, so the effect was genuinely
              two-sided and the default pair leaves the truth outside the
              interval it defends.

Decision:     Taken by the orchestrating agent on 2026-08-21, under DEC-063's
              rule that bounds straddle the expected effect and DEC-041's
              standing delegation of measurement; **agent-taken and open to the
              owner's review or reversal.** S149 ran at `elo0=-5 elo1=5
              alpha=0.05 beta=0.05`, recorded in `adocs/data/S149_sprt.sh` with
              all three outcomes pre-registered in the header before launch.

Why:          A bound pair that cannot contain the truth random-walks to the
              round limit and spends the night to say nothing.

Rejected:     The `elo0=0 elo1=5` default. DEC-063 measured it running 6 h 36 m
              over 9036 games and returning nothing where `elo0=-5 elo1=5`
              returned a verdict in 1 h 41 m over 2312, same binaries.
              `--nonreg` at `elo0=-5 elo1=0`, which brackets from one side and
              could not have separated a positive effect from zero.

Consequences: **DEC-063 now has a second confirming case, and it is the
              cheaper one.** S149 reached H0 in **1 h 05 m over 2522 games** --
              a completed verdict, in the direction nobody predicted, for about
              a sixth of what the default pair spent on nothing. The pattern is
              no longer one observation: when the expected effect is two-sided,
              the two-sided pair is both the correct test and the cheap one, and
              measurement capacity is the binding constraint on the plan.
