id:         S241
goal:       a run's outputs stay out of the tree unless useful there -- rating.sh trims its report and keeps the console beside it, both match scripts write under .tuning/, the gate refuses a file over 20 MB, the two 59 MB S240 reports are trimmed in history and archived, and the policy is a rule
accepts:    `tools/trim_console.py` drops only the "PV continues after" block, for both rules, and counts it per rule and engine, proved by `tests/test_trim_console.py`; `rating.sh` writes `console.txt` unfiltered and `report.txt` through the tool, proved by a `test_rating_script.sh` case; `tools/gate.sh` refuses a 21 MB file and accepts a 19 MB one, in ref and message mode, proved by `test_gate_script.sh` cases 18 to 20; both S240 reports in the tree are the tool's output over the originals with the tail from `=== terminations ===` on byte for byte the original's (59 MB to 0.6 MB each); the originals are in the owner's archive with both run directories as a tar.xz whose sha256 sidecar verifies; history from `5c373ac` on carries the trimmed blobs only, `src/` identical commit by commit, on `achesso` and `s113`; DEC-235, the DATA rule in AGENTS.md, the archive path in .moltke.local.md, DEV_MANUAL.md and adocs/data/README.md say so; the fast suite green in both builds
touches:    tools/, tests/, rating.sh, fastchess.sh, adocs/data/, adocs/, AGENTS.md, DEV_MANUAL.md, .moltke.local.md, git history from 5c373ac
excludes:   trimming S087's reports (16 and 19 MB, under the gate's line, already in history); fastchess.sh's own console (about 60 such blocks a run); the force push, which is the owner's; purging the old blobs from GitHub's object store, which only its garbage collection does
decisions:  DEC-235
closes:
blocks:
paused_by:
author:     the session the owner instructed directly on 2026-09-28, after the push warned -- not a briefed subagent (DEC-185 is for plan steps; DEC-235 records the exception); started 2026-09-28 00:10
done:

## Why this exists

The owner pushed on 2026-09-28 and GitHub warned about two files over its
50 MB line: `adocs/data/S240_rating_report.txt` (56.34 MB on the wire) and
`S240_rating_run1_INVALID_report.txt`. Asked why they were so large and
whether they belonged in git, the session found that each is fastchess's
whole console for a 3340-game gauntlet, copied into the report by `rating.sh`,
and that 98.9 % of the bytes are one warning family -- "PV continues after
threefold repetition" (50827 blocks in run 2) and "after fifty-move rule"
(4238) -- each a four-line block that repeats the game's whole move list.
The blocks come from the anchors' PVs (Leorik 2.4 31790, Leorik 2.1 23161,
Blunder 8.5.5 113, chesso 1) and nothing reads them; what the record cites is
the last 60 lines. S087's reports carry the same at 16 and 19 MB; S240 played
2.5 times the games. The owner's answer, recorded as DEC-235: this class of
file does not enter git unless useful there, lives under `.tuning/` while a
run is worked, goes to the owner's Synckeeper archive when worth keeping, and
the two committed ones come out of history by a rewrite the owner
force-pushes.

## What was done

- `tools/trim_console.py`: bytes in, bytes out, minus the block, counted per
  rule and engine, `--summary` at the end of the input. `tests/test_trim_console.py`
  drives it as a subprocess: twelve cases, including that "Incomplete mating
  PV" -- the same three context lines, counted per side by S238's pairs
  reading -- passes whole, and that a block cut by another game's line loses
  only its own lines.
- `rating.sh`: `fastchess | tee console.txt | trim_console.py --summary | tee -a report.txt`;
  a banner line says so; `OUT` defaults to `.tuning/rating_<mode>_<stamp>`.
  `fastchess.sh`: `OUT` defaults to `.tuning/sprt_<tag>_<stamp>` (two launches
  were restarted on 2026-09-13 because `/tmp` is wiped at boot). Case 7 of
  `test_rating_script.sh` reads the report and the console a stub fastchess
  produced.
- `tools/gate.sh`: a commit adding a blob over 20 MB is refused before the
  suite, in ref and message mode; cases 18 to 20 of `test_gate_script.sh`.
- Both S240 reports replaced by the tool's output over the originals, the
  fastchess part only, with a banner line naming the archive: run 2
  59077250 to about 0.6 MB, 55065 blocks and 220260 lines dropped; run 1
  58133271 to about 0.6 MB. The tail from `=== terminations ===` on is byte
  for byte the original's, checked in the generating script.
- Archive: `/home/max/Synckeeper/Chesso Archive/chesso_S240_rating_runs_2026-09-27.tar.xz`,
  both run directories (`console` as `report.txt`, `games.pgn`, `fastchess.log`,
  ordo outputs; 252 MB unpacked, 9.35 MB packed), `xz -t` and the listing
  checked, sha256 sidecar written as a bare filename and verified with `-c`.
- Documents: DEC-235; the DATA rule in `AGENTS.md`; `.moltke.local.md` names
  the archive; `DEV_MANUAL.md`'s rating section says what a run leaves and
  what the tree takes, and its `/tmp` examples now point under `.tuning/`;
  `adocs/data/README.md`'s two rows say what the committed reports are.
- History: after the completing commit, a `filter-branch --index-filter` over
  `5c373ac^..achesso` replaces the two original blobs by the trimmed ones
  wherever a commit carried them and touches nothing else; `s113` is moved to
  the rewritten `1e9827d` and its worktree's two paths refreshed without
  disturbing S113's uncommitted work. The map of old to new shas is appended
  to DEC-235 in a commit after the rewrite. The owner force-pushes.

## Where it stands, 2026-09-28 02:40

Everything above is in the tree and gated green (`.tuning/coord/S241_gate.log`:
both suites 41 of 41, format clean, `GATE-DONE 4649650`, no `src/` change; a
first run, `S241_gate_run1.log`, was red only on `test_mate_carry`'s 120 s
ctest ceiling under S112's SPRT load at 120.11 s, and that test passed
directly with 74 of 74 assertions in 119 s Release and 118 s tune,
`S241_gate_rest.log`). **What is left is the owner's: the history rewrite and
the force push.** The session's sandbox refused to write the rewrite command,
so it is not run by the agent; the owner runs `.tuning/coord/S241_rewrite.sh`
on a clean tree after this commit, then `S241_verify.sh`, then pushes. The
step completes in the commit that appends the sha map to DEC-235.

## What it does not touch

S112's SPRT keeps running on the binaries it was launched with; its log,
pre-registration and result block name `3d82344` and `1e9827d`, which the
gate compares with the log and not with history. `src/` is identical between
each old and new sha.
