id:         S338
goal:       a mutant whose anchor no longer matches the tree turns the fast suite red in the commit that broke it, not at the next mutation run
accepts:    a fast-suite test validates every mutant under `tools/mutants/` against the tree it runs in, by the tool's own loader and validator and nothing parallel to them, building and running nothing; it is seen red first on a planted break of each kind the validator refuses (an anchor matching no line, an anchor matching twice, a mutant file that does not load), each naming the mutant and the file; it is green on the tree as it stands; its wall time is read and it sits under the fast label's clock with room; DEV_MANUAL.md's mutation section says what the test covers and what it does not (a mutant that validates and is no longer killed is still the mutation run's to find)
touches:    tools/mutation_check.py, tests/test_mutation_check.py, tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   running any mutant; re-reading any kill; changing any mutant's meaning; the parked full mutation pass (`adocs/status.md`, Parked)
decisions:  DEC-171, DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why

Found by S102 phase 1's follow-up, 2026-11-11. S102 gave `src/evaluation.cpp` `outside_pawn_terms` a sixth argument,
and the anchor of mutant PT08 in `tools/mutants/S118_pawn_cache.py` then matched no line. The tool's validator refuses
a whole mutant file on one such anchor, so no mutation run over `tools/mutants/` was possible on that tree, and both
gates were green: no fast test loads the real mutant list. The break was seen only because the follow-up had to
re-read another mutant's kill. S102's delivery re-anchors PT08; this step makes the next one visible.

It is filler under DEC-171: the defect is in a tool, reaches no play, no score and no UCI output, and sits behind the
strength step in progress. Agent-only, no `src/`, owns no run.

## Notes for the implementer

- `tools/mutation_check.py` `validate` takes a worktree and `main` refuses the main tree; the test needs the
  validation alone, on whatever checkout it runs in, without the clean-commit and green-baseline conditions a
  measuring run needs. Whether that is a flag or a function the test imports is the implementer's, stated in the stamp.
- The existing `tests/test_mutation_check.py` gates the driver on fixtures. The new check reads the real list, so it
  moves whenever `src/` moves under an anchor: that is its purpose, and its failure message must say which mutant, which
  anchor, how many matches, so the fix is a re-anchor and never a deletion.
