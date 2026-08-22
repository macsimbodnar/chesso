id:         S160
goal:       a bare fastchess.sh run measures against HEAD, printed loudly, instead of silently against the 2026-08-08 baseline
accepts:    with `REF` unset, `fastchess.sh` resolves the reference to `HEAD` and the launch banner prints the resolved reference sha and its commit date beside the candidate's; `CLAUDE.md`'s decision-procedure line and `DEV_MANUAL.md`'s play-games section state what the default reference is in every invocation they teach; verified by one smoke invocation reading the banner, no match owed; the engine binary is untouched, so INV-6 does not arise
touches:    fastchess.sh, tests/test_fastchess_script.sh, CLAUDE.md, DEV_MANUAL.md, adocs/audit/2026-08-22_adversarial.md, adocs/testing.md
excludes:   rating.sh, which runs the list's absolute regime and pins its own opponents; any change to bounds, time control, book, hash or concurrency — S105 set those; the engine
decisions:  DEC-020
closes:     2026-08-22_adversarial-F02
blocks:
paused_by:
done:      2026-08-22. `REF` defaults to `HEAD` and the banner prints both sides as `<sha>  <commit-date>`, verified in the real repository: `candidate eaad88b 2026-08-22 + uncommitted changes` / `reference eaad88b 2026-08-22` with `REF` unset, and `reference 7b4d9a4 2026-08-08` against the old fixed default, which is the two-week gap the date column exists to show. Two things beyond the accepts, both noted rather than silent. (1) The HEAD default makes a new case reachable -- `REF` unset on a clean tree is candidate and reference at the same commit, the same build twice, and an SPRT between identical engines does not return zero but random-walks until a bound is crossed by luck, one bare `--fast` run in ten reporting a gain that does not exist. That case is refused; `REF=HEAD` passed by hand still runs it, which is what an A/A calibration of the harness is. (2) The banner asked `git diff --quiet`, tree against index, so a fully staged diff read as clean while the binary played was built from that tree; it now asks `git diff --quiet HEAD`. `touches` was amended to add `tests/test_fastchess_script.sh`, `adocs/audit/2026-08-22_adversarial.md` and `adocs/testing.md`: AGENTS.md par.6 wants the behaviour change guarded, and a one-time eyeball is exactly what let the old default freeze for two weeks. Cases 3 and 4 of the smoke test assert the banner's reference line names HEAD with a date, and that a clean-tree bare run is refused with the `SPRT-RUN-FAILED` marker and never reaches the stub. Both observed red: case 3 against `HEAD:fastchess.sh`, which dies `fatal: Needed a single revision` in a sandbox that has no `7b4d9a4` -- the frozen sha, visible as a hard failure; case 4 against the new script with the guard cut out, which played the A/A match and printed `candidate 4f693af` / `reference 4f693af`. Case 4 passes vacuously against the old script, which aborts before fastchess for its own reason, which is why its red was taken against the guardless new script instead. No new decision was written: the operative rule now sits in `CLAUDE.md`'s decision-procedure line, which is read every session, and the step's `decisions:` field named only DEC-020. No match owed -- the engine binary is untouched, so INV-6 does not arise. `adocs/specs.md` states nothing about the harness default, no change. `MANUAL.md` checked: its only fastchess mention is `-check-mate-pvs` in a bug note, unaffected, no change. `README.md` checked, owner-written, no change needed. Gate: `cmake --build build -j12`, `ctest -L fast` 19/19 with `test_fastchess_script` at 0.21 s against 0.09 s before, `./clang-format.sh --check` clean.

## Evidence

2026-08-22_adversarial-F02. `fastchess.sh:43` defaults `REF` to `7b4d9a4`,
the 2026-08-08 pre-achesso baseline, set on 2026-08-08 and never moved — now
several hundred Elo stale (S028 alone measured +188.74 since that sha). A run
that omits `REF` reaches H1 in minutes at `--fast` bounds regardless of the
change under test: the DEC-020 class, armed in the project's per-change
instrument, while `CLAUDE.md:84` itself teaches the bare `./fastchess.sh
--fast`. No recorded verdict is contaminated — every banked run pinned its
reference via the `adocs/data/*_sprt.sh` scripts — so this is a live trap,
not a past contamination.

## Why HEAD and not a refusal

Defaulting to `HEAD` keeps the documented bare invocation meaningful — it
measures the uncommitted diff against its parent, which is the
working-tree-versus-reference contract the script's own header describes —
where a refusal would break every doc that teaches the bare form anyway. The
banner makes the resolved reference impossible to miss either way.

## Cost

Minutes of script and doc work, one smoke run. First in the batch because it
protects every verdict taken after it.
author:    Maksym Bodnar
