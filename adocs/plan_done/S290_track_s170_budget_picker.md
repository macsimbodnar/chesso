id:         S290
goal:       the script that applies S203's budget rule to `adocs/data/S170_cases.tsv` is tracked in the tree and named where the budgets are, so the golden is re-derivable from a fresh clone
accepts:    `.tuning/coord/S099/p2/pick_budgets.sh` (gitignored, used by S099, S110, S111, S286, S083 and S289) is committed under `adocs/data/` or `tools/` byte for byte or with its changes stated; `adocs/data/S170_cases.tsv`'s header, `tests/test_mate_carry.cpp`'s golden site and `DEV_MANUAL.md` "Goldens: named, scripted, re-derived" name the tracked path; re-run on S083's grid (`adocs/data/S083s_sweep_s170.txt`) against the parent's TSV (`git show e972bd4:adocs/data/S170_cases.tsv`), the tracked picker reproduces `01222f3`'s `adocs/data/S170_cases.tsv` budgets (A 1000000, B 100000, C 1000000, D 4000000, E 500000, F 100000) exactly, using the same arguments S083 used; the fast suite green in both builds
touches:    adocs/data/S170_pick_budgets.sh (new, the picker), adocs/data/S290_picker_check.txt (new, its proof), adocs/data/S170_cases.tsv header comments, tests/test_mate_carry.cpp comments, DEV_MANUAL.md, adocs/data/README.md
excludes:   any budget or ceiling value; the rule itself (DEC-156, DEC-162, DEC-299)
decisions:  DEC-142, DEC-171, DEC-299
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-25 -- the S170 budget picker is tracked as adocs/data/S170_pick_budgets.sh (the awk program byte for byte the original's, a usage line, set -euo pipefail and argument checks added); re-run on S083's grid (S083s_sweep_s170.txt) against e972bd4's TSV with S083's arguments it reproduces 01222f3's budgets exactly (A 1000000, B 100000, C 1000000, D 4000000, E 500000, F 100000; adocs/data/S290_picker_check.txt), the accepts amended by the coordinator for the three budget moves since; the TSV header, test_mate_carry.cpp's golden site, DEV_MANUAL and the README rows name the tracked path; comments only, src/ untouched; fast suite 50/50 in both builds. Implemented blind on b860a8a, rebased and built by a second Opus 5.5 subagent, fast-checked twice; stamped by the coordinator.

## Why this step exists (the coordinator, 2026-10-22)

S289's fast check: the S170 budgets are applied by
`.tuning/coord/S099/p2/pick_budgets.sh`, which lives under the gitignored
`.tuning/` and is not named in the TSV's header -- the same at S286 and S110.
DEC-142 wants every golden re-derived by a script named at its site; a script
outside the tree is not re-derivable from a clone. Not reachable in play or on
the UCI surface, so a DEC-171 filler behind the next strength step, named in
the pre-registration of every run taken while it is open.

**The accepts moved (the coordinator, 2026-10-25).** The first accepts asked
for S289's budgets back from S289's grid. Since then the budgets moved three
times -- S292 (C 500000, DEC-310), S293 (C 1500000, DEC-312) and S083
(`01222f3`, DEC-313) -- after S289's guard was reverted (DEC-309;
`.tuning/coord/S290/PENDING.md`, its 20:40 entry), so the proof is re-taken on
the grid that set the budgets the tree carries now, with the arguments S083
gave the original.

## As done (the implementer, 2026-10-25)

The first implementation was made blind on `b860a8a` (DEC-260) in
`../chesso-s290` and proved on S289's grid. This one rebases it onto
`e3621b8` in `../chesso-s290b` (`git apply --3way --index` of
`.tuning/coord/S290/s290.diff`, sha256 checked) and re-takes the proof on
S083's grid with the machine free.

**One conflict, `adocs/data/README.md`, and it was positional**: both sides
appended after the `S289_sprt.sh` row -- this tree's S289 completion rows and
its S292, S291 and S293 sections, the diff's S290 section. Kept all of this
tree's text and put the S290 section after S293's, in date order. Clean:
`DEV_MANUAL.md`, `adocs/data/S170_cases.tsv`, `tests/test_mate_carry.cpp`,
the new files and the step file's move.

**What the rebase made false, and its fix.** Every "from S099 to S289" now
reads "from S099 until S290" (S292, S293 and S083 used the `.tuning/` copy
after S289), and every "re-runs it on S289's grid" now names S083's: the
picker's header (two comment clauses; the awk program untouched), the TSV's
S290 paragraph (whose "the S286 and S289 notes below" named a note the revert
removed: now the S286, S292, S293 and S083 notes, each of which says its
budgets were applied by script), `test_mate_carry.cpp`'s comment,
DEV_MANUAL's goldens paragraph and the README's S290 section. The README's
`S289_sweep_s170.txt` row lost "`S290_picker_check.txt` re-runs it".

**The picker is `adocs/data/S170_pick_budgets.sh`**, beside the fixture it
rewrites and the sweep that feeds it. It is
`.tuning/coord/S099/p2/pick_budgets.sh` (sha256
6c18cf5733cab362c0392b7aeac1c58426aef8b2e9434a8be365153f686b4809) with these
changes and no others, each stated in its own header: the usage line names
the new file; three header paragraphs (provenance, the rule and its
decisions, how the two scripts fit, the exit codes); `set -euo pipefail`; an
argument check before the awk program (exit 2 on a usage error or an
unreadable file). The awk program is the original's byte for byte. Mode
100755.

**The proof is `adocs/data/S290_picker_check.txt`**, every command printed
before its output. `.tuning/coord/S083/reland2/chain.sh` ran the original on
its tree's HEAD TSV (`e972bd4`'s; `cmp` identical) and the `.tuning/` copy of
`S083s_sweep_s170.txt` (the same sha256 as the tracked one). The tracked
picker on those two prints A 1000000 -> 1000000, B 100000 -> 100000,
C 1500000 -> 1000000, D 500000 -> 4000000, E 300000 -> 500000, F 300000 ->
100000, exit 0, and the budgets' diff against `01222f3`'s TSV is empty, as is
the diff of every non-comment line. The whole file's diff is the 20 comment
lines S083's note added by hand and nothing else; the picker writes no
comment. Its stdout and stderr equal the `S170_cases.picked.tsv` and
`picks.txt` S083's run recorded. `01222f3`'s TSV through it comes back byte
for byte, as does the TSV as S290 leaves it; the original prints the same
stdout and stderr on both inputs; the failure exits are 2 and 1.

**Where the tracked path is named.** `S170_cases.tsv`'s header, a paragraph
after the rule's with the command; `test_mate_carry.cpp`, a comment paragraph
in "WHAT RETIRES THIS FILE'S FIXTURE" with the command (comments only);
DEV_MANUAL's "Goldens: named, scripted, re-derived", a paragraph after the
table (the budgets live in the TSV, which `grep -rn 'GOLDEN (DEC-142)'
tests/` does not list), and one clause in the S204 paragraph;
`adocs/data/README.md`, the `S170_cases.tsv` and `S286_sweep_s170.txt` rows,
the S289 goldens and grid rows, the S083r and S083s ceilings rows (each names
`pick_budgets.sh`), the S292 and S293 goldens rows (their files name the
`.tuning/` path), and the S290 section. S083's and S293's notes in the TSV
and `test_mate_carry.cpp` say "applied to the grid by script" or "by the same
grid and rule" and name no path, so the S290 paragraph above them is their
pointer. `MANUAL.md` names neither the fixture nor the picker: no change.

**Left naming the `.tuning/` path, each for a reason.** Evidence files,
added and never edited: `S083r_ceilings.txt`, `S083s_ceilings.txt`,
`S289_goldens.txt`, `S292_goldens.txt`, `S293_goldens.txt`; the played
candidates `S083_candidate.diff` and `S111_candidate.diff` (found by blob id,
DEC-254); `S289_sprt.sh`, a pre-registration. `adocs/plan_done/` is history.
`adocs/decisions.md` (DEC-312's entry says "by `pick_budgets.sh`") is the
coordinator's.

No `src/` change (`git diff --cached -- src/` empty), so no `Bench:` line is
owed and the second tier (DEC-141) does not apply.
