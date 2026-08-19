# S104. Profile-guided optimisation: two compile passes in one build directory,
# over one profile directory.
#
# Included after `add_subdirectory(src)` and applied to the two engine targets by
# name. Deliberately not `add_compile_options`: the tests and the tools link the
# same library but their own translation units are never exercised by the
# workload, and under `-Werror` every one of them would fail the build on
# `-Wmissing-profile`. Scoping the flags to what ships also keeps `ctest` and the
# tuner out of a two-pass build they gain nothing from.
#
# `CHESSO_PGO_DIR` is absolute and shared between the passes: the instrumented
# binary writes `.gcda` files there and the second configure reads them back.
#
# **Both passes must run in one build directory, and this is not a style
# preference.** GCC names a `.gcda` by mangling the absolute path of the object
# file that wrote it -- `#home#max#ws#chesso#build-release-bmi2#src#...` -- and
# looks it up under the same name on the way back in. Two build directories give
# two different object paths, so every lookup misses, every translation unit is
# compiled with no profile at all, and the binary is an ordinary -O3 build
# wearing a PGO label. That is exactly what happened while writing this file and
# it measured as PGO being worth zero. `build_release.sh` reconfigures one
# directory rather than using two.
#
# The profile is never committed. It is a property of one workload run against
# one source tree, and a stale one is worse than none -- which is why
# `-Wcoverage-mismatch` is left as the error `-Werror` makes it. A profile taken
# from different sources should stop the build, not be corrected past.
#
# `-Wmissing-profile` is left to be the error `-Werror` makes it, and it is the
# load-bearing check in this file: it is the only thing that says a translation
# unit was compiled without the profile it was supposed to have. Silencing it
# tree-wide was the first version of this file, and it hid the two-directory bug
# above for as long as it took to notice that PGO was measuring zero. A missing
# profile stops the build, which is the right outcome -- a binary that quietly is
# not profile-guided is worse than one that does not build.
#
# Exactly one translation unit is exempt, by name, because its absence is
# structural rather than a gap in the workload. Every caller of
# `src/search_params.cpp` sits inside `#ifdef CHESSO_TUNE` (`src/chesso.cpp:934`
# and `:1043`), so a release link references none of its symbols, the linker
# drops the object out of the static library, its gcov constructor never runs and
# no .gcda is ever written. No workload can reach it. Naming the one file keeps
# the error live for the other eight.

set(CHESSO_PGO "off" CACHE STRING "Profile-guided optimisation pass: off, generate, use")
set_property(CACHE CHESSO_PGO PROPERTY STRINGS off generate use)

set(CHESSO_PGO_DIR "" CACHE PATH "Absolute directory holding the .gcda profile, shared by both passes")

# Structured as one if/else rather than an early return: `return()` from an
# include() is documented but its behaviour has moved between CMake versions, and
# cmake_minimum_required here is 3.18.
if(CHESSO_PGO STREQUAL "off")

message("-- PGO off")

else()

if(CHESSO_PGO_DIR STREQUAL "")
     message(FATAL_ERROR "CHESSO_PGO=${CHESSO_PGO} needs -DCHESSO_PGO_DIR=<absolute path>")
endif()

if(NOT IS_ABSOLUTE "${CHESSO_PGO_DIR}")
     message(FATAL_ERROR
          "CHESSO_PGO_DIR is '${CHESSO_PGO_DIR}'. It must be absolute: the two "
          "passes run from two build directories and a relative path would name "
          "a different place in each.")
endif()

if(CHESSO_PGO STREQUAL "generate")
     # prefer-atomic, so the workload may run more than one instrumented process
     # against the directory without losing counts to a torn read-modify-write.
     set(chesso_pgo_flags -fprofile-generate=${CHESSO_PGO_DIR} -fprofile-update=prefer-atomic)
elseif(CHESSO_PGO STREQUAL "use")
     if(NOT EXISTS "${CHESSO_PGO_DIR}")
          message(FATAL_ERROR
               "CHESSO_PGO=use but ${CHESSO_PGO_DIR} does not exist. Run the "
               "generate pass and the workload first, or use build_release.sh.")
     endif()

     set(chesso_pgo_flags -fprofile-use=${CHESSO_PGO_DIR} -fprofile-correction)
else()
     message(FATAL_ERROR "CHESSO_PGO is '${CHESSO_PGO}'. One of: off, generate, use")
endif()

foreach(target chesso_engine chesso)
     target_compile_options(${target} PRIVATE ${chesso_pgo_flags})
     target_link_options(${target} PRIVATE ${chesso_pgo_flags})
endforeach()

if(CHESSO_PGO STREQUAL "use")
     # Source-scoped, so it lands after the target options on the command line and
     # wins. TARGET_DIRECTORY because source properties are directory scoped and
     # this file is included from the top level, not from src/.
     set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/search_params.cpp
          TARGET_DIRECTORY chesso_engine
          PROPERTIES COMPILE_OPTIONS -Wno-missing-profile)
endif()

string(JOIN " " chesso_pgo_flag_string ${chesso_pgo_flags})
message("-- PGO ${CHESSO_PGO} (${chesso_pgo_flag_string})")

endif()
