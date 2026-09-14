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
done:       2026-09-14 14:55. **The documented subset invocation passes its own gate.** Red first: the new case run against `git show HEAD:tools/gate_extra.sh` (`719e557`, saved to `.tuning/coord/S225_old_gate_extra.sh`) gave exactly one failure, `FAIL: 11: STAGES reached a stage's environment (STAGES=sanitize ), so a nested gate_extra.sh would not run all five stages`, with cases 1 to 10 green -- so the case is the only thing that moved. The fix is one statement in the driver of `tools/gate_extra.sh`: `stages` is read from `STAGES` exactly as before and then `unset STAGES` runs before the first stage, with a comment naming the guard; `unset` rather than `export -n`, which bites only when the variable was exported. Case 11 of `tests/test_gate_extra_script.sh` drives the sandboxed script with `STAGES=sanitize` exported in its own environment and asserts three things -- exit 0, a `GATE-EXTRA-DONE 1 stages` marker so the documented subset still works, and every line the `ctest` stub recorded of its own environment reading `STAGES=unset`; the stub appends `${STAGES-unset}` per call, because a nested `gate_extra.sh` driven from inside a ctest sees exactly that environment. The stages, their order and their markers are untouched, and the sixteen cuts of `adocs/data/S197_script_mutants.py` regenerate against the fixed script -- every anchor still resolving exactly once -- and are still 16 of 16 killed. **Proof, the real invocation**: `STAGES="sanitize" tools/gate_extra.sh` detached at 14:42:28 with `CLANG_FORMAT_MAJOR=22` exported, marker `GATE-EXTRA-DONE 1 stages 591 s /tmp/chesso_gate_extra_20260914_144226` -- 9 m 51 s wall, the whole `fast` label green under the sanitizer at 38 of 38 with `test_gate_extra_script` Passed 0.36 sec, which is the check that was red on 2026-09-13, and INV-6 across builds both 5950740 nodes. The `fast` label in `build` is 38 of 38 in 118.8 s besides, and `bash -n` parses both changed scripts. Docs: `DEV_MANUAL.md`'s two dated paragraphs shorten to the fix -- the `<regex>` one keeps its content and names the commit `1952c56` where it said "until S225", the `STAGES` one now says the script unexports it and names case 11 as the guard -- and the same section's case count and cost are re-derived, eleven cases at about 0.4 s. The header of `tools/gate_extra.sh` already described the invocation truthfully and is unchanged. `MANUAL.md` is the UCI manual and needs nothing. In passing, the stale "Seven cases" in the `test_gate_extra_script` registration comment of `tests/CMakeLists.txt` became eleven. No `src/` change: `No functional change`. **Fast check, 2026-09-14, four findings, one real and minor**: the test itself inherited an ambient `STAGES` -- `STAGES=sanitize bash tests/test_gate_extra_script.sh tools/gate_extra.sh` gave nine failures in cases 1 and 2 against the fixed script, a red from the caller's shell that would have looked like a real one under `tools/gate.sh` or a bare `ctest -L fast`; fixed by the coordinator with `unset STAGES` at the top of `run_sandbox`'s subshell before case 11's explicit export, the test then green with the variable in the caller's shell and without, and the committed script still red at case 11 alone. Cosmetic: the registration comment reflowed to 80 columns, "unexports" became "unsets" in `DEV_MANUAL.md`. The sixteen cuts of `adocs/data/S197_script_mutants.py` carry no cut for the new `unset STAGES` line; case 11 catches its deletion directly and `DEV_MANUAL.md` states the cuts' scope, so the gap is documented and not closed here.

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
