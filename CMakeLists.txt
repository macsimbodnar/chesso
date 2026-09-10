cmake_minimum_required(VERSION 3.18)

# ASM because the built-in opening book is pulled into the binary by .incbin
# from src/openings_embedded.S. S172, DEC-130.
project(Chesso C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CHESSO_BUILD_GUI "Build the debug gui. Requires SDL2" OFF)
option(SANITIZER "Enable address and undefined behavior sanitizer" OFF)

# S073. ON turns every parameter in src/search_params.hpp from a constant the
# compiler folds into a variable settable over UCI. It is a tuning harness and
# NOT the release binary: the loads it adds cost time a fixed-depth node count
# cannot see. Everything measured for strength is built with this OFF.
option(CHESSO_TUNE "Expose the search parameters as UCI options. Not the release binary" OFF)

# S104. CHESSO_ARCH and CHESSO_PGO. The arch flags are directory wide and are set
# before any target exists, so the tests and the benchmarks are built for the
# same instruction set as the engine -- a benchmark built for a different target
# is measuring a different binary. PGO is target scoped and comes after
# add_subdirectory(src); cmake/pgo.cmake says why.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/arch.cmake)

# Add extra compiler checks
if(MSVC)
     add_compile_options(/W4 /WX)
else()
     add_compile_options(-Wall -Wextra -Werror)

     if(CMAKE_BUILD_TYPE STREQUAL "Debug")
          add_compile_options(-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter)
     endif()

     # TODO: Check if this will speed it up. We don't use floating points anyway
     # string(REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# S197, DEC-167. The two extra flags are what make the option usable as a gate
# stage rather than a thing somebody reads the output of.
#
# -fno-sanitize-recover=undefined, because UBSan recovers by default: gcc's
# documentation says "error recovery is turned on by default" for
# -fsanitize=undefined, and clang's says the instrumented program "prints a
# verbose error report and continues execution upon a failed check". Observed
# here on 2026-09-10 with a signed overflow planted in command_test and built
# at RelWithDebInfo: without the flag the run printed
# "runtime error: signed integer overflow: 2147483647 + 1" and then ran the
# whole test command to completion and exited 0. With it, exit 1 at the first
# report. A stage that ran the recovering form would be green on a log carrying
# undefined behaviour.
#
# -fno-omit-frame-pointer, because ASan's documentation asks for it "to get
# nicer stack traces in error messages"; with UBSAN_OPTIONS=print_stacktrace=1
# the same report came back with six named frames instead of the bare line.
#
# No shipping or measured binary is built with SANITIZER=ON, so neither flag
# moves a recorded figure.
if(SANITIZER)
     message("-- Sanitizer ON")
     add_compile_options(-fsanitize=address)
     add_compile_options(-fsanitize=undefined)
     add_compile_options(-fno-sanitize-recover=undefined)
     add_compile_options(-fno-omit-frame-pointer)

     add_link_options(-fsanitize=address)
     add_link_options(-fsanitize=undefined)
     add_link_options(-fno-sanitize-recover=undefined)
else()
     message("-- Sanitizer OFF")
endif()

enable_testing()
add_subdirectory(src)

# After src, because it applies to chesso_engine and chesso by name.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pgo.cmake)

add_subdirectory(tests)
add_subdirectory(tools)
