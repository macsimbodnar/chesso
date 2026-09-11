# Agent operating rules

Applies to any agent working in this repository. `adocs/` is the project's
memory; these rules say how to read it and how to keep it current. Nothing
here is machine-enforced: the rules work because agents follow them, and a
violation is visible in the diff. Set up by `/moltke:init`, which also wrote
the `## Project rules` section at the end — the per-project answers that
complete this generic base. Change those with `/moltke:rules`.

Claude Code entry point: `CLAUDE.md` containing `@AGENTS.md`. Instructions
layer, most specific wins: `.moltke.local.md` (machine-local, uncommitted)
overrides `## Project rules`, which overrides the base ruleset above it.

## Orient

Start every session by reading, in this order:

1. `adocs/status.md` — last done, in progress, next, blocked, parked.
2. `adocs/plan.md` — what is being built, and the ordered open steps.
3. `.moltke.local.md`, if present — machine-local notes, uncommitted.

That is enough to act on. Go deeper only on demand:

- `adocs/specs.md` — what the project must do and never break. Read whole
  before changing behaviour; it holds current state only.
- `adocs/decisions.md` — why things are the way they are. Never read whole:
  grep the index by id or topic.
- `adocs/plan_done/` — one file per finished step. Read one when you need to
  know how something got the way it is.

Precedence when they disagree: **specs > plan > status**. Filesystem beats
prose: the plan directories are the state and `status.md` is a view of them —
regenerate the view, never bend the directories to match it. Code that
disagrees with specs is a bug or an unrecorded decision, never silently the
new truth.

## The plan

One file per step. The file's directory is its state, and state changes by
moving the file — by hand, deliberately:

| Directory | Meaning |
|---|---|
| `adocs/plan_todo/` | agreed, not started |
| `adocs/plan_current/` | in progress |
| `adocs/plan_done/` | finished, `done:` stamp written last |

Step files are named `S<nnn>_short_name.md`:

```
id:         S042
goal:       one line
accepts:    what proves it done, testable
touches:    areas affected
excludes:   explicitly out of scope
closes:     <!-- audit finding ids, when any -->
paused_by:  <!-- blocking child's id, only while paused -->
author:     <!-- who claimed it, set on start -->
done:       <!-- completion stamp: what proves it finished, written last -->
```

A new id is one more than the highest ever allocated, across all three
directories — ids are never reused or renumbered, even for a deleted step.
Order lives in `plan.md`'s Open list and nowhere else; the next step is the
first entry there.

- **Start**: move todo → current, set `author:`. Respect PLAN's active limit;
  a paused step does not count against it.
- **Discovered mid-step**: trivial and in scope — fix it now, note it in the
  stamp. Blocking — create the blocker directly in `plan_current/`, set
  `paused_by:` on the parent. Independent — a new step in `plan_todo/`,
  however tempting.
- **Finish**: `accepts` holds and the TESTS and DOCS rules are satisfied; then
  write the `done:` stamp, move the file to `plan_done/`, update `plan.md`
  (out of Open, into the last-five Done list) and `status.md`, and commit per
  COMMITS. Completing a blocking child clears the parent's `paused_by:`.
- `plan_done/` is history: never edit, rewrite, or delete anything in it. A
  done step that got something wrong gets a new step or a decision, not an
  edit.
- When the plan meets reality and loses: stop, record a decision, amend the
  plan. Never deviate silently.

## Keep the memory current

- `status.md`: rewrite by hand at the end of any turn that changed plan
  state. Everything under `Parked:` is human memory — carry it forward, prune
  it only deliberately.
- `specs.md`: a behaviour change updates its wording in the same commit.
- `decisions.md`: a choice a future reader would re-derive gets a `DEC-<nnn>`
  entry when it is made, not after. Index on top, newest entry last, ids never
  reused. Format: heading, `Tags:`, `Decision:` (and by whom), `Why:` (one
  line). Decisions belong to the user; agents propose.
- Nothing that matters lives only in a transcript or an agent's memory. The
  repository is the memory.

## Review and audit

- **Fast check**, when REVIEW says so: after a step completes, one small
  subagent over that step's diff — top real problems only, one screen, no
  writes. Trivial → fix now. Real → a new step. Nothing → one line, move on.
- **Full audit**: `/moltke:audit` — an adversarial reviewer on a clean
  context writes a dated report under `adocs/audit/`; findings become steps
  or recorded decisions. Reports are evidence: never overwrite one, and the
  only edit an earlier report takes is a finding's `Status:` line moving
  (open → planned / closed / accepted).

## Project rules

Interview answers recorded by `/moltke:init`; change them with
`/moltke:rules` (or edit by hand), and record every change as a decision.
One line per rule, stable id first.

These are the `achesso` branch's house rules — *agentic chesso*. The end goal
is the strongest CPU chess engine in the world: MIT-licensed and staying MIT,
nothing copy-pasted from another open-source project, its own NNUE or whatever
supersedes it, every change proved by SPRT and modern testing with specialized
tools taken or built ad hoc — built to find out what AI-driven development can
produce. `CLAUDE.md` is the full statement; DEC-013 and DEC-104 are the
decisions. They win over the base ruleset above wherever the two disagree, and
**a moltke upgrade that rewrites this file must re-apply them** — which is why
they are stated here and not only in `decisions.md`. They survived the v1
migration that way (DEC-109).

- GIT: commit freely; never push — the user pushes. No history rewriting, no
  force operations.
- COMMITS: commit at each completed step and at any plan change. Every commit
  is green. Imperative subject under 72 characters; the body says **why** and
  references the step id and any `INV-n`. A commit that touches `src/` ends
  with `Bench: <nodes>`, the total `chesso bench` prints, or `No functional
  change` when the total is the parent's; `tools/gate.sh` checks it, and a
  mismatch is a red gate. From S189's completing commit on. DEC-140.
- TESTS: the suite is green before a step is marked done, in **both** builds —
  `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`
  (`-j8`, the core count of the machine `.moltke.local.md` describes). The tune
  build is in the gate because `src/search_params.hpp` is *deliberately
  different code* under `CHESSO_TUNE=ON` — a constant in one build and a
  settable variable in the other — so the two can diverge, and a break there is
  otherwise found months later by whoever next tries to tune (DEC-118). A defect
  gets a minimized failing test before its fix, and the failure is observed,
  not assumed. Never relax a test, never delete one to get green — deleting is
  a recorded decision. A test asserting X does not happen first establishes the
  precondition that would make X happen. **The second tier, DEC-141:** a step
  that touches `make_move`, `unmake_move`, the generator or the search
  self-plays the Debug binary — four rounds of `fastchess` at 4+0.04 — and
  greps its log for `Assertion` before completing, and its stamp says so; a
  new pruning, reduction or extension rule ships with a direct guard test and
  a mutant that test kills (`tools/mutation_check.py`, S196); and
  `tools/gate_extra.sh` (S197) — the Debug binaries, a sanitizer build, deep
  perft, the prose and citation checks — runs before such a step completes
  and otherwise weekly, noted in `status.md`. **Goldens, DEC-142:** every
  golden number in `tests/` is named as one at its site with the script that
  re-derives it, and is re-derived by that script whenever either end moves,
  never re-read from a run; a golden that cannot be scripted is a finding.
- DOCS: `README.md` is written by hand by the repository owner. **No agent
  writes in it, ever.** The developer-facing document is `DEV_MANUAL.md`, and
  it and `MANUAL.md` are checked at every step completion — concluding neither
  needs a change is valid, not checking is not. A behaviour change updates
  `specs.md`'s current wording in the same commit. Any statement about a flag,
  a default or where output goes is traced to the code path producing it.
  DEC-017.
- SURFACE: `test_uci_surface` is the golden guard over the UCI surface;
  refresh it only after `specs.md` and `MANUAL.md` describe the change.
- AGENTS: **every plan step is implemented by one clean Opus 5 subagent,
  briefed by the coordinator, which implements nothing itself** (DEC-185,
  2026-09-11). The brief is self-contained: the step file, the decisions it
  cites, the files to read, what not to touch, the shape of the report. The
  subagent writes code, tests, its data files and its own step file; the
  coordinator holds the machine, owns the shared documents, runs the Tier-1
  fast check over the result and commits. **One task at a time**: a second
  subagent starts only while the first is blocked on something long -- a match
  holding the machine, a download not yet made. After each task closes the
  coordinator reads the report and not the transcript, writes the handover into
  `status.md`, and says so, so the conversation can be compacted. Audits, fast
  checks and exploration stay free (DEC-106).
- REVIEW: fast check after each completed step, over that step's diff.
- AUDIT: on demand only — `/moltke:audit` when the user asks for it.
- DEPS: never add a dependency without asking; state what it buys and what
  writing it by hand costs, then let the user decide.
- PLAN: **as many active, non-paused steps as are strictly necessary, one per
  agent, and exactly one of those agents is the coordinator.** The coordinator
  is the machine holder: a match, an SPSA run, a fit or a timing is started by
  it and by nobody else, so the two classes of work never contend. It also owns
  the shared documents -- `plan.md`, `status.md`, `specs.md` and
  `decisions.md` are written through the coordinator, because two agents
  stamping the same file is how the memory loses an edit. Every other active
  step is agent-only work that owns no run and writes its own step file.
  *Strictly necessary* is the bound and it is not "the list is long": a further
  step starts because the machine would otherwise sit idle, or because an
  active step cannot advance without it. DEC-113.
- COPYING: **nothing is copied, ever.** No source from another engine, no
  tables from another engine, no NNUE training data derived from another
  engine's evaluation or search. Ideas, techniques and published articles are
  used freely — reading the documented state of the art and implementing it
  here is the plan (DEC-014). Copying it is not; inspiration from another
  open-source engine is taken only where its licence consents, adapted, never
  copy-pasted (DEC-104). Another engine's constants are never seeds, wherever
  they are republished (DEC-084 as amended by DEC-105). Running another
  engine's *binary* as a tool creates no derivative work and is encouraged.
  DEC-016.
- MEASUREMENT: **a change that alters play is decided by SPRT, not by
  argument.** A change claimed behaviour-neutral proves it instead with
  identical node counts and best moves from `tools/search_bench.py` (INV-6).
  One change at a time — two at once and neither number means anything. A
  verdict of zero is recorded as zero, and the feature may still be kept with
  the reason stated. A run's pre-registration states its bounds pair's
  worst-case expected games from the nElo formula and its abort rule beside
  the three outcomes, and a fixed-rounds A/A of 1000 games follows every
  change to the harness — fastchess version, book, adjudication, machine —
  read with `adocs/data/S105_pairs.py` before the next verdict. DEC-143.
- MACHINE: a match runs on every core the machine has. `fastchess.sh` defaults
  to it and the default is not lowered to be polite — nothing else should be
  running during a match anyway. Efficiency cores are included knowingly
  (DEC-048 supersedes DEC-042), trading variance for throughput because
  measurement capacity is the binding constraint on the plan. `CONCURRENCY`
  overrides when a run genuinely has to share the machine, and a run that
  lowers it says why.
- RUNS: the agent runs tests, measurements and evaluation tuning itself
  (DEC-041), without asking. **Four hours is the line (DEC-155):** a run
  expected to take less than four hours starts when it is ready, during the
  day; four hours or more is scheduled for the night if there is better work
  to do meanwhile. The estimate is stated before the run starts and comes
  from the measured throughput in `.moltke.local.md`, not from a guess. **NNUE training is still
  the owner's**: the agent builds the trainer, prepares the data and states the
  run, and the result comes back as a network measured by SPRT like any other
  change. The line is *running the network training*, not writing it. DEC-015
  as amended by DEC-041.
- CHESS: **chess judgement comes from a tool, never from the agent.** No agent
  assesses a position, move, line or result from its own reasoning — whether a
  position is winning, whether a move is a blunder, whether an ending is
  theoretically won, the material balance after a sequence, opening soundness.
  Getting a position onto a board is itself a tool job. `CLAUDE.md` has the
  tool per question and `TOOLCHAIN.md` has the one Stockfish invocation that
  lies; DEC-023 is the failure that produced the rule.
- BUGS: **a bug that has been found gets fixed before anything else starts.**
  Not noted, not scheduled, not carried into the next change. A known defect in
  the tree contaminates every measurement taken after it.
  **Scoped by reach, DEC-171:** that clause binds a defect reachable in
  ordinary play, on the UCI surface as GUIs and harnesses drive it, or able
  to move a reported score, move or line. Any other defect is a step
  scheduled as filler behind the next strength step, closed by the block
  boundary it sits in, and named by finding id in the pre-registration of
  every run taken while it is open.
- WATCHERS: **a long run is detached and the thing watching it terminates on
  its own.** An SPRT takes hours here and a fit takes tens of minutes, so both
  start detached — `nohup ... &` — and are never held open by the turn that
  launched them. The run prints a terminal marker as its last action, success
  and failure both, before any watcher exists.

  A watcher has four exits and all four are mandatory: success marker, failure
  marker, watched-process death, hard time ceiling. A manual stop is a belt,
  never load-bearing. **Banned forms:** `tail -f LOG | grep RE` never exits,
  and `tail -f LOG | grep -m1 RE` is worse — grep exits on match, tail learns
  only by SIGPIPE on its next write, and a finished log never writes again.
  Poll the whole file instead, so a marker written before arming is still
  caught.

  moltke v1 ships no `--watch` primitive, so the mechanism is a poll loop
  wrapped in `bash -c` — never a follow. macOS has no `timeout`, so the
  deadline is inlined:

  ```
  bash -c '
    end=$(($(date +%s)+32400))
    until grep -qE "RUN-(DONE|FAILED)" "$1"; do
      kill -0 "$2" 2>/dev/null || exit 3
      [ "$(date +%s)" -ge "$end" ] && exit 124
      sleep 30
    done' _ RUN_LOG RUN_PID
  ```

  In Claude Code arm that through `Monitor` with `persistent: true`, which
  outlives the turn; an agent without such a tool polls the log on its next
  turn and does not pretend a watcher is armed. Ceiling at least 2x the
  expected run — **and a ceiling is wall-clock only while the machine is
  awake**: S024's first run hibernated for 42 hours with a 9 h ceiling armed
  and the watcher did not fire. A step does not complete while a watcher it
  armed is still alive, and a result taken from a watcher is acknowledged
  before the turn ends. DEC-061, DEC-109.
- POWER: **no timed match starts on battery.** `pmset -g ac` must report an
  adapter first. S024's first run drained the machine from 78 % to 1 %,
  hibernated mid-match, and produced 8 time forfeits and two halves that do not
  look like the same experiment — 1132 pairs at -5.06 +/- 11.17 before the
  sleep against 137 at +35.63 +/- 30.99 after. It was aborted rather than
  reported. DEC-109.
