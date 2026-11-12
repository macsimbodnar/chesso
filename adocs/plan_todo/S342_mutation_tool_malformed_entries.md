id:         S342
goal:       the mutation tool refuses any malformed entry of a mutant file by name, before the baseline, whatever is missing or mistyped
accepts:    (1) an entry from a file's own `m` that lacks `file`, `id`, `klass` or `note`, one that is not a mapping, and a mutant whose `file` is not a string are each refused naming the entry and its file, with no traceback and no baseline run, where today each dies on a `KeyError`, `ValueError` or `AttributeError`, the last two cases after the baseline has run; each seen red first; (2) a row left `unmeasured` because its only listed row did not run or timed out says why in its note; (3) `DEV_MANUAL.md` says a baseline row that reads `(Not Run)` is refused as red, where it now says every did-not-run row is named after the green line; (4) every existing case of `tests/test_mutation_check.py` passes unchanged; no mutant's meaning moves
touches:    tools/mutation_check.py, tests/test_mutation_check.py, DEV_MANUAL.md
excludes:   running any mutant on the real tree; re-reading any kill; the parked full mutation pass
decisions:  DEC-171, DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why

Found by S339's implementer and its fast check, 2026-11-12, beyond that step and not fixed there. Probed on the fixed
tool (`.tuning/coord/S339/evidence/`), the `id` and the `klass`/`note` halves read from the code:

- An own-`m` entry without `file` dies on a `KeyError` in `validate`; one without `id` by the same path.
- An own-`m` entry that is not a mapping dies in `normalise` on `dict(entry)`.
- A mutant whose `file` is not a string dies on an `AttributeError` at `rel.startswith`, through the tool's own `m` too.
- An own-`m` entry without `klass` or `note` passes `validate` and dies on a `KeyError` in `main` when its row is
  built, after the baseline; the `finally` still reverts the mutant.
- From the fast check, cosmetic: an `unmeasured` row from a lone `(Not Run)` or a lone timeout carries no note; one
  sentence of `DEV_MANUAL.md` holds for Skipped and Disabled rows and not for `(Not Run)`.

None is reachable from `tools/mutants/` as it stands: 264 mutants, 276 pairs, all validating, no entry of these
shapes. Filler under DEC-171, the lowest in Open: a tool, no play. Agent-only, no `src/`, owns no run. It binds before
the parked full mutation pass.
