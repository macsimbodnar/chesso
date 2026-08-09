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
