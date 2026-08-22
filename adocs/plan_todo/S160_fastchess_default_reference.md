id:         S160
goal:       a bare fastchess.sh run measures against HEAD, printed loudly, instead of silently against the 2026-08-08 baseline
accepts:    with `REF` unset, `fastchess.sh` resolves the reference to `HEAD` and the launch banner prints the resolved reference sha and its commit date beside the candidate's; `CLAUDE.md`'s decision-procedure line and `DEV_MANUAL.md`'s play-games section state what the default reference is in every invocation they teach; verified by one smoke invocation reading the banner, no match owed; the engine binary is untouched, so INV-6 does not arise
touches:    fastchess.sh, CLAUDE.md, DEV_MANUAL.md
excludes:   rating.sh, which runs the list's absolute regime and pins its own opponents; any change to bounds, time control, book, hash or concurrency — S105 set those; the engine
decisions:  DEC-020
closes:     2026-08-22_adversarial-F02
blocks:
paused_by:
done:

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
