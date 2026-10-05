id:         S258
goal:       the ten SPRT pair readings `S105_pairs.py` scored from Black's side are re-read under each run's own candidate name, and every place that quotes or used their wrong variance is corrected or shown unaffected
accepts:    first, `adocs/data/S105_pairs.py`'s own command line reads a `fastchess.sh` PGN again. Since S257 its `main()` passes the default `chesso-a`, so `report()` refuses every such PGN (exit 1, loud), and that command is the post-SPRT pairs reading in the verdict checklist and the A/A reader AGENTS.md's MEASUREMENT rule names. `main()` derives the candidate by `S024_pair_stats.is_candidate` and falls back to `chesso-a` only where a PGN names it. S105's own two PGNs read byte-identical, and a `cand-` PGN reads the pentanomial fastchess printed; DEV_MANUAL.md names the command, and its sentence on what `report()` refuses says what the code does (not White in exactly one game of every complete pair). Then each of `S095`, `S097_v1`, `S097_v2`, `S132`, `S132_confirm`, `S188`, `S231`, `S236`, `S236_v2` and `S237` (`adocs/data/<run>_sprt_pairs.txt`) is re-read from its PGN under its `cand-<sha>` name with the S257 reader. Each re-read pentanomial equals the one fastchess printed in that run's log; a mismatch is a finding, never averaged away. The ten corrected readings go into a new file beside the old ones, and the old files are not edited (evidence). Every consumer of the ten wrong variances is listed: `adocs/data/README.md` rows, `adocs/status.md`, `DEV_MANUAL.md` cost tables, `.moltke.local.md` throughput notes, step files, decisions, and any pre-registration's worst-case game estimate or abort rule that took one. Each consumer is either corrected (README rows and DEV_MANUAL, with the date and S258 named) or shown not to move a conclusion. A consumer in `plan_done/` or `decisions.md` is not edited; if its conclusion moves, the step reports it for a new decision. The scan that found the ten (`pair score 0.0` count equal to `white won both`) is re-run over every `*_pairs.txt` and finds no eleventh.
touches:    adocs/data/S105_pairs.py; adocs/data/S258_reread.py and adocs/data/S258_reread_pairs.txt (the new reading and its script); adocs/data/README.md; DEV_MANUAL.md; tests/CMakeLists.txt (the self-test, `test_s105_pairs`); adocs/data/S198_pairs.py (read, not written)
excludes:   editing any recorded `*_pairs.txt`, log or PGN; re-deciding any SPRT verdict (fastchess computed those itself and the misread never reached them)
decisions:  DEC-171, DEC-143, DEC-254
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-06. `S105_pairs.py <run>/games.pgn` reads a fastchess PGN again: red on the unfixed tree, exit 1 over S132's PGN; green, exit 0 and `[149, 406, 664, 501, 191]`, fastchess's own Ptnml; S105's two PGNs byte-identical by `cmp`; `--self-test` 14 cases as `test_s105_pairs` (fast), five mutants killed. `S258_reread.py` re-reads the ten under their `cand-<sha>` into `S258_reread_pairs.txt`: all ten equal fastchess's Ptnml and all ten recorded readings are the Black's-points reading exactly (S231 0.3105 -> 0.3025, S237 0.2968 -> 0.3205); the scan finds no eleventh, and the 24 other checkable blocks equal fastchess's, none mirrored. README rows and DEV_MANUAL corrected; the status.md and plan_done quotes are listed and move no conclusion; S237's 'PGN order' note is found wrong. Gate green in both builds (43/43) and clang-format. MANUAL.md checked, no change.

## Why this exists (2026-10-05, the coordinator, from S257's report)

S257's implementer found that ten recorded SPRT pair readings were scored as
Black's points. Each one's PGN names `cand-<sha>`/`ref-<sha>`, and
`S105_pairs.py` was run on it with its default or a literal name matching
neither side. The signature: the `pair score 0.0` count equals the `white won
both` count, which under correct scoring are disjoint sets. A scan of every
`*_pairs.txt` on 2026-10-05 finds exactly these ten.

Confirmed directly on S231, PGN `.tuning/sprt_s231_20260920_044538/games.pgn`:
- read under `cand-55891bb`: pentanomial `[556, 1464, 2097, 1352, 566]`,
  variance **0.3025**, which matches fastchess's own Ptnml;
- recorded: `[580, 1438, 2049, 1378, 590]`, variance **0.3105**.

All ten PGNs are still under `.tuning/` on this machine. The SPRT verdicts are
fastchess's and are unaffected. The variances were quoted onward, at least in
the README rows and in `status.md`, and may have fed a budget. A measurement
defect that reaches no play: a filler under DEC-171.

**Widened the same day by S257's fast check.** S257 made `report()` refuse a
name that is no side, which is right, but `S105_pairs.main()` still passes
`chesso-a`. The documented `S105_pairs.py <pgn>` reading of a fastchess run
therefore exits 1 every time, and nothing documents a replacement. Re-reading
the ten needs exactly that reader, so the fix comes first here.

## As built (2026-10-06, the implementer)

**The command line, red first.** On the unfixed tree
`python3 adocs/data/S105_pairs.py .tuning/sprt_s132_20260921_172443/games.pgn`
exited 1: `S105_pairs: 'chesso-a' is White in 0 of the 2 games of round 3 in
... (sides cand-474c288 and ref-778c7b0), so it is not a side of this PGN`.
`main()` now passes each PGN's name from `side_name()`: the one name
`S024_pair_stats.is_candidate` accepts (imported, as S257's reader does), or
`chesso-a` only where none does and the PGN names it; anything else exits 1
with one sentence naming every side. Written in `S105_pairs.py` rather than
imported from `S198_pairs.candidate_name`, which exits on the `chesso-a` case
and is outside `touches:`; the duplicate is the few lines that collect the
names. **Green:** the same command exits 0 and prints `[149, 406, 664, 501,
191]`, the last `Ptnml(0-2)` of `adocs/data/S132_sprt.log`; S105's own two
PGNs read byte-identical by `cmp` against a capture taken before the edit, and
equal `S105_calibration_pairs.txt` up to the path. `S198_pairs.py
--self-test`, `S199_drift.py --self-test` and the band reading are unchanged.

**Test.** `S105_pairs.py --self-test`, 14 cases, 0.1 s, in the fast label as
`test_s105_pairs`: `main()` over S198's fabricated four-pair PGN (imported,
pair variance 0.5000 by hand) under `candidate`, `cand-1a2b3c4`,
`cand-1a2b3c4+0123456789ab` and `chesso-a`; three refusals through `main()`,
each asserted to be `side_name`'s sentence and not `report()`'s later guard
(no side, two `cand-` sides, `chesso-b` alone); S105's two PGNs at the golden
`0.2343 +/- 0.0148 vs 0.2395 +/- 0.0152` (`S105_calibration_pairs.txt`,
re-derived by this script); and S219's band renamed `cand-5047070`, whose
printed pentanomial must equal the last `Ptnml(0-2)` of
`S219_aa_calibration.log`, read live rather than typed. Five mutants, all
killed: `main()` back to `report(p)` (7 fails), the `chesso-a` fallback
dropped (3), the fallback taken without the name check (3), `>= 1` for `== 1`
so two `cand-` sides pass (1), an exact match on `candidate` for
`is_candidate` (6).

**The ten, re-read.** `adocs/data/S258_reread.py`, written to
`adocs/data/S258_reread_pairs.txt` and re-derived identically, reads each PGN
under its own name with `S105_pairs.report` and checks three things per run;
all thirty hold. The derived side is the banner's `cand-<sha>`; the corrected
pentanomial equals the last `Ptnml(0-2)` of the run's committed `_sprt.log`
on all ten, so no mismatch is reported; and the recorded pentanomial is
exactly the same PGN scored as Black's points, which proves the diagnosis
rather than assuming it. Variance recorded -> corrected: S095 0.3068 ->
0.3113, S097_v1 0.3160 -> 0.2959, S097_v2 0.3189 -> 0.2966, S132 0.3047 ->
0.2944, S132_confirm 0.2865 -> 0.2604, S188 0.3101 -> 0.3049, S231 0.3105 ->
0.3025, S236 0.3051 -> 0.3046, S236_v2 0.3121 -> 0.2994, S237 0.2968 ->
0.3205. Two went up; "inflated" was not the rule. The pair score means were
wrong too (S237 1.0255 -> 0.9854, S097_v1 1.0156 -> 0.9977), and the bucket
percentages with them; the `white won both` count names no side and stands.
The PGNs are under `.tuning/` on this machine only; elsewhere the script exits
1 naming the missing PGN.

**The scan, re-run, finds no eleventh.** 37 `*_pairs.txt` files, 39 pair
blocks, the signature (`pair score 0.0` equal to `white won both`) on exactly
the ten. Widened past the brief: each block's pentanomial against fastchess's
`Ptnml(0-2)`, quoted in the file or, failing that, the sibling log's last.
24 blocks are equal, the ten differ, none is mirrored (read from the
reference's side), and five have nothing to check against: S105's two
calibration PGNs under `chesso-a`, whose log prints no Ptnml, and the band
re-read closing each of three later A/A readings.

**A wrong diagnosis, found on the way.** `S237_sprt_pairs.txt`'s coordinator
note (2026-09-26) explains the mismatch as `S105_pairs.py` pairing "games in
PGN order, which concurrency 12 does not keep", and S235's agreement as
"chance of ordering". `report()` groups by the `Round` tag; the cause was the
literal `candidate` over a `cand-` PGN, as the Black's-points check shows.
Ten later readings (S022_v1, S022_v2, S112, S113, S114, S114_v2, S115, S116,
S131, S238) end on a line pointing at that note; S112's and S238's carry no
tally at all, and the other eight's equal fastchess's. Evidence, so not edited; the
README's `S237_sprt_pairs.txt` row now says what the cause was.

**Consumers.** Corrected, each naming 2026-10-06 and S258: the nine
`adocs/data/README.md` rows that quote a mean and variance (S095, S097_v1,
S097_v2, S132, S132_confirm, S188, S231, S236, S236_v2), the S237 row's
reason, the `S105_pairs.py` row, and two new rows. Fixed in passing, in a row
being corrected: S097_v2's "116.7 plies" is the file's 117.6. `DEV_MANUAL.md`:
the `CAND` sentence now says what `report()` refuses (not White in exactly one
game of every complete pair) and how the command line takes its name, and
"What the run prints when it ends" names the post-SPRT pairs command and the
re-read; DEV_MANUAL held no copy of the ten. Not editable, quoted and listed for the
coordinator, none moving a conclusion: `status.md` 2097, 1996, 1784, 1872,
1484, 2182, 1389, 1315 (each a parenthetical variance beside an evidence
path); `plan_done/` S095:680, S097:1119 and 1816, S132:640, S188:753,
S231:1126, S236:797 and 885 (each the reading quoted beside the verdict).
Nothing took them: every pre-registration prices its pair in nElo (41861
games at the midpoint, variance-free) and aborts on forfeits or throughput;
no DEV_MANUAL cost table, `.moltke.local.md` note, decision, audit or
`S199_drift.tsv` row carries one; the ledger reads fastchess's own lines.
The verdicts are fastchess's and were never read from the ten.

**Docs.** `MANUAL.md` checked: it names no pairs reader, no change.

**Gate.** Both builds and clang-format green, 43/43 fast tests each,
`test_s105_pairs` among them. No engine change, so no Debug self-play is
owed.
