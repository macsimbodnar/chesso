id:         S104
goal:       the release build targets the machine's instruction set and is profile-guided, so count_bits stops being a software popcount
accepts:    `objdump -d build/src/chesso | grep -c popcnt` is greater than zero, where it is **0** at 20d058a; identical node counts and identical best moves from tools/search_bench.py against the preceding commit at two depths, which discharges INV-6 and is why **no SPRT is owed** (DEC-083); the speed change measured by interleaved runs with the spread recorded, and converted to Elo at the published 1.43 per percent with the conversion named as a conversion; three build targets exist and the one that ships to a rating list is named -- a bmi2 target, an avx2 target with magics for pre-Zen3 AMD where PEXT is microcoded, and a portable baseline; `-march=native` never ships in a distributed binary; PGO uses a fixed-depth search over many positions as its workload, never perft and never one position, and the profile is regenerated per build and never committed
touches:    CMakeLists.txt, cmake/, DEV_MANUAL.md
excludes:   using _pext_u64 in the sliding attack lookup, which is S032 and is a code change; any change under src/
decisions:  DEC-083, DEC-049
closes:
blocks:
paused_by:
done:

## Measured before the step, 2026-08-19, at 20d058a

The shipped binary contains **zero `popcnt` instructions**. `CMakeLists.txt`
adds `-Wall -Wextra -Werror` and nothing else, so the release flags are
`-O3 -DNDEBUG -std=gnu++20`; `std::popcount` without `-mpopcnt` compiles to a
software SWAR popcount, and `count_bits` is called once per piece per
evaluation in the mobility and king-safety loops, in `game_phase`, and
throughout move generation.

Rebuilt with `-march=native`: **159 `popcnt` instructions**, and

| build | depth 14 from startpos | nodes | nps |
|---|---|---|---|
| shipping | 1170 / 1213 / 1222 ms | 6730511 | 5.76 M |
| `-march=native` | 991 / 1008 ms | **6730511**, same PV | 6.72 M |

**+16.7 %, node-identical.** At the published conversion that is about +24 Elo
of self-play at short time control, for one line of CMake. LTO on top measured
1054 / 1191 against 1076 / 1089 -- inside the noise on this machine, so it is
carried for the PGO pairing rather than claimed on its own.

## Why no SPRT

DEC-083. A speed-up below about 0.7 % is invisible to an SPRT at long time
control; this one is well above that, but the instrument is still wrong for it.
The node count is identical, so the two builds play identical games and a match
between them measures nothing but the clock. The timing is the measurement.
