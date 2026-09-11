id:         S214
goal:       three measurement tools stop failing silently -- `analyse_game.py` refuses to return a score it never read, `spsa_driver.py check` proves every axis reaches the search, and a fit's provenance stamp records every flag that selects which vector is emitted -- and, by DEC-184, the two copies of the completion gate are checked against each other and the tuner's last-group assertion names what it detects
accepts:    `tools/analyse_game.py` raises, or exits non-zero with the position named, when no `info` line carrying both a score and a `pv` arrived before `bestmove`, and it ignores `lowerbound`/`upperbound` lines for the final score, with a test or a recorded reproduction against a stubbed engine that answers `bestmove` alone; `tools/spsa_driver.py check` probes node-count reachability for **every** parameter in the config, not the hardcoded `RfpMargin` alone, and a parameter whose two probe values search identically fails the check by name; `tools/tuner.cpp`'s provenance stamp carries `--epochs`, `--report`, `--patience` and `--threads` beside the commit, digest, K, seed, lr and split, because `--report` and `--patience` select which vector is emitted; `DEV_MANUAL.md`'s tuner and SPSA sections state the new behaviour where they describe the old; **DEC-184** `tools/plan_prose_check.py` gains a `--gate` mode that reads the completion command from `AGENTS.md`'s TESTS rule and from `DEV_MANUAL.md`'s test section and exits non-zero when they differ, registered in the fast suite in `tests/CMakeLists.txt` beside `test_plan_params`, observed red against a one-character edit to either copy before green; `tests/test_tuner_groups.cpp`'s precondition assertion on `TEMPO_EG_BASE + TEMPO_COUNT == PARAM_COUNT` carries a message naming what it detects -- a parameter block appended after `tempo` and given no group of its own is silently covered by `tempo` -- so a failure names the right thing (S041's parked finding, accepted as to structure)
touches:    tools/analyse_game.py, tools/spsa_driver.py, tools/tuner.cpp, tools/plan_prose_check.py, tests/, tests/CMakeLists.txt, tests/test_tuner_groups.cpp, DEV_MANUAL.md
excludes:   the tuner's clamp treatment in its gradient (F24, folded into S126's file), the regularisation question (DEC-170 records the ruling), the corpus phase column (F36, folded into S134's file), `eval_spread`'s candidate margins (F25, folded into S039's file)
decisions:  DEC-170, DEC-171, DEC-184
closes:     2026-09-10_adversarial-F28, 2026-09-10_adversarial-F29, 2026-09-10_adversarial-F35
blocks:
paused_by:
author:
done:

## Why this exists

Three low findings of `2026-09-10_adversarial` in the tools the rules lean on,
none touching `src/`, so the step is filler beside any run (DEC-171, DEC-172).

- **F28.** `tools/analyse_game.py` initialises `score, best = 0, "-"` and
  returns them if no `info` line carrying both a score and a `pv` arrives; it
  also accepts bound lines. It is the tool `CLAUDE.md` mandates *because* agent
  chess judgement is banned, so a silent 0.00 is the DEC-023 failure arriving
  through the instrument that exists to prevent it.
- **F29.** `spsa_driver.py check` validates presence and bounds for every
  parameter and then probes reachability for one, `RfpMargin`. An axis wired
  to a variable nothing reads would random-walk and read as tuned. S127's
  full-set run is what this guards.
- **F35.** The provenance line `tools/tuner.cpp` emits omits four flags, two
  of which decide which vector the run emits; a fit is therefore not
  reproducible from its own stamp.

## Cost

Agent work, two to three hours; no run, no `src/` change.
