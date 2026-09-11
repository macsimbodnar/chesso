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
              `adocs/specs.md` for what proves it. Nothing that matters may
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
Amended:      2026-08-22 by DEC-105 -- the seed rule is provenance-based: a
              number that originates as another engine's tuned output is not
              "open literature", wherever it is republished.
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
Amended:      2026-09-04 by DEC-133 -- three of its rulings rested on figures
              the 2026-09-04 plan review's source pass narrowed: (a) Ethereal
              removed only its pre-move-loop check extension, so the in-loop
              form is reopened as S188; (b) Lynx's +11.4 was measured at CCRL
              3226-3293, not ~2850, so S099 joins the reserve at its head;
              (d) S133's +65 and +88 are release-bundle deltas. S181 and S185
              record the corrected bands and sources.
Amended:      2026-09-11 by S185 -- the two ledgers this decision rests on are
              cited where they are used, both fetched by the 2026-09-04
              literature check (`adocs/data/2026-09-04_plan_review_literature_check.md`,
              rows A4, A5 and A27). **Ethereal**: commit `e755a814`, "Add elo
              estimates to search steps", 2020-01-22, Ethereal 11.82,
              https://github.com/AndyGrant/Ethereal/commit/e755a8140fba -- all
              rows at 12.0+0.12, one thread, 8 MB; history **-759.05 +/- 57.40**,
              late move reduction **-248.59 +/- 10.48**, quiet move pruning
              **-175.08 +/- 7.24**, beta pruning **-31.95 +/- 3.21**. The
              Context line below reads "-759, -249, -175, -60, -9" from that
              same table (extensions -59.87, ProbCut -9.08) and **the ledger
              prices search steps only** -- no evaluation term appears in it,
              which is why `plan.md`'s "single digits for most evaluation
              terms" was deleted rather than sourced. **Stash**:
              `mhouppin/stash-bot`'s `CHANGELOG.md`,
              https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md --
              v26 initiative from threatened pieces **+10.13 +/- 6.50**, v27
              mobility zone **+19.95 +/- 9.63**, v31 connected pawns
              **+25.38 +/- 10.40**, v32 king proximity in the passed-pawn term
              **+22.27 +/- 9.86**. Neither ledger's numbers moved; what changed
              is that the tree now says where they come from.
Amended:      2026-09-11 by S181 -- **every Lynx figure this decision and the
              steps it ruled on cite is banded**, in
              `adocs/data/S181_lynx_bands.md`: each pull request's `merged_at`
              from the GitHub API against the two releases it falls between and
              the CCRL Blitz 1CPU rating of each, on the list computed
              2026-09-05 and read 2026-09-11. Ruling (b)'s two premises do not
              survive it. **"+11.4 at ~2850" was measured at 3224-3291**
              (#1662, between v1.9.1 and v1.10.0) -- above the "~3100" that
              demoted S110 and S111, so the criterion separated nothing, which
              is `2026-09-04_plan_review-F02` and DEC-133's reason for moving
              S099 to the reserve head. **"Capture history failed four SPRTs at
              ~2600 (Lynx)" is withdrawn**: no source was ever located for the
              four, and Lynx *merged* capture history as #634 on 2024-02-02
              into v1.3.0 at a banded 2653. S023's demotion stands on the
              figure that traces, Weiss #428's -4.17 +/- 4.83 STC against
              +3.66 +/- 3.29 LTC, measured by an engine CCRL rates 3055.
              Ruling (c)'s S098 groupings move too: cutnode, !improving and
              Lynx's PV-min-moves patch are 3119-3138 and TT-capture and
              deeper/shallower 3138-3224, where the file read high-2800s and
              "~3000-3100". DEC-176 records what all of that changes, which is
              the record and not the order.
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
              **S055 before S136** (`adocs/plan.md:359` entry 39 against `:370`
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
Amended:      2026-09-04 by DEC-137 -- the resumed pass produced no commit in
              fourteen days; the enrichment becomes step S186, ordered before
              block 3, and is no longer a standing promise for nights.
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

## DEC-100  2026-08-21  an accepts clause that names another step file is written against that step still being pending
Tags:         workflow, plan, accepts, agent-proposal, s139, s141, dec-017

Context:      S141's `accepts:` named three step files -- S039's, S085's and
              S120's -- and required each to name the file its change lands in.
              S085 completed at `43bf189` before S141 started, so its file was
              in `adocs/plan_done/`, which AGENTS.md sec 10 hard-prohibits
              writing and a `PreToolUse` hook refuses. One third of the accepts
              was therefore unproducible by any means the step is allowed to
              use, and had to be discharged in the stamp instead of satisfied.
              It was also unnecessary: the edit had landed at `4fc359f` while
              S085 was still current. This is the second occurrence -- S140 hit
              the same wall and its `excludes:` now names `adocs/plan_done/`
              outright -- and it is exactly the class S139 existed to remove,
              simply not one of the five steps S139 covered.

Decision:     **Agent proposal, S141, 2026-08-21 -- NOT owner-approved. Nothing
              in the workflow changes until the owner takes it.** Proposed: when
              an `accepts:` clause requires an edit to another step's file, the
              writer checks that step is still in `plan_todo/` or
              `plan_current/`, and names `adocs/plan_done/` in `excludes:` when
              the finding it closes has a completed step among its subjects. A
              clause that survives to a step's start with its subject already
              completed is amended out and discharged in the stamp with the
              reason, which is what S141 did.

Why:          A completed step's `touches:` constrains nothing -- no further
              change lands under it -- so the clause buys nothing even where it
              could be produced.

Rejected:     **Extending `tools/plan_prose_check.py` with the check.** The
              mechanical part is trivial: an id in an `accepts:` whose file sits
              in `plan_done/`. The judgement is not, and it is the whole of it.
              An accepts may legitimately *reference* a completed step -- S120's
              names S094 for a reason it records, S039's names S073 -- and only
              a clause that demands a *change* to the completed file is a
              defect. A check that cannot tell those apart flags every third
              step, and the recorded failure mode of a noisy check in this
              repository is that it gets switched off, not obeyed.
              **Waiving the rule for `plan_done/` and editing it anyway.**
              Refused outright: `plan_done/` is the project history and the
              prohibition is a hard one, hook or no hook.

Consequences: If taken: the writer of an `accepts:` owes one `ls adocs/plan_*`
              per step id it names, and a finding split across steps at
              different lifecycle stages -- `2026-08-20_plan_review-F06` was
              split across two -- is written as separate clauses per step rather
              than one clause naming all of them. If not taken: the discharge
              route stays what S141 used, and the cost is one amended accepts
              per occurrence.


---

## DEC-101  2026-08-22  quiet history is not carried across `go`; the published +12.5 measured -1.65 here
Tags:         search, move-ordering, history, measurement, sprt, dec-019,
              agent-proposal, s093

Context:      S093 verdict 1 shipped butterfly-indexed quiet history with a
              malus and gravity ageing (`40f5b56`, H1, `Elo 10.73 +/- 6.70` over
              6412 games). Verdict 2 was the second half of the same published
              mechanism: keep the table from one `go` to the next inside a game
              rather than rebuilding it, which is what every engine surveyed
              does. Lynx PR #637 measured **+12.5 +/- 6.6 over 6419 games, LOS
              100 %** for exactly that, and the same project's PR #457 measured
              every alternative worse than keeping -- decay to 50 % between
              searches -16.6, decay to 90 % -6.3, always clear -12.1.

              Built and measured here as `adocs/data/S093_sprt_v2.sh` against
              `40f5b56`, bounds `elo0=0 elo1=5`, all three readings
              pre-registered before the first game. **H0 accepted in 6 h 35 m
              over 15398 games: `Elo -1.65 +/- 4.22`, `nElo -2.14 +/- 5.49`,
              `LOS 22.22 %`, `LLR -2.96`, 49.76 %, 0 forfeits in 15399.** That
              is a well-measured null and not an ambiguous one: the interval
              excludes the +5 the bounds were set to find and also excludes a
              loss of 6 or more.

Decision:     **Agent proposal, S093, 2026-08-22, under DEC-041 -- NOT
              owner-approved. Nothing is settled until the owner takes it.**
              Proposed: quiet history stays per-`go`, and verdict 2 is reverted
              in full to `40f5b56`'s behaviour -- the hoist into
              `quiet_history_t`, the pointer in `search_state_t`, the
              descendant rule, the `ucinewgame` clear, the persistence tests and
              the `MANUAL.md` paragraph on carried-history lifetime.

              Three reasons. It does not gain, and the pre-registered H0 clause
              says so. It is **two transfer failures in one step** -- verdict 1
              measured a third of its own prior and verdict 2's prior came back
              negative -- so keeping this on Lynx's number after measuring it
              here is the precise move DEC-019 forbids. And the complexity has a
              demonstrated cost: the hoist produced a `nullptr` dereference in
              `tools/datagen.cpp` that compiled clean and that no fast-label
              test caught, which is one silent defect bought for zero Elo.

Rejected:     **Keeping the hoist alone and dropping only the persistence.** It
              is the tempting middle: it turns an accidental lifetime into an
              explicit one, and it is INV-6-provable against `40f5b56`. It loses
              because with the table cleared every `go` again the descendancy
              code is dead, the `ucinewgame` case is vacuous, and a refactor
              with no behaviour change and no live tests is not worth the
              surface it adds. Revisit it when S023's capture history or S099's
              correction history needs the same lifetime for a reason that is
              being measured.

              **Keeping persistence anyway on the published figure.** AGENTS.md
              does permit keeping a measured zero with the reason stated, and
              S005, S006 and S015 all were. Those were kept because the feature
              cost nothing and removed a state the design did not intend. This
              one costs a table lifetime, a descendancy rule and a UCI-visible
              contract, and the point estimate is negative.

              **Re-running at other bounds.** 15398 games and +/- 4.22 is not a
              resolution problem. There is nothing a wider pair would find.

              **Blaming the descendant rule and re-running without it.** It was
              named as a candidate explanation before the games were played, and
              it is the honest one to check -- it is chesso's own hardening and
              not what Lynx priced. It is not taken because the fastchess path
              replays `position startpos moves ...` in full before every `go`,
              so the previous root is always in the chain and the keep path
              always engages, which `tests/test_engine.cpp` drove directly at
              the time. Settling it properly means instrumenting how often the
              keep path engages in a real match, which is a step and not a
              re-run, and it is not worth one against a -1.65.

Consequences: Quiet history is built fresh for every `go`, like the killers, the
              countermoves and the PV. `search_state_t` owns it by value and its
              lifetime is the search's. DEC-019 gains its next entry and its
              second form: not "the figure measured zero here" but "the figure
              measured a third here, and its sibling measured negative". The
              design is not lost -- S093's step file keeps the three-stage
              red-first evidence, the descendancy rule, the derived integration
              bound and both verdicts with their figures, so reviving this means
              re-measuring a finished design rather than rebuilding one. The
              S093 accepts clause requiring a clear on `ucinewgame` and on a
              non-descendant position is discharged by this verdict: with no
              carried history there is nothing to clear.

## DEC-102  2026-08-22  a step that consumes a table bound owns what it propagates; S130's excludes made it unsatisfiable
Tags:         search, quiescence, transposition, soundness, bound-sign, scope,
              plan, agent-proposal, s130, s106, dec-019

Context:      S130 lets quiescence take a transposition entry's score as its
              stand pat where the stored bound certifies the direction: a lower
              bound may only raise it, an upper bound only lower it. The step
              was written with `excludes: any change to what is stored`, and the
              read side was implemented exactly to that contract.

              **It is not sound inside that contract, and the SPRT measuring it
              was killed at 103 games because of it.** Quiescence's final store
              marks the entry `TT_PV_NODE` -- exact -- when `best_value >
              alpha0`. Before S130 that is correct by quiescence's own
              definition: the value of a node no capture improves is what
              standing pat is worth. After S130 `best_value` can be the
              substituted bound, so the node writes `value == s` on evidence
              that says only `value >= s`, at the same key and the same depth,
              over the very entry that certified it, where every later
              quiescence probe answers an exact entry unconditionally.

              The upper-bound mirror is unconditional rather than incidental. An
              upper-bound entry only reaches the stand-pat site when its score
              is above alpha -- `tt_entry_answers()` would have answered the
              node otherwise -- and alpha at the probe is still `alpha0`. So a
              cap always leaves `best_value > alpha0` and the store always chose
              exact. Instrumented over three positions at depth 12, **386 of the
              561 exact stores at that site were laundered bounds, 69 %**.

              This is S106's bound-sign class from the propagation side. S106
              swept the eight store and probe sites and found them correct; this
              defect did not exist yet, because S130 is what creates a path from
              a stored bound into a stand pat.

Decision:     **Agent proposal, S130, 2026-08-22, under DEC-041 -- NOT
              owner-approved. Nothing is settled until the owner takes it.** The
              orchestrator amended the step in the working tree; this entry
              records the amendment and the general rule for the owner to
              accept or reverse.

              Two parts.

              **The general rule.** A step that consumes a transposition bound
              also owns what it propagates. Reading a bound and writing the
              result are one mechanism, and a step whose excludes separate them
              is unsatisfiable as written rather than merely narrow. The
              property to hold is one sentence: *a node never stores a claim
              stronger than the weakest thing that produced its value.*

              **The scope amendment.** S130's `excludes:` is amended to put the
              store's node *type* in scope. The stored *score* and the entry's
              `eval` field stay excluded and stay unchanged. A blocking child
              step was considered and rejected: the correction is a no-op
              without S130's substitution, so it cannot be measured on its own,
              and splitting them would put an unsound engine through an SPRT.

Rejected:     **Landing S130 as first written and letting the SPRT decide.**
              This is the option that was actually running. It fails AGENTS.md
              §0 -- a known defect contaminates every measurement taken after it
              -- and it would have priced a mechanism that nobody would keep.

              **A blocking child step under `--step block`.** The correct shape
              when a discovery is separable. This one is not: with the
              substitution removed the fix changes nothing, so it has no
              measurable identity of its own and no verdict to record.

              **Keeping the read side and deleting the substitution's effect on
              the final store by not letting a substituted stand pat reach
              `best_value`.** That is a different feature -- the fail-soft floor
              is most of what the published technique does -- and it would
              measure something other than what the literature priced.

              **Storing nothing at all where the stand pat was substituted.**
              Sound, and simpler than degrading the type. Rejected because it
              throws away the node's work at exactly the nodes the table already
              knows something about, and because the degraded store is
              fixpoint-stable: a raise nothing beats re-stores the same
              `TT_BETA_NODE(s)` it read, instead of upgrading its own evidence.

              **Comparing `best_value` against `static_eval` at the store to
              recover exactness where the cap did not actually hide anything.**
              Tighter and provably sound, and it keeps a few more exact entries.
              Rejected as an extra condition on a path that fires on 0.008 % of
              stand-pat sites: one more place to get a bound sign wrong, bought
              for nothing measurable.

Consequences: Quiescence carries the stand pat's bound kind beside its number,
              in `node_type_t` so it maps onto the store with no translation,
              and the final store degrades rather than applying the window test
              unconditionally. With no entry every branch reduces to the
              pre-S130 code exactly, which is what keeps the no-table case
              byte-identical.

              One asymmetry is now load-bearing and has its own test. When a
              searched move takes over the maximum, a *raised* floor leaves
              nothing behind -- it sat above the static score, so a line that
              beat it beat the static score too -- but a *lowered* floor does:
              the static score it displaced may beat the winning line as well,
              so the value is only bounded below. The obvious specification, "a
              value a real capture search beat is exact", is wrong for the cap
              direction, and the third test is the one that says so.

              S130's `touches:` grows `adocs/specs.md`; the "absent, machinery"
              row has to state the degraded store together with the
              substitution, since neither is sound without the other. The
              measured verdict now covers both, which is stated in the SPRT
              script's pre-registration.

## DEC-103  2026-08-22  S130 is kept at a measured zero, and the bound-type rule is the ground that carries it
Tags:         search, quiescence, transposition, measurement, s130, s112, s022, s116, s119, s120, dec-019, dec-041, dec-063
Context:      S130's SPRT against `293a45b` at `elo0=0 elo1=5` ran 16784 games
              in 7 h 12 m and reached no bound: Elo 1.14 +/- 4.04, nElo 1.48
              +/- 5.26, LOS 70.95 %, LLR -0.71 inside (-2.94, 2.94), 0 time
              forfeits in 16788 games written to the PGN. It oscillated rather
              than travelled -- over 839 samples the LLR stayed inside
              [-1.71, +0.69] -- and the pre-registered no-verdict clause named
              8000 games where this ran past twice that. The pre-registered
              explanation holds: the substitution fires on 0.23 % of
              quiescence stand-pat sites, too thin for +/-5 bounds to resolve.
              The nearest-shaped published record, Lynx PR #1319
              (qsearch-only, over an existing probe and eval-field read),
              merged at about zero over 60152 games -- +1.14 +/- 4.04 is
              Lynx's number -- while Weiss's +10.78/+12.09 and Ethereal's
              +11.04 did not transfer: DEC-019, third time this session. An
              earlier run was killed at 103 games because it was measuring the
              unsound exact-store; the fix and its 69 % laundered-store count
              are DEC-102's record.
Decision:     Agent-proposed under DEC-041, annotated for the owner to
              reverse. **Recorded as zero and kept**, on four grounds. The
              point estimate is positive where S093 verdict 2's was negative
              and reverted. The house rule keeps a measured zero with the
              reason stated -- S005, S006 and S015 are the precedent. The thin
              mechanism is itself a target: S119 and S120 both move the 1.12 %
              probe hit rate the substitution is starved by -- the S015 shape,
              forward-looking and labelled so, and this ground expires if
              those steps land without moving the rate. And, load-bearing: the
              bound-type rule -- a node never stores a claim stronger than the
              weakest thing that produced its value -- is durable
              infrastructure that S112, S022 and S116 each need before they
              feed the same stand pat, since each written the natural way
              re-introduces the laundering the killed run was measuring.
              **No re-run at two-sided bounds**: +/- 4.04 over 16784 games is
              tighter than most verdicts this project produces, and
              [-2.9, +5.2] already excludes the 5-Elo loss a re-run would be
              asked to exclude.
Rejected:     Reverting the substitution and keeping only the store rule --
              with no entry every branch reduces to the pre-S130 code exactly,
              so the substitution costs nothing measurable and removing it
              buys nothing measurable, and reverting deletes the consumer
              S119 and S120 would re-price. Re-running two-sided to buy a
              label for a number already in hand -- two more machine-hours,
              DEC-063's lesson.
Consequences: specs.md's "absent, machinery" row carries the layer, its
              numbers and the degraded store. The stand pat carries its bound
              kind and the final store degrades, which every later stand-pat
              consumer (S112, S022, S116) inherits and must not remove. If
              S119 and S120 land without moving the hit rate, ground three
              expires without re-opening this entry -- ground four stands
              alone.

## DEC-104  2026-08-22  The end goal, stated in full and in one place
Tags:         project, identity, scope, licensing
Context:      The goal had been written as "the strongest open-source chess
              engine in the world" since DEC-013, and its full content --
              the licence, the provenance rule, the evaluation end-state and
              the measurement discipline -- lived scattered across CLAUDE.md,
              specs.md, plan.md and four decisions. On 2026-08-22 the owner
              restated it in full and asked that it be documented clearly,
              with the history and the reasons left unchanged.
Decision:     By the owner. The end goal is: the strongest CPU chess engine in
              the world, built by AI-driven development, open source under the
              MIT licence -- and it stays MIT. Nothing is copy-pasted from
              another open-source project. The techniques are bleeding edge:
              taken from the literature and research, invented on the spot, or
              adapted -- never copy-pasted -- from other open-source engines
              where their licence consents. The evaluation end-state is an own
              NNUE, or whatever supersedes it, trained by this project on its
              own data. Every change is decided by rigorous testing -- SPRT
              and the other modern methods -- with specialized tools taken
              where they exist and built ad hoc where they do not, for
              training, fine-tuning and testing.

              This sharpens the goal sentence -- "open-source" becomes "CPU",
              the licence is named -- and changes no history and no reason.
              Nothing is voided: DEC-013 (identity), DEC-014 (literature
              first, experiment second), DEC-016 (nothing copied), DEC-015 as
              amended by DEC-041 (the NNUE training run is the owner's), and
              DEC-054 with DEC-071 (the 3000 mark is pursued without a
              network) all stand.
Rejected:     Leaving the goal implicit across three documents -- the wordings
              had already drifted, and none carried the licence or the testing
              discipline as part of the goal.
              Reading "CPU" as a new constraint on the plan -- it is not: the
              target scale was already the CCRL 1CPU entry (DEC-089) and no
              GPU has ever been a runtime dependency. It names the arena the
              claim is made in.
Consequences: CLAUDE.md, adocs/specs.md and adocs/plan.md state the same goal
              and cite this entry. One operative nuance is now written down:
              inspiration from another open-source engine is taken only where
              its licence consents, and it is adapted, never copy-pasted --
              while copying stays banned in every form and another engine's
              constants are not even seeds (DEC-016, DEC-084 untouched).

## DEC-105  2026-08-22  A seed's origin decides, not its venue: engine-tuned numbers are never seeds
Tags:         provenance, licensing, tuning, identity
Amended:      2026-09-04 by DEC-134 -- the rule binds the seed sections of the
              pending step files too, which its Consequences did not name; a
              constant quoted in another engine's commit message is that
              engine's constant, and S180 reseeds the seven files written
              under DEC-084's venue reading.
Amends:       DEC-084, whose seed rule was stated by venue
Context:      DEC-084 allows "a number from open literature -- papers, the
              wiki, articles" to seed a fit, and forbids anything from another
              engine's source or tables even as a starting point. Stated by
              venue, that has a hole: the Chess Programming Wiki republishes
              engine-tuned artifacts -- PeSTO's piece-square tables are
              rofchade's tuned output and sit on the wiki's Piece-Square
              Tables page -- so a venue rule launders exactly what the
              provenance rule refuses. The owner raised the wider question on
              2026-08-22: whether "not even as seeds" is the right rule at
              all, given two goals -- never being accusable of AI-laundered
              copying, and the engine being accepted as real work. The
              community record answers it: Rybka was banned on semantic
              equivalence of its evaluation, not on literal copying; Fat
              Fritz 2 retrained its network and was condemned and recalled
              anyway, because the artifact lineage and the originality claim
              were false; TCEC acceptance runs on disclosure of code and
              data provenance. The standard is stricter than copyright law,
              and a seeded-then-refit vector converges near its seed by
              construction, so "we refit it" is the defence that already
              failed. The technical value of engine seeds here is near zero:
              the fit is over chesso's own corpus, and S028 landed +188.74
              from hand-written starting values.
Decision:     By the owner. The seed rule is provenance-based. A number may
              seed a fit or an SPSA run only if it originates in a
              publication about the technique -- a paper, an article, the
              wiki's own derivations and example formulas. A number that
              originates as another engine's tuned output is never a seed,
              wherever it is republished: a tuned table on the wiki is still
              an engine's table. "Not even as seeds" is affirmed. One
              relaxation stays on record without being policy: constants from
              a permissively-licensed engine, taken with recorded
              attribution, would be legally clean -- available only as a
              per-case owner decision in the DEC-087 Fathom style, because it
              trades away the originality claim the project exists to make.
Rejected:     Keeping the venue wording -- it admits PeSTO-class laundering.
              Permissive-licence constants as standing policy -- legally
              sound, reputationally the exact "AI remixed other engines"
              headline this project must never hand out.
              Dropping "not even as seeds" -- the benefit is convergence
              speed only; the cost is the provenance story that is the
              project's strongest defence.
Consequences: plan.md's DEC-084 paragraph, CLAUDE.md's first foundation,
              AGENTS.md section 0 and specs.md's Non-goals all carry the
              provenance form. DEC-084 itself carries an Amended pointer;
              its "no constant ships unfitted" rule is untouched. A public
              provenance and originality statement in MANUAL.md was proposed
              and is not yet written; it needs the owner's voice and blocks
              nothing.

## DEC-106  2026-08-22  No rule demands a subagent; the Tier-1 review keeps the habit and drops the mechanism
Tags:         workflow, review, tooling
Amends:       AGENTS.md section 9's Tier-1 wording; section 10's permission is
              untouched
Context:      AGENTS.md section 9 required the Tier-1 fast check after every
              `--step done` to run as "one small subagent over that step's
              diff", while section 10 separately granted that subagents may be
              spawned freely. The obligation is what broke: a harness can gate
              the Agent tool -- this session's configuration forbade calling it
              unless the user asked -- and a ruleset that *requires* a spawn
              then leaves the agent choosing which instruction to violate. S160
              hit it exactly: the step completed green and its Tier-1 review was
              skipped and reported as skipped, because the two rules could not
              both be obeyed. The check itself was never the problem; naming its
              mechanism was.
Decision:     By the owner, 2026-08-22, on being shown the collision: "i want to
              allow subagents when you need them". Section 9's Tier-1 check
              stays a habit after every `--step done` and no longer names a
              mechanism -- a subagent where one earns its keep, inline over the
              diff where it does not. Section 10's permission is the operative
              half and is unchanged: spawn one freely whenever it is useful. The
              rule removed is the obligation, not the practice and not the
              permission. Restated as a section 0 house rule so a moltke
              upgrade that rewrites AGENTS.md has to re-apply it.
Rejected:     Removing Tier 1 altogether -- the fast check is cheap and it has
              already caught a stale specs claim one commit after S130.
              Banning subagents in this repository -- it would take
              `/moltke:audit`'s adversarial_reviewer with it, and the audits are
              where the found bugs come from: the 2026-08-22 report is the batch
              S160 to S165 is working through.
              Leaving the mandate and relaxing the harness instead -- the
              ruleset should not depend on one machine's tool gating, and
              section 0 exists precisely so repository rules survive it.
Consequences: AGENTS.md section 0 carries the house rule and section 9's Tier-1
              paragraph no longer says "subagent". A Tier-1 review is still
              owed after every completion and its absence is still reported;
              only the choice of instrument moved. S160's own skipped review is
              still owed and is taken under this rule.


## DEC-107  2026-08-23  S162's insurance SPRT is abandoned on a census: the changed path never fired in 3314 games
Tags:         testing, measurement, sprt, workflow
Amends:       S162's `accepts:`, which required one `--nonreg` verdict as
              insurance
Context:      S162 makes checkmate outrank the 100-halfmove draw. Its accepts
              asked for one `--nonreg` verdict as insurance "since the change
              does alter play in its class", with a null pre-declared as an
              expected and acceptable outcome. The owner questioned the run
              while it was in flight: no search rule and no evaluation term
              moved, so what could it measure? The answer was taken from the
              games the run had already played rather than from the argument.
              **The clock zone is reached in real play, which is the half that
              refutes the cheap dismissal**: over 3285 completed games at
              8+0.08, 102 peaked at a halfmove clock of 80 or more -- the depth
              this engine reaches means a root at 80 can see clock 100 inside
              its tree -- and 55 reached clock 100 on the board itself. So the
              new `is_check()` and move generation do execute in about 3 % of
              games and the branch is not dead code. **The changed behaviour
              still never fired**: of 3314 games, the positions that were
              checkmate at a clock of 100 or more numbered **0**, and at 90 or
              more also 0. The run agreed: 3304 games, `LLR 0.09`, 2.9 % of the
              way to a bound, `Elo -1.37 +/- 8.67` -- a true zero on
              `elo0=-5 elo1=0` sits on the H1 boundary and drifts, so the
              likely ending was the 20000-round limit with no verdict after
              another four hours of the plan's binding constraint.
Decision:     Owner's, on the evidence above. The run is killed and no SPRT is
              owed for S162. What replaces the verdict is stronger than the
              verdict would have been and it is what the step stamp records:
              the census (0 firings in 3314 games, 102 games entering the
              zone), the unit case that pins the boundary node directly at
              depth 1 on two tool-verified positions, INV-6-identical node
              counts and best moves, and the 1700560-position enumeration
              showing the insufficient-material test below cannot intercept a
              mated node. **The general rule this sets: an insurance run whose
              only reachable outcome is the outcome already pre-declared is not
              a measurement, and a census over the games that would carry the
              effect decides it in ninety seconds instead of four hours.** It
              applies to a correctness fix on a rare boundary, not to a change
              that alters the tree everywhere -- INV-6 is untouched and a
              play-altering change is still decided by SPRT.
Rejected:     Letting it finish -- at 2.9 % of a bound after 3304 games the
              expected cost was hours for the pre-declared null.
              Calling the change behaviour-neutral and discharging INV-6 on
              node counts alone -- it is not: the census shows the branch
              executing in about 3 % of games, and identical bench counts only
              say no bench FEN approaches the boundary.
              Restarting at wider bounds or a longer control -- a wider band
              cannot resolve an effect the census measured at zero firings, and
              the deeper the control the rarer the shuffle that reaches the
              clock.
              Constructing a book of high-clock positions to force the path --
              that measures a distribution the engine does not play, so the
              Elo it returns would not transfer (DEC-019's failure mode from
              the other direction).
Consequences: S162 completes with a census in place of a verdict, and
              `adocs/specs.md` says so rather than pointing at a run. plan.md's
              "Only S162 and S165 owe a match at all" becomes S165 alone.
              S165's own `--nonreg` stands: its guard changes what null move
              pruning does at every defender node inside a mate proof, which is
              not a boundary the census can bound. The census script is one
              python pass over a run's PGN and is worth reaching for again
              before booking a night on a rare-path fix.

## DEC-108  2026-08-23  The intermediate rating checkpoint is dropped; the engine is re-rated once, near the goal
Tags:         measurement, rating, plan, machine-time
Amends:       S152's `goal:` and its position in the pending order; folds S128's
              calibration question into whatever the next rating run is
Context:      S088 measured 2559 CCRL Blitz, 95 % +/-25, SOFT, on 2026-08-18.
              Twenty-seven steps have completed since and six of them carried an
              SPRT. Asked to estimate current strength, the agent could only
              offer arithmetic: the kept positive point estimates sum to about
              +90, which DEC-063's own correction factor -- S068's pooled
              estimate falling from +12.18 to +5.02 -- cuts to about +49, and
              S104's +18.22 % landed after the rating run and is unpriced
              because what a ply is worth here has never been measured. So the
              gap between +90 and +49 is real and a checkpoint would close it.
              S152 exists for exactly that: "the engine's absolute rating is
              re-measured once after the search block, before the speed block,
              so 45 to 55 verdicts are not accumulated without an end-to-end
              check."
Decision:     The owner's, 2026-08-23, on being shown the estimate: do not
              re-measure now. **The engine is re-rated once, when it is close to
              the 3000 mark, and not at a block boundary.** S152 is re-targeted
              to that trigger and moved to the end of the pending order so
              nothing derives it as next-in-order; its id is not reused and its
              file is kept whole. **S128 is folded into that same run** rather
              than kept as a separate night: its question is whether S088's
              **121.8 Elo anchor spread over five references** -- 64.7 without
              Leorik 2.1, against the 30 the procedure allows -- is scale
              compression from 10+0.2. DEC-077 named the control as the leading
              candidate and priced the compression at a ratio of 0.853 across a
              440-Elo CCRL span. That is a property of how the final number is
              read, not of the current engine, so it is answered by running the
              eventual gauntlet at two controls instead of by replaying the old
              one now. S128's file is folded into S152's, DEC-086's precedent,
              and its id is not reused.
Rejected:     Keeping S152 at the block boundary -- it buys information and no
              strength, and five hours of the binding constraint is the price.
              The plan's own framing is that measurement capacity is what limits
              it, and a checkpoint spends that on a number nobody acts on: a
              disappointing checkpoint would not change the pending order, which
              was itself set by an adversarial review against the published
              record (DEC-081 to DEC-089).
              Retiring S152 outright -- the end-to-end check is still owed
              before the 3000 claim is made, and an absolute number is the only
              thing that can falsify a chain of self-play verdicts. Deferred is
              not retired.
              Estimating strength from the SPRT sum instead and recording that
              as the current rating -- it is arithmetic on early-stopped
              estimates and it is not a measurement. The +90 figure in
              particular is exactly the kind of number this project has been
              wrong with before, and it is recorded in this session's transcript
              as an estimate with its range and nowhere else.
Consequences: The last measured rating stays 2559 +/-25 soft until the run near
              the goal, and no document may quote anything else as the engine's
              rating. An estimate may be given when asked, with its correction
              and its unpriced terms stated, and it is never written into
              `specs.md` as a measurement. Between here and there the evidence
              of progress is the per-change SPRT ledger and nothing else, which
              makes the pre-registered reading in each run's script the load
              bearing artefact rather than a formality. When the run does come
              it carries S128's second control, so budget two gauntlets and not
              one.

## DEC-109  2026-08-29  moltke v1: the workflow becomes rules, and the house rules are re-applied by hand
Tags:         workflow, moltke, watchers, measurement, documents
Context:      The owner upgraded the moltke plugin from 0.13.0 to 1.0.0. v1
              replaced the enforcement product with rules: no hooks, no
              `bin/moltke.py`, no `--step`, `--validate` or `--watch`, no
              `.moltke.json`, and no `step` skill -- only `init`, `rules` and
              `audit` remain. `AGENTS.md` had to be replaced with the v1
              template, and its own par.0 said a rewrite must re-apply the
              house rules, which is the clause this migration exists to honour.
              The plugin ships `adocs/migration_prompt.md` as the procedure and
              it was followed, with three deviations decided by the owner.
Decision:     Migrated, by the owner's instruction. `AGENTS.md` is the v1
              template byte-for-byte above `## Project rules`, and all ten of
              the old par.0 house rules are re-applied there as named rule
              lines -- COPYING, MEASUREMENT, MACHINE, RUNS, CHESS, BUGS, DOCS,
              AGENTS, plus GIT/COMMITS/TESTS/SURFACE/PLAN/DEPS from the
              interview table. Two rules are new rather than carried:
              **WATCHERS**, which keeps DEC-061's four mandatory exits and the
              banned forms but replaces the deleted `--watch` primitive with
              the `bash -c` poll loop and an inlined BSD deadline; and
              **POWER**, which forbids starting a timed match on battery.
              Deviations from the migration prompt, all three the owner's:
              AUDIT is on demand only rather than propose-on-risk; the
              `test_command` harvested from `.moltke.json` is corrected from
              `-j12` to `-j8`, the core count of the machine now in use; and
              `adocs/testing.md` is deleted after its only non-duplicated
              content -- the six invariant-to-covering-test rows -- moved into
              `specs.md` beside the invariants they guard. Deleted with it:
              `.moltke.json`, `.git/moltke_watch/`,
              `.git/moltke_audit_baseline.json`. `.moltke.local.md` moved from
              `.git/info/exclude` to `.gitignore`. `plan.md` is reshaped into
              `## Open` and `## Done recently`, both hand-maintained.
Rejected:     Keeping 0.13.0 to preserve enforcement -- versions cannot coexist
              in one Claude config root, so the choice was already made by the
              upgrade, and hooks had stopped firing before this entry was
              written.
              Keeping `testing.md` untouched -- 88 of its 94 rows are per-step
              records that `plan_done/` and `decisions.md` already hold in
              full, and nothing prunes or appends it any more, so it would rot
              from the moment the checker left.
              Deleting `adocs/worklog.md` as well -- it is 208 KB of forensic
              history that the 0.x hooks appended and nothing writes now. Kept
              frozen: it costs nothing and it is the only record of some
              sessions.
Consequences: Nothing is machine-enforced any more. A step completes by hand --
              write the `done:` stamp, move the file to `plan_done/`, move its
              entry from `plan.md`'s Open list into `Done recently` and drop
              the oldest of the five, rewrite `status.md`, commit -- and a
              missed edit is a missed edit, visible only in the diff. Two
              parked items in `status.md` died with the checker they described
              and were removed: the `bin/moltke.py` path item, and the one
              explaining why "last done" could name an older step than the
              newest completion. That second behaviour is gone with its cause:
              `Done recently` is now in completion order, so "last done" is
              S167 and no longer S145. Watchers lose the primitive that
              registered them under `.git/`, so a watcher's existence is no
              longer derivable from the filesystem -- the WATCHERS rule
              compensates by making the poll loop's four exits explicit, and
              `status.md` carries a `Watching:` line that has to be true.

## DEC-110  2026-08-30  The clang-format pin moves from major 22 to major 23
Tags:         toolchain, tests, gate, macos
Amended:      2026-09-07 by DEC-146 -- the pin stays 23 and the workstation
              exports `CLANG_FORMAT_MAJOR=22`, which is the cost this entry's
              Consequences predicted, paid rather than removed.
Context:      `ctest -L fast` went red on this machine with no code change
              behind it. `test_clang_format_script` failed six assertions, all
              of them because `clang-format.sh` resolves a binary and then
              refuses it: "clang-format 22 not found. Found, but wrong version:
              /opt/homebrew/opt/llvm/bin/clang-format (23.1.0)". Homebrew's
              llvm keg was replaced by 23.1.0 on 2026-08-29 at 23:50, which is
              after S024's code commit `e424032` (2026-08-27 22:21) and during
              that step's SPRT runs. `brew list --versions llvm` reports 23.1.0
              and the Cellar holds nothing else, so major 22 is not on the
              machine and cannot be selected. The completion gate ends in
              `./clang-format.sh --check`, so until this was settled no step
              could be marked done here at all.
Decision:     By the owner, asked directly. `REQUIRED_MAJOR` becomes 23.
              `DEV_MANUAL.md` and `TOOLCHAIN.md` carry the new number, and
              `TOOLCHAIN.md`'s Ubuntu package name becomes `clang-format-23`.
              The escape hatch is unchanged: `CLANG_FORMAT_MAJOR` still
              overrides, and the pin's reason -- output moves between major
              versions, and an unpinned formatter rewrites files nobody touched
              -- is unchanged and is why this is a decision rather than a
              widened range.
Rejected:     Installing clang-format 22 and keeping the pin. It preserves the
              Linux workstation's toolchain untouched, but homebrew ships no 22
              formula, so it means a versioned tap or a hand-built LLVM that
              the owner would have to run, on a machine that is on vacation
              duty for documents and short runs.
              Widening the pin to a range, or dropping it. That is the failure
              the pin was written against: two machines formatting the same
              tree differently, and a real change buried in a reformat.
Consequences: 23 reformats exactly one construct in this tree, and it is
              S024's: `struct continuation_history_t` collapses from three
              lines to one, because `AllowShortBlocksOnASingleLine` reaches a
              single-member struct under 23 and did not under 22. That hunk
              lands in this commit and is reverted with the rest of S024 in the
              next one, so after the revert the tree is byte-identical under 22
              and 23 and the pin move costs no reformatting at all. What it
              does cost is the Linux workstation: it must have clang-format 23
              before it can complete a step, and until it does, its gate fails
              exactly the way this machine's just did. `.moltke.local.md`
              records the version actually present here.

## DEC-111  2026-08-30  S024's MacBook attempt is discarded from achesso and preserved on a branch
Tags:         plan, measurement, sprt, machine, S024
Context:      S024's one-ply continuation history was built, tested and
              committed here on 2026-08-27 (`e424032`) and never measured to a
              verdict. Three SPRT runs at `elo0=0 elo1=5` against `25998fe`
              failed to reach a bound: run 1 aborted at 2582 games for spanning
              a power transition, run 2 interrupted at 6054 games after
              03:54:58, run 3 -- a resume of run 2, pooling its statistics --
              interrupted at 7988 more after 05:12:29. The pooled estimate is
              `Elo 2.87 +/- 4.37` over 14038 games, `LLR 0.74` of `+/-2.94`.
              At that effect size the bound pair wants about 56000 games, some
              21 hours at the 2700 games/h measured here, and this machine has
              twice survived four to five hours of a full-core match. The owner
              is away from the Linux workstation and returns to it; the code
              was sitting unmeasured in the trunk in the meantime, which is
              exactly what an SPRT-decided project does not do.
Decision:     By the owner. The work is erased from `achesso` -- `e424032`'s
              `src/` and `tests/` reverted, the step file moved back to
              `plan_todo/` -- and S024 is redone properly on the Linux
              workstation, from zero games, at a bound pair chosen before the
              first game. `git revert`-equivalent and not a history rewrite:
              the GIT rule forbids rewriting and unpushed does not exempt it,
              and the two commits stay in the log.
              **Nothing is thrown away.** Branch `s024_mac_attempt` carries the
              implementation, its tests, and 14 MB of run evidence under
              `adocs/data/S024_mac_attempt/` that was gitignored and would have
              died with the machine: both PGNs, the three run logs, the
              fastchess log, the resume config carrying the tournament's
              statistics, and the resume script, each with its digest and a
              README saying what it is. The branch is deleted once S024 lands.
              The step file keeps the research, the design, the mutation each
              test was observed red under, and the pre-match node counts, so
              the reimplementation starts from knowledge rather than from
              nothing.
Rejected:     Resuming a fourth time. It is 15 hours from a verdict on a
              machine that dies at five, and each resume re-opens the same
              exposure.
              Switching the bound pair to `-5/5` or `--nonreg` and pooling the
              14038 games into it. That is choosing the hypothesis after seeing
              the data: the SPRT's error guarantee assumes the pair is fixed
              first, and the numbers would look like a verdict while being one
              in name only.
              Keeping the unmeasured feature in the trunk until the workstation
              is available. A play-altering change with no verdict contaminates
              every measurement taken on top of it, which is the BUGS rule's
              reasoning applied to an unproven feature rather than a defect.
              Dropping the two commits from history instead of reverting --
              forbidden by the GIT rule, and it would have destroyed the record
              of why the runs failed.
Consequences: `achesso`'s engine is byte-identical to `10c350a`'s in `src/` and
              `tests/`, so any measurement taken here from now on rests on a
              tree with no unproven feature in it. S024 returns to
              `plan_todo/`, and `plan_current/` is empty, which is what frees
              the machine-light lane DEC-112 opens. `adocs/data/S024_sprt.sh`
              stays committed and is marked stale at its head: its pinned `REF`
              and its inherited bound pair must both be re-decided before it
              runs again. The estimate is recorded in the step file with an
              explicit instruction not to use 2.87 as a prior that shortens the
              real run.

## DEC-112  2026-08-30  The plan is scoped to the machine for the duration: a machine-light lane, temporarily at the head of Open
Tags:         plan, machine, measurement, workflow
Amended:      2026-09-05 by DEC-144 -- the lane is dismissed, the `## Machine
              scope` section deleted from plan.md and the Open list re-sorted
              for the workstation; the lane's reasoning stays here.
Context:      The owner is on vacation with the MacBook and returns to the
              Linux workstation, where the heavy runs belong. This machine now
              has its own measurements rather than an estimate: 2700 games/h on
              8 threads at 8+0.08, and two observed deaths after four to five
              hours of a full-core match, which is what ended S024 here
              (DEC-111). Read against the pending order, that is a machine
              which can finish a document step, a behaviour-neutral change
              discharged on node counts, or one short verdict -- and cannot
              finish a step that owes three. Two hard limits sit underneath:
              `_pext_u64` is BMI2 and this is an M1, so S032 cannot be measured
              here at all; and `selfplay_v2.tsv` is 683 MB, gitignored, and
              never made the machine move, so every fit and corpus step is on
              the workstation by construction. Left alone, the plan's first
              entry would have been a step this machine cannot complete, and
              the alternative to scoping it is an idle machine for the
              duration.
Decision:     By the owner, who asked for exactly this: work that can be done
              without a long SPRT, and a proposed plan modification rather than
              a silent reordering. `plan.md` gains a `## Machine scope` section
              stating what this machine can and cannot take, with the reason
              per excluded step, and its Open list is **reordered, not
              rewritten** -- fifteen machine-light entries lifted to the head,
              everything else keeping the relative order the 2026-08-19 review
              gave it. Ten of the fifteen touch no engine and own no match
              (S153, S158, S157, S150, S155, S156, S154, S143, S144, S146),
              three are behaviour-neutral and discharge on identical
              `search_bench` node counts and best moves plus a timing (S147,
              S020, S030), and two own one self-contained verdict each if a
              short run is wanted (S148, S159). No id is renumbered, no step's
              content changes, and the section says how to put the order back.
Rejected:     Lifting S095, S131 or S116 into the lane because each is one
              verdict. They sit inside the search block's dependency order --
              "each step's consumers exist before it" -- and pulling one ahead
              of S109 buys a verdict measured against a tree the plan is about
              to change. S148 and S159 depend on nothing in that block, which
              is why they are the two that can move.
              Lifting S117 with the other speed steps. Its `accepts` reproduces
              the taper exactly and after S055 that is one division rather than
              two, so it follows S055, which owes its own SPRT.
              Leaving the order alone and taking steps out of it by hand. The
              plan says order lives in the Open list and nowhere else; an agent
              picking a different entry each session is the drift the rule
              exists against.
              Waiting for the workstation. Fifteen steps' worth of work is
              available that the workstation would otherwise spend its time on
              instead of matches, which is the wrong way round: the machine
              that can measure should be measuring.
Consequences: The next step is `plan.md` Open entry 1, S153, as always -- the
              rule does not change, only the list does. S153 is itself about
              whether a document step and a match-owning step may be active at
              once, and its `accepts` is stale: it names `.moltke.json`,
              `plan_active_max` and `--validate`, all deleted by moltke v1
              (DEC-109), so taking it starts by restating its acceptance in v1
              terms. Restoring the block order when the workstation is back is
              a decision and not a tidy-up, so the reason the lane existed
              stays findable.

## DEC-113  2026-08-30  Steps run concurrently, as many as are strictly necessary, with one coordinator that holds the machine
Tags:         workflow, plan, measurement, moltke, s153, dec-096, dec-112
Context:      DEC-096 protects machine time from document work, on the grounds
              that measurement capacity is the binding constraint. The converse
              was never recorded: an agent-only step leaves that same constraint
              idle, and the two classes contend for nothing. The
              2026-08-21 adversarial audit measured the cost from the project's
              own `done:` stamps and S153 re-checked every term of it:
              S089 21 m 59 s, S094 2 h 07 m, S094 1 h 23 m, S107 1 h 37 m 52 s,
              S085's SPSA 8 h 21 m and its verification 1 h 15 m -- 15.10 h of
              stamped run in the 65.4 h since the S088 anchor, 23 % of the
              window, five SPRT verdicts at one per 13 h. Against that, 48 of
              the 60 pending steps name an SPRT or a bound in `accepts`. At the
              observed rate the remaining verdicts are weeks of calendar and
              days of compute, and the gap is idle machine.
Decision:     By the owner, asked directly while S153 was active: allow any
              strictly necessary number of active steps, one per agent, with
              exactly one agent the coordinator and machine holder. The
              coordinator alone starts a match, an SPSA, a fit or a timing;
              every other active step is agent-only work that owns no run.
              AGENTS.md's PLAN rule is rewritten to say it. The audit's own
              suggestion -- raise a cap from 1 to 2 -- is narrower than what was
              chosen and the cap is gone rather than raised.
              Two clauses are the agent's reading of "coordinator" and are
              recorded as such so they can be corrected: the shared documents
              (`plan.md`, `status.md`, `specs.md`, `decisions.md`) are written
              through the coordinator, and *strictly necessary* means the
              machine would otherwise idle or an active step cannot advance --
              not that the Open list is long.
Rejected:     Keeping the strict one-step sequencing and recording why. It is
              the safer rule and it is what produced the measured 23 %; the
              owner declined it.
              The audit's literal suggestion, `plan_active_max: 2`. There is no
              such knob to raise: moltke v1 ships no `.moltke.json` (DEC-109),
              so the rule is prose in AGENTS.md and a number in it would be an
              arbitrary ceiling on a bound the owner deliberately left open.
              A per-step machine flag, so an agent could ask whether it may run.
              Nothing enforces any of these rules; a second marker is a second
              thing to keep true, and "the coordinator holds the machine" is one
              sentence that needs no bookkeeping.
Consequences: `plan_current/` may hold more than one file, and the `author:`
              field is what says which agent owns which. The `done:` stamp, the
              Open list edit and the `status.md` rewrite that finish a step all
              go through the coordinator, so a concurrent agent finishes by
              handing its stamp over rather than by writing plan.md itself.
              MEASUREMENT is untouched and still binds: one change at a time in
              the tree, whatever runs beside it, because a second play-altering
              change landing mid-match makes both numbers meaningless -- what
              overlaps a match is document and process work, not a second patch.
              This buys nothing while DEC-112's machine-light lane is in force:
              the first ten Open entries own no match and this MacBook cannot
              take the heavy verdicts anyway. The lever is worth its 23 % when
              the workstation is back, which is the situation it is recorded
              for. `2026-08-21_adversarial-F05` is closed by this entry.

## DEC-114  2026-09-01  A second mate motif is worth constructing, and it is its own step
Tags:         testing, pruning, rfp, mates, s145, s155, s168, dec-095
Context:      S145 replaced a three-position mate gate with 48 constructed
              forced mates, and the 2026-08-21 audit's F07 said the replacement
              carries the same monoculture. S155 counted it rather than argued
              it: `adocs/data/S155_motif_census.py` over the tracked TSV reports
              two material signatures and one is the colour mirror of the other,
              a lone queen as the mating force in 48 of 48, `lead` 760 in 48 of
              48, a pawn wall on three non-adjacent files throughout, and eight
              family labels that are one geometry under two file shifts, a
              mirror and a colour swap. Breadth in mate distance is real --
              16 / 16 / 8 / 8 over distances two to five -- and breadth in shape
              is absent. The narrowness is close to forced: reverse futility can
              only hide a node that is lost by force while the side to move is
              materially ahead, and a frozen clump behind a blocked pawn wall is
              close to the only way to build that property. S155's `accepts`
              required the second-motif question to be answered either way.
Decision:     By the owner. **Yes, a second motif is worth constructing**, with
              a mating piece that is not a queen -- a smothered or otherwise
              knight-delivered mate, or a back-rank mate -- and it is **S168**,
              its own step, not work folded into S155. S155 stays documentary
              and closes: the qualifier, the reason the narrowness is forced,
              and the explicit list of what the gate cannot catch are written
              wherever the breadth is claimed.
Why:          One mating piece across the whole set means a defect that depends
              on the mating piece is invisible to the gate that exists to catch
              exactly that class of defect.
Rejected:     Answering no on S145's own measurement -- that the reading is
              almost entirely a function of mate distance, 16 of 16 at two
              against 0 of 8 at five. Defensible, and rejected because that
              measurement is taken over one motif and therefore cannot say
              whether shape matters.
              Folding the construction into S155. A new family owes its own two
              proofs, its own regeneration of the TSV and its own
              reverse-futility sweep before `MATE_IN_THREE_FLOOR` and the
              mate-in-two timing assertion can be restated; that is a step, and
              carrying it here would have made a documentary step one that
              cannot close.
              Replacing the 48 rather than extending them. Every one was
              independently re-proved a forced mate at its claimed distance;
              they measure the hazard the rule actually has.
Consequences: The written limits now say what this gate cannot speak for --
              back-rank, smothered and any knight mate, king hunt, open-line
              mate or line-opening sacrifice, promotion mate, and any position
              with a realistic material balance -- in `tests/test_engine.cpp`,
              `adocs/specs.md`, `adocs/data/S145_mate_set.py`, `DEV_MANUAL.md`
              and `MANUAL.md`. S168 sits third in DEC-112's machine-light lane:
              it owns no match, but it does own two oracles and a sweep. When it
              lands, those five statements and the census's own expected output
              are what it must correct. `2026-08-21_adversarial-F07` is closed
              by S155.

## DEC-115  2026-09-01  The mined breadth set is asserted at depth 10 and a floor of 143, not at the cheaper depth that separates wider
Tags:         testing, mates, rfp, s145, s156, s142, suite-cost, dec-019
Context:      S145 built `adocs/data/S145_mined_set.tsv` -- 318 positions taken
              one per game from chesso's own SPSA games, labelled by stockfish
              at a node limit -- and scored it once by hand. Nothing read it
              afterwards, which `2026-08-21_adversarial-F08` found and S156 was
              written to settle either way: assert it, or discharge the clause
              in writing. Measured before deciding, at `120497e` on the machine
              `.moltke.local.md` describes, exact counts against the value the
              weakened guard gives:

              | depth | ships | `RfpMinPly` 1 | gap | wall |
              |---|---|---|---|---|
              | 8 | 113 | 101 | 12 | 3.0 s |
              | 9 | 143 | 135 | 8 | 7.4 s |
              | 10 | 145 | 141 | 4 | 17.6 s |

              The cheap depth separates three times wider at a sixth of the
              cost, which is the opposite of what was expected and is why the
              choice was put rather than taken.
Decision:     By the owner. **Assert it, at depth 10 with a floor of 143** --
              the depth and the floor S156's own `accepts` named -- as
              `tests/test_mate_breadth.cpp` in the fast label, scoring the
              whole set as a count with a floor plus zero mate scores with the
              wrong sign. The depth 8 reading is recorded rather than used.
Why:          The floor is the number a future red has to be judged against, so
              it keeps the provenance it was placed with rather than being
              re-derived at a depth chosen for cost.
Rejected:     Depth 8 at a floor near 107. Wider separation and 3.0 s instead
              of 18.28 s, and rejected because it re-places a floor that
              already had a measured origin. It is the standing alternative to
              lowering the floor the next time it goes red, and it is written
              down in `DEV_MANUAL.md` and in S156's step file so it does not
              have to be re-measured.
              Discharging the clause in writing with no assertion. That leaves
              the file as data nothing reads, which is the finding rather than
              its answer.
              Scoring through `adocs/data/S145_mined_set.py` from the suite.
              It drives the engine through python-chess, which is deliberately
              not a dependency of anything the fast label runs and is not
              installed on this machine at all. The gate reads the same TSV
              in-process instead, and its count was cross-checked against an
              independent standard-library subprocess driver: both read 145
              exact, 147 right sign, 0 wrong sign.
Consequences: The fast label goes from 28.50 s over 21 tests to 45.92 s over
              22, and `test_mate_breadth` is 18.28 s of it -- the largest
              single line in the gate, which is why it is its own binary rather
              than a case inside `test_engine`. In Debug it is 697 s, 38 times
              the Release figure, so it carries its own measured timeout of
              1500 s and `ctest --test-dir build-debug -L fast` is eleven
              minutes longer than it was.
              The floor's separation is narrower than S145 recorded -- 145
              against 141 where S145 read 146 against 139, the gap down from 7
              to 4 as S142, S149 and S165 moved the tree. 143 still sits
              strictly between the two, and a re-measurement is owed whenever
              it next goes red rather than a reflexive lowering.
              Reaching the values below the floor needs a patched tree: S142
              made 2 `RfpMinPly`'s declared minimum, so `setoption` is refused
              and a sweep that does not notice measures one engine against
              itself -- the first sweep taken for S156 did exactly that.
              `adocs/data/S156_mined_floor_sweep.py` relaxes the bound in a
              throwaway git worktree and rebuilds the gate there with the
              weakened value as its compiled-in default, which is how the red
              is observed.

## DEC-116  2026-09-01  The mate-in-three floor is 8, and it is re-derived whenever either end of it moves
Tags:         testing, mates, rfp, s145, s154, s156, s165, dec-019, dec-095
Context:      `2026-08-21_adversarial-F06` said `MATE_IN_THREE_FLOOR` had one
              position of margin with thirteen tree-reshaping steps queued
              behind it, and that the comment's claim -- the floor "fails when
              the guard fails and not when the tree shifts underneath it" --
              was asserted and not measured. S154 measured it three ways at
              `fc5526e`, on the machine `.moltke.local.md` describes.

              **The claim is true, and by a wide margin.** The set was run
              through the binary built at each of the seventeen commits that
              touched `src/` since `14748c9` placed the floor, and through nine
              transposition table sizes from 1 MB to 256 MB. Positions changing
              verdict: **0**, everywhere except `aa8c077`, which moved one and
              moved it upward. The table sweep moved the node total 5.9 % over
              the set and 17 % over the mates in three, so the tree did shift
              and the verdicts did not follow. One ply of the guard itself
              moves five positions.

              **The finding is the other one: 7 had stopped separating.** S145
              placed it between 8 shipping and 6 with the guard removed.
              `aa8c077` -- S165, null move pruning guarded at both edges of the
              mate band -- lifted both ends to 9 and 7. `7 >= 7` is green, so
              from 2026-08-23 to 2026-09-01 the assertion could not fail for
              the reason it exists. The gate as a whole still caught a removed
              guard, through the mate-in-two clause, which is why nothing was
              red and nobody noticed.
Decision:     **The floor is 8**, strictly between the 9 that ships and the 7 a
              removed guard gives, by the agent on the measurement above. It is
              a test constant and not an engine default, so no play changes and
              no SPRT is owed; `tools/search_bench.py` is unaffected because no
              engine source is touched.

              The standing part is the second clause: **a floor is re-derived
              from a fresh sweep whenever either end moves, never re-read.**
              S156 wrote the same rule for the mined set's 143 and this is the
              case that shows why -- there the floor was owed a re-derivation
              after a red, here it was owed one after a green, and the green is
              the harder one to notice.
Rejected:     Keeping 7 with the measurement recorded beside it, which is what
              the step's `accepts` anticipated. Rejected because the
              measurement said the floor was inert, and recording that beside
              an assertion that cannot fail is documentation of a defect rather
              than a fix.

              Widening `MATE_DEPTH_SLACK` past 8. The window looked like the
              tighter fence -- one position now first reports its mate at
              exactly the last iteration searched -- but at slack 12 the
              shipping guard and the removed guard both find 10 of 16, so the
              separation is gone entirely and the pass costs 4.1 s against
              0.7 s. The window is a cost budget and the mate-in-three count is
              a reading of lateness under it, not of loss.

              A floor of 9. No margin at all: the one position sitting at the
              window edge would redden it with no guard having failed, which is
              the mechanism F06 was written about.
Consequences: `tests/test_engine.cpp` asserts `>= 8`, observed failing at
              `REQUIRE( 7 >= 8 )` with the default weakened to 1 in a throwaway
              worktree. The mate-in-two clause is restated where it is
              asserted: at `RfpMinPly` 1 it is 13 of 16 found and **9 of 16 on
              time**, so what goes red there is seven positions and not three,
              and `adocs/data/S145_rfp_sweep.log` carries a header saying so
              rather than being rewritten.

              Three numbers the comment quoted from the `RfpMaxDepth` axis had
              also moved and are refreshed: 40 of 48 with the rule off against
              34, and 6 of 8 and 5 of 8 mates in four and five at
              `RfpMaxDepth` 0 against 4 and 3. The ceiling is costing more than
              S148's question was queued on. That axis is still S148's and no
              default was touched here.

              `adocs/data/S154_floor_margin_sweep.py` is the harness and it
              refuses a setting the engine declines: it sends `isready` and
              stops on `info string refused`, which is the failure S156
              recorded turned into an error instead of a silent null row.

## DEC-117  2026-09-01  The constructed mate set carries three motifs, the mating piece is enforced rather than assumed, and a motif is regenerated with `--only`
Tags:         testing, mates, rfp, s145, s155, s168, dec-114, dec-116, dec-095, toolchain
Context:      DEC-114 is the owner's decision that one mating piece across a
              gate built to catch mating-piece defects is not enough, and left
              the shape open: "a smothered or otherwise knight-delivered mate,
              or a back-rank mate". S168 built it, and three things the step was
              queued on turned out to be false when measured.

              **A candidate probe chose the wall and the forces.** Four walls
              and forces, one family each at the tracked seed and budget: a king
              and two knights over S145's wall accept **nothing** in 6000 tries;
              over that wall plus an h-file pawn pair closing a corner they
              accept four; a king and rook fills every mate distance over
              either. So the pocket is what makes a non-queen mate constructible
              at all, and it is a measurement rather than a preference.

              **A king and two knights does not give a knight mate by
              construction.** A mobile knight standing beside a wall pawn
              unfreezes the capture the non-adjacent files deny and the freed
              pawn queens with check: of the first fourteen knight positions,
              **twelve were mated by a pawn**. Walking the same lines separates
              the cases perfectly by *promotions available*, 4 apiece against 0
              for the two real knight mates, where "pawn moves" does not
              separate at all -- all fourteen have them.

              **The geometry did not survive being checked either.** The rook
              mate lands on the mated side's own back rank in **13 of 32**, so
              "back-rank" would have been a label the file cannot carry.
Decision:     By the owner on the probe, 2026-09-01: **both forces over the one
              new wall**, giving three motifs -- a queen in 48 rows, a lone rook
              in 32, two knights in 2. By the agent on the measurements: a motif
              may declare `mates_with`, and a candidate is refused unless every
              move that mates at the end of its line is that piece and no
              promotion is available anywhere along it; the rook families are
              named for their force and not for a geometry; and **`generate`
              takes `--only`, which is now how a motif is added**, because the
              stockfish proposer is version-bound -- re-running family `shift0`
              here returned 6 of its 8 tracked rows and two different ones -- so
              a plain regeneration on a second machine replaces positions that
              S145 landed rather than adding to them.

              `MATE_IN_THREE_FLOOR` is **11**, DEC-116's rule applied a second
              time: the ends moved to 12 shipping and 10 with the guard removed,
              and `REQUIRE( 10 >= 11 )` is the red observed in a worktree. Test
              constants only; no engine source is touched and no SPRT is owed.
Rejected:     One force rather than two. The knight covers the axis DEC-114
              names first and the rook is the one that fills mate in two, so
              taking one would have left either the non-sliding mating piece or
              the gate's strongest assertion without a second piece.
              Refusing a knight candidate whose line offers any pawn move.
              Stricter, and it refuses the motif rather than the defect: all
              four families accepted nothing out of 113 proposals, because every
              knight line has pawn moves on it.
              Keeping the `backrank` name with the 13-of-32 measurement recorded
              beside it. A label that says what a reader would otherwise have to
              check is worth more than a label that matches the step's title.
              Regenerating the whole file. S168's `excludes` forbids replacing
              the 48, and on this machine a full `generate` would have.
Consequences: The gate is 82 positions and 0.95 s, against 48 and 0.79 s, in a
              fast suite that runs 45 s. What it still cannot catch is a written
              list in five places -- `tests/test_engine.cpp`, `adocs/specs.md`,
              `DEV_MANUAL.md`, `MANUAL.md` and `adocs/data/S145_mate_set.py` --
              and the list is shorter by one line and longer by another: a mate
              delivered by a knight is covered at mate in two and nowhere else,
              and a smothered mate, a king hunt, an open-line mate, a promotion
              mate and any realistic material balance are still outside it.
              `S155_motif_census.py` is the count and `adocs/data/S168_*.log`
              is the evidence. DEC-114 is discharged.

## DEC-118  2026-09-01  The completion gate covers both builds: `build-tune` is built and tested beside `build`
Tags:         workflow, testing, tuning, moltke
Amends:       DEC-025
Context:      `src/search_params.hpp` is deliberately different code in the two
              builds -- `inline constexpr int` in the shipping build, a plain
              `int` settable over UCI under `CHESSO_TUNE=ON` (S073). The gate
              DEC-025 fixed builds and tests only the shipping one, so the two
              can diverge with the gate green. They did: a
              `static_assert(ASPIRATION_MIN_DEPTH >= 2)` added to
              `tests/test_engine.cpp` while fixing an S085 review finding
              compiled in `build` and failed to compile in `build-tune`
              (`read of non-const variable 'ASPIRATION_MIN_DEPTH' is not
              allowed in a constant expression`), and the gate reported
              success. `build-tune` is the binary every tuning run plays --
              S085's SPSA drove it for 60000 games and S127 will drive it
              again -- so a break in it is discovered whenever someone next
              tries to tune, which can be months after the commit that caused
              it.
Decision:     Proposed by the agent, carried by S143, which the owner had
              already agreed into the plan with exactly this in its `accepts:`.
              The gate becomes
              `cmake --build build -j8 && ctest --test-dir build -L fast
              --output-on-failure && cmake --build build-tune -j8 && ctest
              --test-dir build-tune -L fast --output-on-failure &&
              ./clang-format.sh --check`.
              The red was observed before the change, not assumed: with the
              static assertion reintroduced the old gate exits 0 and the
              extended gate exits 2 at the `build-tune` compile.
              Measured cost on the DEC-109 MacBook, 8 cores, on mains: the tune
              build's `fast` label is 45.9 s over the same 22 tests the
              shipping build runs in 42.4 s, and building `build-tune` adds
              0.5 s no-op, 1.4 s for a full rebuild with its ccache warm and
              18.0 s with `CCACHE_DISABLE=1`. The gate roughly doubles: the
              extended gate ran green end to end in 93.6 s on a warm tree,
              against about 45 s before.
Rejected:     Building `build-tune` without running its suite -- it catches the
              compile break that prompted this and nothing else, and the two
              builds differ at runtime as well as at compile time, which is the
              divergence class a suite covers and a build does not. Adding a
              script that runs the gate -- moltke v1 has no hooks and nothing
              runs the gate for anyone (DEC-109), so a script would be a second
              place for the command to drift from the rule that is the gate.
              The PGO and portable release targets -- a separate question about
              release coverage, S143's `excludes:` puts them out of scope.
Consequences: A step completion pays about 94 s instead of about 45 s, and a
              fresh clone configures `build-tune/` once from DEV_MANUAL.md's
              Build section before the gate can run at all -- every `build*`
              directory is gitignored. The gate command now lives in two places,
              `AGENTS.md`'s TESTS rule and DEV_MANUAL.md's Test section, and
              they are changed together. A green gate is still necessary and
              never sufficient, unchanged from DEC-025.

## DEC-119  2026-09-01  The stale citations are re-anchored in their own step, before S144 rewrites the files that hold them
Tags:         workflow, documents, plan, tooling
Context:      S144 converts the bare `:line` continuations in the pending step
              files into full `path:line` citations. `baseline()` in
              `tools/plan_prose_check.py` reads a step file's DRIFT baseline as
              the commit that last wrote that file, so rewriting a file moves
              its baseline and every citation in it is then compared against a
              snapshot taken after the drift instead of before it. Measured at
              HEAD on 2026-09-01: `--citations` flags 97 -- 72 DRIFT, 25
              ANCHOR, 0 BOUNDS -- over 20 of the 53 pending step files, and 92
              of those 97 sit in the 18 files S144 has to edit. Running S144
              first would clear 92 live staleness reports without repairing
              one, and the checker would print green over a tree that had got
              no better. Among them is the class the tool was built for: seven
              pruning and reduction steps cite the mate-safety gate at
              `tests/test_search.cpp:1887` or `:1923` and it opens at 2808.
Decision:     The owner's, chosen from four options put by the agent. The
              repair is its own step, S169, and it lands before S144. Two
              effects, two numbers: 97 flags to 0 at S169, 383 loose references
              to zero or a stated remainder at S144. S169 is placed first in
              the Open list and carries `blocks: S144`.
Rejected:     Repairing inside S144 in one commit -- the same total work, but
              one diff does both and neither number is separately attributable,
              against the project's own rule that one change is measured at a
              time. Recording the citations baseline in the step file so an
              edit no longer relaxes the drift check -- it is the root cause
              and it is worth doing, but it repairs nothing on its own: the 97
              stay red afterwards and still need this step, so it is a
              candidate for its own step and not a substitute for one.
              Converting only and banking the 97 in `adocs/data/` as evidence
              -- cheapest, and it takes the class out of the checker's reach so
              that only someone reading that file would ever find it again.
Consequences: A step that rewrites a pending step file for any reason moves
              that file's DRIFT baseline, so the same laundering is available
              to any future document step and is not special to S144. Nothing
              in the tool stops it; what stops it here is that the exposure was
              measured before the edit rather than after. S169's own proof
              cannot be the checker's verdict for the same reason, so it ships
              a tracked mapping of baseline text against text at the new range
              instead.


## DEC-120  2026-09-01  A citation in a plan document repeats its path, and a bare `:line` is a flag
Tags:         workflow, docs, tooling, plan
Amended:      2026-09-04 by DEC-135 -- a citation from a pending step file into
              code names a symbol and carries no line number; the path is
              still repeated. S187 converts the existing citations.
Context:      S138 brought 201 full `path:line` citations inside
              `tools/plan_prose_check.py --citations`. It could not reach the
              other class: a bare `:line` continuation whose path is inherited
              from an earlier sentence. The checker counted those and reported
              them `loose, ungated` rather than resolving them, because
              guessing the inherited path produced sixty impossible line
              numbers when S138 tried it -- S095 wrote
              `(transposition_table.cpp:96-103), and :449 already computes`
              where `:449` means `src/search.cpp`, and S098 carried a page of
              continuations whose subject was named paragraphs earlier. So the
              class went stale in bulk with nothing saying so, which is not
              hypothetical: of the eight pending steps that pointed an
              implementer at the wrong mate-safety test, the three the
              2026-08-20 review missed were exactly the three that wrote it
              bare and the review grepped for the full path. Measured at S144:
              383 of them over 19 of the 53 pending files.
Decision:     The owner's, on the agent's proposal. Two halves. **The rule**: a
              citation repeats its path, stated in `plan.md`'s "How this file
              works" beside the ordering rules, so it is read before a step
              file is written. **The enforcement**: `--citations` fails a bare
              continuation as `BARE` instead of counting it, and never tries to
              resolve one -- the abstention S138 measured stays, what changes
              is that abstaining now costs a red run instead of a line of
              output. S144 converted the 383 that existed.
Rejected:     Teaching the checker to inherit the path from the nearest
              preceding sentence -- the measurement says it gets it wrong, and
              a check that prints garbage is a check that gets switched off.
              Leaving the count as a report and trusting the rule -- the rule
              was already implicit in S138's reasoning and 383 references were
              written the other way after it. An allowlist for the
              illustrations in S144's own file -- unnecessary, since a step
              file leaves the checked set when it moves to `plan_done/`.
Consequences: A wrong path is caught only when its line is past the end of
              that file (BOUNDS) or a quoted `TEST_CASE` title contradicts it
              (ANCHOR); an in-range wrong path is caught by neither, and DRIFT
              is blind on the commit that writes the citation because that
              commit becomes the baseline -- DEC-119 again. So the evidence for
              a conversion is the mapping it was made through and not the
              checker's verdict: `adocs/data/S144_pathings.tsv` carries, per
              citation, the text the cited range held at the commit that wrote
              the line and the text the new range holds at HEAD, and the two
              are equal on every one of the 322 the relocation decided. The 58
              the relocation could not separate -- a block that sits in both
              `quiescence()` and `negamax()`, or one that grew a comment under
              the citation -- carry their reason in the same file.

## DEC-121  2026-09-02  The Polyglot 781-constant table is the published format specification, kept and cited
Tags:         licensing, provenance, openings, polyglot
Context:      `src/openings.cpp` carries `const uint64_t polyglot_randoms[781]`,
              beginning `0x9D39247E33776D41`, the Zobrist constants that define
              the Polyglot book key. They are verbatim and necessarily so: a
              different table computes a different key and reads no published
              book at all. The 2026-08-22 audit raised them as a copied
              third-party table (finding F06) and noted S146's own `excludes`
              had put them out of scope, so the 5.2 MB book had a pending
              provenance step and the table beside it had none. COPYING says
              tables are copied never, not rarely, and DEC-084 as amended by
              DEC-105 says republication does not launder an engine's table --
              so the exception, if there is one, has to be stated rather than
              assumed.
Decision:     By the owner, on the agent's research. The table is
              **format-defining specification, not an engine's table**, and it
              stays, with the specification cited at the table itself. The
              evidence, checked rather than argued: all 781 constants appear,
              in the same order, in the format description at
              `https://hgm.nubati.net/book_format.html` -- the document
              `src/openings.cpp:15-18` already cites -- and that document's own
              note on copyright says the algorithm "may be freely implemented
              by all GUIs, adapters and engines, including closed source ones",
              that "Polyglot itself is GPL but the GPL only covers actual code
              and not algorithms", and that "a table of random numbers cannot
              be covered by copyright". Verified 2026-09-02 by extracting the
              781 sixteen-digit constants from the live page (sha256
              `bd95784721dbc0bfd4d734e87281b87f91e917886dc3b347f38f6e6bda5eb31b`)
              and comparing them element by element against the array: equal at
              every index.
Rejected:     Deleting the Polyglot key path with the table -- it removes the
              ability to read any published book in the format the rest of the
              world uses, for a table whose own publisher disclaims copyright,
              and it would take `get_key` and its nine pinned test keys
              (`tests/test_openings.cpp`) with it. Re-deriving a private random
              table -- the keys would be self-consistent and would match no
              book anywhere, which is the whole point of a shared format.
              Leaving the table uncited and relying on this entry alone -- the
              next reader meets the constants in the source, not in
              `decisions.md`, and F06 is exactly the finding that raises them
              again.
Consequences: The exception is narrow and stated so it cannot be stretched: it
              covers constants that **define an interchange format**, published
              in that format's own specification, where a different value
              produces a non-interoperable result. It does not reach a tuned
              table -- piece-square values, king-safety weights, reduction
              tables -- whatever document republishes it. `src/openings.cpp`
              carries the citation and the copyright note at the array, so the
              ruling is where the constants are. The 5.2 MB book blob is a
              separate artifact and a separate ruling; this entry says nothing
              about it.


## DEC-122  2026-09-02  A reported mate line is completed all or nothing, and never partially
Tags:         search, uci, reporting, mate, transposition-table
Context:      `state->pv_table` holds one move per main-search ply, so a mate
              found inside quiescence was reported with a line that stopped at
              the iteration depth -- the score right, the line short. S147
              completes the line by walking the transposition table from the
              end of the stored one. That walk can fail: an entry is evicted, a
              bound node carries no move, or the moves it reads lead somewhere
              other than the mate. What to do on a failed walk is the choice,
              and it is not obvious -- a partial extension is longer than what
              the search produced and looks like progress.
Decision:     By the owner, on the agent's proposal. The walk builds its moves
              **aside**, and the reported line is extended **only** when the
              walk reaches checkmate at exactly the distance the score claims.
              Anything else leaves the line exactly as the search produced it.
              The rule is stated at `extend_mate_pv()` in `src/search.cpp`, in
              `adocs/specs.md`'s Behaviour section, and here.
Rejected:     Appending whatever the walk found -- it publishes a line the
              engine cannot stand behind, and it converts a visible truncation
              into an invisible wrong answer, which is worse: a short line is
              caught by `-check-mate-pvs` and by `test_mate_pv`, and a
              plausible wrong one is caught by nothing. Extending to the
              claimed length without requiring checkmate at the end -- the
              length is the symptom and the mate is the property. Falling back
              to a search when the walk stalls -- a mate solver inside a
              time-controlled search, priced and rejected in S147's step file.
Consequences: The completion can never make the reported line worse than the
              search made it, which is what allows it to run unconditionally on
              every mate report without a measurement behind each case. It also
              means the guarantee is one-sided: the line either reaches the
              mate or is the short line, and a residue of short lines is
              expected rather than a defect in the walk. DEC-123 is the size of
              that residue and what is done about it.


## DEC-123  2026-09-02  S147's guarantee is over a line the search produced; a table-inherited mate score is a separate defect
Tags:         plan, search, uci, reporting, mate, measurement
Context:      S147's `accepts` asked for **no** `Incomplete mating PV` line from
              a `fastchess.sh --fast` run. The fix landed and the run was taken:
              3000 games, 1 h 55 m 30 s, 0 forfeits, **138** such lines from the
              unfixed reference `8736aec` and **10** from the fixed build. The
              ten are not the defect S147 was written against. Each has a line
              exactly as long as its iteration is deep, and the cause was
              reproduced rather than argued: replaying a game move by move
              through one engine process gives `mate -8` at depth 3 with 3
              plies where 16 are needed, while the same position at the same
              time with a **cold** table reports no mate at any depth and
              `cp -725` at depth 15. The score is read back from the
              transposition table, proved by an earlier search of the same
              game, and the line that proved it has been overwritten since -- a
              16 MB table is about a million entries and one 150 ms search
              visits more nodes than that.
Decision:     By the owner, on the agent's recommendation. **S147's guarantee is
              over a mate line the search itself produced**, held at zero by
              `test_mate_pv` over 706 mate lines of every iteration across both
              S145 sets, and S147 closes on it. The table-inherited case is a
              named residual with a number -- 10 in 3000 games against the
              reference's 138 -- and becomes **S170**.
Rejected:     Carrying the mating line across searches so a later search can
              reuse it -- the option that reaches zero, kept as S170's leading
              candidate rather than folded into a step whose `excludes` did not
              scope new engine state. Reporting a non-mate score when the line
              cannot be shown -- it discards a score that is right to fix a
              line that is not. A bounded mate search in the reporting path --
              it closes the two deep cases for a move list and a mate test and
              the other eight only with a mate-in-8 solver inside a
              time-controlled search.
Consequences: `-check-mate-pvs` stops being a warning nobody reads and becomes a
              count that moves: 10 in 3000 games is the standing figure, and a
              run that reports materially more has found something. The bound is
              stated in `adocs/specs.md` and in `DEV_MANUAL.md` beside the
              instrument, so neither document claims a silence the engine does
              not deliver. `test_mate_pv` stays at zero tolerance because every
              case in it is `ucinewgame` and one search, which is a cold table
              by construction -- a test for S170 has to replay a game.


## DEC-124  2026-09-02  A mate line the search cannot rebuild is carried, completed at two plies, and matched to the score it is printed with
Tags:         search, uci, reporting, mate, transposition-table, measurement
Context:      DEC-123 named the residual S147 could not close -- 10
              `Incomplete mating PV` lines in 3000 games -- and made it S170,
              whose step file called the cause "a score read back from the
              table" and offered carrying the line as the leading candidate.
              Reproducing all four games move by move through one process at
              fixed node budgets (`adocs/data/S170_replay.py`, cases in
              `adocs/data/S170_cases.tsv`) found **three** causes and not one,
              and the third is not a table effect at all.
              (1) A score inherited across searches. The one entry carrying it
              survives; the line's worth of entries below it does not. Case C
              reports `mate 7` at depth 11 warm and no mate at any depth cold,
              `cp 885` at depth 13.
              (2) A proof this search made and then overwrote. Case D reports
              `mate -6` at depth 10 with 10 plies of 12 on a **cold** table, so
              nothing was inherited: the mid-line entry was evicted between the
              iteration that wrote it and the walk that wanted it.
              (3) A score and a line from different iterations. An aborted
              iteration supplies the line that will be played
              (`src/chesso.cpp`, the `has_result` block) while the score stays
              the last completed iteration's, so the printed pair can claim a
              mate the printed line does not reach. Measured directly: with the
              other two fixes in and this one out, exactly the three
              duplicated-depth lines of cases A, B and C stay short, and case D
              does not.
Decision:     By the agent inside the step, on the measurements above and under
              DEC-122, which every part of this obeys unchanged.
              **The line is carried.** `proven_mate_line_t` keeps the last line
              the engine was shown to deliver together with the position each
              of its moves is played from, and a stalled walk asks it for the
              move at this position *and at this remaining distance* -- the key
              says where, the distance says the stored proof is of the mate
              being claimed. The store lives in the UCI layer because it has to
              outlive a `go`; `search_state_t` carries a pointer to it that is
              null unless a caller supplies one, so every test that builds a
              state of its own keeps exactly the behaviour it had.
              **Two plies from the mate the defender's move is looked for**,
              and only when *every* legal reply is mated in one. S147 priced
              this and rejected it for want of evidence; case D is the evidence,
              and the forcedness requirement is what keeps the published line a
              principal variation rather than a defender's blunder that happens
              to end in mate.
              **A mate line is completed against the score it is printed
              beside**, not only against the score its own iteration returned.
Rejected:     Printing the aborted iteration's own score -- an unfinished
              iteration's score is a bound and means nothing, which is the rule
              `iterative_deepening_search()` already states. Printing the
              completed iteration's line instead of the aborted one's -- the
              line would then not start with the move that will be played, the
              defect S021's root fail-high handling exists to prevent.
              Withholding the aborted iteration's line -- it is the only thing
              that tells a GUI what the engine is about to play. Taking the
              first defence that ends in mate at two plies rather than
              requiring every reply to be mated -- it would publish a line at a
              distance the position is not at whenever the score is wrong,
              which is precisely what DEC-122 exists to stop.
              Holding transposition table entries longer so the walk finds
              them -- it alters play and owes an SPRT of its own; S170's
              `excludes` puts it out of reach.
Consequences: The `pv` guarantee in `MANUAL.md` and `adocs/specs.md` now holds
              for a mate score the search did not itself prove, and
              `DEV_MANUAL.md`'s instrument 3 loses the "read a count against 10
              in 3000" bound DEC-123 gave it. `-check-mate-pvs` is a zero-
              tolerance check again, so the next `Incomplete mating PV` line in
              any run is a new defect. The engine now carries reporting state
              across searches for the first time -- one line, cleared by
              `ucinewgame` -- and the rule that keeps it honest is that nothing
              reads it to decide, order or prune a move. `tests/test_mate_carry.cpp`
              is the guard and it is the first test here that replays whole
              games through one process; it asserts a mate line was seen before
              asserting none is short, because what a table holds at a given
              ply is fragile and a vacuous pass would look like a green one.


## DEC-125  2026-09-02  S170's guarantee is over a mate distance the position holds; a distance the table contradicts is a separate defect
Tags:         plan, search, uci, reporting, mate, transposition-table, measurement
Corrected:    2026-09-03 by DEC-127, in one claim and not as a whole. "There is
              no 18-ply line to publish" is false: such a line exists, and the
              engine prints it itself at a larger Hash. Everything else here
              stands -- the run, the counts, and the creation of S171.
Context:      S170's `accepts` asked for **no** `Incomplete mating PV` line from
              a `fastchess.sh --fast` run. The three fixes landed and the run
              was taken: 3000 games against S147's build `b3f82eb`, 1 h 56 m
              48 s, 0 forfeits, `LLR -0.73` and no bound. **12 such lines over
              3 distinct searches from the reference and 5 over 1 from the
              candidate.** The five are one search, at depths 9 to 13 with
              lines of 9 to 13 plies where 18 are needed, and the score is what
              is wrong there rather than the line. Position
              `8/4ppk1/2p2np1/p7/NPP1p3/P6q/3b4/1Q3R1K w - - 0 34`, 50 plies
              into the game: reported `mate -9` in the game, and asked cold the
              engine itself reports no mate until depth 18, then `mate -7` with
              a complete 14-ply line held to depth 24 over 7.3 billion nodes.
              `stockfish` says `#-7` at depth 30 and again at depth 36. So the
              distance reported is not the distance the position holds, there
              is no 18-ply line to publish, and the all-or-nothing rule
              (DEC-122) refused -- correctly.
Decision:     By the owner, on the agent's recommendation. **S170's guarantee is
              over a mate score whose distance is the position's own value**:
              such a score is reported with a line that reaches it, whether the
              search proved it, inherited it from an earlier search, or lost
              the proof to eviction. Held at zero by `tests/test_mate_carry.cpp`
              over the four games S147's run produced, and measured at 12 lines
              over 3 searches down to 5 over 1 in 3000 games. The mate distance
              the table hands back and a deeper search of the same position
              contradicts is a separate defect with its own number and becomes
              **S171**, which the BUGS rule puts first in the Open list.
Rejected:     Widening S170 to chase the distance -- its `excludes` forbids
              changing any score, the reporting work is done and green, and a
              search-correctness question owes an SPRT the reporting work does
              not. A second `--fast` run before ruling -- the case reproduces
              deterministically in about twelve seconds without a match, so two
              more hours would confirm the rate and not the defect, and the
              machine is the binding constraint on the plan. Recording the
              wrong distance as a known limitation and creating no step -- the
              BUGS rule is written against exactly that.
Consequences: `-check-mate-pvs` is not yet the zero-tolerance check DEC-124
              predicted: the standing figure is **5 lines from 1 search in 3000
              games**, against S147's 10 and the unfixed engine's 138, and it
              is stated in `adocs/specs.md` and beside instrument 3 in
              `DEV_MANUAL.md` so neither document claims a silence the engine
              does not deliver. S171 owns the number and the run that removes
              it. `adocs/data/S170_cases.tsv` carries the case as a
              reproduction with `guard: no`, because a test that asserted a
              line for it would be asserting that a wrong score gets one.


## DEC-126  2026-09-02  S146 is parked: the header book's origin is not on this machine
Tags:         plan, licensing, openings, provenance, parked
Context:      S146 asked where `src/openings.book` -- 5.2 MB of Polyglot book
              compiled into every shipped binary -- came from, and DEC-016's
              first foundation is that nothing here is copied and nothing is
              bundled whose licence is unstated. The agent's search found
              nothing: the repository trail ends at `628d827` (2025-04-16) with
              no URL, no attribution and no licence, and the decoded sha256,
              a GitHub code search and three public book collections all
              returned no match. The owner then named
              `books/8moves_v3.pgn` from `official-stockfish/books`, and that
              was refuted by measurement rather than by argument: the PGN holds
              129613 distinct positions over 34700 sixteen-ply games, the book
              holds 154916, and **11703 are shared -- 7.6 % of the book**. A
              converter can drop positions; it cannot invent the 143213 the
              PGN never reaches. Asked again, the owner does not remember, has
              checked the history himself, and thinks the file may be on his
              other computer.
Decision:     By the owner. **S146 is parked**, its file moved back to
              `adocs/plan_todo/` and its Open entry tagged `parked, DEC-126`
              beside S029's. It resumes when the other machine can be searched,
              or when the owner takes one of the three standing options instead
              -- build a replacement from a source this project can name,
              delete the book and the `Use Book` option, or keep the blob with
              its provenance recorded as unknown.
Rejected:     Recording `8moves_v3.pgn` as the origin because the owner named
              it -- the numbers say it is not, and a false provenance in
              `MANUAL.md` is worse than an admitted gap, which is the one thing
              this step exists to prevent. Deleting the book now to close the
              exposure -- that is the owner's call and he has not taken it;
              `Use Book` defaults false, so nothing is measured through the
              blob and the exposure is licensing, not strength. Leaving the
              step in `plan_current/` as blocked -- it would hold a slot under
              the PLAN rule against an unblocking event that has no date.
Consequences: The blob ships and its origin stays unknown, stated here rather
              than nowhere. What the episode did settle is recorded and
              committed: the `polyglot_randoms[781]` table is DEC-121, and
              `books/8moves_v3.pgn` -- committed, played by `rating.sh` and
              previously unattributed -- is now pinned in `books/fetch_book.sh`
              by both digests, verified byte-identical to the CC0-1.0 upstream
              (zip `7e1e9dd1...`, file `5835239f...`). Whoever resumes S146
              searches on the decoded book's own fingerprint: sha256
              `47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5`,
              2610256 bytes, 163141 entries, and a start position offering only
              `d2d4`, `g1f3` and `e2e4` at weight 54 each.
Discharged:   **2026-09-03 by DEC-131**, not reversed -- the owner took the
              second of the three standing options above and the blob was
              deleted, so "the blob ships and its origin stays unknown" is
              history and not current state. The parking held for one day; what
              unparked it was S172 building `tools/make_book`, which is what
              "build a replacement" had always needed and never had. The
              fingerprint above is now the fingerprint of a file that is in no
              working tree, only in `62d07d4`.


## DEC-127  2026-09-03  `mate -9` is a sound distance, not a wrong one; S171 becomes a reporting step
Tags:         search, uci, reporting, mate, transposition-table, measurement, plan
Context:      S171 was created by DEC-125 as a wrong-score defect: one search in
              S170's 3000-game run reported `mate -9` for
              `8/4ppk1/2p2np1/p7/NPP1p3/P6q/3b4/1Q3R1K w - - 0 34` where the
              same engine on a cold table says `mate -7` at depth 18 and
              `stockfish` says `#-7` at depth 30 and 36, and DEC-125 concluded
              that no line of the claimed length existed. **That conclusion was
              never measured, and it is wrong.** The line exists: 18 plies from
              that root, legal throughout, ending in checkmate, verified move by
              move with python-chess. The engine publishes it itself -- the same
              warm replay at `Hash=256` reports `mate -9` at depths 9 to 12 with
              a complete 18-ply `pv` and warns about nothing. What differs at
              `Hash=16` is only which entries survive.
              `tools/mate_trace.cpp`, written for this, replays the game through
              the real UCI layer and then walks the reported line printing the
              table entry behind every position. It shows the whole chain: exact
              entries of this search at plies 0 to 9, the score inherited at ply
              11 from a generation-11 depth-7 entry claiming black mates in 4 --
              which `stockfish` confirms at depth 30 -- and the completion walk
              stalling at ply 13 on one missing slot, five plies short of the
              mate, with the entry that certifies the continuation sitting in
              that position's own children.
              So `mate -9` is **sound but not optimal**: a mate at that distance
              is deliverable, and the position's value is `mate -7`. A
              depth-limited search naming a longer mate than the game value is
              the search, not the report, and S148 and S154 own that ground.
Decision:     By the owner, on the agent's recommendation. **S171 is rescoped
              from the score to the line**, and its `excludes` is amended to
              permit exactly the reporting change it forbade: when
              `complete_mate_pv()`'s walk stalls with nothing to read, it looks
              one ply down and takes the move whose child the table certifies
              **exact** at the distance the line still owes. Reporting only, and
              all-or-nothing per DEC-122 unchanged. `certified_mate_move()` in
              `src/search.cpp` is that, and `adocs/data/S170_cases.tsv`'s
              `E_mate_minus9` row becomes a guarded case rather than a bare
              reproduction.
Rejected:     Closing S171 as "not a defect" -- the short line is a real
              reporting gap that fires in every match log, and leaving a warning
              that fires on correct behaviour makes the instrument unreadable,
              which is what DEV_MANUAL.md's "read a count and not a silent log"
              already concedes. Keeping it as a score step -- that is mate
              finding at shallow depth with a warm table, unbounded in cost, and
              it owes an SPRT that a reporting fix does not. Accepting a bound
              entry rather than an exact one in the lookahead -- a lower bound
              of "mate in n" leaves a faster mate open, so a line built on one
              can be a road the position would not take; the all-or-nothing gate
              would catch a walk that fails to reach the mate but not one that
              reaches it by a defender's blunder.
Consequences: The BUGS-rule urgency that put S171 first in the Open list does
              not apply, because the score was never the defect; the step keeps
              its place because it is started and cheap, not because a bug is
              open. `MANUAL.md`'s known-bug entry, `adocs/specs.md` and
              `DEV_MANUAL.md`'s instrument 3 all state DEC-125's refuted claim
              and are corrected in the same commit. The standing
              `Incomplete mating PV` figure stays **5 lines from 1 search in
              3000 games** until a `fastchess.sh --fast` run replaces it: the
              machine was on battery when the fix landed and the POWER rule
              forbids a timed match there, so the run is owed and named in the
              step file rather than assumed.


## DEC-128  2026-09-03  S171 is postponed to the desktop workstation: its census run needs a machine this one cannot be
Tags:         plan, measurement, machine, sprt, postponed
Context:      S171's fix is in and green at `136b03f`: `certified_mate_move()`
              completes a mate line across a table slot the walk has lost,
              `tests/test_mate_carry.cpp` guards it red-then-green, INV-6 is
              discharged on identical node counts and best moves, and the gate
              passes in both builds. What the step still owes is one thing --
              a `fastchess.sh --fast` census, 3000 games at 8+0.08, about two
              hours, reporting **0** `Incomplete mating PV` lines from the
              candidate. Two attempts on this MacBook failed for machine
              reasons and not for code reasons. In the morning `pmset -g ac`
              reported `No adapter attached` and the POWER rule forbids a timed
              match on battery (DEC-109, and S024's drained run is why). With
              the adapter attached the same afternoon, `fastchess.sh`'s own
              load guard printed `about 387% of a core is already busy`:
              Spotlight was indexing PDFs through ten `CGPDFService` workers
              beside `mds_stores`, roughly half of the eight cores. The run was
              killed about a minute in, before a single game finished -- its
              `games.pgn` is zero bytes -- because at 8+0.08 a match sharing
              half the machine risks time forfeits, and the figure it would be
              read against was taken on an idle machine.
Decision:     By the owner. **S171 is postponed, not blocked and not
              abandoned.** Its file moves back to `adocs/plan_todo/` with the
              run written into it, and its Open entry leaves position 1 for the
              parked cluster tagged `postponed, DEC-128`, so the next step
              derives as S020 rather than as a step nothing can advance. It
              resumes on the owner's desktop workstation, where the census is
              the first thing taken. The run needs no re-derivation:
              `REF=457e355 nohup ./fastchess.sh --fast > .tuning/sprt_s171_matepv.log 2>&1 &`
              -- 3000 games, `Hash=16`, `UHO_Lichess_4852_v1.epd`, reference
              `457e355` being the commit before the fix -- accepted at 0 lines
              from the candidate against whatever the reference prints in the
              same run.
              **No branch was made, because there is no work to put on one.**
              The whole of S171 is committed at `136b03f` on `achesso` and the
              tree is clean; the aborted run produced two empty files under
              `/tmp` and a 29-line log in gitignored `.tuning/`, none of which
              is evidence of anything. This is the opposite case to DEC-111,
              where a branch was the right home for gitignored evidence that
              would otherwise die with a machine.
Rejected:     Taking the run on the loaded machine -- the script prints that
              guard precisely so this decision is not taken by accident, and a
              two-hour census whose forfeit rate is unknown is worth less than
              a delayed one. Turning Spotlight indexing off to clear the
              machine -- a system-wide change to the owner's machine for one
              measurement, and his call rather than an agent's. Leaving the
              step in `plan_current/` as blocked -- it would hold a coordinator
              slot under the PLAN rule against an unblocking event with no
              date, which is what DEC-126 declined for S146 five days earlier.
              Writing the `done:` stamp with the run outstanding -- the accepts
              names that run, and a stamp that skipped it would be false in the
              one document whose whole value is that it is not.
Consequences: The standing rate stays **5 `Incomplete mating PV` lines from 1
              search in 3000 games** in `adocs/specs.md` and beside instrument
              3 in `DEV_MANUAL.md`, and neither document claims a silence that
              has not been measured. No bug is open against this: DEC-127
              already established that the score was never the defect, so the
              BUGS rule does not force the step ahead of anything. The census
              carries across machines without qualification, unlike a timing --
              both engines play in the same run on the same machine, so the
              reference's count is measured beside the candidate's rather than
              read from a figure taken elsewhere, and DEC-049 is untouched. The
              standing 5 is this MacBook's and stays attributed to it. The next
              step in the Open list is S020.


## DEC-129  2026-09-03  The opening book's UCI surface is the one Stockfish had, `Use Book` is removed, and a book that will not load leaves the engine bookless
Tags:         uci, surface, openings, book, options, s172
Context:      The owner asked for an opening book loadable over UCI "following
              the same semantics of stockfish". Three things had to be settled
              before any code was written. **What Stockfish's semantics are**:
              modern Stockfish has no book at all -- removed in 2016 -- so the
              reference is the surface it carried while it had one, which is
              also the one the UCI specification itself describes. **What
              happens to `Use Book`**, the name chesso has advertised since the
              `bitboard` branch, which is not a UCI option name and which no
              GUI looks for. **What a bad path does**, which had no answer
              because no path could be given: `load_book_from_file()` had sat
              in `src/openings.cpp` since the `bitboard` branch with no caller
              anywhere in the tree and had therefore never run once.
Decision:     By the owner, 2026-09-03, from three options put to him.

              **1. The surface is `OwnBook` (check, default false), `Book File`
              (string, default `<embedded>`) and `Best Book Move` (check,
              default false).** `<embedded>` and an empty value both mean the
              book compiled into the binary; anything else is a path to a
              Polyglot `.bin`, loaded the moment the option is set.

              **2. `Use Book` is removed, not aliased.** No script, config or
              harness in this repository sets it -- `grep` over `*.sh`, `*.py`
              and `*.json` finds it only in prose -- so nothing breaks that an
              alias would have saved, and two names for one setting is the kind
              of thing that is still there in five years.

              **3. A book that will not load leaves the engine with no book**,
              reported on the UCI channel as `info string book [<path>] not
              loaded: <why>. Playing without a book`, in both builds. There is
              no fallback to the built-in book. Loading refuses a file that
              does not open, whose size is not a whole number of sixteen-byte
              entries, or whose keys are not sorted.

              **4. Selection is weight-proportional, and `Best Book Move` takes
              the heaviest entry.** This changes play: the engine drew
              uniformly among the position's entries and never read the
              `weight` field at all, so a line the book gave one game of weight
              was played as often as one it gave two hundred. That is neither
              the Polyglot format's semantics nor Stockfish's.
Why:          The option names are the protocol's, so a GUI's book checkbox
              reaches the engine; the refusal is loud because the value is a
              path a person typed; and the weight is what the format's weight
              is for.
Rejected:     Keeping `Use Book` as a hidden alias -- offered and declined; see
              the grep above for why it costs nothing. Adding only `OwnBook`
              and `Book File` and leaving selection uniform -- it would ship a
              book reader that ignores the one field the format uses to say
              which move it prefers. Falling back to the built-in book when a
              named one fails -- a harness that asked for one book and silently
              got another is measuring a configuration nobody chose, which is
              the exact class of contamination DEC-020 cost this project a
              night to learn. Logging the failure only -- `LOG_E` compiles to
              `if (false) std::clog` under `NDEBUG` and the shipping binary is
              a Release build, so a mistyped path would be answered by silence:
              the same defect S137 removed one option along, and DEC-093 is its
              ruling.
Consequences: **No SPRT is owed and that is not a shortcut.** All of it is off
              by default, and S158 established that no measurement this project
              has ever taken played a book move, so the selection change alters
              no verdict on record. DEC-085 is untouched: a rated CCRL run
              requires own books disabled and `OwnBook` still defaults false.
              The `setoption` value parser had to change with it -- it read one
              token, so `Book File` was the first option here whose value could
              contain a space and a path with one arrived truncated. That was a
              defect before this step and it is now covered by a test observed
              red against the old parser. `test_uci_surface` moves from three
              golden option lines to five, after `MANUAL.md` and `specs.md`, per
              the SURFACE rule. S146 is untouched: this changes which book can
              be loaded, never where the built-in one came from.


## DEC-130  2026-09-03  The built-in book is embedded as a raw binary through `.incbin`, not as a hex string or a generated array
Tags:         build, openings, book, embedding, portability, s172
Context:      `src/openings.book` was a 5220541-byte C header holding one
              `#define BOOK "<hex>"`, decoded by `hex_string_to_vector()` into
              a 2610256-byte heap buffer at every process start. So the
              repository tracked twice the bytes it needed, and every one of
              the thousands of engine processes a match night starts parsed
              5.2 MB of ASCII to rebuild a constant. The owner asked whether
              modern C++ offers something better, stating a preference for the
              less complex and more performant option over the elegant one.
Decision:     By the owner, 2026-09-03. The book is tracked as the raw Polyglot
              file `src/openings.bin` and pulled into the binary by `.incbin`
              from `src/openings_embedded.S`, a preprocessed assembler source
              carrying one `#if defined(__APPLE__)` arm for Mach-O's symbol
              underscore and section name. `enable_language(ASM)` and one
              `set_source_files_properties` in `src/CMakeLists.txt` is the
              whole build change. The engine probes the bytes where the linker
              put them: nothing is decoded and nothing is copied.
Why:          Half the tracked bytes, no startup work at all, and four lines of
              assembler against a 16 MB generated source file.
Rejected:     A generated `constexpr uint8_t[2610256]` header -- portable to
              MSVC and needing no assembler, at the price of a 16 MB source
              file that has to be compiled on every clean build. Keeping the
              hex string and only adding the generator tool -- the smallest
              diff, and it keeps both costs it exists to remove. C++23's
              `#embed` -- this tree is C++20 and Apple clang does not have it;
              it is what would replace this if the standard moves.
Consequences: **Measured, not asserted: startup is 4.7 ms +/- 0.4 before and
              2.8 ms +/- 0.1 after**, `hyperfine -N --warmup 50 -m 500` over
              `printf 'uci\nquit\n' | chesso` against Release builds of
              `32982a2` and the candidate, 574 and 976 runs, x1.67 +/- 0.15.
              It is not a strength claim -- a match's time control does not
              charge for process start -- and it is the whole reason the
              embedding changed. **The contents did not change**: 2610256
              bytes, 163141 entries, sha256 `47a817350459843da2a20e1d5cba2846
              2d9df30bdb99c93097bd3cb66ce78fb5`, exactly the figures S158
              recomputed from the hex header, so the book's unrecorded origin
              is still S146's question and this step did not touch it. **The
              MSVC arm is gone and was never used**: `MASM` has no `.incbin`,
              so a Windows build would need the generated-array form; the tree
              has an `if(MSVC)` in `CMakeLists.txt` and has never been built
              with it. **The ELF arm of the `.S` was written blind** -- this
              machine is the macOS one -- and is what a first Linux build will
              exercise. `.balign 8` in that file is load-bearing: the probe
              reads a Polyglot key with an eight-byte load, and an `.incbin`
              lands wherever the previous section contents left off unless it
              is told otherwise.


## DEC-131  2026-09-03  The unaccounted book is deleted and the shipped one is built here from the CC0 PGN
Tags:         licensing, openings, provenance, plan, tooling
Context:      DEC-126 parked S146 with three standing options and no date: search
              the owner's other machine, build a replacement from a source this
              project can name, or keep the blob with its provenance recorded as
              unknown. Two things changed. The owner has since asked directly
              for the second option. And S172 built `tools/make_book`, which is
              what made it available at all -- until 2026-09-03 nothing in this
              repository could produce a Polyglot book, so "replace it" named a
              tool that did not exist. DEC-126's refutation still stands and is
              why this is not a re-attribution: `books/8moves_v3.pgn` shares only
              11703 of the old book's 154916 positions, 7.6 %, so the new file
              is a different book and is not being passed off as the old one's
              origin.
Decision:     By the owner, 2026-09-03. **`src/openings.bin` is deleted and
              rebuilt from `books/8moves_v3.pgn` with this project's own tool**,
              at its defaults:

                  ./build/tools/make_book build books/8moves_v3.pgn \
                      --out src/openings.bin

              2755712 bytes, 172232 entries over 129613 positions, sha256
              `3b89a4ad9146e266ae9296778067aaedcb7f57f3cf0ff2086b9ae6df15b873dd`.
              The input is CC0-1.0 and pinned in `books/fetch_book.sh` by both
              digests; the SAN is read by the engine's own `algebraic_to_move`
              and keyed by its own `get_key`, so no other engine's code, table
              or output is in the path. The old 163141-entry file is gone from
              the tree, recoverable from `62d07d4` and from nowhere else. S146
              is unparked and closed.
Rejected:     Keeping the blob with its origin recorded as unknown -- the option
              DEC-126 left open and the owner has now declined; DEC-016's first
              foundation is that nothing ships here whose licence is unstated,
              and an admitted gap is still a gap. Waiting for the other machine
              -- an unblocking event with no date, and the replacement no longer
              costs anything now that the tool exists. Deleting the book and the
              option outright -- DEC-129 had just built the surface and a GUI
              expects `OwnBook` to do something. Pruning with `--min-games` or a
              shallower `--max-ply` -- both are choices that want a reason, and
              there is none: 16 plies is the full depth of every line in that
              PGN and `--min-games 1` drops nothing, so the defaults reproduce
              the PGN faithfully and the digest above is what a reader checks.
Consequences: **The book is reproducible from two committed inputs**, which is
              the property the old one could never have: `make_book` sorts its
              output by key and then by weight, so two runs are byte-identical
              (verified) and anyone can regenerate the shipped file. **The book
              is different and this changes play when it is switched on**:
              172232 entries against 163141, lines 16 plies deep, and the start
              position now offers e2e4 at weight 12956 and d2d4 at 12493 where
              the old book offered `d2d4`, `g1f3` and `e2e4` at 54 each. **No
              SPRT is owed** and this is the same discharge S172 took: `OwnBook`
              defaults false, and S158 established that no measurement on record
              has ever played a book move, so no verdict in this repository is
              affected. INV-6 holds the default configuration to identical node
              counts and best moves. **Weight now means something it never did**
              -- every game in the PGN is recorded `1/2-1/2`, so an entry's
              weight is the count of the 34700 lines that played the move, which
              is what DEC-129's weighted draw selects on. **The engine leaves
              the book by move 9**, which is shallower than the old blob went
              and is a property to remember before reading anything into an
              opening. `polyglot_randoms[781]` is untouched and stays DEC-121.

## DEC-132  2026-09-04  The sliding-attack magic numbers stay for now, and a step regenerates them under a project seed
Tags:         provenance, licensing, movegen, plan
Context:      The 2026-09-03 audit's clean list noted that the 128 magic
              constants in `src/bb_tables.hpp` were produced in this repository
              by commit `a5dbe68` ("Generatd magic numbers", 2023-03-28), whose
              generator used the xorshift seed 1804289383 from the "Bitboard
              chess engine in C" series the `bitboard` branch followed -- so the
              numbers coincide, value for value, with that series' published
              set. The code was the owner's and the origin is in git; COPYING is
              about tables taken from another engine, and these were generated
              here. Still, a reader meeting the constants finds no note, a
              value-for-value match with a published table is exactly the shape
              COPYING exists to question, and the generator that produced them
              did not survive into this branch.
Decision:     By the owner, 2026-09-04. The constants stay as they are for now
              -- inherited foundation under DEC-013, the project's own generator
              output, origin in git -- and a plan step, S179, recovers or
              rewrites the generator, commits it under `tools/`, and regenerates
              both arrays under a seed this project chooses, so the coincidence
              ends and the provenance becomes a command rather than an argument.
              Until S179 lands, `src/bb_tables.hpp` carries a comment at the
              arrays naming this entry.
Rejected:     Recording provenance alone and never regenerating -- cheapest, and
              it leaves a published-set coincidence in the tree that every
              future audit re-raises. Regenerating inside the audit batch -- it
              is not a bug, BUGS does not apply, and it is behaviour-neutral
              work for an idle moment, proved on perft and node counts rather
              than on a match. Doing nothing -- the question was asked, and
              DEC-126 is the record of what an unanswered provenance question
              costs later.
Consequences: S179 sits second in the Open list, after S178, machine-light: a
              generator in `tools/`, both arrays replaced, `bench_movegen`'s
              perft verification, `test_perft` and `tools/search_bench.py`'s
              node counts as the proof that move generation is unchanged --
              magics are perfect hashes into tables whose size the relevant-bit
              counts fix, so only the constants move. The relevant-bit counts,
              shifts and table layout are not the series' and do not change.

## DEC-133  2026-09-04  Third review of the plan: DEC-087 corrected on three rulings, S099 to the reserve, check extensions reopened, S133 kept
Tags:         planning, search, evaluation, measurement, provenance
Context:      The 2026-09-04 plan review (`adocs/audit/2026-09-04_plan_review.md`,
              no high, five medium, five low) and its independent literature
              pass (`adocs/data/2026-09-04_plan_review_literature_check.md`)
              fetched every figure DEC-087 argued from. Three rulings rest on
              figures the sources narrow. (a) Ethereal commit 3f4ef537
              (2018-06-25) removed only the check extension applied *before*
              the move loop, +4.14/+4.54 at bounds [-3, 1]; Ethereal's master
              still extends a checking move inside the loop, and the wiki's
              Ethereal page lists the technique. Stormphrax #67 removed check
              extensions with no Elo in the message, in a network engine far
              above the band. Weiss 1.2 at 3055 and Stash, the plan's own
              existence proofs, carry the in-loop form. (b) Lynx pull request
              #1662, the "+11.4 at ~2850" that kept S099 in the main order,
              merged 2025-04-15 between Lynx v1.9.0 and v1.10.0, which the
              CCRL Blitz list rates 3226 and 3293 -- above the "~3100" that
              demoted S110 and S111. All three correction tables' evidence is
              above 3100; the criterion separates nothing. S110's twelve
              figures carry no source at all. (d) Berserk 4.3.0's "~+65" is the
              author's estimate for the whole release, which bundles the
              king-side tables with space, imbalance tables, a wider king area,
              history pruning, phased move generation and TT buckets; Leorik's
              "~+88" is the 2.4 to 2.5 CCRL delta of a release that also
              shipped PEXT, threads and .NET 8, and its piece-square form is
              linear in both king squares rather than bucketed. No isolated
              figure for king-relative tables exists in anything fetched.
Decision:     By the owner, 2026-09-04, on the agent's report, item by item.
              (a) S096 stays retired by id -- ids are never reused -- and the
              in-loop check extension is a new step, S188, placed directly after
              S097 whose extension plumbing it uses, one SPRT, whatever it
              returns. (b) S099 moves to the head of the reserve tail as the
              probe for the correction-history family, run on a spare night
              when the machine is idle; S110 and S111 stay in the reserve and
              are gated on its verdict; S181 re-bands every Lynx figure by
              merge date and redraws S098's "sub-3000 evidence" grouping. (d)
              S133 stays where it is, its two figures restated as
              release-bundle deltas with no isolated number, kept on adoption
              breadth (Berserk, Leorik, Lynx) and on the owner's approval of
              2026-08-19.
Rejected:     Keeping S096 retired on the corrected record -- zero cost, but
              every surveyed hand-crafted engine at the band has the in-loop
              form and DEC-019 says a published figure decides what to try.
              Folding the check extension into S097 as a third verdict --
              cheaper by a step, but it blurs S097's two-verdict attribution.
              Keeping S099 at entry 12 with a restated reason -- it places a
              technique whose only evidence is at 3200 ahead of steps with
              sub-3000 records. Demoting or retiring S133 -- the technique is
              real and adopted; only its number is not isolated.
Consequences: DEC-087 carries an Amended line pointing here. `plan.md`'s
              DEC-087 section carries the three corrections inline. The Open
              list moves S099 to the reserve head and inserts S188 after S097.
              S181 records the Lynx bands, S185 the sources. DEC-087's
              structure -- four blocks, search first -- is untouched.

## DEC-134  2026-09-04  A constant quoted in another engine's commit message is that engine's constant; the seven seeded sections are reseeded
Tags:         provenance, licensing, tuning, plan
Context:      `2026-09-04_plan_review-F01`. Seven enriched step files -- S095,
              S097, S098, S109, S113, S114, S132 -- were written on 2026-08-19
              under DEC-084's venue rule and seed their constants from numbers
              Stockfish, Weiss, Lynx, Ethereal and Berserk quote in commit
              messages and pull-request bodies: ProbCut's `beta + 200`, late
              move pruning's `depth * 10` with cap 3, the time-management pair
              2.0 / 1.0, a history clamp of 2, singular depth 8 and margin 3,
              "one pawn" and "three plies". S114 states in so many words that
              "numbers quoted from message prose are legal seeds". DEC-105
              (2026-08-22) made the seed rule provenance-based and its
              Consequences named `plan.md`, `CLAUDE.md`, `AGENTS.md` and
              `specs.md`; the step files were not on the list and nobody
              re-read them. Nothing in the engine carries these constants; the
              defect is that an implementer following the files as written
              starts a sweep or an SPSA run from another engine's number, and
              DEC-105's own reasoning -- a seeded-then-refit vector converges
              near its seed by construction -- is why that is a breach and not
              a formality.
Decision:     By the owner, 2026-09-04: stick to the rule. A number that
              originates as another engine's tuned or shipped value is that
              engine's constant wherever it is quoted; a commit message is a
              venue like the wiki. DEC-105 binds the seed sections of every
              pending step file. S180 reseeds the seven, each seed in one of
              three forms: a value from a publication about the technique with
              its URL, a derivation procedure over chesso's own data or scale
              that the owning step runs at its start, or the range midpoint,
              stated as such. Units are not exempt by name -- "one pawn" is
              expressed in chesso's own material scale.
Rejected:     A per-case exception in the DEC-087 Fathom style admitting
              prose-quoted constants -- it contradicts DEC-105's own reasoning,
              and the owner's stated ground for the rule is comfort that no
              engine number is anywhere in the lineage. Midpoint for all seven
              -- kept as the fallback where no derivation is cheap.
Consequences: DEC-105 carries an Amended line pointing here. Every future
              enrichment writes seeds in one of the three forms; S186's
              accepts says so. S180 sits third in the Open list, before any of
              the seven can be started.

## DEC-135  2026-09-04  Citations from pending step files into code name symbols, not lines
Tags:         workflow, docs, plan
Context:      `2026-09-04_plan_review-F07`, the third recurrence of
              `2026-08-20_plan_review-F01`'s class. S169 re-anchored 97
              citations on 2026-09-01; three days later `--citations` flags 59
              over 54 files, one inside S159's accepts. Four steps carry
              citations that were wrong when written -- S109's accepts, S055's
              taper divisions, S024's Note, S119's `rating.sh` line -- which the
              checker cannot see because its baseline is the file's own last
              commit. Every drifted citation sits beside the symbol it names:
              the symbol is the durable reference, the line number the
              decaying one.
Decision:     By the owner, 2026-09-04. A citation from a file in
              `adocs/plan_todo/` or `adocs/plan_current/` into source or test
              code names the file and a symbol -- a function, a constant, a
              macro or a `TEST_CASE` title -- and carries no line number. A
              citation into a document quotes the phrase it points at.
              DEC-120's rule that a citation repeats its path stands.
              `tools/plan_prose_check.py --citations` fails a `file:line` form
              in the pending directories and a symbol absent from the named
              file. S187 converts the existing citations and adds the check,
              and decides whether the mode then joins the fast suite, since
              the reason for keeping it out -- line drift on every source
              commit -- no longer applies to the symbol form.
Rejected:     A fourth re-anchoring pass plus a symbol check on line citations
              -- cheaper once, and the recurrence rate says it is paid every
              three days. Leaving line numbers with a symbol beside them --
              that is the state at HEAD and it still reads wrong.
Consequences: DEC-120 carries an Amended line pointing here. `plan_done/` is
              history and keeps its line citations as written; `specs.md`'s
              invariant table and `decisions.md` are outside this rule until a
              later decision. Step files written from this date use the symbol
              form; S180 to S188 do.

## DEC-136  2026-09-04  The plan's cost and Elo lines are derived from the ledger and re-derived at every verdict
Tags:         planning, measurement
Context:      `2026-09-04_plan_review-F03` and `-F04`. The cost line prices a
              verdict at 45 to 75 minutes where the seven runs stamped since
              S105 averaged 4 h 40 m (median 5 h 26 m), so the pending 45 to
              55 verdicts cost 210 to 256 machine-hours and not 75 to 110; the
              plan itself records that S093 overran and leaves the total
              standing. The Elo arithmetic ("the midpoint clears 3000") has no
              recorded inputs anywhere in the repository and applies no
              published-to-measured discount, where the ledger since it was
              written measured that ratio at 0.29 to 0.38 (S093), a wrong
              sign (persistence), about 0.1 (S130) and a negative (S149); the
              plan's own low end lands at 2949.
Decision:     By the owner, 2026-09-04. Both lines are derived from recorded
              inputs and from nothing else. The cost line from the wall time
              of every SPRT run stamped in `plan_done/` since S105, by effect
              class -- block-class effects at the fast end, +5-class effects
              at the ledger's mean -- times the pending count per class
              (S182). The Elo line from a tabled list of per-step published
              figures with their sources, and a third discount, the ledger's
              own transfer ratio, under a rule written before the number is
              computed (S183). Both are re-derived in the completing commit of
              every step that lands a verdict, the way `status.md` is
              rewritten. "The midpoint clears 3000" survives only if the
              re-derived midpoint does; if the re-derived high end does not
              clear 3000, that is a DEC-071 question put to the owner and not
              a plan edit.
Rejected:     Deleting the arithmetic -- the ledger is the evidence of
              progress, but a checkable heuristic is better than none. A
              caveat on the existing numbers -- a wrong number with a caveat
              still schedules the nights. Changing the bounds regime to make
              the old figure true -- a measurement-policy decision, separate,
              and not taken here.
Consequences: `plan.md`'s "What this costs" and Elo paragraphs are rewritten
              by S182 and S183; the old figures are struck through with the
              date, not deleted. Until then they stand as written and the
              plan says so.

## DEC-137  2026-09-04  The DEC-097 enrichment pass becomes a step before block 3, and the review's source table is tracked
Tags:         planning, docs, provenance
Amended:      2026-09-05 by DEC-145 -- on the owner's instruction every
              pending file is enriched in Open order, block 1 and 2 files
              included; S186 becomes the verification of block 3's files.
Context:      DEC-097 (2026-08-21) resumed the SOTA enrichment "in parallel,
              on nights". `git log --since=2026-08-21 -- adocs/plan_todo/`
              shows 40 commits and no enrichment; 35 of 54 pending files carry
              no "Technical details" section; block 3's order rests on "the
              Stash ledger" with no path, commit or URL in any tracked
              document (`2026-09-04_plan_review-F06`). The review's independent
              literature pass fetched 33 of the plan's figures at source,
              confirmed most, narrowed ten, and located both ledgers: the
              Ethereal figures are the table in Ethereal commit e755a814, the
              Stash figures are entries in `mhouppin/stash-bot`'s
              `CHANGELOG.md`.
Decision:     By the owner, 2026-09-04. The literature pass is tracked as
              `adocs/data/2026-09-04_plan_review_literature_check.md` and is
              the source table S185 records from. The enrichment is no longer
              a standing promise for nights: it is S186, ordered before S134,
              the first block-3 step, and block 3 does not start until S186
              has run over its files. Block 1 and block 2 files are not
              re-enriched; S180 and S181 correct what their enrichment got
              wrong.
Rejected:     Enrichment on demand, per step at its start -- a step started on
              an unsourced figure repeats F06 at the moment the figure matters
              most. Enriching everything now -- block 3 is 28 entries away
              and the machine-scope lane has work with a closer payoff.
Consequences: DEC-097 carries an Amended line pointing here. S186 takes
              S185's unverified list as its work list. `adocs/data/README.md`
              carries the literature check's row.

## DEC-138  2026-09-04  Techniques the surveyed engines carry and this plan gives no step, considered and deferred
Tags:         planning, search, evaluation, scope
Context:      The review's literature pass inventoried the documented
              techniques of Stockfish, Ethereal, Berserk, Weiss, Stash and
              Leorik -- read as names in their current sources and write-ups,
              never as code -- against the Open list. Beyond the omissions the
              plan already records (mate distance pruning and the two-ply
              killer reset in `plan.md`'s 2026-09-03 section; the null-move
              verification search and check extensions in DEC-087, the latter
              reopened by DEC-133; threads in DEC-089), the following have no
              step and no record: negative extensions and double or triple
              extensions (S097 names them as later refinements and no step owns
              them); table-move and table-PV reduction and extension terms;
              opponent-worsening; pawn history; minor-piece, major-piece, threat
              and last-move correction histories; material imbalance tables;
              trapped-piece terms; minor behind pawn, long-diagonal and
              bad-bishop terms; closedness and complexity scaling; queen
              relative pin; king defenders; hanging-piece and pawn-push threats
              beyond S101's "attacked by a lesser piece"; a KPK bitbase and
              specialised endgames beyond S124's scaling; time-management
              factors by move type; and MultiPV, pondering, Chess960 and
              contempt, which are interface or style items. The wiki states an
              Elo figure for none of them.
Decision:     By the owner, 2026-09-04. All of the above are recorded as
              considered and given no step in phase one. The extension family
              and the further correction tables are present in the current
              versions of Stockfish, Berserk, Weiss and Stash, all far above
              the mark, and no record of their value at this band was found;
              they belong to the reserve's band or to phase two. The
              evaluation terms are hand-crafted refinements the parked network
              supersedes (DEC-054) and are reopened only if block 3's fits show
              a specific term binding. The interface items are not strength and
              are scheduled when a user needs them. A future audit that finds
              one of these absent cites this entry.
Rejected:     A step per item -- twenty-odd unpriced steps ahead of a list
              that already owes over two hundred machine-hours. A reserve
              entry per item -- the reserve is for steps with evidence at a
              stated band, and none of these has a number.
Consequences: S097's "later refinements" sentence and S101's excludes cite
              this entry when S180 and S186 next touch those files. The
              inventory itself is in
              `adocs/data/2026-09-04_plan_review_literature_check.md`, part B.

## DEC-139  2026-09-05  The 2026-09-04 test review is digested: eleven steps, four rule amendments, one finding accepted
Tags:         testing, audit, workflow, planning
Context:      The owner asked for research on chess-engine testing technique
              and an assessment of this engine's tests. The assessment is
              `adocs/audit/2026-09-04_test_review.md` -- three medium and seven
              low findings, the suite's fault detection measured by
              fault injection (31 of 32 non-equivalent hand-written bugs
              caught; the survivor is the fifty-move boundary) and its reach
              by coverage -- and the survey with its fourteen recommendations
              is `adocs/testing_strategy.md`, every claim fetched at source or
              marked unverified. The recommendations were put to the owner one
              at a time on 2026-09-05.
Decision:     By the owner, item by item. R1 to R6, R8 to R11 and R14 become
              steps S189 to S199 and the rule amendments DEC-140 to DEC-143;
              R3 (S191) is ordered before S109 and R14 (S199) takes its first
              point after the S109 block. **R7, calibrating the harness on
              this MacBook, is refused: the workstation returns soon and a
              number about a machine that is leaving is not worth 26 minutes
              of it**, so `2026-09-04_test_review-F10` moves to `accepted` and
              the calibration is owed on the workstation as the first run
              taken there, under DEC-143's rule and doubling as S198's A/A.
              R12, a `NodesTime` screening option, is deferred to S127's
              design and noted in that file. R13, coverage as a periodic
              report, folds into S197 as a documented command. The strategy
              document is a plan input in the sense DEC-062 gave
              `eval_tuning_strategy.md`.
Rejected:     Calibrating here anyway -- see above. A step per rule clause --
              four decisions carry the rule changes and the steps carry the
              work. Bundling the eleven into fewer steps -- each closes a named
              finding or implements one recommendation, and a stamp per
              finding is what the audit re-run needs to move a status.
Consequences: The Open list grows to 74 entries; the nine machine-free steps
              join the machine-light lane after the plan review's document
              steps, S198 sits before the first SPRT-owing step and S199 after
              S109. The report's findings read `planned` (F10 `accepted`). The
              review's evidence directory `adocs/data/2026-09-04_test_review/`
              is append-only like the rest of `adocs/data/`. S179, S182 and
              S127 are amended: the Zobrist keys join the magic regeneration
              (F08), the cost line carries the bounds cost table (R11a), and
              S127 records the `NodesTime` option as a design question.

## DEC-140  2026-09-05  A commit touching `src/` carries its bench signature, and the gate checks it
Tags:         workflow, git, testing, inv-6
Context:      INV-6 is discharged by a person running `tools/search_bench.py`
              on two builds and comparing by eye; the number is recorded in
              step stamps and nowhere a gate can read. Every CI the strategy
              document surveyed checks a bench signature from the commit
              message before anything else runs, and the review's
              fault-injection pass showed the signature moving on 21 of 33
              injected bugs -- including a one-ply reverse-futility floor
              drift that two of the three mate gates did not see -- and
              staying still on 12 the suite caught, so the two instruments are
              complementary and only one is automatic.
Decision:     By the owner, 2026-09-05. From S189's completing commit on,
              every commit that touches `src/` ends with `Bench: <nodes>`, the
              total `chesso bench` prints, or `No functional change` when the
              total is the parent's; `tools/gate.sh` runs the TESTS rule
              command and then compares the built binary's bench with the
              message, and a mismatch is a red gate. `AGENTS.md`'s COMMITS
              rule carries the line. A commit that changes the bench position
              set is itself a `Bench:` line.
Rejected:     Keeping INV-6 a procedure -- it stayed one for a month and the
              review found it compared by hand and recorded nowhere. A remote
              CI service -- the machine is the constraint and a local script
              gives the same guarantee at commit time.
Consequences: S189 builds the command and the script. Stamps keep quoting the
              three `search_bench.py` counts for continuity with the recorded
              baselines; the signature is the gate's number.

## DEC-141  2026-09-05  The TESTS rule gains a second tier: Debug self-play, a mutant per new search rule, and a scheduled extra gate
Tags:         testing, workflow, invariants
Context:      Both gated builds are Release, so every `assert(` in `src/` is
              dead in them and INV-2 and INV-4 run only when somebody runs the
              Debug binaries by hand (`2026-09-04_test_review-F01`). The
              null-move and reduction guards had no direct test and their
              whole coverage was one golden count (F02). Sanitizer runs have
              been "absent from the gate" since the 2026-08-14 review.
              Stockfish self-plays a Debug binary in CI and runs its command
              set under sanitizers; S145 proved a test bites by showing it red
              under a stated mutation, and the review generalised that into a
              measured kill rate.
Decision:     By the owner, 2026-09-05. Three clauses in the TESTS rule.
              (1) A step that touches `make_move`, `unmake_move`, the
              generator or the search self-plays the Debug binary -- four
              rounds of `fastchess` at 4+0.04 -- and greps its log for
              `Assertion` before completing, and its stamp says so. (2) A new
              pruning, reduction or extension rule ships with a direct guard
              test and a mutant that test kills, run through
              `tools/mutation_check.py` once S196 lands and through the
              review's driver before. (3) `tools/gate_extra.sh` (S197) -- the
              Debug binaries, a sanitizer build, deep perft, the prose and
              citation checks -- runs before such a step completes and
              otherwise weekly, noted in `status.md`.
Rejected:     Putting the Debug binaries or the sanitizers in the automatic
              gate -- 7 to 20 minutes per run against a 108 s gate, and
              DEC-025 stands. Leaving the cadence to judgement -- that is the
              state the two reviews found.
Consequences: S190, S191, S196, S197. The S109 block's four rules each arrive
              with a case and a mutant. `DEV_MANUAL.md` "Test" carries the
              exact commands when the steps land.

## DEC-142  2026-09-05  Every golden number in the tests is named as one and re-derived by its script, never re-read
Tags:         testing, workflow, dec-116
Amends:       DEC-116, generalised from one floor to every golden
Context:      The review found that a large share of the fast suite's
              sensitivity comes from golden numbers -- static-score anchors,
              mate-line floors, node budgets, a soft-limit scaling asserted on
              a fixed position's tree -- that every legitimate search or
              evaluation change also moves: 16 cases red on an eval sign flip,
              `test_mate_carry` red on 21 of 22 search mutants, the scaling
              case red on eight search mutants that were not time-management
              defects. They caught the injected bugs; they will also redden on
              S024, S109, every refit, and a floor re-derived under time
              pressure is how a gate gets weakened. DEC-116 already states the
              rule for the mate-in-three floor. The piece anchors are derived
              by a gitignored script (`status.md`, parked since 2026-08-23).
Decision:     By the owner, 2026-09-05. Every golden value or floor in `tests/`
              is named as a golden at its site with the command that
              re-derives it; a golden is re-derived by its script whenever
              either end of it moves, with the margin stated, and never
              re-read from a run; a golden that cannot be scripted is a
              finding; `anchors.py` enters the repository. Where a golden
              stands in for a property, the property gets its own case so
              coverage survives a re-derivation. The TESTS rule carries the
              sentence.
Rejected:     Deleting the goldens for properties alone -- they are the best
              detectors the suite has. Leaving re-derivation to each step --
              the state that produced a floor that stopped separating for
              nine days (DEC-116).
Consequences: S192. Every search or evaluation step's stamp names the goldens
              it re-derived and the script it ran.

## DEC-143  2026-09-05  A pre-registration prices its bounds pair, and every harness change is followed by a fixed-rounds A/A
Tags:         sprt, measurement, bounds, dec-063
Context:      The nElo run-length formula (Van den Bergh,
              `adocs/testing_strategy.md` section 1.1) prices a `{0,5}` or
              `{-5,0}` pair at 41861 expected games when the truth sits at the
              interval's midpoint and 25591 when it sits on a bound -- 17.9 h
              and 10.9 h at the 2337 games an hour seven recorded runs on this
              machine average -- and the ledger agrees: 4 h 40 m mean, one
              6 h 36 m no-verdict run. The plan's 45 to 75 minutes holds only
              for effects far outside the interval. DEC-063 asks every
              pre-registration for the reading of each outcome; it does not
              ask what the pair costs. S105 calibrated the harness once with
              two fixed-rounds A/A runs and no calibration has followed a
              machine change or a fastchess upgrade since; an SPRT A/A passes
              with probability alpha and measures nothing.
Decision:     By the owner, 2026-09-05. A run's pre-registration states the
              pair's worst-case expected games from the formula and its abort
              rule beside the three outcomes. A fixed-rounds A/A of 1000 games
              is taken after every change to the harness -- fastchess version,
              book, adjudication, machine -- and read with
              `adocs/data/S105_pairs.py` and `tools/forfeit_report.py` before
              the next verdict; the first is the workstation's, and it
              doubles as S198's. The MEASUREMENT rule carries both sentences;
              `DEV_MANUAL.md` "Which bounds" carries the cost table when S182
              lands it.
Rejected:     Calibrating the MacBook now (DEC-139). An SPRT A/A as the
              calibration -- see above. Changing any pair -- the cost is a
              property of the pairs and the right answer is to write it down.
Consequences: S182's accepts carries the table; S198 lands the harness flags
              with the workstation's A/A; every `adocs/data/S*_sprt.sh` header
              from here on states its worst-case games.


## DEC-144  2026-09-05  The machine-scope lane is dismissed and the Open list is re-sorted for the workstation: bugs, the two calibrating runs, an instrument lane, then the four blocks
Tags:         plan, machine, measurement, workflow, dec-112, dec-113
Amends:       DEC-112, whose lane this removes
Context:      DEC-112 lifted fifteen machine-light steps to the head of the
              Open list while the owner worked from the MacBook, and said that
              restoring the order is a decision and not a tidy-up. On
              2026-09-05 the owner, leaving for the day, asked that the plan be
              reordered in the best way to reach the goal under the project's
              rules, assuming every step runs on the Linux workstation, and
              that decisions be taken rather than deferred where possible.
              Since DEC-112 the list gained the 2026-09-03 audit batch (done),
              S178 and S179, the plan review's nine steps and the test review's
              eleven, and three decisions already fix the head: DEC-128 (S171's
              census is the first thing taken on the workstation), DEC-143 (a
              fixed-rounds A/A of 1000 games follows the machine change, before
              the next verdict, and doubles as S198's) and DEC-140/DEC-141 (the
              bench signature binds every `src/` commit from S189's completing
              commit on; S191, S196 and S197 precede S109). Everything the
              lane's table said the M1 could not measure is measurable on the
              workstation.
Decision:     By the agent, under the owner's instruction of 2026-09-05 to
              reorder and take decisions; the owner confirms or amends on
              return. (1) plan.md's `## Machine scope` section is deleted and
              the Open list re-sorted; the section's text is in the file's
              history at `66cbc54`. (2) The head: S178 and S173, two tool bugs
              under the BUGS rule; S171, the census, first run on the
              workstation; S189 and S179, agent-only work during the census,
              both proved on node counts; S198, the harness flags and the
              calibrating A/A, before any verdict. (3) An instrument lane of
              the sixteen document and test steps, interleaved with the only
              three runs that depend on nothing in the search block so a single
              agent always has an entry to take while a run plays: S180, S184,
              S148, S187, S190, S159, S193, S191, S196, S197, S192, S195, S194,
              S151, S181, S185, S182, S183. S180 and S184 first because each
              removes a hazard an implementer would follow; S187 next so every
              later edit to a pending file is under the symbol check; S191,
              S196, S197 before S109 (DEC-141); S182 before the first verdict
              lands; S183 after S181 and S185, its inputs. (4) Blocks 1 to 4 in
              the 2026-08-19 order as DEC-133 corrected it, nothing inside them
              moved: S199's first point after S109, S188 after S097, S186
              before S134, S152 after S129 as the close of the main order
              (DEC-108). S020 and S030 return to block 2. (5) Reserve S099,
              S023, S025, S110, S111; parked S029. (6) The reading rule: the
              coordinator takes the first Open entry that owns a run; while it
              plays, the next entry that owns no run may start in list order; a
              change to `src/` waits, since it either alters play (MEASUREMENT,
              one at a time) or is behaviour-neutral and owes a timing that
              needs the idle machine. Dependencies are the entries above; no
              entry is started out of order to fill the machine.
Rejected:     Keeping S148 and S159 behind the search block -- each verdict
              would then be taken against the tree it will play in, but the
              workstation would idle through sixteen document steps; both
              depend on nothing in the block, which is why DEC-112 could move
              them, and S127 refits every search parameter at the end anyway.
              S151 ahead of S148 -- its control of at least four times 8+0.08
              prices a `{-5, 0}` pair near 72 hours worst case (DEC-143's
              formula at a quarter of 2337 games an hour), and whether that
              pair or a cheaper fixed-rounds reading is wanted is the owner's;
              it sits behind the two cheap verdicts with the question deferred.
              A list with the document steps in one run and the verdicts in
              another -- a single agent reading it would launch a run and find
              no next entry. Reordering inside the blocks -- the order is the
              product of two reviews and the owner's rulings (DEC-081 to
              DEC-089, DEC-133) and no new measurement has arrived to move it.
Consequences: The next step is S178. S171's Open entry loses its `postponed`
              tag. `status.md` names the workstation's first runs in order and
              the one question deferred to the owner. Nothing in the engine
              changed and no run started. A future absence from the workstation
              re-derives a lane from DEC-112 rather than from memory.


## DEC-145  2026-09-05  Every pending step is enriched for its implementer, one agent per file, sequentially, in Open order
Tags:         plan, research, process, dec-097, dec-137, dec-105, dec-135
Amends:       DEC-137, whose "block 1 and block 2 files are not re-enriched" clause this supersedes for the pass
Context:      DEC-097 resumed the SOTA enrichment "in parallel, on nights" and
              produced no commit in fourteen days; DEC-137 turned block 3's
              share into S186 and left block 1 and 2 alone. On 2026-09-05 the
              owner asked that, starting from the next step and moving forward
              one step at a time in the new order, an agent enrich each step
              file with details from the literature and whatever else helps
              the weaker agent that will implement it. At that moment 20 of
              the 74 pending files carried a 2026-08-19/20 "Technical details"
              section, seven of them seeding constants from other engines'
              commit prose (DEC-134), and the rest none.
Decision:     By the agent, under the owner's instruction of 2026-09-05. One
              agent per pending file, one file at a time, in Open order,
              appending `## Implementation guide (2026-09-05)` with a fixed
              shape: what the step is, the technique as published with the
              wiki's or the paper's definition, chesso's form written in the
              agent's own words, the symbols the change touches as they read at
              HEAD (DEC-135 form, path repeated per DEC-120), constants and
              seeds in a DEC-105 form only -- a literature value with its URL,
              a derivation over chesso's own data, or the range midpoint -- the
              tests DEC-141 and DEC-142 require, the measurement plan with its
              pair priced per DEC-143, the completion checklist, this
              repository's own recorded traps, every source read with its URL
              or the word unverified, and the questions deferred to the owner.
              An existing 2026-08-19 section is kept; a seed in it that
              originates in another engine is replaced in place in a DEC-105
              form with the replacement noted, which is S180's accepts done
              early -- S180 verifies and stamps. Engine records are read as
              commit messages, pull-request bodies, changelogs and release
              notes, never as source or tables (DEC-016). The pass edits no
              header field -- a stale `accepts` becomes a deferred question --
              and no file but the step's own; one commit per file, each green
              on the `--touches` check. It is not a plan step and enters no
              order. Block 3's files are brought to S186's accepts, so S186
              becomes a verification of them.
Rejected:     Parallel agents -- the owner asked for one step at a time, as on
              2026-08-20 when the parallel pass was stopped. Skipping the
              twenty enriched files -- DEC-134 found their seeds wrong, the
              older sections predate DEC-135 and the improving-flag plumbing
              S108 landed, and the instruction starts from the next step
              without exception. Rewriting the old sections -- they are the
              record of what was known on 2026-08-19; the new section corrects
              them by name where they are wrong. Editing header fields -- the
              accepts belong to the step's decision record, not to research.
Consequences: `grep -L 'Implementation guide (2026-09-05)' adocs/plan_todo/*.md`
              names what the pass has not reached, and the recipe replaces any
              census in `status.md`. Where the pass reaches one of S180's seven
              files, S180's remaining work is to verify. The owner's deferred
              questions are gathered in the report of 2026-09-05 and in
              `status.md`'s Parked list.

## DEC-146  2026-09-07  The workstation runs the format gate with `CLANG_FORMAT_MAJOR=22`; the pin stays 23
Tags:         toolchain, tests, gate, machine, linux, dec-110
Amends:       DEC-110, whose pin is unchanged and whose predicted workstation
              cost this pays
Context:      The owner returned to the Linux workstation on 2026-09-07 and the
              gate was red there, with no code change behind either half of it.
              The second half is this one: clang-format 23 is not on this
              machine and cannot be installed from what it carries. The LLVM
              apt line here is `llvm-toolchain-noble-22`, the repository offers
              14 to 22, and the only unsuffixed binary is Ubuntu's 18.1.3, so
              `clang-format.sh` resolves it, refuses it by version and exits 1,
              and `test_clang_format_script` fails six assertions in both
              builds -- the same six, and for the same reason, that DEC-110's
              Context recorded on the MacBook and its Consequences predicted
              for this machine in as many words. `apt.llvm.org` does serve
              `llvm-toolchain-noble-23` (checked, HTTP 200), so installing it
              was available and costs one new apt source and a sudo install.
Decision:     By the owner, asked directly on 2026-09-07. `REQUIRED_MAJOR`
              stays 23 in `clang-format.sh` and every committed document keeps
              that number, because the pin has not moved. This machine exports
              `CLANG_FORMAT_MAJOR=22` for the gate, and `.moltke.local.md`
              records it as this machine's. Measured before the choice was put:
              `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` prints nothing
              and exits 0, so the tree is byte-identical under 22 and 23 --
              which is what DEC-110's Consequences said would hold after
              S024's revert, now confirmed from the other side -- and the fast
              suite is 27/27 in both builds with the override exported.
Rejected:     Installing clang-format 23 here. It keeps one number across both
              machines and needs no override in any command, but it adds an apt
              source for a toolchain this machine otherwise does not carry --
              `.moltke.local.md` pins LLVM 22 suffixed as a second front end
              beside the reference `g++ 13.3` -- and the override costs nothing
              while the two majors format this tree identically.
              Moving the pin back to 22. That reverses an owner decision in
              order to suit a machine, and it would redden the MacBook exactly
              the way 23 reddened this one.
              Widening the pin to a range, or dropping it. DEC-110's rejection,
              unchanged: two machines formatting the same tree differently is
              the failure the pin was written against.
Consequences: Every gate invocation on this machine carries
              `CLANG_FORMAT_MAJOR=22`, and `tools/gate.sh` (S189) reads the
              variable from the environment rather than hard-coding a major.
              The equivalence is a measurement and not a guarantee: the day a
              construct formats differently under the two majors, the override
              stops being free and the choice returns, with `./clang-format.sh
              --check` under 23 as the arbiter and this machine unable to run
              it. What would show it is a MacBook check reddening on a tree
              this machine calls clean.

## DEC-147  2026-09-07  S178's two deferred questions answered: the remaining import-format leniency and the token as written both become S200
Tags:         tools, make_book, pgn, docs, s178, s200
Context:      S178 taught `movetext_to_san()` the glued move number indication
              (`1.e4`, `2...Nc6`) and left two questions in its section 10,
              both outside its `accepts:` and neither a bug. **One**, PGN
              8.2.2.1 also allows whitespace between the digit sequence and the
              period(s), so `1 . e4`, `1 .e4` and `1. ... e5` are legal import
              format; after S178 the leading `.` or `...` token has `dot == 0`,
              reaches `algebraic_to_move()`, gets 0 and cuts the game short by
              name. Honest, and still not the whole standard. **Two**, S178
              erases the indication in place, so the cut-short message names
              the move part (`e4`) and not the token the PGN wrote (`1.e4`),
              which is a worse pointer into the file being fixed.
Decision:     By the owner, asked on 2026-09-07 with S178's completion. Both
              are taken, and both land in **one** follow-up step, S200: the
              leading-dot run is dropped by the same rule, and the message
              carries the original token. The owner's condition on the second
              was "if not too complex"; it was sized before the step was
              written -- `movetext_to_san()` returns a two-field struct
              carrying the move and the token as written, its one caller in the
              tree reads the second field for the message only, and no other
              file in `src/`, `tests/` or `tools/` names the function. About
              fifteen lines. The condition is met and no re-evaluation is owed.
Rejected:     Two separate steps, one per question. Correct by the letter of
              "one goal per change", and ceremony here: both edits are in
              `movetext_to_san()` and the one loop that reads it, both are
              covered by the same fixture file, neither alters play, and no
              verdict is at stake for either -- the rule exists so two changes
              cannot contaminate one measurement, and there is no measurement.
              Folding either into S178. S178 is in `plan_done/`, which is
              history and is never edited.
              Leaving the message as S178 left it. The message exists to point
              at a place in a PGN the reader must fix; `e4` appears many times
              in a file and `1.e4` appears once.
              Teaching `algebraic_to_move()` to skip a leading indication
              instead. S178's `excludes:` keeps the engine's parser out of the
              tool's lexing, and the parser is on the engine's move path where
              this has no business being.
Consequences: S200 sits in `plan_todo/` and enters the Open list where the
              owner puts it; it is not urgent -- the tool refuses honestly
              today for every form it does not read. After it, the message text
              changes for a glued token, so any test grepping a cut-short
              message must expect the token as written; `tests/test_make_book_tools.sh`
              property 3 greps `Qxf7` from a spaced fixture and is unaffected.
              The remaining leniencies of S178's `excludes:` -- `e8Q` without
              `=`, `0-0` with zeros, a `P` prefix -- stay unread and stay each
              their own decision.

## DEC-148  2026-09-07  `algebraic_to_move()` refuses a leading character that is not a piece or file letter; the disambiguation walk stays lenient
Tags:         engine, parser, tools, make_book, bugs, s201, dec-140
Context:      Found 2026-09-07 while sizing S200's second half. The parser
              reads the piece letter at position 0 only, so any other leading
              character fell to the pawn branch, and the disambiguation walk --
              which records only `a`-`h` and `1`-`8` -- swallowed the real
              piece letter on its way to the destination square. `.Nf3` came
              back as the legal pawn push `f2f3` where the token means `g1f3`;
              `make_move()` applied it and no caller could see it. python-chess
              refuses `.Nf3`, `.e4` and `..e4`, checked the same day. Reachable,
              because `1 .Nf3` is legal PGN import format (8.2.2.1), so
              `make_book` could build a book from a wrong board and report
              `games cut short 0`. It is the shape S174 closed for a token the
              parser *cannot* read, still open for one it reads as something
              else. Bounded: the function is called only by the two tools and
              the tests, never on the engine's UCI move path, which is long
              algebraic; `books/8moves_v3.pgn` carries no such token.
Decision:     By the owner, shown the evidence on 2026-09-07 and asked both
              halves. **It is a bug and it is fixed before anything else
              starts**, which is the BUGS rule; S201 was created directly in
              `plan_current/` for it. **The gate is narrow**: after the suffix
              strip and the castling forms, a token whose first character is
              not in `[a-hKQRBN]` returns 0, and an empty token returns 0.
              Characters after the first stay exactly as tolerated as they
              were.
Rejected:     The wider gate, every character in `[a-h1-8KQRBNx=]`. It also
              closes the lenient interior forms (`N.f3`, `N,f3`, `N*f3`, all of
              which give the *correct* move today) and would match python-chess
              exactly, but it narrows what the tools accept from real-world PGN
              for no defect: the wrong-move class is entirely a leading-
              character effect, and the narrow gate closes all of it.
              Filing it as a step and letting it wait its turn in the Open
              list. Defensible, since the engine's own move path never calls
              the function, and refused: a known defect in the tree
              contaminates every measurement taken after it.
              Recording it as deliberate leniency and changing no code. The
              parser's contract since S174 is that a token it cannot read
              returns 0; a token it reads as a different piece is not leniency.
              Fixing it in `movetext_to_san()` instead, by never emitting such
              a token. S200 does that and it is worth having, but it leaves
              `pgn_to_positions` and any future caller on the wrong board.
Consequences: `xd5` and a leading `-` are refused too, which no PGN writes.
              Verified over the whole shipped corpus rather than argued: the
              book rebuilt from `books/8moves_v3.pgn` through the changed
              parser is byte-identical at `77f47f1b...db06b58` -- 34700 games,
              172232 entries, 0 cut short -- so roughly 278000 real SAN tokens,
              with captures, disambiguation, promotions and castling among
              them, parse exactly as before. INV-6 identical. DEC-140's
              `Bench:` line binds from S189's completing commit on and S189 is
              open, so this commit owes none. S200's `excludes:` keeps the two
              apart and S200 does not wait on this.

## DEC-149  2026-09-07  `make_book build`'s temporary has the fixed name `<out>.tmp`, not a unique one
Tags:         tools, make_book, filesystem, s173, dec-131
Context:      S173 replaces the destination atomically -- write beside it,
              `rename` over it -- and its `accepts` says "the temporary is
              removed on every failure path and nothing is left beside the
              destination". That cannot hold literally: a `SIGKILL` mid-write
              kills the process before it can `unlink`, so *something* survives
              unless the name is one the next run reuses. The step file's
              implementation guide raised it as a question for the owner and
              proposed `mkstemp` on `<out>.tmp.XXXXXX`, which is the textbook
              form -- `O_CREAT|O_EXCL` removes the race between choosing a name
              and creating it, and two concurrent builds to one destination
              cannot collide.
Decision:     By the owner, asked on 2026-09-07 with both readings and their
              costs on screen. **The fixed name.** `<out>.tmp`, opened
              `O_WRONLY|O_CREAT|O_TRUNC` at 0666, which is what the replaced
              `std::ofstream` asked for and so keeps the mode `rename` carries
              onto the destination unchanged -- the guide's third deferred
              question answers itself under this choice, so no `fchmod` and no
              `mkstemp` are needed. At most one temporary is ever left behind,
              by a kill the process cannot survive, and the next build to the
              same `--out` truncates it. The `accepts` therefore holds as
              written and the fixture test can assert `! -e <out>.tmp`
              unconditionally.
Rejected:     `mkstemp` on `<out>.tmp.XXXXXX`. Safer against two concurrent
              builds to one destination and free of the name race, and it was
              the guide's own recommendation; refused because it leaves one
              unreclaimable file per kill, so the property the step is written
              to guarantee would have had to be weakened in the `accepts` to
              "every failure path the process lives through". The concurrency it
              buys is not exercised: `--out` is pointed at `src/openings.bin`
              about once, by hand, and the fixture test builds one book at a
              time.
Consequences: Two `make_book build` runs writing to the same `--out` at the same
              moment clobber each other's temporary and one of them renames a
              file the other was still writing. Nothing in the tree does that,
              and no test can catch it if something starts to. The temporary is
              in the destination's own directory, not `$TMPDIR`, because
              `rename(2)` is atomic only within a file system and fails `EXDEV`
              across one -- that is not a choice, it is the reason the whole
              approach works. `SIGXFSZ` is ignored in `main` for the same step:
              by default it kills, and it killed the tool mid-write under
              `ulimit -f 200` -- exit 153, a 204800-byte truncated book left at
              `--out`, and `dump` accepted it. Ignored, the limit arrives as
              `EFBIG` from `write`, which the loop reports and cleans up after.
              This is the first POSIX header in `tools/` or `src/`
              (`<fcntl.h>`, `<unistd.h>`): nothing standard gives `fsync`, and
              `std::filesystem::rename` alone would not have.


## DEC-150  2026-09-08  S171's `accepts` of 0 is unreachable inside its own `excludes`; it takes the measured pair instead
Tags:         search, mate-pv, s171, s170, measurement, dec-122, dec-127, dec-128
Context:      S171's census ran on the workstation on 2026-09-07, the run DEC-128
              postponed there: `REF=457e355 ./fastchess.sh --fast`, 3000 games at
              8+0.08, `Hash=16`, concurrency 12 of 12, 1 h 17 m 49 s, **0 time
              forfeits**, Elo +3.24 +/- 8.91 with `LLR -0.12` -- the two builds
              are INV-6 identical, so that is the number it should print. The
              count the step exists for: **8** `Incomplete mating PV` lines from
              **1** search from the candidate, **0** from **0** from
              `ref-457e355`. The `accepts` names 0 from the candidate.
              The 8-against-0 split is which side met the position and not a
              difference between the builds: the same case replayed against both
              binaries at `nodes 300000` produces three short lines that are
              byte-identical, so the residual predates `136b03f` and
              `certified_mate_move()` never claimed it.
              What it is, measured and not argued. The score is **correct** --
              `stockfish` gives `#+6` at depth 20 and depth 30 for the position,
              and `#+5` after the move chesso plays -- and it is read off the
              table at **depth 3 on 1224 nodes**, too shallow to build the 11
              plies a `mate 6` owes. The line that iteration did build continues,
              in the table, into a chain proving `mate 8`. Both lookups in
              `complete_mate_pv()` are keyed on the distance still owed, so both
              refuse, and DEC-122's all-or-nothing rule leaves the line short and
              visible, which is what it is for. By depth 7 the same search
              reports `mate 8` with a complete 15-ply line and by depth 11 a
              complete 11-ply `mate 6`.
              No walk can close it: the line the score names is not in the table
              to be found at that moment, and building it would mean searching,
              which that path may not do. Every other route -- keeping entries
              longer, resizing the table, changing its replacement policy,
              anything that alters play -- is named in S171's own `excludes` as
              out of scope and owing an SPRT of its own.
Decision:     By the owner, asked on 2026-09-08 with the census, the
              reproduction, the Stockfish readings and the three options on
              screen. **The `accepts` is amended to the measured pair** -- 8
              lines from 1 search against 0 from 0, 3000 games, this workstation,
              recorded as measured -- and S171 completes on it, with the
              instrument work and `F_mate6_inherited_no_line` as the tracked
              reproduction. **S202** owns the class: a mate score inherited at a
              depth too shallow to back it, free to alter play under its own
              SPRT.
Rejected:     Widening S171's `excludes` so it may alter play and attempting the
              fix inside it. Refused because it makes S171 a different and much
              larger step than the one that was agreed, and because an SPRT plus
              a second census is several more machine-hours spent on a property
              that changes no game -- the engine's play is INV-6 identical either
              way.
              Leaving S171 open with the result recorded and no decision.
              Refused because a step held open on an `accepts` its own
              `excludes` forbids it to reach never closes.
Consequences: The standing figure for `-check-mate-pvs` becomes **8 lines from 1
              search in 3000 games** on the workstation, beside **0 from 0** for
              `457e355` in the same run, replacing the MacBook's 5 from 1 --
              which stays attributed to that machine (DEC-049 untouched). A count
              is read against 8 from now on. `certified_mate_move()` keeps its
              own evidence: `tests/test_mate_carry.cpp` was red then green at
              `136b03f` and the five stride-1 cases still report 0 short lines.
              An accepts written as an absolute count over a table-pressure
              property is what produced this: S170 left a residual too, and the
              lesson recorded here is that such a step states a *ceiling and a
              paired reference*, not a zero.
              One hypothesis was implemented and reverted on the way: that the
              walk stalled on a position whose single legal move carried a bound,
              which `certified_mate_move()` declines by construction. It was read
              off a walk of the match's line over a differently-warmed table, the
              guard case stayed red under it, and `src/search.cpp` is untouched.


## DEC-151  2026-09-08  The mate-line cases carry a `stride`, and a game's table is stride 2
Tags:         tools, testing, mate-pv, s171, s170, instruments
Context:      `adocs/data/S170_replay.py` and `tools/mate_trace.cpp` both replayed
              **every** ply of a game through one engine process. A game gives one
              engine only the positions *it* moves from -- it never searches the
              ones its opponent moved from, so half the plies write nothing to its
              table. Searching every ply is therefore a table no game produces.
              This was found the hard way: the census case above was read back
              from the log three times at stride 1 and came up clean each time,
              and reproduces at stride 2 on the first attempt. It is not a small
              difference -- at `nodes 1000000` from ply 56 or 64, every mate line
              the stride-2 replay sees is short.
Decision:     By the owner, 2026-09-08, as part of the S171 close. Both
              instruments take a stride: `--stride` on `mate_trace`,
              `--stride-override` and a sixth TSV column on `S170_replay.py`, and
              `adocs/data/S170_cases.tsv` gains a `stride` column that
              `tests/test_mate_carry.cpp` reads. Rows A to E stay **stride 1**,
              the shape they were found and measured in; nothing about them
              changes and they still report 0 short lines.
Rejected:     Converting the existing five cases to stride 2. Refused because
              their numbers -- the `expected_mate_lines()` floors and the
              red-then-green readings of S170 and S171 -- were all taken at
              stride 1, and moving them would invalidate the record without
              measuring anything new.
              Leaving the tools as they were and reproducing by hand. Refused
              because the reproduction is the evidence, and a case that cannot be
              re-derived from the file is not one.
Consequences: The mate-line case set has probed only stride 1 until now, so the
              whole class this shape hides is unmeasured -- S202 inherits that.
              `start` and `stride` must leave the last ply on the schedule, which
              the guard test now checks when it reads a row: a schedule that steps
              over it would pass by never looking.

---

## DEC-152  2026-09-08  The bench set keeps an illegal position, benches at depth 14, and the argv form ships with it
Tags:         testing, gate, signature, S189
Context:      S189 left four questions to the owner before its position set and
              its depth could be fixed. Three of them change what the signature
              is or what the step touches, and all three are the kind a future
              reader re-derives from the code rather than from a record. The
              signature is a compiled-in constant plus a compiled-in list of
              eight FENs, so every one of these choices is frozen into
              `24880255` and into every `Bench:` line after it.
Decision:     By the owner, 2026-09-08, answering S189 section 10.
              **1.** `KILLER_POS` stays in the set although python-chess reports
              it `TOO_MANY_WHITE_PAWNS|TOO_MANY_WHITE_PIECES` -- nine white
              pawns, `is_valid() == False`. The engine loads it, `test` already
              searches it, and it is the only position in the set carrying both
              the en passant capture (`f5e6`) and twelve promotions. A signature
              needs determinism, which OpenBench states as its only
              requirement; legality buys nothing here.
              **2.** `BENCH_DEPTH` is **14**, by the rule the step set itself --
              the largest depth whose mean is at most five seconds on this
              workstation. Measured, `hyperfine -w 1 -r 5`, idle, on mains:
              3.445 s +/- 0.028.
              **3.** The OpenBench argv form `./chesso bench` ships now rather
              than later, so `src/main.cpp` joins `touches:`; both forms print
              the same number.
              **4.** `tests/test_gate_script.sh` and `tests/CMakeLists.txt` join
              `touches:` too, so `tools/gate.sh` has a test of its own.
Rejected:     Deriving a legal variant of `KILLER_POS` by removing an
              uninvolved white pawn. Refused: it would add a ninth FEN constant
              that exists only to satisfy a validator nothing in the pipeline
              runs, and the position would then differ from the one `test` and
              the fault-injection pass measured.
              A shallower `BENCH_DEPTH`. Refused as a deviation from the step's
              own stated rule; the cost it buys off is 6.8 s per Release fast
              suite and 157 s in `Debug`, which lands on S197's weekly gate
              rather than on the per-commit one, and a case that benched
              shallower would not test the depth that ships.
              Shipping `gate.sh` untested and checking its nine decisions by
              hand at completion. Refused: it would make the gate the one piece
              of the gate nothing gates -- and the test earned itself
              immediately, finding the `pipefail` bug that sent a missing
              signature line through the trap's generic marker.
Consequences: Changing the depth or the set changes the signature deliberately
              and is itself a `Bench:` commit, which `MANUAL.md` now says. The
              number is per standard library until S179 regenerates the Zobrist
              keys, so a disagreement between machines is investigated before it
              is called a behaviour change. `Debug` runs of the fast suite gain
              157 s, which S197 inherits.

## DEC-153  2026-09-08  Fixed rounds are a mode of `fastchess.sh`, the A/A is read as a band, and the seed rides in the PGN
Tags:         measurement, harness, calibration, S198, S199
Context:      S198 deferred five questions to the owner before its run could be
              launched. Two of them decide what the instrument is -- there was
              no way to play a fixed-rounds match at all, since every mode of
              `fastchess.sh` passed `-sprt` -- and one decides whether a PGN
              can say what it played. The remaining two answered themselves:
              the workstation's `fastchess` is `alpha 1.8.1 20260720-daa3ea2`,
              the version the script's comment already names, and a forfeit
              rate over 1 % is a new step by the step's own rule.
Decision:     By the owner, 2026-09-08, answering S198 section 10.
              **1.** `ROUNDS=<n>` becomes a mode of `fastchess.sh` rather than a
              standalone runner, so a calibration exercises the exact
              invocation a verdict uses and S199's drift readings reuse it. It
              drops `-sprt` entirely and the banner prints
              `bounds none -- fixed <n> rounds, a calibration or drift reading,
              NOT a verdict` where a reader of the log meets it.
              **2.** "One A/A shows the distribution unchanged" is read as a
              **band check**, which amends S198's goal. The run is the
              workstation's own baseline; S105's after-run figures are a sanity
              band at `|z| > 1.96` on the pair variance. A difference is
              recorded and attributed by shape -- a lower 1.0-pair fraction
              points at the engine, a wider `GameDuration` spread at the machine
              -- and the run is never repeated for a better number. The seed is
              never the attribution: it draws a different sample of the same
              book and cannot move a distribution.
              **3.** Each PGN carries the seed in its own `[Event]` header,
              `chesso <tag> <stamp> srand=<seed>`, so a file separated from its
              log still says what it played. Outside S198's `accepts` and taken
              anyway, because fastchess records the seed nowhere at all.
Rejected:     A standalone fixed-rounds runner in `S105_calibration.sh`'s shape.
              Refused: it calibrates a copy of the command line, and the copy
              drifts from the one verdicts are taken with -- which is the whole
              failure DEC-020 is about, one level up.
              Reading the A/A as a literal "unchanged" against S105. Refused as
              unsatisfiable: S107, S108, S149, S165 and more have landed since
              2026-08-20 and a stronger engine self-plays a different
              pentanomial, so a strict reading would fail on the engine's own
              progress and say nothing about the harness.
              Re-running the calibration if it lands outside the band. Refused:
              re-running until a number agrees is choosing the sample after
              seeing it, and the band is a sanity check rather than a test that
              can be failed.
Consequences: `fastchess.sh` grows two environment overrides, `SRAND` and
              `ROUNDS`, both refused by name when they are not unsigned
              integers, and `tests/test_fastchess_script.sh` asserts eleven
              properties instead of eight. A fixed-rounds run is never quoted as
              a verdict. S199's drift readings have their mechanism, and every
              future harness change under DEC-143 has one too.

---

## DEC-154  2026-09-08  Redrawing the Zobrist keys retires the S170 case set; the key half leaves S179 and becomes S203
Tags:         hashing, testing, mate-pv, s179, s170, s171, s202, fixtures, dec-139, dec-142
Context:      S179 was written as two commits under one step: the magic numbers
              first, node-identical and provable, then the Zobrist keys, whose
              node counts move once and were to be recorded. The magics landed
              exactly as written -- 0 of 128 values shared with the tutorial set,
              128 of 128 reproduced from seed 20260904, every node count and best
              move identical, `GATE-DONE 24880255 (no functional change)`.
              The key half then turned `test_mate_carry` red, and not because it
              introduced a defect. `adocs/data/S170_cases.tsv` is six games
              **mined from S147's 3000-game run**: the class it guards is table
              eviction, and which entries evict which is precisely what the keys
              decide. Measured over four arbitrary seeds -- 20260904, 12345,
              999983, 777777777 -- **every one** breaks it, with 3, 3, 4 and 3 of
              the six cases going vacuous. The test's own message anticipates
              this state and says such a case "needs re-choosing, not deleting".
              Under 20260904 one case also publishes `mate 9` on a 5-ply PV,
              which is the class DEC-122 leaves short and visible and **S202**
              owns, not a wrong score: perft, `test_perft`, the hash-versus-full-
              recompute test and a Debug self-play were green, and the 851 keys
              are distinct with no three- or four-subset XORing to zero.
              So the key half does not invalidate the engine. It invalidates a
              fixture, and re-deriving that fixture is a fresh 3000-game match,
              a re-mining of the `Incomplete mating PV` cases and a re-measuring
              of the six `expected_mate_lines()` floors -- work S179's `accepts`
              never contemplated.
Decision:     By the owner, 2026-09-08, asked once the four-seed measurement was
              in hand. **Split.** S179 completes on its magics half, with its
              `accepts` amended to that scope and the key clauses moved out.
              **S203** owns the Zobrist redraw and owns re-mining the case set
              with it; `tools/magic_gen`'s `zobrist` mode, `project_random_next`
              and `CHESSO_PROJECT_SEED` ship in S179 and are already what S203
              needs, and until it lands the tool reports `matches init_zobrist:
              no`, which is the honest reading and the precondition S203 flips.
Rejected:     Finishing S179 whole by re-mining the fixture inside it. Refused
              because it makes one step own two changes with two different proofs
              and roughly two hours of match time that its cost section prices at
              seconds -- and "one change at a time" is the rule that makes either
              number mean anything.
              Landing the keys and recording `test_mate_carry` as knowingly red.
              Refused outright: every measurement taken afterwards would be taken
              over a suite that is not green, which is the contamination BUGS
              exists to prevent.
              Choosing a seed under which the fixture happens to survive. Refused
              because the four-seed measurement says no such seed is meaningful --
              the fixture is tied to a key set, not to a good or bad one, and
              picking for a green suite would be fitting the seed to the test.
Consequences: DEC-139's exposure stays open one step longer: the keys are still
              drawn through `std::uniform_int_distribution` over
              `std::mt19937_64`, whose result the standard leaves
              implementation-defined, so the node-count baselines this repository
              records remain a property of the standard library as well as of
              this code. `2026-09-04_test_review-F08` is therefore **not** closed
              by S179 and passes to S203.
              A second, more general consequence, and the reason this entry is
              longer than the choice: **`adocs/data/S170_cases.tsv` is a fixture
              whose validity depends on the Zobrist key set**, and nothing at its
              site said so. Any future change to the keys retires it the same
              way. It is named as such in the file and in
              `tests/test_mate_carry.cpp` from S179's completing commit, which is
              DEC-142's rule reaching a fixture that is not a number.

---

## DEC-155  2026-09-08  A run under four hours goes during the day; the night is for what is longer
Tags:         workflow, runs, machine, scheduling, dec-041, dec-061
Context:      RUNS said the agent "schedules anything lasting several hours for
              the night when there is better work to do meanwhile", which sets no
              threshold. In practice that turned into deferring runs the machine
              could simply have taken -- S203's mining run was written up as
              "budget a night, not a turn" on an estimate of about 1 h 20 m,
              which is not a night by any reading. Measurement capacity is the
              binding constraint on the whole plan (DEC-048), so an hour of
              daylight left unspent is a cost and not a courtesy.
Decision:     By the owner, 2026-09-08. **Four hours is the line.** A run
              expected to take less than four hours starts when it is ready,
              during the day, without asking. A run expected to take four hours
              or more is scheduled for the night if there is better work to do
              meanwhile. The estimate is stated before the run starts and comes
              from the measured throughput in `.moltke.local.md` -- 2277 games an
              hour at 8+0.08 with concurrency 12 -- and not from a guess.
Rejected:     Keeping "several hours" unquantified. Refused because it was read
              conservatively every time it was read, which is the expensive
              direction on a plan whose bottleneck is the machine.
              A shorter line, an hour or two. Refused because the machine is not
              wanted for anything else while a match runs -- MACHINE already says
              nothing else should be running during one -- so the only real cost
              of a daytime run is the owner's own use of the desktop, and four
              hours is where that starts to bite.
Consequences: RUNS in `AGENTS.md` carries the number. An estimate is now part of
              starting a run, which is a small discipline the WATCHERS rule wanted
              anyway: a watcher's ceiling is "at least 2x the expected run", and
              that phrase presumed an expectation nobody was required to write
              down.

---

## DEC-156  2026-09-08  The S170 budgets are re-swept by one stated rule, not re-mined; the grid is a knife edge and the file says so
Tags:         testing, mate-pv, fixtures, s203, s170, s202, dec-142, dec-154, measurement
Context:      DEC-154 sent the Zobrist redraw to S203 with the job of re-mining
              `adocs/data/S170_cases.tsv` from a fresh match. The match was run
              -- `ROUNDS=1500 ./fastchess.sh`, 3000 games at 8+0.08, 1 h 18 m 10 s,
              **0 time forfeits** -- and it produced **no case material at all**:
              3 `Incomplete mating PV` warnings over 2 distinct roots, **all of
              them from the old-keys reference and none from the candidate**.
              That is not evidence the redraw fixed anything; under an equal-rate
              null, 3 lines all landing on one side has probability 0.125. What
              it does say is that the class has become far rarer than when the
              set was built: S147 read 10 lines over 4 games in 3000, this run
              read 3 in total across both engines. A mining campaign for six
              fresh cases would be many hours with no guarantee of converging.
              The cheap alternative turned out to work. All three affected cases
              came back by re-sweeping their node budget alone -- no game, no
              match -- because a case goes vacuous when its budget no longer
              reaches the table state, not because the game stopped containing
              one. Re-sweeping is also what S170 did when it chose the budgets in
              the first place.
Decision:     By the owner, 2026-09-08, with the sweep table in hand.
              **Re-sweep, under one rule stated once and applied to every row:**
              the cheapest budget at which the case reports at least its floor of
              mate lines with all of them complete. `adocs/data/S203_case_sweep.sh`
              is that rule as a script, which is what DEC-142 requires beside a
              golden, and it re-derives the budgets and the floors both.
              A 300000 -> 1000000, C 1000000 -> 1500000, D 1000000 -> 4000000;
              B, E and F untouched. Floors: a re-swept row takes half the lines it
              reports, rounded down; a row whose budget did not move keeps the
              floor it was measured with. A rose 5 -> 6 and D rose 1 -> 3, so no
              live floor was lowered. F was `guard no` with a stale floor of 6
              against 4 reported, corrected to 2.
Rejected:     Mining fresh cases from matches. Refused on the run's own evidence:
              0 candidate cases in 1 h 18 m, against a fixture that needs six.
              Choosing each budget because it happened to be green. Refused
              because that is fitting the fixture to the test, the same move the
              seed choice was refused for in DEC-154 -- hence one rule, stated
              before it was applied, and the whole grid recorded rather than the
              winning row.
              Dropping D, whose only clean budget costs 7.9 s and sits past a wide
              red window. Refused because the window is information and the row is
              where it is recorded.
Consequences: **The grid is a knife edge and both files now say so** rather than
              implying it: `C_mate7_depth11` reports 13 mate lines at 1500000
              nodes and **0 at both 1000000 and 2000000**. The sweep is to be
              re-run after anything that moves the tree, not only after a key
              change, and a step that moves the tree and leaves this test green by
              luck has learned nothing. This is the fragility DEC-154 named in the
              abstract, now measured.
              **D carries an S202 reproduction and it is recorded, not cleared.**
              Between 1200000 and 3000000 nodes `D_mate_minus6_depth10` publishes
              `mate -6` at ply 35 depth 11 with a 10-of-12-ply PV, at a depth that
              also publishes a complete 12/12. That is the class DEC-122 leaves
              short and visible; it is cheaper than anything S202 currently has to
              work from, and S202's file carries it.
              The fast suite costs more: D at 4000000 nodes is about 7.9 s where
              its old budget was under 1 s.

## DEC-157  2026-09-08  S114 seeds its null-move cap from the midpoint with the measured bound named beside it, and S180 fixes the stale shipping values it rewrites
Tags:         seeds, dec-105, dec-134, s180, s114, s132, s184, search-params, workflow
Context:      S180's implementation guide closed with three questions its own
              `accepts` could not settle, and all three had to be answered before
              seven step files could be rewritten.
              **One.** `NULL_MOVE_EVAL_CAP` is a mate-safety cap, and DEC-105
              leaves only the range midpoint once the engine values are refused:
              8 of 0..16, which is near the *off* end for a term whose job is to
              stop the search reducing into a hidden mate. The alternative is a
              measured bound -- sweep the cap downward and take one below the
              largest value the two mate suites still pass at -- which is the
              third bound kind `src/search_params.hpp`'s own header defines and
              the kind `RfpMinPly`'s floor is. S180 runs nothing, so it can only
              write one of them down.
              **Two.** The F01 table of `adocs/audit/2026-09-04_plan_review.md`
              lists ten seeds; the seven sections hold about a dozen more
              engine-originated or formless ones. `accepts` binds "every seed"
              but its walk-row-by-row clause names only F01's, so the stamp's
              scope was ambiguous.
              **Three.** S114's "`NULL_MOVE_BASE` seed 2, ships today" has been
              wrong since S085 moved it to 3, and S132's `TM_NODE_MIN_DEPTH`
              "seeded beside `ASPIRATION_MIN_DEPTH`" quotes that gate's pre-S085
              value of 5 against the 2 that compiles. Both are S184's class (F05)
              and both sit inside sections S180 rewrites, and S184's `touches`
              names neither file.
Decision:     By the owner, 2026-09-08, before the rewrite started.
              **One: the midpoint, with the measured bound named beside it.**
              S114's section 4 seeds `NULL_MOVE_EVAL_CAP` at 8 and states in the
              same bullet that a midpoint is a poor seed for a safety cap, that
              the mate instruments decide the seed's admissibility before any
              sweep, and that S114 may take the measured bound instead and record
              which it used. A step file may name an alternative; it may not run
              one, and S180 does not.
              **Two: the stamp lists both, F01's ten rows first.** `accepts` is
              not amended -- it already says "every seed" -- and the inventory
              rows follow the F01 walk as a second list.
              **Three: S180 fixes them in passing**, stamps it, and S184's file is
              told so it does not go looking. A wrong shipping value inside a
              section being rewritten is cheaper to fix than to hand over.
Rejected:     Seeding `NULL_MOVE_EVAL_CAP` from the measured bound. Not refused on
              its merits -- it is the better seed -- but S180 measures nothing by
              its own `excludes`, and a step file that states a number it did not
              produce is the thing this whole step exists to stop.
              Amending S180's `accepts` to name the inventory rows. Refused as a
              step file edited to match the work rather than the reverse; "every
              seed" already covers them.
              Leaving the two stale values to S184. Refused because the sections
              are being rewritten anyway, and a rewrite that copies a wrong number
              forward launders it.
Consequences: S114's section 4 carries both the midpoint and the P4 procedure, and
              whichever S114 uses goes in its stamp. S184 no longer owns the two
              values named here and its file says so; the rest of its F05 class is
              untouched. Once S184 extends `tools/plan_prose_check.py --params` to
              `adocs/plan_todo/`, a shipping value quoted wrongly in a step file
              becomes a red fast test rather than an audit finding.

## DEC-158  2026-09-09  The reverse futility ceiling stays at 15 and the deep mates it loses are its measured price
Tags:         search, reverse-futility, mate-safety, sprt, s148, s085, s145, dec-019, dec-095, dec-116
Context:      S085's SPSA run moved `RFP_MAX_DEPTH` from S033's 6 to 15 and the
              vector holding it was verified at +21.02 Elo. S145 then built a set
              of proved mates and measured that the deep classes are found only
              when the ceiling is low, which made 15 a trade that had never been
              priced rather than a tuned value: the mate in two class is complete
              at every setting, so nothing in the gate before S145 could see it.
              The published record pointed the other way from S085 -- Stockfish
              removed both of its futility depth caps in July 2021 as
              Elo-neutral at STC and LTC and reverted them five weeks later on
              mate finding alone, 2427 mates falling to 1246 on ChestUCI at 1M
              nodes -- so the question was whether an Elo-neutral cap is what
              finds mates here too. DEC-019 says a published figure decides what
              to try and never what to conclude, so it was an SPRT.
Decision:     By the owner, 2026-09-08, pre-registered in
              `adocs/data/S148_sprt.sh` before a game was played; applied
              2026-09-09 on the run's word. **The default stays 15.**
              The challenger was chosen by a rule fixed before the grid was run,
              DEC-105 form (b): the largest ceiling at which both the mate in
              four and the mate in five exact counts over the 82-row set are
              non-zero. Over every value from 0 to 15 that is **4**
              (`adocs/data/S148_rfp_ceiling_sweep.log`), the mate in five class
              being a cliff -- 11, 9, 6, 4, 1 of 16 at ceilings 0 to 4 and 0
              from 5 up. At 4 the set reads 52 of 82 exact against 39 at 15,
              mates in three 18 of 24 against 12, deep classes 7 of 16 and 1 of
              16 against 1 and 0, and the mined breadth set 159 of 318 against
              145. `short` and `sign` are 0 at all sixteen settings.
              **4 lost.** `{-5, 0}` nElo, alpha = beta = 0.05, 8+0.08, Hash 16,
              UHO, `-repeat`, `-check-mate-pvs`: **H0 accepted at LLR -2.95,
              nElo -7.31 +/- 5.60, Elo -5.66 +/- 4.33 over 14808 games in
              6 h 19 m 35 s, 0 time forfeits** (`adocs/data/S148_sprt.log`). So
              a ceiling low enough to find the deep mates costs more than five
              nElo of ordinary play at this control, and the mates the engine
              does not find -- 1 of 16 at four, 0 of 16 at five -- are what the
              pruning costs and are recorded as such in `specs.md`, `MANUAL.md`,
              `DEV_MANUAL.md`, `src/search_params.hpp` and the mate suite's own
              comment. A verdict of "keep the incumbent" is a result and the
              step completes on it.
Rejected:     Shipping 4 for its mate property on the strength of the sweep. That
              is the argument DEC-019 exists to refuse, and the games refused it
              too: the sweep is a strength reading about 82 constructed
              positions and the SPRT is about 14808 games.
              A second verdict at C2 = 6. Pre-registration offered it as a
              fallback and the owner declined it on 2026-09-08, before the
              result was known: 6 is 5 of 16 and 0 of 16 on the deep classes,
              so it buys less than 4 did and would have to survive the same
              cost. A rejection ends the step.
              Promoting the mate in four count to an asserted floor. The
              accepts made it conditional on the shipped value being non-zero
              and it is -- 1 of 16 -- but a floor of 1 has no margin between its
              ends, which is what DEC-116 rejected for the mate in three. Both
              deep classes stay in the `MESSAGE`, with the comment saying which
              and why.
              Making the ceiling a function of the score, the shape Stockfish
              carries since `fa8b6add`. Excluded by S148 by name: it is a
              feature and not a constant, and it deserves its own step and its
              own verdict. Its constants are that engine's and are not quoted
              (DEC-134).
Consequences: `RFP_MAX_DEPTH` is a settled value and no longer an open question
              in any document; a future step that wants the deep mates has to
              find them without lowering this bound, and the number to beat is
              on record. The declared range is untouched at 0 to 63 (DEC-095)
              so the tuner still keeps every value. Two readings the coarse
              grids could not give are now on record and cost nothing to reuse:
              the plateau starts at **10**, not at 15 -- every count from 10 up
              is identical, so the shipping value confines nothing that three
              lower values do not also fail to confine -- and the mate in four
              class is 1 of 16 at the shipping value rather than the "0 of 8"
              S145 recorded, the set having grown to 82 rows at S168. The run
              also measured the `Incomplete mating PV` class from both sides:
              **7 lines from the candidate against 13 from the reference**, so
              the class is not this ceiling's and the lower ceiling produced
              fewer, which is a reading S202 inherits. And the verdict is one
              of the entries S151 re-takes at a control at least four times
              8+0.08 before its magnitude is banked -- though the magnitude
              here decides nothing, the incumbent having been kept.

## DEC-159  2026-09-09  `--citations` joins the fast suite, because the symbol form removed the reason it was out

Tags:         workflow, plan-hygiene, tests, gate, s187, s141, s150, dec-120, dec-135
Context:      `tools/plan_prose_check.py --citations` has been a manual check
              since it was written, and `tests/CMakeLists.txt` states the
              reason at `test_plan_touches`: "any source commit shifts lines
              under fifty step files at once, so gating on citation freshness
              would make red the normal state and this the check that gets
              weakened to clear it." That was true of the `path:line` form and
              measured -- S169 re-anchored 97 citations on 2026-09-01 and 59
              had drifted again three days and four source commits later. It
              is the reason `--touches` (S141) and `--params` (S150) are in the
              suite and this mode was not: neither of those reads a line
              number. DEC-135 then removed the line number from the citation
              itself, and S187 converted the 572 that existed, so the premise
              the exclusion rested on no longer holds and the question had to
              be re-asked rather than inherited.
Decision:     By the owner, 2026-09-09, answering S187's section 10. **The mode
              is registered in the fast label as
              `test_plan_citation_freshness`.** In the symbol form a commit
              that moves lines moves nothing the check reads; what turns it red
              is a renamed or deleted symbol with a pending step still citing
              it. That coupling -- a rename must fix every pending file citing
              the old name, in the same commit -- is the point of registering
              it and not a cost of doing so. Measured at 0.45 s over the 66
              pending files, median of five, against 6.3 s under the retired
              DRIFT class, which ran `git show` once per baseline-and-path
              pair. It skips itself with exit 0 outside a git checkout, as
              `--touches` does.
Rejected:     **An Amended line on DEC-135.** DEC-135 decided the form of a
              citation; this decides what the gate does about it, and the thing
              a future reader re-derives is why the 2026-08 exclusion stopped
              applying. That is an entry, not an amendment.
              **The CMake comment and `DEV_MANUAL.md` as the only record**,
              which is what S187's `accepts:` asked for. Both are written and
              both say why, but a comment in a build file is not where this
              repository keeps a decision, and the exclusion it reverses is
              quoted in three documents.
              **Leaving the mode manual.** It would have kept a check nobody
              runs at the moment it matters: the citation that goes stale is
              the one whose symbol was renamed by the commit that renamed it,
              and that commit is exactly when a manual check is not run.
Consequences: `tools/plan_prose_check.py --citations` is now part of the TESTS
              command's `ctest -L fast`, in both builds, and a red run there is
              a real finding rather than noise. A commit that renames or
              deletes a symbol cited by a pending step file must update that
              file in the same commit. `adocs/plan_todo/` and
              `adocs/plan_current/` are the gated set;
              `adocs/plan_done/` is history and stays out. `--prose` remains
              the one mode outside the suite, because which tense a sentence
              should take is a judgement and a `plan.md` rewrite would redden
              an unrelated step. The planted-case gate on the checker itself,
              `tests/test_plan_citations.py`, is separate and was registered by
              the same step: it is what keeps a recogniser that stopped firing
              from hiding behind a green real set.

## DEC-160  2026-09-09  S159 closes on its census: the ageing reading of S149's 11 Elo is refuted before a game, and no SPRT is spent on it
Tags:         search, move-ordering, killers, s149, s159, measurement, dec-019, dec-063, dec-099
Context:      S159 existed to test one reading of S149's verdict. S149 implemented
              CPW's killer-distinctness guard, measured **-11.02 +/- 10.53 Elo,
              H0 accepted over 2522 games**, and reverted it. S159's hypothesis --
              written as a hypothesis, which is why this was answerable -- was
              that the unguarded shift is incidentally an *ageing* mechanism, that
              the guard removed the ageing along with the duplication, and that
              the 11 Elo is the cost of the staleness the guard then preserved.
              The step's own order of work put the census before the games, "as
              S165 counted reachability first". It was run on 2026-09-09 over
              18166063 nodes, on HEAD and on HEAD with S149's guard re-applied to
              an instrumented copy, and it answers the question:

                                                        HEAD   S149's guard
                stores re-storing the move in slot 0    72.0 %       72.4 %
                nodes with both slots equal            44.0 %        0.0 %
                nodes offering a distinct slot 1       45.4 %       88.8 %
                ... of those, stale                     1.96 %       1.91 %

              **The stale share of distinct second-killer offers is flat**, 1.96 %
              against 1.91 %. The guard did not preserve proportionally staler
              killers; it raised the stale count only because it roughly doubled
              the offer count. The 11 Elo attaches to offering the second killer
              twice as often, not to its age. And on HEAD the unguarded shift is
              already an aggressive ageing mechanism -- it discards slot 1 on 72 %
              of stores -- so the candidate S159 built, clearing the table once
              per iteration, had **0.89 % of nodes** to act on.
Decision:     By the owner, 2026-09-09, with the census table in hand. **S159
              completes on the census and its `accepts:` is amended**: the clause
              requiring an ageing scheme decided by SPRT is met by a measurement
              that removed the reason to take one, and candidate A -- `killers_clear`
              called from the depth loop in `iterative_deepening_search` -- is
              reverted unrun. The killer table's lifetime stays one `go`. Candidate
              B is recorded neutral as the step predicted: node-identical and
              best-move-identical at depths 9 and 12 (INV-6), its only observable
              being the duplication fence in `tests/test_search.cpp` going red for
              a change that alters no game.
Rejected:     Spending the run anyway, `{-5, 5}` nElo, 10465 games and 4 h 36 m
              worst case at the measured 2277 games/h. It was pre-registered and
              ready, script and all. Refused on what the census had already
              priced: measurement capacity is the binding constraint on the plan,
              and a night on a mechanism measured at 0.89 % of nodes is a night
              not spent on the S109 block. **This is not a licence to argue a
              verdict.** DEC-019 stands unchanged and this entry does not weaken
              it: what was measured here is the *size of the mechanism*, cheaply
              and directly, not its Elo, and the census was written into the run
              script's header before the games precisely so it could not be
              reread afterwards as whatever the games happened to say. A census
              that had come out large would have bought the run, not replaced it.
              Substituting a second candidate -- the per-node ply+2 reset, or
              S149's guard combined with the clear. Refused on 2026-09-09 before
              the census, and the census removed the reason to revisit it.
              Keeping the clear on a null. Refused by the owner's rule of the
              same day: it implements no published rule and removes no unintended
              state, so an extra action with no measured return does not ship.
Consequences: **The killer heuristic is closed for now, on two measurements
              rather than one.** S149 measured the guard and S159 measured the
              mechanism behind the reading of it; the table keeps two slots, an
              unguarded shift and a per-`go` lifetime, and the fence in
              `tests/test_search.cpp` keeps the numbers where the next agent to
              notice the duplication will find them. What the census leaves open
              is a different question with a number on it: the second killer is
              offered on 45.4 % of nodes today and 88.8 % under the guard, and it
              is *presence*, not age, that the 11 Elo is attached to. No step is
              created for it -- S159's `excludes:` keeps the slot count at two and
              the reserve already holds S023 and S025 on this block -- and
              creating one is a decision, not a drift.
              **S149's instrumentation driver is not reproducible and this is how
              that was found.** Its 11 positions -- "startpos 13, kiwipete 13,
              lasker 18, promo-mess 12, 9bishops 14, kpk 22, perpetual 16,
              mate-QR 15, underpromo 14, tactical 13, checkfest 13" -- are named
              in `adocs/plan_done/S149_killer_slot_dedupe.md` and in
              `adocs/audit/2026-08-21_adversarial.md` and their FENs are recorded
              in neither, so nobody can reproduce 66.0 % / 44.4 %. S159's set is
              recorded, one in-repo source per row, with its driver, its
              instrumentation as a patch and its outputs, under `adocs/data/S159_*`.
              A census is evidence and evidence that cannot be re-run is an
              anecdote.

## DEC-161  2026-09-09  `test_mate_carry` fires on any change that moves the tree, and that is a defect in the guard rather than budget drift to be re-swept
Tags:         testing, mate-pv, fixtures, s203, s204, s159, dec-142, dec-156, bugs
Context:      S159's candidate turned `test_mate_carry` red against a green HEAD:
              four assertions over four cases, two incomplete mating PVs (S202's
              class) and two cases reporting no mate line at all, which
              `expected_mate_lines`'s own message calls "gone vacuous". The BUGS
              rule made the question -- engine or fixture? -- the next thing to
              answer, and it was answered with the script DEC-142 puts beside the
              golden: `adocs/data/S203_case_sweep.sh`, the whole grid, both sides,
              recorded as `adocs/data/S204_sweep_head.txt` and
              `adocs/data/S204_sweep_killer_iter_clear.txt`.
              Of nine stride-1 budgets, **`C_mate7_depth11` reports a mate line in
              exactly one cell at HEAD -- 1500000 nodes -- and that cell is its
              configured budget.** `B_mate6_shallow`'s budget is 100000, the
              lowest in the grid and the edge at which the case switches on. Under
              the candidate both spikes move. Short lines behave identically:
              they are scattered across the grid at HEAD -- A 2 and 2, B 5, 6, 5
              and 7, E 8, F 2 -- and merely miss the pinned budgets.
Decision:     By the owner, 2026-09-09. **This is a bug in the fixture and it gets
              a step, S204, before anything else starts** -- not a re-sweep, and
              not a reason to hold S159. S159's candidate was reverted for its own
              reasons (DEC-160), which returns the gate to green, so S204 is the
              next step rather than S159's blocker and no `paused_by:` is set.
Rejected:     Re-deriving the budgets and floors against S159's candidate, which
              is what DEC-156's consequences prescribe -- "the sweep is to be
              re-run after anything that moves the tree". Refused because on a
              grid this sparse that re-pins each golden to a fresh spike at every
              tree-moving change, and DEC-156's own `Rejected:` refuses precisely
              that -- "choosing each budget because it happened to be green ...
              fitting the fixture to the test". DEC-156's one-rule answer
              constrains *which* green cell is taken and does not stop the taking
              recurring, so the tension is real and belongs to S204.
              Deleting or relaxing the test. Refused by the TESTS rule and by the
              fixture's own message: a vacuous case needs re-choosing, not
              deleting.
              Running S159's SPRT with the gate red. Refused: a known defect in
              the tree contaminates every measurement taken after it, which is
              the whole of the BUGS rule.
Consequences: **DEC-156 is not void and is not amended here.** Its re-sweep
              prescription stands until S204 either upholds it with the tension
              resolved or replaces it, and S204's `accepts:` requires that in
              writing. What DEC-156 did not have is the count: it recorded that
              the grid is a knife edge; this records that one case's green is a
              single cell in nine, which is why "re-sweep after every tree change"
              cannot be the whole answer.
              **A guard that fires on every tree-moving change trains agents to
              re-pin it.** S148's stamp already establishes the same test going
              red on its candidate and being read as budget calibration; that
              reading was right and is the second instance. A third would be a
              habit, which is what S204 exists to prevent.
              The mate guards written for the recurring "pruning that hides a
              mate" failure -- `tests/test_engine.cpp` `"engine: mate safety"` and
              `tests/test_search.cpp`'s mate cases -- were green on the same
              candidate, 30 of 31 fast tests passing. They are not implicated and
              S204 does not touch them.


## DEC-162  2026-09-09  `test_mate_carry` guards a fixture-wide majority and a per-case short-line ceiling, and stops asserting a per-case floor at a pinned cell
Tags:         testing, mate-pv, fixtures, s204, s202, s170, dec-122, dec-142, dec-156, dec-161
Context:      DEC-161 recorded the defect and sent the shape to S204. The count
              S204 took from the two recorded grids says what has to change.
              Per case, over the nine stride-1 budgets, cells reporting at least
              one mate line: A 8/9 both sides, B 9/9 then 8/9, **C 1/9 at HEAD
              and 3/9 under S159's clear, with the pinned budget being that one
              cell**, D 4/9 then 2/9, E 7/9 then 6/9. The union over the nine is
              non-zero for every case on both sides -- C is 13 lines against 48
              -- so **no case lost its mate; the cell holding it moved.**
              The short-line half is the same. Across the stride-1 grid at HEAD
              short lines appear in 11 cells and merely miss the pinned budgets,
              and the rule that chose those budgets -- "the cheapest budget at
              which the case reports at least its floor of mate lines with all
              of them complete" (DEC-156) -- is what selected for that.
              The part neither DEC-156 nor DEC-161 had is the class split.
              `adocs/data/S204_class_census.py`, over the five guarded cases at
              nine budgets, **428 mate lines, 39 short, 0 that run the claimed
              distance and fail to be checkmate** -- and its mate counts
              reproduce `adocs/data/S204_sweep_head.txt` cell for cell. So one
              of the two things the fixture merged into a single failure list is
              budget-independent and the other is not.
              Cost bounds the answer: the fixture is 26.30 s over 188 M nodes
              of budget, and a per-case union over all nine budgets is 1863 M --
              **9.9x the fixture, about 4.3 minutes** -- which is not a
              fast-suite test.
              *Corrected 2026-09-09, hours after this entry was written, by the
              Tier-1 check over S204's diff.* It read "13.5x that", which is not
              a ratio of the fixture at all: 13.5 M is the sum of the nine
              budgets, the multiplier for one search of a case whose own budget
              is 1 M, and it is 135x for `B_mate6_shallow` at 100000 and 3.4x for
              `D_mate_minus6_depth10` at 4000000. The 4.3 minutes was right and
              is what the rejection rests on; the ratio beside it was wrong.
              `adocs/plan_done/S204_mate_carry_knife_edge_guard.md` carries the
              same wrong figure and is not edited -- `plan_done/` is history.
Decision:     By the owner, 2026-09-09, on the agent's proposal, with the count
              and the census in hand. **The budgets do not move.** The guard
              becomes three assertions:
              **1. The mate-reaching invariant, asserted at zero and pinned to
              nothing.** A published mate line whose `pv` is at least the
              distance the score claims must be checkmate at exactly that
              distance. This is DEC-122's own guarantee, it held in 428 of 428
              lines across the grid, and no budget is chosen to make it true.
              **2. Vacuity becomes fixture-wide: a majority of the guarded cases
              -- 3 of 5 -- must report a mate line.** The per-case
              `expected_mate_lines` floors are deleted. One cell of a sparse
              grid is one sample and not a property the engine has, which is
              exactly what C at 1/9 measures; the majority is a property, and it
              is a rule stated once over the set rather than a number per case.
              **3. The short-line residue is bounded per case, not asserted
              zero.** The ceiling is the largest short-line count any cell of
              the recorded grid shows for that case on either side: A 5, B 11,
              C 0, D 1, E 8. C's zero is earned -- 0 short in all 18 of its
              cells -- and is not a budget choice. The ceilings are re-derived by
              `adocs/data/S203_case_sweep.sh`, which is what DEC-142 requires.
Rejected:     A per-case union of budgets for the vacuity floor. It is the
              robust shape and it was refused on measured cost: 4.3 minutes
              against 26.30 s, 9.9x, in a suite that gates every commit. It belongs in
              `tools/gate_extra.sh` if S197 ever wants it.
              A contiguous green window per case, which DEC-161's shape list
              offered. Dead on the data rather than on cost: C has no window
              wider than one cell on either side -- {1500000} at HEAD and
              {1000000, 2000000, 4000000} under the clear.
              Dropping the count entirely and asserting only completeness.
              Refused because until a differently-sourced vacuity guard exists,
              a fixture that sees nothing at all passes.
              Keeping `short == 0` at the pinned budget. Refused because it
              leaves half the defect standing: two of S159's four reds were
              short lines landing on a pinned cell, and the budget stays chosen
              to dodge a residue DEC-122 says is expected.
Consequences: **DEC-156 is amended here, not upheld.** Its re-sweep
              prescription -- "the sweep is to be re-run after anything that
              moves the tree" -- stands for the *budgets*, which is what it was
              written about and which nothing here moves. It no longer applies
              to the floors, because there are none: a tree-moving change that
              silences one case is now green by design, and only a change that
              silences three of five is red. The rule that chose the budgets
              (DEC-156) also selected cells with no short lines; that selection
              pressure is removed by assertion 3, so a future re-sweep is
              choosing on the mate count alone.
              **The guard is deliberately looser than it was, in one direction
              only.** It cannot notice one case going quiet or a short line
              appearing under a case's grid ceiling. It gains what it did not
              have: it fires on a change that stops mates being found across the
              set, and it fires on a published line that does not reach its
              mate at *any* budget rather than at one. Both halves were shown
              red on a mutant before this was called done, which is the second
              tier of DEC-141 applied to a fixture.
              **S202's class is bounded here and still owned there.** The
              ceilings are the size of the residue as measured, and closing S202
              is what allows them to go to zero. A step that lowers one is
              recording progress; a step that raises one is relaxing a test and
              needs a decision.

## DEC-163  2026-09-09  S193's accepts is amended before the step starts: the R2 clause names the mechanism that works, and R13 to R17 join the enumeration
Tags:         testing, plan, s193, vacuity, dec-139, dec-141, dec-142
Context:      Two things in S193's `accepts:` did not survive contact with the
              code, and its enrichment guide (2026-09-05) put both to the owner
              rather than letting the implementer decide.
              **The R2 clause names a mechanism that cannot do what it says.**
              The accepts asked for `ucinewgame` before `tests/test_engine.cpp`'s
              "a search with no limit is still bounded", to clear the stop flag
              `position` leaves set. At HEAD `command_ucinewgame` in
              `src/chesso.cpp` calls `stop_and_join_search()`, which *sets*
              `stop_search_signal`; the only callers of `begin_search_session()`,
              which clears it, are `command_go` and `command_test`, and that
              function is declared in no header, so a test cannot call it. The
              clause as written would be satisfied by an edit that leaves the
              case exactly as vacuous as the review found it.
              **R13 to R17 and three temp-file names are in the goal and not in
              the accepts.** The 2026-09-04 test review's F05 list has five more
              vacuous assertions than the accepts enumerates -- a history of one
              entry, a bound true by an earlier clamp, a literal compared with
              itself, a depth floor no writer can go below, a `make_move` that
              refuses nothing -- and F09's temp-file family includes
              `tests/test_corpus_hash.cpp`, which the review's own enumeration
              missed. The goal's words ("vacuous assertions are made
              falsifiable", "temp-file hazards removed") cover all of them; the
              accepts does not.
Decision:     By the owner, 2026-09-09, on the three questions in S193 section 10.
              1. The R2 clause becomes "the stop flag cleared through a completed
              `go depth 1` before the case, and the case shown to spend at least
              half its 200 ms budget". The `ucinewgame` wording is dropped rather
              than kept and deviated from.
              2. R13 to R17 and `tests/test_corpus_hash.cpp`'s three fixture
              names are in scope; the accepts gains a clause naming them.
              3. R7 takes the asserting form, not the rewording one:
              `legal_moves()` asserts `position_is_reachable` after `make_move`,
              which is the property its comment already claims. Subject to trap
              T7 -- if a Debug binary approaches its CMake timeout the fallback
              wording is taken instead and the stamp says so.
Rejected:     Keeping the accepts verbatim and recording a deviation in the stamp
              (question 1's alternative). Refused because a stamp note does not
              stop the next reader from re-deriving the wrong mechanism, and the
              accepts is what a later audit reads.
              Deferring R13 to R17 to a follow-up step. Refused on cost: each is
              minutes, none touches `src/`, and a second step over the same files
              pays the gate twice for the same class of defect.
              Rewording `legal_moves()`' comment instead of asserting it.
              Refused because the perft and JSON counts do pin legality but do
              not pin it *at this helper*, which is what its call sites read the
              comment as promising.
Consequences: S193's `accepts:` carries the amended R2 clause and an R13-to-R17
              clause, both marked with this id. The step's own guide keeps its
              section 10 as the record of what was asked.
              R7's cost is now a step obligation: the Debug `test_movegen` and
              `test_search` timings are taken after the change and reported in
              the stamp against their 600 s ceilings, and taking the fallback is
              a stamped outcome rather than a silent one.
              `2026-09-04_adversarial-F01` reads `Status: open` at `fbffd36`, so
              R12 stays unregistered under the accepts' own condition; nothing
              here changes that.


## DEC-164  2026-09-09  A guard's firing is observed through a write-only probe the engine's own instantiation does not contain
Tags:         testing, search, s191, dec-141, dec-083, performance
Context:      S191 had to make thirteen guards on null move pruning, reverse
              futility and late move reduction directly testable. Its enrichment
              guide offered two observables and put the choice to the owner.
              The **transposition-table** one infers each decision from what the
              node left in the table -- an entry at the passed position means a
              null move was made, `explored_nodes == 1` means reverse futility
              returned, `entry->depth` says what depth a move was searched at.
              It touches no `src/` file, so it owes no Bench line, no INV-6, no
              timing and no Debug self-play. It is also indirect: every reading
              depends on the table's replacement rule, and the guide's own
              section 5 lists five ways a case could go green or red for a
              reason that is not the guard.
              The **counter** one records the decisions themselves. Direct, and
              it puts `src/search.cpp` in `touches:` with everything that
              follows.
Decision:     By the owner, 2026-09-09: the counter. And by the agent, on the
              measurement that route then produced: the probe is compile-time,
              not a runtime pointer test. `search_node_probe_t` records one
              node's decisions -- whether the null move was made, whether
              reverse futility returned, and per legal move the reduction it was
              first searched with and whether it was re-searched -- and nothing
              in the search reads a field of it back. `negamax` became
              `negamax_at<bool PROBING>`, instantiated `<false>` for the engine
              and `<true>` only at the node `negamax_probed()` drives, with the
              recursion always `<false>`. That last is exact rather than an
              approximation: a probe names one ply, and every child of the
              driven node is at another one.
              The owner also accepted the guide's four smaller proposals in one
              block: a seventh null-move case for M04, a second tracked mutant
              file for the six guards the append-only 2026-09-04 file does not
              cover, the positive reverse-futility band edge recorded as inert
              rather than written as a case that cannot meet its own
              precondition, and "takes no null-move cutoff" asserted as the
              stronger "makes no null move".
Rejected:     **A runtime pointer test, which is what the first implementation
              was.** Resolving `state->probe` once per interior node and testing
              it once per move measured **1.49 % fewer nodes per second, sd
              0.66 % over 13 interleaved paired `chesso bench` runs** -- past
              this machine's noise floor, and moving the field beside the ones
              every node already touches did not recover it. A permanent 1.5 %
              of engine speed for observability the shipping binary never uses
              is not a trade worth making. Note the instrument: hyperfine's
              block layout read 1.01x in both directions with sigmas too tight
              to believe across drift, and two runs of the same binary differed
              by 0.9 %; the paired interleaved form is what resolved it, and it
              is what CLAUDE.md's rule 5 asks for.
              **`#ifdef CHESSO_TUNE` around the probe**, which is where
              `search_lmr_reduction_probe` lived. Free in the shipping build and
              the cases would then run in `build-tune` only -- the guards the
              gate ships would have no direct test, which is the whole finding
              (2026-09-04_test_review-F02) restated one build over. The same
              reasoning is why that function stopped being tune-only here: a
              case asserting a guard refused to reduce says nothing unless the
              reduction table would have reduced, and that has to be checkable
              in both gated builds.
              **The transposition-table observable.** Free, and the owner
              declined it for directness.
Consequences: `negamax` is a template with two instantiations, and a future
              reader asking why finds this entry. The engine's instantiation
              contains no probe code: INV-6 is identical at depths 9 and 12,
              `chesso bench` is the parent's **26851183**, and the completing
              commit carries `No functional change`. Re-measured after the
              change, **0.14 % +/- 0.24 % at 95 % over 33 pairs**, which is
              nothing this machine can resolve.
              A guard added later -- S109's four rules first -- is observed the
              same way: add a field, write it under `if constexpr (PROBING)`,
              and drive the node with `negamax_probed()`. Adding a runtime test
              to `negamax_at<false>` instead is the thing this entry forbids.
              `adocs/data/S191_mutants.py` joins
              `adocs/data/2026-09-04_test_review/mutants.py` as tracked
              evidence; S196 folds both into `tools/mutants/`.


## DEC-165  2026-09-10  A mutation run's `(Timeout)` is `unmeasured` only when the ceiling is the whole evidence; beside a `(Failed)` it is a kill
Tags:         testing, tooling, s196, dec-141, mutation
Context:      S196's guide wrote one rule for a ctest ceiling: "Any `(Timeout)`
              row: `unmeasured`, not a kill." The trap it was written against is
              real -- Release ceilings are 60 s per test, a busy machine hits
              them, and a naive parser reads the resulting non-zero exit as the
              suite detecting the mutant when nothing asserted anything.
              The first full pass produced the case the rule did not
              anticipate. `M22_castling_rights_on_capture` leaves
              `test_uci_surface` still running at its 60.07 s ceiling -- the
              same test passes in **9.26 s on the unmutated worktree in the same
              run** -- while `test_chesso`, `test_movegen`, `test_engine` and
              `test_invariants` fail on assertions, `test_invariants` on
              `after make e8g8 castling ... material expected -880 got -85`.
              Four binaries caught the mutant and the tool called the row
              `unmeasured`. It is not one row: `M31_lazy_bound_no_margin` hangs
              the same test in the same pass with `test_search` and
              `test_engine` red beside it, so the rule cost two of forty
              mutants on its first outing.
              The re-run the rule prescribes does not recover it. The hang is
              the mutant's own behaviour, not the machine's load, so a quiet
              machine reproduces it exactly and the row is `unmeasured`
              for good -- a kill hidden by the rule meant to protect the score.
Decision:     By the owner, 2026-09-10: narrow it. `unmeasured` when the run's
              only failing rows are `(Timeout)`, or when the build hits its
              ceiling; a `(Timeout)` beside any `(Failed)` is `killed`. In
              `tools/mutation_check.py` the ctest reader reports `asserted` --
              a row that failed for any reason other than its ceiling -- and
              `inconclusive`, which is the run saying nothing: this tool's own
              ceiling over the whole of ctest, or a non-zero exit with no
              failing row parsed, which is ctest failing to run rather than a
              test failing.
              The rule generalises past the ceiling, and the same reading of the
              evidence settles the other two places it could bite. `killed` is
              now decided before `unmeasured`, so **an assertion that fired is a
              detection whatever else in the run was inconclusive** -- a mutant
              that leaves the engine unable to print a bench line reports its
              kill instead of hiding behind the unreadable signature. And a
              non-zero ctest that ran nothing is `unmeasured` rather than a
              kill, which is the naive-parser trap in its other form.
Rejected:     **Keeping the original rule.** M22 then reads `unmeasured` on
              every pass, the run exits non-zero by design, and the score is a
              mutant short with no way to earn it back.
              **Trusting the exit code alone**, which is what the rule was
              written against: ctest exits non-zero on a bare ceiling, and a
              busy machine would then manufacture kills.
              **Raising `test_uci_surface`'s ceiling** so the hang becomes a
              plain failure. It treats one mutant's symptom, leaves the rule
              wrong for the next one, and slows a red run by a minute.
Consequences: `tests/test_mutation_check.py` holds both halves as cases --
              a stub ceiling alone reads `unmeasured` and its verdict differs
              from `expected`, a ceiling beside a real red reads `killed` and
              carries the failing assertion, an engine that stops printing a
              bench line still reports its kill, and a ctest that exits non-zero
              having run nothing reads `unmeasured`. All four were observed red
              against the tool's own earlier semantics before the fix landed.
              A row that is genuinely the machine's still reads `unmeasured`
              and is still re-run with `--only` on a quiet machine, which is
              why no pass runs beside a match (MACHINE rule).
              DEV_MANUAL.md "Mutation check" states the amended table, and
              S196's guide is amended in the two places that carried the old
              rule rather than silently overtaken.

## DEC-166  2026-09-10  The full mutation pass stays on demand and out of the weekly gate; its table is evidence under `adocs/data/`
Tags:         testing, tooling, s196, s197, dec-141, workflow
Context:      S196's section 10 deferred two questions to the owner, and S197's
              section 10 asks the first of them again from the other side.
              **Whether `tools/gate_extra.sh` runs a full pass weekly.** A pass
              is forty builds and forty runs of the fast suite. DEC-141 clause 3
              names the Debug binaries, a sanitizer build, deep perft and the
              prose checks in that script and does not name this; S197's
              `accepts:` does not list it either. `adocs/testing_strategy.md` R9
              asks for a pass "after any change to `tests/` that adds or removes
              coverage", which is a trigger and not a schedule.
              **Where the kill table lives.** The stamp always carries it; the
              question is whether the rows are also written where the next pass
              can diff against them.
Decision:     By the owner, 2026-09-10: **on demand only** -- S197 does not call
              the full pass, its `accepts:` needs no amendment, and R9's trigger
              is what runs it. And **both** -- the stamp carries the table, and
              `adocs/data/S196_full_pass.tsv` carries the machine-readable rows
              with a line in `adocs/data/README.md`, so the pass after the next
              change to `tests/` has a baseline to diff rather than a paragraph
              to read.
Rejected:     **A weekly full pass in `gate_extra`.** About an hour added to a
              weekly run whose value is the whole picture in one place, for a
              measurement whose input -- the mutant list and the suite -- moves
              only when somebody changes one of them, and who then runs it.
              **The stamp alone.** A table in prose is not something the next
              pass can diff; the point of a kill rate is the second reading.
Consequences: `adocs/data/` joined S196's `touches:`, as did
              `tests/test_mutation_check.py` and `tests/CMakeLists.txt` for the
              self-test the same section proposed and the owner took.
              The 2026-09-04 review's evidence directory is unchanged and stays
              unchanged: `adocs/data/README.md`'s rule is added, never edited,
              and the live mutant list is the copy under `tools/mutants/`.
              S197's section 10 question 3 is answered here; an implementer who
              reads it needs no new interview.

## DEC-167  2026-09-10  The extra gate asserts the sanitizer `bench` total against the Release one; the coverage recipe stays a documented command
Tags:         testing, tooling, s197, dec-141, dec-025, dec-166, inv-6, sanitizers, coverage
Context:      S197's implementation guide deferred six questions to the owner
              before `tools/gate_extra.sh` was written. Question 3 -- whether
              S196's full mutation pass joins the weekly run -- was already
              answered by DEC-166 (no). The other five were open at the moment
              the step started, and two of them change what the step builds.
              **The sanitizer's blind spot.** ASan and UBSan report what they
              instrument. A read of uninitialised memory that is neither out of
              bounds nor undefined by their definition changes the search tree
              and is reported by neither; the engine's own `bench` total is a
              27-million-node signature over twelve positions that such a read
              moves. The two builds differ only in instrumentation, so the
              totals are equal or something is wrong.
              **The coverage comparison.** The 2026-09-04 test review's
              `coverage_unexecuted.txt` was produced with Apple clang and
              `-march=native`; every commit since `5cffb70` has shifted its
              line numbers, so comparing it against a fresh run is by-eye work
              that a small stdlib script over `llvm-cov export -format=text`
              could make mechanical.
Decision:     By the owner, 2026-09-10, answering S197 section 10:
              **(1) assert.** Stage 4 compares the sanitizer build's `bench`
              total with `build/src/chesso bench`'s and fails the stage on a
              difference. `adocs/specs.md`'s INV-6 row gains the clause, so
              `adocs/specs.md` joins S197's `touches:`.
              **(2) six binaries.** The Debug stage runs `test_chesso`,
              `test_openings`, `test_movegen`, `test_evaluation`, `test_search`
              and `test_engine`. The three mate binaries stay out.
              **(4) no script.** `DEV_MANUAL.md` documents the `llvm-cov`
              recipe as an on-demand command and the comparison stays by eye;
              `tools/coverage_unexecuted.py` is not written, here or as a
              step of its own.
              **(5) the flags go in.** The `SANITIZER` block gains
              `-fno-sanitize-recover=undefined` and `-fno-omit-frame-pointer`,
              with the section 6 (a) red-first observed and quoted in the stamp.
              **(6) its own bullet.** `status.md` carries
              `Extra gate: last GATE-EXTRA-DONE <date> <sha> <mm:ss>` as a
              bullet of its own, not a clause inside `Watching:`.
Rejected:     **Recording both bench totals in the stamp without asserting.**
              Stays inside the `accepts:` as written and touches no spec, but a
              future divergence is then visible only to whoever reads that
              run's log -- which is the class of miss this step exists to stop.
              **The three mate binaries in the Debug stage.** They drive
              `make_move` under the same asserts, but `test_mate_breadth` alone
              is about 697 s in Debug (`tests/CMakeLists.txt`), roughly
              tripling a stage estimated at 7 minutes, for assertions the six
              already exercise on every make and unmake.
              **`tools/coverage_unexecuted.py` in this step.** Widens
              `touches:` and owes a test of its own, for a comparison made a
              few times a year against a baseline taken on a different
              compiler.
              **`tools/coverage_unexecuted.py` as a new step.** Same work,
              deferred; the owner declined the capability rather than its
              timing, so a step in `plan_todo/` would be a placeholder for a
              decision already made.
              **A clause inside `Watching:`.** A missed week is then invisible
              whenever `Watching:` reads "nothing", which is exactly when the
              cadence is most likely to have lapsed.
Consequences: `adocs/specs.md` joins S197's `touches:` for the INV-6 row alone;
              no other spec moves and no UCI surface changes. The `SANITIZER`
              option means something stricter after this: a UBSan report is a
              non-zero exit rather than a line on a green log. No shipping or
              measured binary is built with the option, so no recorded figure
              moves. The stage-4 assertion is a third reading of INV-6, beside
              `tools/gate.sh`'s signature check and `tools/search_bench.py`'s
              per-position counts -- the first that compares two *builds* of
              the same commit rather than two commits.

## DEC-168  2026-09-10  S192's three open questions: form A for the drop half, two named constants for the 563/567 pair, and the M06a guard decided by measurement
Tags:         testing, goldens, time management, mutation
Context:      S192 reached the owner with five section-10 questions and two of
              them had been answered by events. **Question 1, row 6's margin,**
              asked whether the five `test_mate_carry` per-case floors should
              stay at the observed count or drop to `max(1, count - 1)`. S204
              deleted those floors on 2026-09-09 (DEC-162): what the fixture
              guards now is a per-case *ceiling* on short mating PVs, derived by
              `adocs/data/S203_case_sweep.sh --ceilings` from two recorded
              grids, plus a fixture-wide majority with no per-case number in it.
              There is no floor left to set a margin on. **Question 5,** the
              stale `status.md` Parked note and F03's "gitignored" premise, is a
              coordinator edit rather than a decision: `.tuning/anchors.py` has
              been tracked since `c56ab41`.
              The other three change what the step writes.
Decision:     By the owner, 2026-09-10, answering S192 section 10:
              **(3) form A, no `src/` hook.** The soft-limit case is rebuilt on
              a root with exactly one mate in one, where the loop counts a
              stability of exactly `depth - 1` and a fall of exactly 0 whatever
              the pruning rules do deeper. The loop's two update rules are *not*
              extracted into a pure function for the test to feed, so `src/`
              does not move, no INV-6 run and no `Bench:` trailer is owed, and
              DEC-141's Debug self-play tier does not arm. What the loop owes
              for a non-zero fall stays the identity
              `scale == search_time_scale_percent(stability, drop)`, asserted at
              whatever fall the tree produces, with no precondition on the
              number; the arithmetic of the fall is held by the pure case
              "the time scale moves with stability and with a falling score",
              which runs no search.
              **(4) two named constants.** `QUIET_ROOK_EVAL` and
              `QUIET_ROOK_EVAL_CHEAP` at file scope in `tests/test_search.cpp`,
              one golden comment over the pair, eleven sites reading them. A
              refit edits two lines instead of eleven.
              **(2) measure, then decide.** Whether the reverse-futility ply
              floor needs a guard case of its own is settled by re-running M06a
              against the replaced suite, not by argument.
Rejected:     **Extracting the loop's update rules into `src/`.** It is the only
              way to feed the loop a constructed *non-zero* fall, and the
              extraction is outside any hot path, but it turns a tests-only step
              into one that owes INV-6, a bench signature and a self-play tier
              for a single assertion whose arithmetic is already held by a pure
              case.
              **A comment at each of the nine 563/567 sites.** The literal
              reading of the `accepts:`; it leaves a refit editing nine numbers
              and nine comments, which is the re-derivation-under-pressure the
              step exists to remove.
              **Adding the ply-floor guard unconditionally.** Completes S191's
              guard set by construction, but writes a case before knowing
              whether anything is uncovered.
              **Accepting a single golden detector for the ply floor.** Would
              have been recorded as a finding rather than fixed, and the BUGS
              rule's spirit is against carrying a known gap forward when the
              measurement that resolves it costs one mutation run.
Consequences: `tests/test_engine.cpp`'s soft-limit case stops asserting on a
              fixed position's tree, so the eight search mutants that reddened
              it for no defect stop doing so -- and M06a loses that detector,
              which is why (2) is measured before the step completes. `src/` is
              untouched by S192, so no measured figure moves and no SPRT is
              owed. The pair `QUIET_ROOK_EVAL` / `QUIET_ROOK_EVAL_CHEAP` becomes
              the single edit point for the next refit's static anchors, beside
              `adocs/data/S192_anchors.py` which derives them.

---

## DEC-169  2026-09-10  The truncation-bound reading is re-taken when LAZY_EVAL_MARGIN moves, not only after a refit
Tags:         evaluation, tuning, goldens, testing, build
Context:      DEC-057 says a truncation residual belongs to the weights and not
              to the position, so `tests/test_eval_model.cpp`'s four pinned
              positions are re-chosen at every refit and at no other time.
              S206 bisected a drift in the instrument that chooses them --
              `truncation_scan` read 135399 / 99 / 30 at S076 and 138331 / 105 /
              33 on 2026-09-10, over the same corpus, with every non-zero entry
              of the model's starting vector identical at the two shas -- and
              found two movers, neither of them a weight and neither of them a
              defect. **`21b4a21`, S085's SPSA vector, is all of 99 -> 105 and
              30 -> 33**: it raised `LAZY_EVAL_MARGIN` from 150 to 184, and both
              `eval_model::evaluate` and `evaluate()` clamp the tapered
              mobility-plus-king-safety sum at that margin, so wherever the
              clamp binds the two agree exactly and the taper's truncation
              residual is not there to be measured. Unclamping restores it.
              HEAD with the margin at 150 reads its parent's 134408 / 99 / 30 to
              the row. **`883c255`, S104's `CHESSO_ARCH=native`, moved the 2.0
              column alone by -991**: `-march=native` lets GCC contract the
              model's `mobility[t] * params[...] + sum` into an FMA and the
              model's double moves by an ulp, which only a threshold rows sit
              exactly on can see -- a residual is a multiple of 1/24 and
              2.0 = 48/24, where 2.8 is not. The same commit with
              `-ffp-contract=off` reads 135399 again.
Decision:     By the agent, 2026-09-10, on the measurement, since it names a
              second trigger rather than choosing between options. **The four
              pinned positions are re-derived after a refit *and* after any
              change to `LAZY_EVAL_MARGIN`**, which is what S039 exists to do,
              and the `GOLDEN (DEC-142)` note in `tests/test_eval_model.cpp`
              says so at its site. Both movers are classified **(a)**, a
              deliberate change whose effect on this reading is correct: the
              margin is a search parameter that legitimately governs where the
              two implementations are forced to agree, and the arch flag makes
              the model's arithmetic no less right than it was. Neither is a
              bug and the BUGS rule does not arm. The counts recorded in
              `DEV_MANUAL.md` and at the golden's site are HEAD's.
Rejected:     **Calling the FMA a defect and building the tools with
              `-ffp-contract=off`.** It would pin the 2.0 column against build
              flags for no gain: the column is a diagnostic, nothing asserts it,
              and the assertions that do exist keep 0.075 between themselves and
              a rounding wobble -- the tolerance is 3 against a worst of 2.875
              and the bound clauses are `> 2.0` and `> 2.8`. Making the
              measurement flag-dependent to protect a number nothing reads is
              the wrong trade, and `-ffp-contract=off` on the engine would cost
              speed on the path that matters.
              **Re-pinning the four positions now.** They still qualify at
              HEAD's weights, and re-choosing them belongs to a refit (DEC-057),
              which is S126's business; S206's `excludes:` says so.
              **Leaving the second trigger undocumented and letting S039 find
              it.** S039 moves the margin by design, and an instrument whose
              reading moves for an unnamed reason is what this step was opened
              on in the first place.
Consequences: S039 re-decides `LAZY_EVAL_MARGIN` and owes a fresh
              `truncation_scan` reading with it, recorded beside the new margin;
              so does any SPSA run that includes the margin among its axes.
              `adocs/data/S206_truncation_drift.sh` re-derives the six readings
              and the two counterfactuals from clean worktrees and is the
              evidence. A timing or count taken from `build/` is on
              `-march=native` and the model's float arithmetic is contracted
              there, which is a difference from a `portable` build that shows up
              only at exact thresholds.

## DEC-170  2026-09-11  The 2026-09-10 adversarial audit is digested: eight steps, four rulings, and the low findings folded into the files that own them
Tags:         audit, plan, correctness, originality, measurement, tools
Context:      `adocs/audit/2026-09-10_adversarial.md`: 37 findings over the two
              founding rules, five high. Three are defects the engine ships --
              F08 a pre-root repetition scored as a dead draw in ordinary play,
              F09 a stack overflow and F10 an out-of-bounds table index, both
              from `position fen` -- and two are breaches of the founding rules:
              F01 three tables copied from a GPL-3.0 tutorial engine, F04
              one-sided resign adjudication in both harnesses under a comment
              claiming the opposite. Part D found no measured Elo added in
              nineteen days (F14) and no record of parallel search anywhere
              (F15). Part E holds 21 low findings. The owner asked on
              2026-09-11 that the plan be updated with the report and re-sorted
              for the 3000 mark, and delegated every engine-related question to
              the agent: the owner is asked only when the workstation is at
              risk, the development's ethic would change, or a choice is not in
              line with the goal and its restrictions. This entry and DEC-171
              to DEC-175 are those settlements, proposed and recorded by the
              agent and open to the owner's veto on reading.
Decision:     By the agent under that delegation, 2026-09-11. **Eight steps,
              each closing the findings it names:**
              S207 -- F08, the repetition rule (DEC-173), one `--nonreg` SPRT;
              S208 -- F09 and F10, a third refused class at the load boundary,
              node-identical;
              S209 -- F11, F12, F13, the UCI option surface, node-identical;
              S210 -- F17 to F23 as one batch, plus the still-open
              `2026-09-04_adversarial-F01` that F19 re-triggers, an SPRT only
              if F22's census finds reach;
              S211 -- F01, F02, F03, the originality remediation, bench-identical;
              S212 -- F04, F05, F06, F07, F31, F32, the harness (DEC-174),
              closed by a fixed-rounds A/A;
              S213 -- F26, F27, F33, comments and dead API;
              S214 -- F28, F29, F35, the tools.
              **Four rulings without a step:** F14 is answered by the reorder,
              DEC-172; F15 by DEC-175; F34 -- the tuner has no regularisation
              -- is decided *against* regularisation: the two exact
              degeneracies S100 proved are removed structurally by S134, which
              is the better answer to a ridge than a penalty that biases every
              other column, and the question reopens only if S126's fit shows a
              second ridge; F23's behavioural half stays accepted as
              `2026-08-14_test_review-F05` and S067 left it, and S210 corrects
              the justification alone. **Six findings amend the five pending files
              that own them**, dated sections appended today and the ids
              added to each file's `closes:`: F16 and F25 in
              S039, which also moves to sit directly before S122 (DEC-172);
              F24 in S126; F30 in S199; F36 in S134; F37 in S186. **Two
              sentences in `adocs/specs.md` are corrected today** rather than
              scheduled: F07's claim that `./rating.sh` re-derives 2559, and
              the `+/-25` now carries Part B's reading that the defensible
              interval is no narrower than `+/-60` once anchor choice is
              counted. `CLAUDE.md`'s stale "hand-written and untuned" is S211's.
              The report's `Status:` lines move to `planned` or `accepted` with
              the id, the one edit an audit report takes.
Rejected:     One step per finding -- 37 steps, most of them an hour's work
              that would each cost a completion cycle. Fixing all 23 code
              findings before the next strength step -- the literal BUGS rule,
              and DEC-171 is where the owner scoped it. Leaving Part E
              unassigned because it is low -- a finding with no home is
              re-found by the next audit, which is what DEC-138 exists to stop.
              Regularising the tuner -- see above.
Consequences: `adocs/plan_todo/` gains S207 to S214; ids allocated 207 to 214;
              the Open list is DEC-172's. Nine of the report's 37 findings are
              closed by no code change, six by documents. The next adversarial
              re-run reads this entry before reporting any of the 37 again.

## DEC-171  2026-09-11  The BUGS rule is scoped by reach: a defect in ordinary play, on the UCI surface as it is driven, or in a reported score or line is fixed first; the rest is scheduled behind the next strength step
Tags:         workflow, bugs, plan, priority, agents-md
Amends:       AGENTS.md `## Project rules`, BUGS
Context:      BUGS says a found bug is fixed "before anything else starts. Not
              noted, not scheduled." The 2026-09-10 audit produced 23 code
              findings, sixteen of them low and none of those sixteen reachable
              in an adjudicated match or from anything a GUI sends. Read
              literally, the rule puts a day of edge cases -- a 256-ply
              shuffle, a 4999-ply `position` line, `movestogo 0`, a custom
              command's race -- ahead of the first strength step in nineteen
              days (F14). The owner's instruction of 2026-09-11: "the bugs are
              important to solve but Elo is more important unless the bug is
              relevant or can impact engine evaluation like UCI bug or PV bugs."
Decision:     By the owner, 2026-09-11, recorded and worded by the agent. A
              found defect is fixed before the next play-altering change starts
              when it is **(a)** reachable in ordinary play -- a game a harness
              or a GUI produces -- or **(b)** on the UCI surface as GUIs and
              harnesses drive it, including a crash or corruption from
              `position`, `setoption` or `go` with the tokens they send, or
              **(c)** able to move a reported score, best move or line. Any
              other defect -- one that needs an illegal position, an
              implausible input length, a custom command, or that lives in a
              comment or in code nothing calls -- is a step scheduled as filler
              behind the next strength step, and it is not carried past the
              block boundary it sits in. BUGS in `AGENTS.md` carries the scope
              in one added sentence. Applied to the audit: S207 (F08, ordinary
              play) is first; S208 (F09, F10: a crash and a write off the end
              of a table from `position fen`) and S209 (F11 to F13: the option
              surface) land before S024; S210 and S213 sit behind S109 as the
              block boundary's daytime filler, beside S194, and S214 -- tools
              only, no `src/` -- is S024's filler.
Rejected:     Leaving BUGS unamended and reordering anyway -- "never deviate
              silently" is the rule the rule sits under. Dropping the low
              findings -- they are still defects and still fixed; only their
              place in the order moves. Scoping by severity word alone -- the
              audit's "low" is a judgement; reach is a test.
Consequences: A run's pre-registration lists, by finding id, the known defects
              DEC-171 has scoped behind it, so a verdict taken on a tree with
              known unreachable defects says so on its face. The block
              boundary after S109 (S199's first point) is where S210, S213 and
              S194 must be closed by, not merely scheduled; S214 closes beside
              S024.

## DEC-172  2026-09-11  The Open list is re-sorted for the 3000 mark: the owner's bug criterion first, then strength steps as the night runs, with the document lane as filler beside them; the coordinator chains through the night, and S151 is a fixed-rounds estimate at the block boundary
Tags:         plan, priority, measurement, machine, scheduling, dec-144, dec-113, dec-155, s151, s039
Amends:       DEC-144's order; S151's `accepts:` (design (iii)); S039's place
Context:      F14: the last kept positive verdict is S093 on 2026-08-22, the
              first strength step sat seventh in the order behind six
              document and test entries, and 8 of 57 open steps touch no
              `src/`. The owner asked on 2026-09-11 for the plan to be
              prioritised for 3000 Elo within the project's constraints, said
              the first entry must be one that holds the machine for a long
              time because the review is the last thing before sleep and the
              next word will be "next", and delegated engine-related choices to
              the agent. S151's pair had been the owner's open question since
              DEC-144.
Decision:     The priority is the owner's; the order and the rulings below are
              the agent's under the 2026-09-11 delegation.
              **The order.** S207 first: the one high finding that fires in
              ordinary play, and a `--nonreg` run priced at 4 to 18 hours --
              the night's run. Behind it, as filler that owns no run and
              touches no `src/`, the instrument lane's four document steps --
              S182 and S183, which F14 named as the two that would turn the
              feasibility claim into a number, and S185 and S181, which feed
              them -- in dependency order. Then the two bug steps the
              owner's criterion selects, S208 and S209, node-identical daytime
              work. Then S024, the largest ordering gain surveyed, two night
              runs; S214 as its filler. Then S211 (originality), S151, S212
              (its A/A is the last harness change before the next verdict),
              S109, S199's first drift point, and the three filler steps the
              boundary closes -- S210, S213, S194. The search block continues
              in DEC-133's order from S091; block 2, block 3 and block 4 stand,
              with one move: **S039 sits directly before S122**, its consumer,
              because F16 showed that sizing the margin before S121, S123,
              S125 and S101 change the very sum it clamps would leave S122
              inheriting a stale number.
              **The reading rule** of DEC-144 stands and gains two clauses. A
              run that ends while the owner is away is completed and the next
              run-owning entry is taken without waiting -- the list is the
              authority and "next" is a convenience. Between the verdicts of a
              multi-verdict step, a `src/` entry that is behaviour-neutral on
              node counts may land, because it cannot contaminate the second
              verdict's attribution; a play-altering one may not.
              **S151** takes design (iii) of its own section 7: S085's vector
              against `3488506` in a fixed 1000-pair match at `32+0.32`,
              `Hash=64` (DEC-088's pressure invariant held), read as an
              estimate with its interval, about 3.4 hours -- under DEC-155's
              line, so a daytime run while the agent writes the next step's
              code. Its `accepts:` is amended from "re-tested" to "measured ...
              read as an estimate", the amendment recorded in its file. The
              standing rule the step writes is the block-boundary form: the
              longer-control reading is one fixed 1000-pair match at `32+0.32`
              taken beside S199's drift point at each block boundary, not a
              re-take per verdict, which is the only scope that meets the
              accepts' own budget sentence. `Hash=64` and ratio 4 answer its
              questions 2 and 6; extensions are inside the scope (question 4)
              because the record names them; S148's verdict owes no
              retroactive re-take (excludes).
Rejected:     S024 first with F08 unfixed -- the owner's criterion and BUGS
              both put an ordinary-play defect ahead of a feature, and the
              defect moves reported scores. S151 as design (i), a `{-5, 0}`
              SPRT of 44 to 72 hours -- three days of the binding constraint
              for a number that adds no strength. Keeping S194 at the head --
              a test of a path no measurement has ever exercised. Dropping
              S182 and S183 -- F14 says they are the two steps that would turn
              "the midpoint clears 3000" from an impression into a number, and
              they cost the machine nothing. Interleaving block 2's speed
              steps into the search block as daytime filler -- each needs the
              idle machine for its timing, and the order would then encode a
              daily cadence rather than dependencies.
Consequences: `adocs/plan.md`'s Open list is rewritten; `status.md` names the
              night's run. The cost and Elo paragraphs of `plan.md` stand as
              written until S182 and S183 rewrite them, now entries 4 and 5.
              S151's throughput assumption (584 games an hour at `32+0.32`) is
              checked in its first hour and the ceiling is set from the
              pricing. S199's reading rule gains the F30 term in its file.

## DEC-173  2026-09-11  A repetition is a draw in the tree when the earlier occurrence lies strictly after the root, or when it is a third occurrence; the test that pinned two-fold-anywhere is re-stated, not relaxed
Tags:         search, draw, repetition, testing, s207
Context:      `2026-09-10_adversarial-F08`. `is_position_repeated` returns true
              on the first hash match anywhere in the history window, and
              `negamax_at` scores `DRAW_SCORE` on it above the root. A position
              that occurred once before the root and recurs inside the tree is
              therefore a dead draw to the search. Through `chess.engine` at
              `go depth 10` the same board reads `cp 0` on 4249 nodes with the
              history `g1f3 g8f6 f3g1` and `-900` on 325965 nodes without it;
              python-chess says no draw exists and stockfish scores it `-687`.
              `tests/test_search.cpp` "the losing side takes an available
              repetition" asserts the two-fold-anywhere convention on a
              position with one prior occurrence, and its comment states the
              mechanism as intended. The TESTS rule reserves re-stating a test
              to the owner; the owner delegated engine-related questions to the
              agent on 2026-09-11.
Decision:     By the agent under that delegation. **The convention is the
              published refined one** (CPW *Repetitions*; Stockfish PR #925,
              read as prose): inside the tree a first recurrence is a draw,
              because the side that repeated can repeat again past the
              horizon; a recurrence of a position from before or at the root is a
              draw only when it is the **third** occurrence, which is the
              occurrence FIDE 9.2 lets a player claim, because the side to
              move at the root chooses again and the opponent cannot force the
              third alone. The root's history size is the boundary and travels
              in `search_state_t`. **The test is re-stated**: its precondition
              becomes a position that has already occurred twice before the
              root, so the property it asserts -- a draw score beats a lost
              position -- is unchanged and its old precondition becomes the
              red-first case for the new rule. That is a re-statement with the
              property preserved and the precondition strengthened, not a
              relaxation, and this entry is the record the TESTS rule wants
              before an agent touches it. The change alters play and takes one
              `--nonreg` SPRT (S207); a zero is kept with the reason stated,
              because the oracle contradicts the old score on a position from
              ordinary play and the SPRT is the rule, not the motive.
Rejected:     Keeping two-fold-anywhere -- older engines used it, it is the
              weaker convention, the one engine that A/B'd the refinement
              measured a gain, and this engine's own oracle refutes it on the
              reproduction. Deleting the test -- the property is worth keeping.
              Calling the fix behaviour-neutral -- node counts move wherever a
              pre-root two-fold was reached, so INV-6's neutral half is not
              available.
Consequences: S207 owns the change, the tests and the run. `adocs/specs.md`'s
              search row states the rule when S207 lands. Every earlier game
              analysis that read `cp 0` on a pre-root repetition was reading
              this defect, `tools/analyse_game.py` included.

## DEC-174  2026-09-11  Resign adjudication is two-sided in both harnesses, the score stays 400, and a fixed-rounds A/A re-calibrates the regime
Tags:         measurement, harness, adjudication, rating, s212, dec-143, dec-077
Context:      `2026-09-10_adversarial-F04`. `fastchess.sh` and `rating.sh` pass
              `-resign movecount=3 score=400` without `twosided`, which the
              installed fastchess defaults to false; `rating.sh`'s comment
              claims both are two-sided and names that as the protection the
              rating run relies on. Counted from the tracked PGNs: 76 % of the
              A/A's games and 84 % of the S088 rating run's ended by
              adjudication; one-sided resignations were 11 of 676 decisive
              adjudications in self-play (1.6 %) and 514 of 2627 (19.6 %) in
              the rating run, where chesso conceded alone 311 times and its
              opponents 203, opponent-specific in direction and correlated
              with the anchor's solved rating at r = -0.505 on n = 5.
Decision:     By the agent, as measurement design under the 2026-09-11
              delegation. `twosided=true` in **both** harnesses. In self-play
              the throughput cost is bounded by the 1.6 % of adjudications that
              were one-sided, so it is close to free, and it removes the
              hazard the evaluation block creates when a candidate's scale
              moves (S039, S122, S126): under one-sided adjudication the side
              with the larger scale resigns first in equal positions. In the
              rating harness it is the property the comment already promised.
              `score=400` and `movecount=3` stay: 600 is fishtest's setting and
              the audit's note, and moving it is a throughput trade that gets
              its own entry if wanted. The change is followed by a 1000-game
              fixed-rounds A/A read with `adocs/data/S198_pairs.py` (DEC-143),
              with games an hour recorded so the cost is a number. F04's
              hypothesis about the anchor spread is recorded as a second
              candidate beside DEC-077's time-control candidate, to be read
              off S152's two-sided run.
Rejected:     Keeping one-sided for throughput -- the cost is 1.6 % of
              adjudications, and S105 already found the book bought x1.20 for
              nothing at the pair level; there is no throughput worth an
              instrument that is wrong against foreign scales. Moving to 600
              in the same step -- two harness changes under one A/A cannot be
              priced apart.
Consequences: S212. S152's gauntlet is two-sided where S088's was not, and its
              stamp says so beside the comparison. `rating.sh`'s comment
              becomes true by the change and not by rewording.

## DEC-175  2026-09-11  Parallel search is phase two: the 1CPU list is phase one's arena, and no SMP step exists until the mark
Tags:         search, threads, smp, phase-two, dec-089, dec-014, dec-138
Context:      `2026-09-10_adversarial-F15`. `Threads` is advertised `min 1 max
              1` and honoured honestly; no step, reserve entry or decision
              anywhere names SMP, Lazy SMP or a parallel search, while the end
              goal in `CLAUDE.md` and `specs.md` is the strongest CPU engine,
              which no single-threaded engine is. Phase one's target is the
              CCRL Blitz **1CPU** list (DEC-089), which single-threaded play
              satisfies, so nothing is wrong today; the gap is that the framing
              made a structural requirement of the end goal invisible rather
              than deferred.
Decision:     By the agent under the 2026-09-11 delegation. Parallel search is
              **phase two**, entered by the decision that opens phase two and
              not before. The form, when it comes, is the published default --
              Lazy SMP: one shared transposition table, one `search_state_t`
              per thread, threads differing by depth offset and by the noise of
              a shared table -- adapted from the literature and never copied
              (DEC-016, DEC-104). One constraint binds phase one now: a table
              added to the search states which of the two it is, per-thread or
              shared, at its declaration -- the S093-style hoisted tables, the
              S024 continuation table, S099's correction table -- so the move
              to threads is a decision about each table and not a rewrite of
              them. A future audit that finds SMP absent cites this entry, the
              DEC-138 form.
Rejected:     A reserve step -- the reserve is for steps with evidence at a
              stated band, and the published figure (about +180 at LTC for
              eight threads, DEC-085's record) is a phase-two rating, not a
              phase-one one. Building it now -- the 1CPU list does not reward
              it and the machine has twelve threads to spend on verdicts, all
              of which it does spend.
Consequences: `adocs/specs.md`'s Open items carry one line. S024, S099, S110
              and S111 state the per-thread-or-shared property when they land.


## DEC-176  2026-09-11  The Lynx bands are corrected and the record moves, not the order: S023's Lynx evidence is withdrawn, S098's node-type layers become 3100-band, and no step changes position
Tags:         planning, measurement, sources, dec-087, dec-133, dec-019
Amends:       DEC-087 (b) and (c), whose band words this replaces
Context:      `2026-09-04_plan_review-F02`. DEC-087 ruled on the
              correction-history family and on S098's layers with band words
              nothing in the tree sourced -- "~2600", "high-2800s", "~2850",
              "only above ~3100". S181 banded every Lynx figure those rulings
              cite: each pull request's `merged_at` from the GitHub API
              against the two releases it falls between, and the CCRL Blitz
              1CPU rating of each release on the list computed 2026-09-05,
              read 2026-09-11 (`adocs/data/S181_lynx_bands.md`). Three of the
              words were wrong by 180 to 380 points and one claim was wrong in
              kind:
                (i) "+11.4 at ~2850" (#1662) was measured at **3224-3291**,
                    above the "~3100" that demoted S110 and S111, so DEC-087
                    (b)'s criterion separated nothing. DEC-133 already
                    answered this by moving S099 to the reserve head.
                (ii) "Capture history failed four SPRTs at ~2600 (Lynx)" has no
                    source, and Lynx **merged** capture history as #634 on
                    2024-02-02 into v1.3.0 at a banded 2653; the
                    closed-unmerged capture-history pull requests are later
                    refinements dated 2024-09 onward at 2925 and above.
                (iii) S098's cutnode (#1233), !improving (#1135) and
                    PV-min-moves (#1230) are **3119-3138**, and TT-capture
                    (#1529) and deeper/shallower (#1535) **3138-3224**, where
                    the file read high-2800s and "~3000-3100".
                (iv) S109's Lynx LMP band (#512) is **2420-2430**, 180 points
                    *below* the "~2600" written, which strengthens that step's
                    sub-3000 claim rather than weakening it.
              S097 recorded the discrepancy as a scope concern on 2026-08-19
              against Lynx's own README and could not resolve it; the list
              agrees with the README to within a few points, so the README was
              right and the repository's banding of that era was about 300 low.
Decision:     By the agent under the owner's delegation of 2026-09-11
              (engine-related questions are the agent's; the owner is asked
              only when the workstation is at risk, the ethic would change or
              the goal is not served). **The corrected bands change the record
              and no step's position.** Specifically:
              (a) S023's Lynx sentence is **withdrawn as unverified** and the
                  demotion stands on the figure that traces -- Weiss #428,
                  -4.17 +/- 4.83 STC against +3.66 +/- 3.29 LTC, measured by an
                  engine CCRL Blitz rates 3055, which is above chesso's target.
                  A reserve position argued from a sourced figure at a higher
                  band is better founded than one argued from four runs nobody
                  can find, so S023 does not move.
              (b) S098 stays **one step, three verdicts, in that order**, in
                  the main order. History scaling stays first as the only layer
                  with evidence in this engine's band; node type and
                  post-re-search keep their places because (d) reads the
                  re-search (c) produces. What changes is what the file may
                  claim: (c) and (d) rest on 3100-band evidence, a zero from
                  either is an expected outcome, and neither may be argued for
                  on "it worked below 3000".
              (c) S099, S110 and S111 stay where DEC-133 put them. The
                  re-banding is the *reason* DEC-133 was right, not a new
                  question.
              (d) Every band word in a step file is now either a range from the
                  table with its read date, or absent. A band figure chooses
                  bounds and never a conclusion (DEC-019).
Rejected:     Re-promoting S023 out of the reserve because its Lynx evidence
              evaporated -- the Weiss figure is the stronger evidence and it
              points the same way; a promotion would spend a verdict on a
              technique whose only sourced measurement is negative at short
              control above this band. Demoting S098's node-type layer to the
              reserve on its corrected 3119-3138 band -- late move reduction is
              the single most expensive feature in the Ethereal ledger
              (-248.59 on removal) and its refinement is where a 3000-mark
              engine's search work is; the band changes the expected magnitude,
              not whether the work belongs. Re-reading the CCRL list at each
              future citation -- it drifts 0 to 4 points in a fortnight, so the
              read date beside the figure is the discipline and a re-read is
              not owed.
Consequences: S181's stamp carries the table. DEC-087 gains a second `Amended:`
              line naming it. S097's band caution and its scope concern 1 are
              closed in the README's favour; S098's "honest split" is redrawn;
              S099, S109, S110, S023 and S132 carry ranges. `2026-09-04_plan_review-F02`
              is closed by S181.


## DEC-177  2026-09-11  The 16-a-side load bound stands and KILLER_POS becomes legal: one white pawn is removed, and the bench signature moves once as the price
Tags:         rules, fen, bench, testing, s208, dec-170
Amends:       S208's accepts, whose "identical `bench` signature" clause this
              replaces
Context:      S208 refuses a placement with more than 16 pieces of one colour
              at the load boundary, which is what stops the F09 crash -- 27
              white pieces generating 277 moves into a `move_t[270]` on the
              stack, `*** stack smashing detected ***`, exit 134 in the
              Release binary that ships. Its accepts also asks for INV-6
              discharged on an identical `bench` signature, "because no legal
              position changes".
              **Those two clauses contradict each other and the step could not
              have known it.** `KILLER_POS`
              (`src/data_structures.hpp`) is one of the eight bench positions
              and has **17 white pieces** -- nine pawns -- and
              `src/chesso.cpp`'s own provenance comment says so: "knowingly
              illegal by FIDE piece count ... It is kept: **the engine loads
              it**, `test` already searches it, it carries both the en passant
              capture and twelve promotions, and a signature needs determinism
              and not legality." S208 is exactly the change that makes "the
              engine loads it" false. Measured: with the refusal in and the
              constant untouched, `set_position` refuses it and leaves the
              previous board, so `bench` searches a duplicate position and the
              signature reads **30746008** against 26491479.
              The constant is also reached by `position killer`, by the `test`
              command's expected-bestmove table, and by two position lists in
              `tests/`, so there is no resolution in which it stays unloadable.
Decision:     By the agent under the owner's delegation of 2026-09-11
              (engine-related questions are the agent's; the owner is asked
              only when the workstation is at risk, the ethic would change or
              the goal is not served). **The bound stays at 16 a side and
              `KILLER_POS` becomes a legal position**, by deleting the pawn on
              h3: `rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P4/P1P1P3/RNBQKBNR w KQkq
              e6 0 1`. Checked with python-chess 1.11.2, not reasoned about:
              `Status.VALID`, 16 white and 14 black, **12 promotions** and the
              **f5e6** en-passant capture both still present, legal moves 42 ->
              **48**. So every property the constant was kept for survives and
              the one that justified an exception is gone.
              **The bench signature therefore moves once**, S208's commit
              carries a `Bench:` line rather than "No functional change", and
              the accepts' identical-signature clause is read as satisfied by
              `tools/search_bench.py` instead -- its three positions do not
              include `KILLER_POS`, so node-identity there still says what
              INV-6 wants it to say about every legal position.
Rejected:     **Refusing at the UCI `position fen` path only**, leaving
              `load_FEN` permissive for internal callers -- it keeps the
              signature, but it splits the contract S161 established in as many
              words ("the load boundary is the contract every downstream
              consumer assumes") and leaves `datagen`, `pgn_to_positions` and
              any corpus tool on the crashing path. **Replacing `KILLER_POS` in
              the bench set alone** -- it does not solve anything: `position
              killer`, `test` and the two test lists would still call
              `load_FEN(KILLER_POS)` and get false. **Raising the bound to 17
              or 18** -- the audit's maximiser was run under the 16-a-side rule
              and returned 224 against a 270 buffer; 17 and 18 are unmeasured,
              and a bound nobody has measured is not a bound. **A move-count
              bound at load time instead of a piece count** -- the honest form
              of the rule, since what threatens the buffer is the move count
              and not the pieces, but counting means generating into a buffer
              larger than `MAX_MOVES` and it is a design this step's `excludes`
              does not carry. Recorded as the better rule if `MAX_MOVES` ever
              has to be defended again.
Consequences: S208's stamp records the contradiction, the measurement that
              exposed it and this ruling. `src/data_structures.hpp`'s constant,
              `src/chesso.cpp`'s bench provenance table (42 -> 48 moves,
              "valid NO" -> "yes") and its `test` expected-bestmove row are
              re-derived; `adocs/specs.md` and `DEV_MANUAL.md` carry the new
              bench signature. The eight-FEN set keeps promotions, en passant,
              castling and quiescence mates, and is now legal throughout --
              which is strictly better than the note explaining why it was not.

## DEC-178  2026-09-11  The protocol's case rule wins over DEC-093's mis-cased-name refusal; `Hash` refuses in two shapes; the `clean-tt` race is guarded by the join, and measured by hand under TSan
Tags:         uci, protocol, tests, surface, sanitizers, tooling
Context:      S209 closes three findings of the 2026-09-10 audit (F11 to F13)
              and its `accepts` left three things open that had to be decided
              rather than assumed. **First**, `UCI.txt` says an option's name
              "should not be case sensitive" and the step's Shape says to fold
              the name once before the chain of comparisons -- which reaches
              the search parameters too, and `tests/test_uci_surface.cpp`
              pinned `setoption name Rfpmargin value 100` answering
              `info string refused [Rfpmargin], unknown option`. That
              assertion is S137's and DEC-093's: a tuner sending a misspelled
              name must hear about it. The two cannot both stand. **Second**,
              the `accepts` names one refusal line for `Hash`, and
              `std::from_chars` distinguishes two cases where `std::stoll`'s
              single `catch` distinguished none. **Third**, the `accepts` asks
              for a TSan-driven case in `build-sanitize` or, failing that, the
              reproduction by hand -- and `build-sanitize` is the ASan tree.
Decision:     By the agent, under the RUNS and BUGS rules. **(a) Every
              advertised option name folds, the search parameters included**,
              and the setter is still called with the canonical spelling from
              `src/search_params.hpp` `search_param_info`, so `rfpmargin` is
              `RfpMargin`. `unknown option` now means a name that is no option
              in any casing, and the golden assertion is **re-stated, not
              relaxed**: `RfpMargn`, one letter short, still answers
              `unknown option`, and `tests/test_search_params.cpp` "a mis-cased
              parameter name is still the parameter" holds the other side.
              DEC-093's half that matters -- a tuner hears its mistake --
              survives; what it loses is a mistake the protocol says is not
              one. The fold needs a precondition and gets a case:
              `tests/test_uci_surface.cpp` "no two advertised option names
              collide when folded", over the 33 the tune build advertises,
              because two names differing only in case would be one option to
              a folded comparison and the first branch of the chain would take
              both. **(b) `Hash` answers two lines**, `not an integer` for a
              token it cannot read in full and `out of range` for a
              well-formed integer no `long long` can hold, the same split the
              search parameters have made since S137. **(c) The suite guards
              the join, not the race**: after `go infinite`, `clean-tt` has to
              have put `bestmove` on stdout by the time it returns, which is
              deterministic in both builds and is the cause rather than the
              symptom. The race itself is measured by hand under a fourth,
              TSan build: **38, 41 and 36 reports on `013600d`, 0 on the
              candidate**, and the A/B taken in the same build tree by adding
              the one line to it -- `tt_reset`'s `memset` against
              `tt_store_entry` and `tt_get_entry` in the search thread, which
              is F11's claim.
Rejected:     **Folding only the five non-parameter names** -- `hash` would
              work and `rfpmargin` would be refused, two rules on one surface
              and neither of them the protocol's. **Special-casing the
              parameter names to keep DEC-093's mis-cased refusal** -- it
              preserves a refusal the protocol this repository ships forbids,
              and a tuner reading `unknown option` for a name that differs only
              in case has been told something false. **One `Hash` message for
              both classes** -- "not an integer" for `99999999999999999999`
              says something false about a well-formed integer, which is the
              failure class S137 removed one layer down. **A `TSAN` option in
              `CMakeLists.txt` and a fourth gate build** -- ASan and TSan
              cannot share a binary, so it is a build configuration and a gate
              stage of its own for one command no GUI sends, and the join test
              already fails if the cause returns. Recorded as the thing to
              build if a second race is ever found. **Leaving `clean-tt` alone
              because a GUI never sends it** -- it is documented in `MANUAL.md`
              and reachable from any script, and DEC-171 scopes the UCI surface
              as harnesses drive it.
Consequences: `MANUAL.md` and `adocs/specs.md` state the case rule, the two
              `Hash` lines and the join before `tests/test_uci_surface.cpp` is
              refreshed (SURFACE). A parameter added later must not collide
              with an existing name when folded, and the new surface case is
              what says so. The `go` tokens `command_go` reads with
              `std::stoll` are the same class and are **not** changed here:
              S210's `accepts` already owns that decision by name. The TSan
              trap that made the first reproduction print 0 reports on a
              defective tree -- `FATAL: ThreadSanitizer: unexpected memory
              mapping`, ASLR, `setarch -R` -- is written into `TOOLCHAIN.md`
              beside the build line, because a sanitizer that never started
              and a fixed engine print the same number.


## DEC-179  2026-09-11  The goal stands at 3000 CCRL Blitz without a network, reached by a longer hand-crafted list; the network comes after the mark
Tags:         planning, rating, nnue, dec-071, dec-054, s183, s217
Context:      S183 re-derived the plan's Elo arithmetic from recorded inputs
              and applied the project's own measured published-to-measured
              ratio (0.38 at its most generous, median 0.00). The whole
              remaining list lands at **2658** from recorded inputs and at
              **2707 to 2817** on the plan's own range, against the 2559 soft
              anchor and the 3000 mark; the high end is 183 short. Its stamp
              named DEC-071 as the decision it put to the owner. Four options
              were presented on 2026-09-11 in an interactive review of the
              parked list, with the agent recommending the first: un-park
              S029 after the search block and let the network replace block
              3's hand-crafted rebuild; change nothing and re-derive after
              S024 and S109 (DEC-136); extend the hand-crafted list; write a
              trigger that un-parks S029 if the post-S109 re-derivation still
              falls short.
Decision:     **By the owner**: "Extend the hand-crafted list. My plan is to
              add the network after the goal of 3000 Elo. Multiple engines did
              that, so should we." DEC-071 and DEC-054 stand unamended: the
              mark is 3000 without a network, S029 stays parked, and it is
              un-parked by a decision taken when S152 reads the mark, not
              before. **S217** is created: a sourced inventory of the
              hand-crafted techniques that engines rated 3000 to 3130 on the
              CCRL Blitz 1CPU list carried at a network-free version, and this
              plan lacks -- banded per DEC-087 as corrected by DEC-176, priced
              per DEC-143 -- from release notes, commit-message prose, papers
              and the authors' own statements, never from source (DEC-016).
              The coordinator turns the gap into steps and records which gap
              techniques got none and why.
Rejected:     **Un-park S029 after the search block** -- the agent's
              recommendation, on the end goal naming its own network and on
              block 3's cost; the owner's order is a hand-crafted engine at
              the mark first, as the engines DEC-071 lists did, and the
              network on top of it. **Change nothing** -- leaves the shortfall
              unaddressed until two verdicts land and starts no new work.
              **A trigger** -- defers the same question to a date; the owner
              answered it now.
Consequences: `adocs/plan.md`'s Elo paragraph says the question is answered
              and by what; S217 sits in the Open list as documents-only filler
              behind S024's run. DEC-136's re-derivation runs over the
              extended list when S217's steps are created, and a finding that
              the gap is empty is recorded as such -- then the shortfall is
              the discount and not a missing technique, and the decision that
              follows says what the extended list is instead. The delegation
              of 2026-09-11 is unchanged: dropping or reopening the goal stays
              the owner's.


## DEC-180  2026-09-11  S109 ships late move pruning in its published form; the gives-check exemption is S218, its own step and its own SPRT
Tags:         search, pruning, lmp, s109, s218, dec-082, dec-141
Context:      S109's `accepts` states that the gives-check exemption binds the
              three per-move rules, which run after `make_move` where
              `is_check_move` exists, and not late move pruning, whose
              skip-quiets flag is honoured at the generation stage where no
              pre-make gives-check predicate exists and where the published
              form carries no exemption. Buying LMP the exemption with a
              post-make prune departs from the published form, and the
              `accepts` reserved that choice to the owner. Three options were
              presented on 2026-09-11: the published form alone with the mate
              guard as the safety net (the agent's recommendation); the
              exemption from the start; the published form and then the
              exemption as its own SPRT.
Decision:     **By the owner**: the published form ships in S109 and is
              measured by S109's one verdict; the exemption is **S218**, a
              step of its own directly behind S109 in the Open list, decided
              by its own SPRT against S109's shipped form. If S109's own
              "pruning does not hide a forced mate" case goes red without the
              exemption, the exemption is S109's fix under the TESTS rule and
              S218 folds into it -- a red guard is a bug, not an option.
Rejected:     **The exemption from the start** -- measures a variant with no
              published evidence behind it and pays a make/unmake per pruned
              quiet inside the one verdict that is supposed to price the
              published form. **The published form with no follow-up step** --
              the owner wants the exemption measured rather than left to the
              guard.
Consequences: S109's `accepts` and body name S218 where they named the
              owner's call. S218's file carries the two shapes (post-make skip
              behind the generation flag, or a pre-make gives-check predicate)
              and is re-scoped if S109's verdict is negative. Each is one
              change and one number, which is measurement rule 6.


## DEC-181  2026-09-11  F02 and F03 of the 2026-09-04 audit fold into S210
Tags:         uci, protocol, audit, s210, dec-171, dec-170
Context:      Three low findings of `2026-09-04_adversarial` were still
              `open` after DEC-170 sent F01 (`go infinite` printing `bestmove`
              unasked) to S210. **F02**: a `moves` token that does not parse
              or is not legal is skipped and the rest of the list applied, so
              `position` ends on a board the GUI did not send -- the S176 rule
              for the FEN half, not applied to the moves half -- and the skip
              is silent in the shipped binary. **F03**: the root's move order
              after an aborted iteration rests on the root's table entry
              surviving the iteration, which the replacement rule does not
              guarantee; mechanism shown, occurrence not reproduced.
Decision:     **By the owner**, on the agent's recommendation: both fold into
              S210, the batch of low engine defects with a reach census and
              one SPRT only where the census finds reach. S210's goal,
              `accepts` and `closes` are amended; the report's two `Status:`
              lines move to `planned`.
Rejected:     **Fix F02 now under BUGS** -- reach needs a GUI or harness to
              send a move the engine cannot parse, which is a notation
              mismatch and not ordinary play; DEC-171 scopes the first-fix
              clause to ordinary play, the UCI surface as it is driven, and a
              reported score, move or line. **Accept both** -- F02's silent
              skip is the class S176 already refused on the FEN half of the
              same command, and F03's premise is written in a comment and
              enforced nowhere.
Consequences: S210 closes nine findings. F03's remedy is either a best move
              carried in the search state instead of re-read from the table,
              or the premise asserted and counted over the F22 census sample;
              the step decides and the comment in `src/chesso.cpp` says what
              is enforced.


## DEC-182  2026-09-11  The harness opening book is re-decided from a survey of open-licence books; the owner downloads the pick; a fixed-rounds A/A read against S198's decides whether it stays
Tags:         harness, book, sprt, measurement, dec-083, dec-143, s105, s198, s219
Context:      S105 moved the harness to `UHO_Lichess_4852_v1.epd` for the
              surveyed engines' regime (DEC-083). Measured: games a fifth
              shorter (x1.20 of the x1.67), pair score variance unchanged
              (ratio 1.022), and Pohl's draw-floor argument does not hold at
              this strength -- chesso draws 40.3 % on the balanced book.
              `status.md` carried "whether to keep it is the owner's to
              revisit". Three options were presented on 2026-09-11: keep (the
              agent's recommendation), revert to `8moves_v3.pgn`, or
              re-measure both at the next block boundary.
Decision:     **By the owner**: "find the best book possible online that we
              can use, recompile them; the best type possible for us
              (balanced?) with the only restriction that we use open-source
              material, no stealing from other work that is not permitted;
              find the book yourself but let me download it; after the change
              we re-evaluate everything." **S219** is the step, in four parts:
              (1) the agent surveys the open-licence books -- CC0, public
              domain or permissive, verified at the source, none with an
              unstated licence, which keeps `books/fetch_book.sh`'s rule that
              sp-cc.de is not a source -- and names the candidates with URLs;
              (2) the owner downloads the pick; (3) both digests are pinned
              in `books/fetch_book.sh` and `fastchess.sh`'s `book=` moves to
              it; (4) DEC-143's fixed-rounds A/A of 1000 games follows and is
              read against S198's -- pair score variance, games an hour, draw
              rate and the share of pairs decided by the opening -- and that
              reading, not the survey, decides whether the pick stays: lower
              or equal cost per verdict at variance inside S105's band keeps
              it; a worse cost reverts `book=` and records the number. Where
              two candidates are close the balanced one is preferred, which
              is the owner's stated leaning. **Assumptions, stated for the
              owner's veto on reading**: the decision covers the book
              `fastchess.sh` plays and the one `rating.sh` plays; the engine's
              embedded Polyglot book (`src/openings.bin`, `tools/make_book`)
              is rebuilt from the same material only if that material is a
              PGN game collection under a licence that permits it, and S219
              decides that half or leaves it; "re-evaluate everything" is
              read as the A/A and a re-reading of S105's variance question
              with the new book, **not** as re-running past verdicts -- every
              recorded verdict stays attributed to the book it was taken on,
              the way DEC-049 attributes a figure to its machine. Until the
              pick is downloaded and calibrated, verdicts run on the current
              book and say so.
Rejected:     **Keep as is** -- the owner wants the best open material
              surveyed rather than the first CC0 book that worked. **Revert to
              `8moves_v3.pgn` without a measurement** -- re-baselines every
              verdict on an argument. **Re-measure at the next block
              boundary** -- the owner asked for the change now, and the A/A
              is about half an hour of machine.
Consequences: `fastchess.sh` prints the book in its banner already, so what a
              run measured stays on screen; every pre-registration after the
              change names the book. S199's drift match and S152's rating use
              the new book from then on. The ledger in `adocs/plan.md` notes
              the book change at its row. A book whose licence cannot be
              verified is not a candidate, whatever its quality. **The survey
              ran the same evening** (`adocs/data/S219_book_survey.md`): the
              pick cannot be read off the literature -- three CC0 candidates
              beside the current book, and a documented method, fishtest's
              time-odds fixed-games comparison -- so S219 measures the
              candidates first and the A/A follows the winner; the step file
              carries the amended `accepts`.


## DEC-183  2026-09-11  The self-play corpus stays gitignored and is archived off the repository with its recipe
Tags:         data, corpus, tuning, s065, dec-111, machine
Context:      `.tuning/selfplay_v2.tsv` -- 11003693 positions from 120000
              self-play games, 715 MB -- is the corpus S065's refit and every
              fit since were taken on. It is gitignored, it did not survive
              the move from the MacBook, and regenerating it costs a night;
              the fit scripts are tracked since S192. Three options were
              presented on 2026-09-11: accept the regeneration cost (the
              agent's recommendation, because S082 and S083 replace how the
              corpus is built), archive off-repo with the recipe recorded, or
              chunked gzip on an orphan branch as the S024 evidence was.
Decision:     **By the owner**: it stays gitignored **and** one archive with
              everything needed to restore or regenerate it goes to
              `/home/max/Synckeeper/`, in a compression that works on Linux
              and macOS out of the box. The archive is
              `chesso_selfplay_v2_corpus_2026-09-11.tar.xz`: the TSV, the two
              run logs (`datagen_v2.log`, `dedupe_v2.log`) and
              `CORPUS_README.md` with the row format, the restore command,
              the digests, the generating commit `67eed7e` and the exact
              `datagen` invocation (`--games 120000 --nodes 100000 --threads
              12 --seed 20260814 --allow-tactical 1`, `--quiet-limit` 1000),
              and the `corpus_dedupe` command that rebuilds the derived dedup
              file, which is not archived. Digests: the TSV is
              `ad8c5dfe1a236af8cbf946a67692e5fbfe533c7d38b6c7468c8a8ec202d14e05`;
              the dedup file it rebuilds is
              `0a6b59b6af9f50c4360cac7c87c33250318e712250f2653eb09bcb9f0b64b69f`;
              the archive is 91999956 bytes, sha256
              `d91bd9b2a86939043a8ca744428fed6d4e7c9d7038593d06bd03e86be3c8797d`,
              written beside it as `<archive>.sha256`; the listing and the
              streamed digest of the TSV inside it were verified on
              2026-09-11 before this entry was finished.
Rejected:     **Accept the regeneration cost alone** -- a night of the machine
              is the plan's scarcest resource, and the current corpus is what
              every recorded fit is reproducible from. **An orphan branch** --
              150 to 250 MB in the remote for a file that is not source.
Consequences: A machine move restores from the archive and verifies the
              digest before any fit is trusted. A new corpus -- S082, S083 --
              gets the same treatment when it lands: archive plus recipe,
              named in its step's stamp. The archive is the owner's file; the
              repository records only the recipe and the digests, here.


## DEC-184  2026-09-11  Seven parked hygiene findings are folded into S210, S213 and S214 or accepted; S194's two deferred questions are answered; the enrichment pass continues as filler
Tags:         docs, tests, tooling, hygiene, s210, s213, s214, s194, enrichment
Context:      `status.md`'s Parked block carried seven items "parked, not
              planned: a step is created by a decision and none has been
              taken", some since 2026-08. Each was presented on 2026-09-11
              with a proposed home, against accepting all seven unchanged or
              deciding them one by one.
Decision:     **By the owner**, on the agent's proposal, all seven as
              proposed:
              **1.** Nothing checks that `AGENTS.md`'s TESTS command and
              `DEV_MANUAL.md`'s test-section command agree (S143, DEC-118) --
              a `--gate` mode in `tools/plan_prose_check.py` that fails when
              they differ, registered in the fast suite; **S214**.
              **2.** The `info` line's fields are outside the UCI golden
              (found by S037) -- `test_uci_surface` gains a golden over the
              field set as `MANUAL.md` documents it, values excluded; **S213**.
              **3.** `command_bench`'s per-position `reset_for_new_game()` is
              unobservable and untestable (S195) -- **accepted and kept**: it
              is what makes the signature independent of position order and
              of a list that may one day repeat a position.
              **4.** The tuner's last `--only` group runs to `PARAM_COUNT`, a
              blind spot S041's partition test cannot see (S041) --
              **accepted as to structure**; the precondition assertion that
              does fire gets a message naming what it detects; **S214**.
              **5.** `MANUAL.md` says nothing about what `ucinewgame` resets,
              and its `nodes` wording omits that the last `info` line can be
              under the `go nodes` budget when the final iteration aborts
              (S195, measured 2917 of 5000) -- one sentence each; **S210**.
              **6.** S194's two deferred questions: `adocs/specs.md` joins
              its `touches:` and the coordinator writes the sentence, which
              the PLAN rule already provides for; an unparsable
              `CHESSO_BOOK_SEED` is **refused with an `info string` line**
              rather than ignored silently in Release, `MANUAL.md` first --
              the S172 and S176 pattern: what the engine drops, it says so on
              the channel.
              **7.** The enrichment pass: 37 of the 59 pending files carry
              neither the 2026-08-19 research section nor the 2026-09-05
              implementation guide. It **continues as filler** under the
              2026-09-11 delegation, one file at a time as steps approach the
              top of the Open list, agent-only and owning no run; it is not a
              plan step and never blocks one.
Rejected:     **Accept all seven unchanged** -- items 1, 2 and 6 are each an
              hour that closes a class of drift the project has already paid
              for once (S037, DEC-118, S172). **One by one** -- the owner
              took the bundle.
Consequences: S210, S213, S214 and S194 are amended in their `goal:`,
              `accepts:`, `touches:` and `decisions:` fields; `status.md`'s
              Parked block loses the six items this entry closes and rewrites
              the seventh. Items 3 and 4 are the DEC-138 form: an audit that
              finds them again cites this entry.


## DEC-185  2026-09-11  Every plan step is executed by one clean Opus 5 subagent under the coordinator's instruction; the coordinator implements nothing itself, and one task runs at a time
Tags:         workflow, agents, coordinator, context, dec-106, dec-113
Context:      DEC-106 left how a step is executed to the agent's call, and the
              coordinator of the 2026-09-11 sessions implemented most steps in
              its own context. The owner's instruction, given mid-turn on
              2026-09-11 while S219 was being started: "for each plan step,
              don't do it yourself but coordinate and instruct Opus 5 agents.
              One clean agent each time. This will reduce the context of each
              task and keep the quality high and you will be the coordinator.
              Also compress the context after each task is completed. Don't
              run more than one task at the time if not in particular
              occasions like something is busy and waiting a long time so you
              can move forward with something else."
Decision:     **By the owner.** (1) A plan step's implementation is done by
              **one fresh Opus 5 subagent** per step, or per self-contained
              part of a step, instructed by the coordinator with a
              self-contained brief and a bounded report; the subagent writes
              code, tests, the step's own data files and its own step file,
              and never the shared documents. (2) The coordinator coordinates:
              it orients, writes briefs, reads reports, runs the Tier-1 fast
              check over the result, holds the machine (a match, a fit, a
              timing is started by it and by nobody else -- the PLAN rule
              unchanged), owns `plan.md`, `status.md`, `specs.md` and
              `decisions.md`, and commits. (3) **One task at a time.** A second
              subagent starts only while the first is blocked on something
              long -- a match holding the machine, a download the owner has
              not made -- so the machine and the agent are never both idle.
              (4) After each task closes the coordinator keeps its context
              small: it reads the subagent's report and not its transcript,
              writes the handover into `status.md`, and tells the owner that
              a task closed so the conversation can be compacted; the
              coordinator has no tool that compacts on its own.
Rejected:     **The coordinator implementing steps itself** -- its context
              grows with every file read and quality drifts with it, which is
              what the owner observed. **Several subagents in parallel by
              default** -- they contend for the machine and for the shared
              documents, and DEC-113's bound on active steps already says
              "strictly necessary"; parallelism stays the exception for a
              blocked task. **Sonnet or Haiku for step work** -- the owner named
              Opus 5; quality over cost.
Consequences: `AGENTS.md`'s AGENTS rule is rewritten to this. A step's
              `author:` names the subagent and the coordinator. Briefs are
              self-contained -- the step file, the decisions it cites, the
              files to read, what not to touch, the report's shape -- because
              a clean agent knows nothing the brief does not say. Audits, fast
              checks and exploration stay free (DEC-106).


## DEC-186  2026-09-11  The S159 census set's `promo-mess` row is replaced by the legal `KILLER_POS` under a new name in a new file, and the census is not re-derived
Tags:         instruments, census, data, s216, s159, s208, dec-142, dec-160, dec-177
Context:      S216. DEC-177 made `KILLER_POS` legal by removing one white pawn,
              because S208's load bound refuses more than 16 pieces of a
              colour; row 6 of `adocs/data/S159_census_positions.txt`
              (`promo-mess`, the sixth of eleven rows -- the step file's "row
              16" was a line number) is the old constant, verbatim, and is now
              refused. S159's reader scraped only `info ... nodes` lines, so
              the refusal was skipped and the row inherited the previous
              board's numbers under its own name; S216's subagent showed it
              red on HEAD at depth 2 -- the start position's 452 nodes and
              `d2d4`, illegal on the promo-mess board, printed under
              `promo-mess`. The accepts named three resolutions for the row
              and asked whether the census is re-derived at all.
Decision:     By the agent under the 2026-09-11 delegation, on the subagent's
              proposal. **(a) Replace**, as a new file
              `adocs/data/S216_census_positions.txt` beside S159's (the
              directory is append-only), with the legal `KILLER_POS` -- 16
              white pieces, twelve promotions and the `f5e6` en-passant
              capture kept -- **under the new name `promo-mess-s208`**, so no
              future table's row is read against the recorded 142852 nodes.
              **(b) Not re-derived.** The recorded census (`S159_census.md`)
              pins `git archive 99000c1` of 2026-09-09; the bound landed at
              `8aff8ac` on 2026-09-11 and its refusal text is absent from
              `src/chesso.cpp` at 99000c1, so the recorded numbers were
              measured on a board that loaded, and the output says the same:
              142852 / `g7h8q` against the row above's 648113 / `d7c8q`, and
              `g7h8q` is not legal there. Its margin is stated anyway: the
              row is 0.79 % of 18166063 nodes, DEC-160's reading rests on a
              gap of 43.4 points (45.4 % against 88.8 %), and removing the
              row bounds HEAD to 44.6 to 46.0 %. A census run today at HEAD,
              41 commits on, would be a new experiment and not a
              re-derivation, and none is owed.
Rejected:     **Drop the row** -- shrinks a set a recorded decision rests on
              and loses its only en-passant row. **Keep it and report it
              refused** -- every future run on that input is red by
              construction. **Re-derive now** -- see (b); DEC-142's trigger is
              an end that moved, and the recorded census's end did not move,
              the input file did.
Consequences: `S216_census_run.py` is the reader from here on; S159's reader
              and positions file stay as history. An optional verification --
              the new reader over S159's positions at HEAD, expecting exactly
              one `REFUSED` row and exit 1 -- runs after the S219 match, in
              minutes, and is recorded in S216's stamp. S159's six evidence
              files have no `adocs/data/README.md` row (the subagent's
              finding); S216 adds them.


## DEC-187  2026-09-11  The en passant key is a correctness defect, not an efficiency item: S042 is re-scoped as a bug fix and moves to the top of the Open list under the BUGS rule
Tags:         bugs, repetition, zobrist, en-passant, s042, s207, dec-171, dec-173, audit
Context:      S219's first match printed 236 fastchess warnings in 1500 games,
              "PV continues after threefold repetition", from both sides of a
              self-match. A subagent replayed every case with python-chess:
              **236 of 236 are genuine threefolds by the rules of chess** at
              the flagged node, the earlier occurrences before the root (184)
              or at the root (52), never inside the line -- so DEC-173's
              convention is not the cause. The engine reproduces cold, and a
              four-ply case isolates it: the same position with the same
              history reached in two ways scores **-313** one way and **0**
              the other, and python-chess confirms the threefold. **Cause:**
              `make_move` sets the en passant square and xors `ep_randoms`
              on every double pawn push whether or not an enemy pawn can
              capture, so the position immediately after the push hashes
              differently from the identical position reached otherwise, and
              `classify_repetition`'s key compare skips that occurrence --
              which, in every one of the 236, was the oldest of the three.
              The compare itself is right; the key it compares is not. Of the
              236, **52 publish a non-zero score for a drawn line**; the other
              184 read 0 because the search meets a later repetition one ply
              deeper, right by accident. Depths 2 to 27, so not a
              first-iteration artefact.
              The defect was known and mis-triaged: `2026-08-13_adversarial-F08`
              wrote "Efficiency, not correctness ... repetition detection
              cannot be hurt, because a position carrying an en passant
              square always has halfmove_clock == 0 and the lookback is
              bounded by that clock." The lookback is `back <= limit` and the
              tainted entry sits at exactly `back == halfmove_clock`, the last
              iteration -- the argument is off by one, and S042's own file
              carried a section "Not a correctness bug" on it.
Decision:     By the agent under the 2026-09-11 delegation. **(1) S042 is a
              bug fix under DEC-171's first clause** -- reachable in ordinary
              play, moving a reported score and line -- and **moves to Open
              entry 1**, ahead of S024; it is the next step to start when the
              machine frees. **(2) Re-scoped:** a minimized red-first case
              from the reproduction (two histories, one position, the scores
              must agree and be the draw) beside the perft and FEN checks its
              accepts already had; the key moves in every place it is built --
              `make_move`, `load_FEN`, the full-hash recomputation -- as the
              2026-09-04 plan review's F08 said; the tree changes, so the
              commit carries `Bench:`, the Debug binary self-plays four rounds
              (DEC-141), and one `--nonreg` SPRT decides it, priced per
              DEC-143 in its pre-registration. **(3) S219's comparison
              stands**: the same binary carries the defect on both sides of
              every match and the reading is relative; its A/A runs before
              S042's SPRT so the SPRT plays on the calibrated book. S216's
              completion, documents and one instrument, is unaffected.
              **(4)** The F08 triage is corrected here and in S042's file; the
              2026-08-13 report is evidence and keeps its text, its `Status:`
              already `planned`.
Rejected:     **Leave S042 in block 4 as efficiency** -- the rule is that a
              known defect in the tree contaminates every measurement taken
              after it, and this one moves published scores in one game of
              six. **Fix it inside S207's repetition work retroactively** --
              S207 is history in `plan_done/`. **Abort the S219 run** -- it
              measures books with one binary on both sides; the defect
              cancels in the comparison and the night is not lost.
Consequences: `adocs/plan.md`'s Open list is re-sorted by one move; S042's
              file loses its "Not a correctness bug" section for the finding
              and its accepts gains the red-first case, the bench line, the
              Debug self-play and the SPRT. Every pre-registration written
              while S042 is open names it, per DEC-171's last sentence. fastchess's
              "PV continues after threefold repetition" count is now an
              instrument for this class: S042's stamp records it before and
              after over the same games (the S219 PGNs replayed, or a fresh
              1500-game match), and a count that does not fall to zero is a
              second cause.

## DEC-188  2026-09-12  The step subagent is Sonnet 5, not Opus 5: DEC-185's model clause is amended, the rest of it stands
Tags:         workflow, agents, coordinator, model, dec-185
Context:      DEC-185 (2026-09-11) set the operating rule: one clean subagent
              per plan step, briefed by the coordinator, which implements
              nothing itself, one task at a time, and named Opus 5 as the
              model, rejecting Sonnet with "the owner named Opus 5; quality
              over cost". Resuming the session on 2026-09-12 at 00:30, after
              S219's comparison had finished, the owner wrote: "continue the
              work as discussed in the session. You coordinate and fresh
              sonnet5 agents do the implementation. Continue and don't stop
              until I tell you."
Decision:     **By the owner.** The step subagent is **Sonnet 5** (`model:
              sonnet`) from this point on. Everything else in DEC-185 stands
              unchanged: one fresh subagent per step or self-contained part,
              a self-contained brief and a bounded report, the coordinator
              holds the machine and the shared documents and commits, one
              task at a time with the blocked-task exception, the handover
              written into `status.md` at each task close so the conversation
              can be compacted. DEC-185's rejection of Sonnet is VOID on this
              point; its other rejections stand. The coordinator does not stop
              at task boundaries to ask whether to continue: the owner's
              standing instruction is to keep going until told otherwise, and
              the reserved questions (goal, ethic, workstation risk) are the
              only ones that block.
Rejected:     **Keeping Opus 5 for steps that touch `src/`** and Sonnet for
              documents only -- the owner drew no such line, and a split rule
              needs a judgement call per step that the coordinator would then
              own; if Sonnet's work fails the Tier-1 fast check or the gate
              repeatedly, that is a number to bring to the owner, not a
              reason to deviate silently. **Treating the message as a slip**
              -- it names the model explicitly and repeats the coordinator
              framing, so it is an instruction.
Consequences: `AGENTS.md`'s AGENTS rule names Sonnet 5 with this id. A step's
              `author:` names the model that did it, so a future reader can
              attribute quality to the model that produced it: S219's
              pre-launch work was Opus 5, its reading and switch and everything
              after are Sonnet 5. The fast check after each step stays and is
              the instrument that says whether the change cost anything.
