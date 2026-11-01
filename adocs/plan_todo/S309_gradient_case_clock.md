id:         S309
goal:       `test_tuner_gradient` stops running close to its 60 s clock, so a planted recovery added by a later family cannot turn it red on time alone
accepts:    the case's time in the fast suite reads under half its clock on the workstation under the suite's own load, with how it was measured; no bound moves and no plant is dropped or weakened (a split into separate ctest entries or a shared fit is the expected form, and the step says which); both builds green, Debug's clock read too; a plant still goes red on what it exists to catch, observed
touches:    tests/test_tuner_gradient.cpp, tests/CMakeLists.txt
excludes:   any change to the tuner; raising the clock, which hides what this step exists to fix
decisions:  DEC-171, DEC-316
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-01 by S101's fast check. The case took 33.2 s in the suite's log and 23.1 s alone on the
idle machine, against 11.6 s in S125-era logs and a Release `TIMEOUT` of 60. S101's plants are 68000 of
its 132000 epochs. S102 and S122 add families and plants, and a suite on a shared machine has less room.
Green today, so filler by DEC-171; it should land before S102's plants.
