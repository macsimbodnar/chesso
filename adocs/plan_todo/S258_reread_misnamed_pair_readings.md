id:         S258
goal:       the ten SPRT pair readings `S105_pairs.py` scored from Black's side are re-read under each run's own candidate name, and every place that quotes or used their wrong variance is corrected or shown unaffected
accepts:    first, `adocs/data/S105_pairs.py`'s own command line reads a `fastchess.sh` PGN again. Since S257 its `main()` passes the default `chesso-a`, so `report()` refuses every such PGN (exit 1, loud), and that command is the post-SPRT pairs reading in the verdict checklist and the A/A reader AGENTS.md's MEASUREMENT rule names. `main()` derives the candidate by `S024_pair_stats.is_candidate` and falls back to `chesso-a` only where a PGN names it. S105's own two PGNs read byte-identical, and a `cand-` PGN reads the pentanomial fastchess printed; DEV_MANUAL.md names the command, and its sentence on what `report()` refuses says what the code does (not White in exactly one game of every complete pair). Then each of `S095`, `S097_v1`, `S097_v2`, `S132`, `S132_confirm`, `S188`, `S231`, `S236`, `S236_v2` and `S237` (`adocs/data/<run>_sprt_pairs.txt`) is re-read from its PGN under its `cand-<sha>` name with the S257 reader. Each re-read pentanomial equals the one fastchess printed in that run's log; a mismatch is a finding, never averaged away. The ten corrected readings go into a new file beside the old ones, and the old files are not edited (evidence). Every consumer of the ten wrong variances is listed: `adocs/data/README.md` rows, `adocs/status.md`, `DEV_MANUAL.md` cost tables, `.moltke.local.md` throughput notes, step files, decisions, and any pre-registration's worst-case game estimate or abort rule that took one. Each consumer is either corrected (README rows and DEV_MANUAL, with the date and S258 named) or shown not to move a conclusion. A consumer in `plan_done/` or `decisions.md` is not edited; if its conclusion moves, the step reports it for a new decision. The scan that found the ten (`pair score 0.0` count equal to `white won both`) is re-run over every `*_pairs.txt` and finds no eleventh.
touches:    adocs/data/S105_pairs.py; a new `adocs/data/S258_*` reading and its script if one is needed; adocs/data/README.md; DEV_MANUAL.md; tests/CMakeLists.txt if a self-test is added; adocs/data/S198_pairs.py (read, not written)
excludes:   editing any recorded `*_pairs.txt`, log or PGN; re-deciding any SPRT verdict (fastchess computed those itself and the misread never reached them)
decisions:  DEC-171, DEC-143, DEC-254
closes:
blocks:
paused_by:
author:
done:

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
