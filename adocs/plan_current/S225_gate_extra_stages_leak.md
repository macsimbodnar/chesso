id:         S225
goal:       `tools/gate_extra.sh`'s documented single-stage invocation (`STAGES="sanitize" tools/gate_extra.sh`) no longer fails its own gate, because the script stops leaking `STAGES` into the fast label it runs
accepts:    `gate_extra.sh` clears `STAGES` from the environment of every stage it runs (an `export -n STAGES` or an explicit `env -u STAGES` around the `ctest` calls) so that `test_gate_extra_script`, which drives a nested `gate_extra.sh` and expects five stages, passes inside a `STAGES="sanitize"` run; `tests/test_gate_extra_script.sh` (or the test that owns it) gains a case that sets `STAGES` in the outer environment and asserts the nested script still runs all five, observed red against `git show HEAD:tools/gate_extra.sh` first; `DEV_MANUAL.md`'s two dated paragraphs about the trap (2026-09-13) are shortened to the fix; no `src/` change
touches:    tools/gate_extra.sh, tests/, DEV_MANUAL.md
excludes:   any other change to the stages, their order or their markers
decisions:  DEC-141, DEC-167, DEC-171
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-14 14:34, filler while S091 waits for its SPRT night and S222 for its SPSA night
done:

## Why this exists

Found 2026-09-13 while fixing the sanitizer build for S109's second-tier gate:
`STAGES="sanitize" tools/gate_extra.sh` puts `STAGES` in the script's
environment, stage 4's `fast` label inherits it, and `test_gate_extra_script`
drives a nested `gate_extra.sh` that then runs one stage where nine of its
checks over two cases expect five -- `GATE-EXTRA-FAILED: sanitize` after
531 s, with the C++ build green. Proven directly: the test fails with `STAGES`
set and passes under `env -u STAGES`. The header documents the invocation as
supported, so the harness defeats its own documentation. Not reachable in play
and it moves no score, so DEC-171 places it as filler behind the next strength
step; `DEV_MANUAL.md` carries the trap until this lands.

## Cost

Agent work, under an hour; the sanitizer stage once (about nine minutes) as
the proof. No run.
