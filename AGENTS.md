# Agent operating rules

Applies to any agent working in this repository. Additional to, not a replacement for, tool-level configuration.
Claude Code entry point: `CLAUDE.md` containing `@AGENTS.md`.

## 0. Repository overrides — read before section 2

This is the `achesso` branch — *agentic chesso*. The end goal is the strongest
CPU chess engine in the world — MIT-licensed and staying MIT, nothing
copy-pasted from another open-source project, its own NNUE or whatever
supersedes it, every change proved by SPRT and modern testing with specialized
tools taken or built ad hoc — built to find out what AI-driven development can
produce. `CLAUDE.md` is the full statement; DEC-013 and DEC-104 are the
decisions.

The rules below are house rules. They win over the stock ruleset wherever the
two disagree. **A moltke upgrade that rewrites this file must re-apply them**,
which is why they are stated here and not only in `decisions.md`.

- **Nothing is copied. Ever.** No source from another engine, no tables from
  another engine, no NNUE training data derived from another engine's evaluation
  or search. Ideas, techniques and published articles are used freely — reading
  the documented state of the art and implementing it here is the plan (DEC-014).
  Copying it is not; inspiration from another open-source engine is taken only
  where its licence consents, adapted, never copy-pasted (DEC-104). Another
  engine's constants are never seeds, wherever they are republished (DEC-084
  as amended by DEC-105). Running another engine's *binary* as a tool creates
  no derivative work and is encouraged. DEC-016.
- **The agent runs tests, measurements and evaluation tuning itself** (DEC-041),
  without asking, and schedules anything lasting several hours for the night if
  there is better work to do meanwhile. **NNUE training is still the owner's**:
  the agent builds the trainer, prepares the data and states the run, and the
  result comes back as a network measured by SPRT like any other change. The
  line is *running the network training*, not writing it. DEC-015 as amended by
  DEC-041.
- **Chess judgement comes from a tool, never from the agent.** No agent assesses
  a position, move, line or result from its own reasoning. This covers whether a
  position is winning, whether a move is a blunder, whether an ending is
  theoretically won, the material balance after a sequence, and opening
  soundness. Getting a position onto a board is itself a tool job. See
  `CLAUDE.md` for the tool per question, and DEC-023 for the failure that
  produced the rule.
- **A change that alters play is decided by SPRT, not by argument.** A change
  claimed behaviour-neutral proves it with identical node counts and best moves
  instead. INV-6. One change at a time — two at once and neither number means
  anything. A verdict of zero is recorded as zero.
- **A bug that has been found gets fixed before anything else starts.** Not
  noted, not scheduled, not carried into the next change. A known defect in the
  tree contaminates every measurement taken after it. This narrows section 3's
  "correctness jumps the queue" to: it jumps the queue *now*.
- **`README.md` is written by hand by the repository owner. No agent writes in
  it, ever.** The developer-facing document section 2 and section 7 call
  `README.md` is `DEV_MANUAL.md` in this repository — same purpose, same rewrite
  discipline, same checks. `MANUAL.md` behaves exactly as the stock ruleset
  describes. At step completion, "checked `README.md`, owner-written, no change
  needed" is the expected outcome and a valid one. DEC-017.
- **A match runs on every core the machine has.** `fastchess.sh` defaults to it
  and the default is not lowered to be polite -- nothing else should be running
  during a match anyway. Efficiency cores are included, knowingly: DEC-048
  supersedes DEC-042 on that, trading some variance for throughput because
  measurement capacity is the binding constraint on the plan and S027 spent
  about twenty hours of it on six verdicts. `CONCURRENCY` overrides when a run
  genuinely has to share the machine, and a run that lowers it says why.
- **A long run is detached, and the thing watching it must outlive the turn.**
  An SPRT takes three to four and a half hours here and a fit takes tens of
  minutes, so both are started detached — `nohup ... &` — and never held open by
  the turn that launched them. The watcher is the part that gets this wrong: a
  backgrounded shell loop is scoped to its turn and dies the moment the user
  types anything, silently, while the run itself carries on. In Claude Code use
  `Monitor` with `persistent: true`, which lives as long as the session. Any
  agent without that tool polls the log on its next turn instead and does not
  pretend a watcher is armed.
- **A watcher ends when its run ends, and the run's own last line is what ends
  it.** `persistent: true` outlives the turn *and* outlives `/clear`: the
  context holding the task id is discarded, the process is not, so an orphaned
  watcher can only be killed by pid. `tail -f` never exits on its own — a
  9-minute sweep left one holding for two hours. So: the detached run prints a
  terminal marker as its last action, and the watcher is a command that exits on
  that marker rather than a bare `tail -f`, with the failure signatures in the
  same alternation so a crash is not silence. `TaskStop` when the run is read is
  the belt; the self-exit is the braces. DEC-061.

Marker file: `.moltke.json` at repo root.
Present with `"enabled": true` means these rules are active and enforced.
Present with `"enabled": false` means the user declined; do not ask again, do not scaffold.
Absent: ask once whether to set the workflow up, then write the marker either way.

```json
{
  "schema": 1,
  "enabled": true,
  "plan_active_max": 1,
  "plan_stack_max": 3,
  "surface_guard": "cli",
  "test_command": "python3 -m unittest discover -s tests"
}
```

`plan_active_max` counts per author. `surface_guard` is one of `cli`, `api`,
`both`, `none` — `none` only alongside a `decisions.md` entry saying why.
`test_command` is optional; set it and step completion runs it and refuses on
failure.

## 1. Reading protocol and precedence

The SessionStart hook injects the stack, the derived next step, any staleness,
and the machine-local file. **A routine turn starts from that alone — zero
document reads.** Enter the documents on demand, smallest sufficient scope
first:

1. `adocs/status.md` and `adocs/plan.md` — read whole when orientation is
   needed; both are small and bounded by construction.
2. `adocs/specs.md` — read whole before changing behaviour; current state only.
3. `adocs/decisions.md` — never read whole. Grep the index by id, tag, or topic.

Precedence when documents disagree: **specs > plan > status**. Code that
disagrees with specs is a bug or an unrecorded decision, never silently the new
truth. **Filesystem state beats prose**: on any disagreement `plan_current/`
wins and `status.md` is regenerated. **Next is derivable, never asserted**: the
next step is the first in `plan.md` order not in `plan_done/`.

Instructions layer, most specific wins: `.moltke.local.md` (machine-local,
uncommitted, created by the tool) overrides the `## Project rules` section at
the end of this file, which overrides the base ruleset above it.

## 2. File map and write discipline

| Path | Purpose | Write mode |
|---|---|---|
| `README.md` | developer facing: layout, build, test, exact commands | rewrite; checked at every completion |
| `MANUAL.md` | end user facing: install, operate, teams, known bugs | rewrite; checked at every completion |
| `.moltke.local.md` | machine-local: tools, paths, per-platform directives; uncommitted, injected each session | edit freely, keep small |
| `adocs/status.md` | last done, in progress, next, blocked, parked | regenerated by `--step status`; Parked block carried verbatim |
| `adocs/specs.md` | prime directive, invariants, required behaviour — current state only | rewrite in same commit as any behaviour change |
| `adocs/plan.md` | plan description and ordered open steps; done entries pruned to the last 5 | rewrite on any plan change |
| `adocs/plan_todo/` `plan_current/` `plan_done/` | one file per step; the directories are the state machine | moved only by `--step` |
| `adocs/testing.md` | acceptance notes, voluntary documentation; union-merged | append rows with the work |
| `adocs/decisions.md` | living decisions with index, newest last | compact freely, ids stable, before or alongside the change |
| `adocs/audit/` | audit reports and reviews, evidence before fixes | add files, never overwrite a report |

`adocs/plan_done/` is never rewritten or trimmed, and this is enforced: it is
the project history. Every other document holds current state and may be
compacted — git is the archive.

## 3. Prime directive and invariants

`adocs/specs.md` opens with one **prime directive** and numbered **invariants**
`INV-1`, `INV-2`, ... stated as testable properties, referenced by number from
code, tests, and commits. Correctness outranks features: a reproduced defect
jumps the queue.

## 4. Plan lifecycle

Step files are named `S<nnn>_short_name.md`; ids are allocated in creation
order and **never reused or renumbered**. Fields: `id`, `goal`, `accepts`
(testable), `touches`, `excludes`, `decisions`, `closes`, `blocks`,
`paused_by`, `author`, `done` (stamp, written last).

- The plan is common; anyone picks the derived next step. `--step start`
  claims it: `author:` from `git config user.name`, and your own active steps
  are what block you — a teammate's never do.
- `plan_todo/` → `plan_current/` when work starts; `plan_current/` →
  `plan_done/` only when code is complete, the suite is green, and README and
  MANUAL were checked, and no watcher the step armed is still alive (§12).
  The move is the last action of the step, then commit.
- Work discovered mid-step: **trivial and in scope** — fix now, note it.
  **Blocking** — `--step block <parent> <name>` creates the child and pauses
  the parent; at most `plan_active_max` non-paused steps per author, stack
  depth capped by `plan_stack_max`. **Independent** — `--step new` and leave
  it in `plan_todo/`, however tempting.
- When the plan meets the code and loses: stop, write a `decisions.md` entry,
  amend the plan. Never deviate silently. A planning session ends in a commit.

## 5. Git

- The agent **never pushes**. The agent commits; the user pushes.
- Commit on request, on step completion, and on any plan change. Every commit
  is green: build, lint, full suite.
- Imperative subject under 72 characters; body says **why**; reference the
  step id and any `INV-n`.
- No history rewriting, no force operations. Teams run branch-per-member;
  merge semantics are in MANUAL's Teams section (union merges for `testing.md`
  and `status.md`, regenerate status after merging, INV-6 catches id
  collisions).

## 6. Testing

- **Red first.** A defect gets a minimized regression test before the fix, and
  the failure is observed and recorded, not assumed.
- **Non-vacuous by construction.** A test asserting X does not happen must
  first establish the precondition that would make X happen.
- Behaviour changes strengthen or re-target tests. Never relax one, never
  delete one to get green — deleting is a recorded decision.
- `test_command` in the marker is what enforces the green suite at completion;
  `adocs/testing.md` rows are voluntary documentation of what covers what.

## 7. Documentation

**Doc claims are claims about code**: any statement about a flag, a default, or
where output goes is traced to the code path producing it. At every completion
README and MANUAL are **checked** — concluding neither needs a change is valid,
not checking is not. A behaviour change updates `specs.md`'s current wording in
the same commit; the narrative lives in the step stamp and the commit message.
A golden test guards the public surface named by `surface_guard`; refresh it
only after specs and MANUAL describe the change. Copy code, commands, flags,
paths, and versions verbatim; never reword them.

## 8. Decisions

`adocs/decisions.md`, newest last, index on top. Stable ids `DEC-<nnn>`, topic
tags. Format: heading, `Tags:`, `Decision:` (operative sentences), `Why:` (one
line). Decisions belong to the user; agents propose. Supersede by rewrite or
delete — ids never reused, git keeps every earlier version. Trigger: before or
alongside the change, never after.

## 9. Review: fast check by habit, full audit by consent

**Tier 1 — every chunk.** After each `--step done`, one small subagent over
that step's diff: top real problems only, one screen, no writes, no report
file. Trivial → fix now (§4); real → a step; nothing → one console line.

**Tier 2 — proposed.** On real risk (security-touching, public surface, long
unaudited stretch) propose a full audit; the user accepts or postpones. A
postponed proposal is one Parked line in `status.md`.

**Tier 3 — on demand.** `/moltke:audit`: report file under `adocs/audit/`
named `YYYY-MM-DD_type.md` (`.2` suffix same-day), never overwritten. The
auditor runs on a **clean context** — repository path, commit, report path,
type, scope, nothing else; the blue team does not brief the red team. Report
written before any fix. Every finding gets an id, severity, and status
(`open`/`planned`/`closed`/`accepted`), and ends in a step whose `closes:`
names it or a decision. Findings close on a re-run that no longer reports
them, or by recorded decision. Audits run against the code, not the specs.

## 10. Hard prohibitions

The agent does not:

- push, force push, or rewrite git history
- write to `adocs/plan_done/`
- delete or weaken a test to make a change pass
- create plan step files outside the three plan directories
- claim a step complete before the suite is green
- complete a step that another step still declares in `blocks:`
- start independent work while one of its own paused steps sits in `plan_current/`
- arm a watcher lacking a self-terminating exit path (§12)

And one permission, stated so no rule above is misread as denying it:
**subagents may be spawned freely whenever useful** — audits, fast checks,
parallel exploration, anything. Nothing here requires or forbids spawning.

## 11. Memory lives in the repository

Nothing that matters may exist only in an agent's memory, transcript, or
tool-local notes. State into `status.md`, intent into `specs.md`, reasoning
into `decisions.md`, work into the plan directories, machine-specifics into
`.moltke.local.md`. The repository is the memory; everything else is a cache.

## 12. Watchers

A watcher is any background command armed to wake the agent when something
happens: a monitor over a log, a background shell loop, a tail in a pane.

**A watcher terminates on its own on every path.** Four exits, all mandatory:
success marker, failure marker, watched-process death, hard time ceiling. A
manual stop is a belt, never load-bearing — a watcher whose only exit is a
manual stop is a leak armed in advance. The ceiling bounds every mistake in the
other three, so it is never optional.

Arm through the primitive, never by hand:

```
python3 bin/moltke.py --watch RUN_LOG 'RUN-(DONE|FAILED)' --ceiling 8h --pid 12345
```

- Exit 0 marker seen, line printed. 4 `--fail-re` matched. 3 the watched pid
  died, checked once more against the log first. 124 ceiling reached.
- The whole file is scanned each poll, so a marker written before arming is
  still caught — the race a follow loses by construction.
- It registers under `.git/moltke_watch/` on arm and writes its outcome there on
  every exit path, kill included, so watch state is derivable from the
  filesystem (§11). Acting on a result means deleting its record.
- Ceiling at least 2x the expected run. The run prints its own terminal markers,
  success and failure both, before the watcher exists: a watcher with nothing to
  match is unbounded by construction. Prefer the harness's own background task
  when it notifies on exit; a watcher is for runs the session does not own.

**Banned forms.** `tail -f LOG | grep RE` never exits: tail has no last line,
and grep matching is not grep exiting. `tail -f LOG | grep -m1 RE` is worse —
grep exits on match, tail learns only by SIGPIPE on its next write, and a
finished log never writes again. In Claude Code both are refused at arm time,
along with any persistent monitor that is not the primitive; the one escape is
`MOLTKE_UNBOUNDED_OK` in the command, for a genuinely unbounded stream
(per-occurrence events, a dev-server error tail). Arm the primitive
`persistent`: the harness caps bounded monitors at an hour, so the `--ceiling`
is the real timeout and the process still ends itself.

Without the primitive, fall back to a poll loop wrapped in `bash -c` so the
interactive shell is irrelevant — never a follow:

```
timeout 8h bash -c '
  until grep -qE "RUN-(DONE|FAILED)" "$1"; do
    kill -0 "$2" 2>/dev/null || exit 3
    sleep 30
  done' _ RUN_LOG RUN_PID
```

`timeout` is coreutils on GNU userland. On BSD userland (macOS) there is none:
inline the deadline — `end=$(($(date +%s)+SECS))` before the loop,
`[ "$(date +%s)" -ge "$end" ] && exit 124` inside it. On Windows arm no
persistent watchers at all; poll with scheduled wakeups.

**Completion gate.** A step does not complete while a watcher it armed is still
alive, and a result taken from a watcher is acknowledged before the turn ends.
Session end asks: is anything still watching?

## Project rules

Rules written here override the base ruleset above, for this repository only.
This section is committed and travels with the project; machine-specific
instructions go in `.moltke.local.md` instead, which overrides both.

<!-- Loosen, harden, or replace any rule above. Examples:
     "The fast check after each step is skipped; this project reviews weekly."
     "test_command is the smoke suite; the full suite runs in CI only."
     Delete this comment when adding the first rule. -->
