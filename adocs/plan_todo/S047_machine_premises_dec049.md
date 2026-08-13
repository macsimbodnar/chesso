id:         S047
goal:       S032, S029 and S020 premises re-stated against the DEC-049 machine; specs open item refreshed
accepts:    S032 no longer reads blocked on hardware and its accepts allows measuring here, magics kept as the fallback path; S029 drops the hardware caveat; S020 marks the 12 % is_check figure as Apple-machine and requires a re-profile before the step starts; specs.md's open item describes the DEC-049 machine; every pre-DEC-049 figure quoted in these files keeps its conditions attached
touches:    adocs/plan_todo/S032_pext_sliding_attacks.md, adocs/plan_todo/S029_nnue.md, adocs/plan_todo/S020_single_check_computation.md, adocs/specs.md
excludes:   re-profiling now; starting any of the three steps
decisions:  DEC-049
closes:     2026-08-13_plan_review-F04
blocks:
paused_by:
done:

## What is there

S032 says "**Blocked on hardware.** [...] An x86-64 Linux box is needed for
this" and its accepts says "measured on x86-64, since it cannot be measured
here". Since DEC-049 (2026-08-13) the tree builds on an i7-8700K: `uname -m`
x86_64, `nproc` 12, `bmi2` in /proc/cpuinfo. The step stopped being blocked
the day the machine moved.

Same retired premise nearby: S029's "**This is where x86 stops being
optional**" (moot here, and overstated against the literature — published
NNUE engines run integer inference on Apple Silicon via NEON; AVX2 is wider,
not without equivalent); S020's "`is_check` is about 12 % of the profile", an
Apple-clang figure quoted with no condition although DEC-049 requires
conditions attached; `specs.md:181-185` still describes three usable cores
and `opendirectoryd`.

Full evidence: 2026-08-13_plan_review-F04.
