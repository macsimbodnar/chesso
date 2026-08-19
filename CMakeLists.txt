cmake_minimum_required(VERSION 3.18)

project(Chesso)

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

if(SANITIZER)
     message("-- Sanitizer ON")
     add_compile_options(-fsanitize=address)
     add_compile_options(-fsanitize=undefined)
     
     add_link_options(-fsanitize=address)
     add_link_options(-fsanitize=undefined)
else()
     message("-- Sanitizer OFF")
endif()

enable_testing()
add_subdirectory(src)

# After src, because it applies to chesso_engine and chesso by name.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pgo.cmake)

add_subdirectory(tests)
add_subdirectory(tools)
