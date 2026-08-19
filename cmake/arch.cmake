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
