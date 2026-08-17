file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Measured on the DEC-049 machine, 2026-08-14: the whole "fast" label is 18 s
# in Release and the two slowest binaries are 165 s and 216 s in Debug, so the
# Debug budget is a little over twice the slowest of those and the Release one
# is unchanged. A build type this does not name gets the Debug figure, which is
# the safe direction: a wrong timeout that is too long only delays a hang
# report, and one that is too short reports a failure that is not there.
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
     set(CHESSO_TEST_TIMEOUT 60)
else()
     set(CHESSO_TEST_TIMEOUT 600)
endif()
message("-- Test timeout ${CHESSO_TEST_TIMEOUT}s (build type ${CMAKE_BUILD_TYPE})")

# Every doctest binary needs the same include paths and the same engine
# library, and every one of them is expected to finish in seconds. Perft is the
# exception and is registered separately with the "slow" label.
function(add_doctest_target name)
     add_executable(${name} ${name}.cpp)

     target_include_directories(${name} SYSTEM PRIVATE doctest/doctest)
     target_include_directories(${name} SYSTEM PRIVATE json/single_include/nlohmann)
     target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
     target_include_directories(${name} PRIVATE ../src)

     target_link_libraries(${name} PRIVATE chesso_engine)

     add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name})

     # A fast test that stops making progress is a bug. Without an explicit
     # timeout it would burn CTest's 1500 second default before saying so.
     #
     # It has to be the build's timeout and not one number, though. 60 s is
     # right for the Release build the gate runs -- the whole label is 18 s --
     # and impossible for the Debug one, where the assertions INV-2 and INV-4
     # live in are on and the optimiser is off: `ctest --test-dir build-debug
     # -L fast` reported Timeout on test_movegen and test_search until S067,
     # and run directly they take 165.33 s and 215.91 s and pass, 14 of 14 and
     # 44 of 44. So the only command that runs those two invariants reported a
     # failure that was the clock rather than an assertion.
     # 2026-08-14_test_review-F03.
     set_tests_properties(${name}
          PROPERTIES LABELS "fast" TIMEOUT ${CHESSO_TEST_TIMEOUT})
endfunction()

add_doctest_target(test_chesso)
add_doctest_target(test_openings)
add_doctest_target(test_movegen)
add_doctest_target(test_evaluation)
add_doctest_target(test_search)
add_doctest_target(test_engine)

# The tuner's model of evaluate() lives in tools/, so these are the two tests
# that include from there. S028: a model that drifts from the engine tunes the
# wrong function and nothing downstream notices.
add_doctest_target(test_eval_model)
target_include_directories(test_eval_model PRIVATE ../tools)

# S041: the tuner's --only groups have to partition its parameter vector, and
# free_mask() has swallowed the term appended after it four times with nothing
# in tests/ linking tuner.cpp at all. Needs no dataset, so it is a fast test
# like the rest.
add_doctest_target(test_tuner_groups)
target_include_directories(test_tuner_groups PRIVATE ../tools)

# S066: the tuner held out rows and datagen writes a game's rows contiguously,
# so 99.5 % of games had rows on both sides of the validation split. The split
# is cut between games now, and this is what holds it there. Its corpus is a
# fixture it writes into the temp directory and removes again, so it is a fast
# test like the rest.
add_doctest_target(test_tuner_split)
target_include_directories(test_tuner_split PRIVATE ../tools)

# S075: the label the tuner fits against is now a blend of the game result and
# datagen's score column, and `--lambda 0` has to be the pre-S075 fit bit for
# bit. Needs no dataset -- it is arithmetic and one parser -- so it is a fast
# test like the rest.
add_doctest_target(test_tuner_target)
target_include_directories(test_tuner_target PRIVATE ../tools)

# S076: the corpus dedupe pass. Its fixture is a handful of FENs in memory and
# the key comes from the engine it already links, so it is a fast test like the
# rest.
add_doctest_target(test_corpus_dedupe)
target_include_directories(test_corpus_dedupe PRIVATE ../tools)

# S073: the tunable search parameters. Compiled in both configurations and the
# same file in both -- the release build and the tune build each hold their live
# values against the one table that is outside every #ifdef, which is how two
# builds are compared member by member from inside one binary.
add_doctest_target(test_search_params)

# The surface guard reads MANUAL.md out of the source tree, so it needs to know
# where that tree is. Nothing else in the tests looks outside its own directory.
add_doctest_target(test_uci_surface)
target_compile_definitions(test_uci_surface
     PRIVATE CHESSO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# The SPRT harness is a shell script, and until S035 nothing in tests/
# exercised any shell script at all -- which is how a rename could break the
# only tool that can decide a change (INV-6) and stay broken across a commit.
# The smoke run plays no games: fastchess is a stub on PATH and the whole run
# happens in a throwaway git repository.
add_test(NAME test_fastchess_script
         COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_fastchess_script.sh
                      ${CMAKE_SOURCE_DIR}/fastchess.sh)
set_tests_properties(test_fastchess_script PROPERTIES LABELS "fast" TIMEOUT 60)

# The format check is one of the three commands in .moltke.json's
# step-completion gate, and until S054 it selected tracked files only -- so a
# step that added a source file was checked on everything except the files it
# had just added. Same shape as the fastchess smoke run: no games, no builds,
# and every fixture inside a throwaway git repository.
add_test(NAME test_clang_format_script
         COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_clang_format_script.sh
                      ${CMAKE_SOURCE_DIR}/clang-format.sh)
set_tests_properties(test_clang_format_script PROPERTIES LABELS "fast" TIMEOUT 60)

# Perft. Not a doctest binary: it has its own main() and reports through the
# process exit code. Minutes, not seconds, so it carries the "slow" label and
# is skipped by `ctest -L fast`.
add_executable(test_perft test_perft.cpp)
target_include_directories(test_perft SYSTEM PRIVATE json/single_include/nlohmann)
target_include_directories(test_perft PRIVATE ../src)
target_link_libraries(test_perft PRIVATE chesso_engine)
add_test(NAME test_perft COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_perft)
set_tests_properties(test_perft PROPERTIES LABELS "slow" TIMEOUT 1800)

# Move generation benchmark. Registered so it can be run through ctest, but
# under its own label and with no threshold: timings depend on the machine, so
# there is nothing here that can pass or fail. It prints, and you compare two
# runs yourself.
add_executable(bench_movegen bench_movegen.cpp)
target_include_directories(bench_movegen PRIVATE ../src)
target_link_libraries(bench_movegen PRIVATE chesso_engine)
add_test(NAME bench_movegen COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bench_movegen)
set_tests_properties(bench_movegen PROPERTIES LABELS "bench" TIMEOUT 600)

# Evaluation benchmark. Same reasoning and same label as bench_movegen: it
# prints, nothing passes or fails. It exists because a term that changes the
# score changes the tree, so no search timing can price a term on its own.
add_executable(bench_eval bench_eval.cpp)
target_include_directories(bench_eval PRIVATE ../src)
target_link_libraries(bench_eval PRIVATE chesso_engine)
add_test(NAME bench_eval COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bench_eval)
set_tests_properties(bench_eval PROPERTIES LABELS "bench" TIMEOUT 600)

# # Test gui
if(CHESSO_BUILD_GUI)
     message("-- Debug gui build enabled")
     set(PIXELLO_ENABLE_TESTS OFF) # Disable pixello tests
     add_subdirectory(pixello)
     add_executable(debug_gui debug_gui.cpp)

     target_include_directories(debug_gui SYSTEM PRIVATE pixello/src)
     target_include_directories(debug_gui PRIVATE ../src)

     target_link_libraries(debug_gui pixello)
     target_link_libraries(debug_gui chesso_engine)
else()
     message("-- Debug gui build disabled")
endif()

# add_test(NAME debug_gui COMMAND ${CMAKE_CURRENT_BINARY_DIR}/debug_gui)

# Perft generation program
add_executable(debug_perft_app debug_perft_app.cpp)
target_include_directories(debug_perft_app PRIVATE ../src)
target_link_libraries(debug_perft_app chesso_engine)

# Debug main
add_executable(debug_main debug_main.cpp)
target_include_directories(debug_main PRIVATE ../src)
target_link_libraries(debug_main chesso_engine)
