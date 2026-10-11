# S104. Which instruction set the binary targets.
#
# The release build carried no architecture flag at all until this step, so
# `std::popcount` compiled to a software SWAR popcount -- `objdump -d
# build/src/chesso | grep -c popcnt` returned 0 at 20d058a. `count_bits` runs
# once per piece per evaluation in the mobility and king-safety loops, in
# `game_phase`, and throughout move generation, so the whole search paid for it.
#
# Four values, and the fourth is not distributable:
#
#   bmi2      the target that ships to a rating list. x86-64-v3, so POPCNT,
#             LZCNT, BMI1 and BMI2 -- Intel Haswell (2013) onward and AMD Zen3
#             (2020) onward.
#   avx2      the same level with BMI2 taken back out. PEXT and PDEP are
#             microcoded on Zen1 and Zen2 at around 18 cycles against Intel's 3,
#             so a binary that uses them is slower there than one that does not.
#             This target is what those parts run, and it is the reason the
#             sliding-attack lookup keeps a magic path (S032). `__BMI2__` is the
#             switch: defined for bmi2 and native, undefined here.
#   portable  x86-64-v2. SSE4.2 and POPCNT, which is Nehalem (2008) and
#             Bulldozer (2011) onward -- baseline enough to be the fallback and
#             still hardware popcount, which is the point of the step.
#   native    -march=native. Local development and local measurement only, and
#             `build_release.sh` refuses it: a binary built here runs the illegal
#             instruction handler on anything older than this machine. It is the
#             default because `build/` is the directory that gets measured, and
#             a local measurement wants the code the machine can actually run.
#
# Every flag set is put to the compiler before it is used. A silently dropped
# flag is the failure mode that would leave a target named for an instruction
# set it does not target, which is worse than not building.

set(CHESSO_ARCH "native" CACHE STRING "Instruction set target: bmi2, avx2, portable, native")
set_property(CACHE CHESSO_ARCH PROPERTY STRINGS bmi2 avx2 portable native)

if(CHESSO_ARCH STREQUAL "bmi2")
     set(chesso_arch_flags -march=x86-64-v3)
elseif(CHESSO_ARCH STREQUAL "avx2")
     set(chesso_arch_flags -march=x86-64-v3 -mno-bmi2)
elseif(CHESSO_ARCH STREQUAL "portable")
     set(chesso_arch_flags -march=x86-64-v2)
elseif(CHESSO_ARCH STREQUAL "native")
     set(chesso_arch_flags -march=native)
else()
     message(FATAL_ERROR
          "CHESSO_ARCH is '${CHESSO_ARCH}'. One of: bmi2, avx2, portable, native")
endif()

include(CheckCXXCompilerFlag)

# check_cxx_compiler_flag caches per variable name, so the name carries the
# target. Reconfiguring the same directory to a different arch then re-runs the
# check rather than reading the previous target's answer out of the cache.
string(JOIN " " chesso_arch_flag_string ${chesso_arch_flags})
check_cxx_compiler_flag("${chesso_arch_flag_string}" chesso_arch_ok_${CHESSO_ARCH})

if(NOT chesso_arch_ok_${CHESSO_ARCH})
     message(FATAL_ERROR
          "CHESSO_ARCH=${CHESSO_ARCH} needs '${chesso_arch_flag_string}', which "
          "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} rejected on "
          "${CMAKE_SYSTEM_PROCESSOR}. The four targets are x86-64.")
endif()

add_compile_options(${chesso_arch_flags})

if(CHESSO_ARCH STREQUAL "native")
     message("-- Arch native (${chesso_arch_flag_string}) -- THIS MACHINE ONLY, never distributed")
else()
     message("-- Arch ${CHESSO_ARCH} (${chesso_arch_flag_string})")
endif()

# S273. Branch padding, for the Skylake family's jump-conditional-code erratum.
#
# With Intel's microcode fix, a Skylake-derived core (Skylake to Comet Lake and
# Cascade Lake; the workstation's i7-8700K) does not keep a jump that crosses or
# ends on a 32-byte boundary in its decoded-uop cache, so where the hot jumps
# happen to fall decides how much of the search is decoded the slow way. GNU as
# pads conditional, fused and direct unconditional jumps off those boundaries
# (its -malign-branch default, jcc+fused+jmp); calls, returns and indirect jumps
# are not padded. On the workstation, padded against unpadded in the tree's
# layout and four perturbations of it (one of them S270's inline game_tables()
# diff), single-process timings: -1.5 to -3.7 % cycles for +0.5 % instructions,
# wall +1.50 to +3.11 %. The shipped PGO bmi2 build read the same sign at a
# similar size, in a run the machine disturbed. The uop cache's share of the
# uops, DSB / (DSB + MITE) without the microcode sequencer's, read 42 to 44 %
# padded against 23 to 36 % unpadded -- consistent with the erratum, from IDQ
# raw events whose encodings were typed from Intel's list and that nothing on
# this machine cross-checks. An unrelated layout change moved cycles by up to
# 1.8 % unpadded against 0.3 % padded, inside an A/A's own noise, which is what
# a speed reading under 2 % needs. adocs/plan_done/S273_branch_padding.md has the readings.
#
# native and bmi2 take it: native is the measured machine, and bmi2's x86-64-v3
# parts include the Skylake family. On bmi2's parts without the erratum
# (Haswell, Broadwell, Ice Lake on, Zen3 on) the padding is code size and NOPs
# (+0.8 to +0.9 % instructions and +3 % .text in the PGO build), and what that
# costs there has not been measured. avx2 is for Zen1 and Zen2 and portable for
# parts older than Haswell, and neither has the erratum.
#
# Put to the C++ compiler first like the arch flags, but a toolchain whose
# assembler lacks it builds without it and says so, rather than failing: the
# flag changes speed, never what the engine computes. C++ only, the language
# the check asked; the one assembly file holds data and no branch.
# CHESSO_BRANCH_PAD records what was applied, for fastchess.sh's banner: two
# sides that differ in it differ by the build and not only by the commit.
set(chesso_branch_pad OFF)

if(CHESSO_ARCH STREQUAL "native" OR CHESSO_ARCH STREQUAL "bmi2")
     set(chesso_branch_pad_flag "-Wa,-mbranches-within-32B-boundaries")
     check_cxx_compiler_flag("${chesso_branch_pad_flag}" chesso_branch_pad_ok)

     if(chesso_branch_pad_ok)
          add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${chesso_branch_pad_flag}>)
          set(chesso_branch_pad ON)
          message("-- Branch padding on (${chesso_branch_pad_flag})")
     else()
          message("-- Branch padding unavailable: ${CMAKE_CXX_COMPILER_ID} "
               "${CMAKE_CXX_COMPILER_VERSION} rejected '${chesso_branch_pad_flag}', built without it")
     endif()
else()
     message("-- Branch padding off for ${CHESSO_ARCH}")
endif()

set(CHESSO_BRANCH_PAD ${chesso_branch_pad} CACHE INTERNAL
     "Whether this build pads branches off 32-byte boundaries (S273), as applied")
