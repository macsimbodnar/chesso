file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# main.cpp is the only translation unit holding main(). Keeping it out of the
# library lets the test binaries link chesso.cpp (the UCI layer) and supply
# doctest's own main.
list(REMOVE_ITEM SRCS main.cpp)

# S172. The built-in opening book is the raw Polyglot file and reaches the
# binary through .incbin, so the assembler is part of the engine's build. The
# path is handed to the assembler as a define rather than written into the .S,
# because `.incbin`'s own file search differs between toolchains; OBJECT_DEPENDS
# is what makes a changed book reassemble, since neither cmake nor the
# dependency scanner can see through a preprocessor macro to the file it names.
set(CHESSO_BOOK ${CMAKE_CURRENT_SOURCE_DIR}/openings.bin)

set_source_files_properties(openings_embedded.S PROPERTIES
     COMPILE_DEFINITIONS CHESSO_BOOK_PATH=${CHESSO_BOOK}
     OBJECT_DEPENDS ${CHESSO_BOOK})

add_library(chesso_engine STATIC ${SRCS} openings_embedded.S)
target_include_directories(chesso_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# S212. `id name` carries the commit, the arch and the tune flag, and all three
# come from the header the top-level CMakeLists.txt regenerates on every build.
# PRIVATE because chesso.cpp is the only translation unit that includes it: a
# test or a tool linking this library reads the answer over UCI, which is where
# every consumer of it reads it anyway.
add_dependencies(chesso_engine chesso_build_info)
target_include_directories(chesso_engine PRIVATE ${CHESSO_BUILD_INFO_DIR})

# PUBLIC, and it has to be: search_params.hpp declares a parameter as
# `inline constexpr int` without the definition and as `extern int` with it, so
# a tool or a test compiled the other way from the library it links would be an
# ODR violation the linker is not obliged to notice. S073.
if(CHESSO_TUNE)
     message("-- Tune build ON: the search parameters are UCI options. NOT the release binary")
     target_compile_definitions(chesso_engine PUBLIC CHESSO_TUNE)
else()
     message("-- Tune build OFF")
endif()

add_executable(chesso main.cpp)
target_link_libraries(chesso PRIVATE chesso_engine)

# S262. The trace build: the same sources with CHESSO_TRACE defined, so that
# evaluate() records the coefficient of every weight it applies
# (src/eval_trace.hpp). A library of its own rather than a build option,
# because what links it is tools/tuner and the tests of its model, which every
# build directory builds and the gate runs; no engine binary links it, so the
# switch is compiled out of `chesso` in every configuration, release and tune
# included, and chesso_engine above compiles exactly as before. PUBLIC so a
# target linking it sees the trace API, and CHESSO_TUNE follows the engine's
# for the ODR reason written over that block.
add_library(chesso_engine_trace STATIC ${SRCS} openings_embedded.S)
target_include_directories(chesso_engine_trace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(chesso_engine_trace PUBLIC CHESSO_TRACE)
add_dependencies(chesso_engine_trace chesso_build_info)
target_include_directories(chesso_engine_trace PRIVATE ${CHESSO_BUILD_INFO_DIR})

if(CHESSO_TUNE)
     target_compile_definitions(chesso_engine_trace PUBLIC CHESSO_TUNE)
endif()
