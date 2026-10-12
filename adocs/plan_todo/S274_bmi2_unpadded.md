id:         S274
goal:       the shipped bmi2 binary is built without branch padding, native keeps it, and the documentation states that a Skylake-family build gains 1.5 to 3 % from the flag and how to get it
accepts:    (1) `cmake/arch.cmake` applies `-Wa,-mbranches-within-32B-boundaries` for `CHESSO_ARCH` native only, and `CHESSO_BRANCH_PAD` records OFF for bmi2, avx2 and portable (each configured and checked); (2) `DEV_MANUAL.md`'s build paragraph, padding paragraph and arch-table rows state it, each statement traced to the CMake that sets it (DEC-017), and say that a Skylake-family part (Intel 6th to 10th generation desktop) gains 1.5 to 3 % wall from the flag, measured single-process on this machine (S273), with the exact way to build such a binary, while its cost on parts without the erratum stays unmeasured; (3) `build_release.sh bmi2` (the PGO path) builds with the flag in none of its compile commands and `bench` reads 4081329; native's `build/` is unchanged (flag in every C++ compile command, INV-6 trivially: no source moves); (4) both fast suites green, format clean, prose checks clean
touches:    cmake/arch.cmake, DEV_MANUAL.md
excludes:   any change to src/; a second shipped binary (DEC-267 rejected it); native's padding
decisions:  DEC-267, DEC-266, DEC-017
closes:
blocks:
paused_by:
author:
done:

## Why

The owner ruled on S273's bmi2 call (DEC-267): bmi2 runs on Haswell,
Broadwell, Ice Lake on and Zen3 on, which lack the JCC erratum, and the
padding's cost there is unmeasured, so the shipped binary stays unpadded; the
Skylake-family gain is documented as available rather than shipped.

## Lane

Agent work, about 15 minutes of builds; it needs the machine free of timings
and matches, so it runs between S119's hand-back and its SPRT.
