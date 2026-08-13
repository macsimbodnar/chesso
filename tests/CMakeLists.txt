file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

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
     set_tests_properties(${name} PROPERTIES LABELS "fast" TIMEOUT 60)
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
