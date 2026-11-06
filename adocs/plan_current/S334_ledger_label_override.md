id:         S334
goal:       `tools/ledger.py` reads a verdict row's description from a recorded override where a closing commit's subject breaks its rule, the first being `1ffc96e`
accepts:    the ledger's S122 row reads "the king safety family" (or the override's text) while `git log` is unchanged; the override is keyed by full sha, refused when its sha closes no verdict or when the subject already reads out cleanly, each refusal seen red on a fixture; `--table` otherwise byte-equal to today's; plan.md's table regenerated in the same commit
touches:    tools/ledger.py, tests/ (its ledger test, if any), adocs/plan.md, adocs/data/S334_*, adocs/data/README.md
excludes:   rewriting history; amending `1ffc96e`; changing the subject rule for later commits
decisions:  DEC-171, DEC-220
closes:
blocks:
paused_by:
author:     an Opus 5.5 subagent, briefed by the coordinator, 2026-11-06
done:

## Why

S122's H1 commit `1ffc96e` was subjected "Ship S122's fitted king safety family on its H1 (S122, DEC-353)"
instead of the form `ledger.py`'s header asks for ("Record S<nnn>'s H1 for <what>"). The rule strips the run id from
the middle and leaves "Ship 's fitted king safety family on its H1 (S122, DEC-353)" as the row's description,
read from `git log` at every regeneration. History is not rewritten, so the tool needs a recorded exception. The
mistake was the coordinator's (2026-11-05). Filler by DEC-171: it moves no play and no reported score.
