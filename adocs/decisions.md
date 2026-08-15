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
