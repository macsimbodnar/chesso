id:         S311
goal:       a comment rule in `AGENTS.md` and `tools/comment_budget.py`, which reports what breaks it and proves a comment edit left the code identical
accepts:    the owner's word on the rule and on S312's scope, recorded as a decision; the rule in `AGENTS.md`'s Project rules; `tools/comment_budget.py --report <ref>` flags new or grown blocks over 15 lines, new id chains and log-shaped blocks and on HEAD lists the log-shaped set; `--identical <ref>` proves code identity, g++ or clang's preprocessor for C++, tokens for Python, the lexer for shell and CMake, observed red on an injected code change and green on `adocs/data/comment_study/pilot/`; the study's lexer fixtures a fast ctest; a `DEV_MANUAL.md` entry; report only, no gate; no dependency; both builds green
touches:    AGENTS.md, tools/comment_budget.py, tests/CMakeLists.txt, tests/test_comment_budget.py, DEV_MANUAL.md
excludes:   `adocs/data/` and `adocs/plan_done/`, which are evidence; any hunk `adocs/data/S101_pause_check.sh` names, until S101 closes; any match, fit or timing; any comment edit, which is S312's
decisions:  DEC-335, DEC-142, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

`adocs/data/comment_study/report.md`, read whole before starting: its "Proposed rule for `AGENTS.md`" is the draft the owner rules on,
and its lexers are the start of the tool. A rule costs almost nothing and stops about half of 15.5 KB a day, which no sweep does.

## The second machine

No `.tuning/`, books, worktrees or `.moltke.local.md`. `tools/gate.sh` on it may need its own `CLANG_FORMAT_MAJOR` (DEC-146): the pin is 23
and the workstation runs 22, so a rewrapped comment is the thing to check for. The compiler there is Apple clang, so the tool cannot assume
g++, and DEC-049's reference stays g++ on the workstation.
