id:         S339
goal:       the mutation tool never scores a test that did not run as a kill, and refuses a malformed mutant by name instead of crashing
accepts:    (1) a ctest row that reads skipped or not run is not a kill: on a fixture where one test skips with exit 77 an unmutated tree reads no kill and a mutant killed by a real failure still reads killed, the first seen red on the tool as it stands; a run whose baseline has a skipped test says so in its report, naming the test; (2) `validate` refuses a pair that is not an (old, new) pair of strings, naming the mutant and its file, where it now dies on a `ValueError`; (3) the loader refuses a file that brings its own `m` and gives no pairs, naming the file, where `normalise` now dies on a `KeyError`; (4) each seen red first; every existing case of `tests/test_mutation_check.py` passes unchanged; no mutant's meaning moves
touches:    tools/mutation_check.py, tests/test_mutation_check.py, DEV_MANUAL.md
excludes:   running any mutant on the real tree; re-reading any kill; the loader's `*.py`-only, one-directory scan, which stays as it is and is stated in DEV_MANUAL; the parked full mutation pass
decisions:  DEC-171, DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why

Found by S338's implementer, 2026-11-11, beyond its step and not fixed there.

- **A skipped test is scored as a kill.** The tool's failed-row pattern also matches ctest's "did not run ... (Skipped)"
  rows. Seen on a two-test scratch project: an unmutated tree scored `killed`. `test_bare_fen_relay` skips with exit 77
  when no python imports python-chess, so on such a machine every mutant would read killed. Latent on the workstation,
  where the venv answers; the MacBook's state is not read.
- A pair that is not two strings crashes `validate` with a traceback naming neither mutant nor file.
- A file with its own `m` and no pairs crashes the loader the same way; only two evidence copies under `adocs/data/`
  use that form.

Filler under DEC-171: a tool, no play, no score, no UCI output. It binds before the next mutation reading taken on a
machine other than the workstation, and before the parked full pass. Agent-only, no `src/`, owns no run.
