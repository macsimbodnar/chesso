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

# S100: the fit's two unchecked halves -- that the columns load() packs are the
# columns evaluate_position() reads, and that gradient() is the derivative of
# the error it descends. Nothing in tests/ could name either until S100 moved
# dataset_t, load(), evaluate_position(), error_range() and gradient() out of
# tools/tuner.cpp's anonymous namespace into tools/tuner_model.hpp. Its corpus
# is a fixture it writes into the temp directory from the same curated FEN list
# test_eval_model runs over, so it is a fast test like the rest.
add_doctest_target(test_tuner_gradient)
target_include_directories(test_tuner_gradient PRIVATE ../tools)

# S075: the label the tuner fits against is now a blend of the game result and
# datagen's score column, and `--lambda 0` has to be the pre-S075 fit bit for
# bit. Needs no dataset -- it is arithmetic and one parser -- so it is a fast
# test like the rest.
add_doctest_target(test_tuner_target)
target_include_directories(test_tuner_target PRIVATE ../tools)

# S077: the SHA-256 an emitted weight table names its corpus by. Written from
# FIPS 180-4 rather than taken from a library so the value is checkable by
# `sha256sum`, which is only worth anything if the implementation is right.
# Needs no dataset -- it writes its own fixtures into the temp directory -- so
# it is a fast test like the rest.
add_doctest_target(test_corpus_hash)
target_include_directories(test_corpus_hash PRIVATE ../tools)

# S076: the corpus dedupe pass. Its fixture is a handful of FENs in memory and
# the key comes from the engine it already links, so it is a fast test like the
# rest.
add_doctest_target(test_corpus_dedupe)
target_include_directories(test_corpus_dedupe PRIVATE ../tools)

# S161: the FEN load boundary. 2026-08-22_adversarial-F01 -- load_FEN()
# validated syntax and not semantics, so castling rights with no king or rook on
# their squares, and en-passant squares with no victim, were accepted; the moves
# they licensed xor pieces onto empty squares and corrupt the board silently in
# Release. Perft (INV-1) cannot see it because every perft FEN is semantically
# valid. The test file was written by the audit before the fix and observed red.
# No dataset and no search -- it loads FENs and counts flags -- so it is a fast
# test like the rest.
add_doctest_target(test_audit_fen_semantics)

# S175: the Polyglot key on an edge-file en-passant square.
# 2026-09-03_adversarial-F01 -- get_key() looked for the capturing pawn with
# +7/+9 (-9/-7) index offsets, which wrap round the board edge for an en-passant
# square on the a- or h-file, so a same-side pawn on the wrapped square switched
# the en-passant component on against the format. Seven of the shipped book's
# 172232 entries carried such a key and no other Polyglot reader could find
# them. The expected keys are python-chess's, an independent implementation of
# the same specification; the file was written by the audit before the fix and
# observed red, 7 of 10. No dataset and no search, so it is a fast test like the
# rest.
add_doctest_target(test_audit_polyglot_key)

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

# S156: the mined breadth set, scored as a count with a floor. It reads
# adocs/data/S145_mined_set.tsv out of the source tree, so it needs the same
# definition test_uci_surface does.
#
# Its own binary and not another case inside test_engine, for one measured
# reason: 318 positions at depth 10 cost 18.28 s in Release on the machine
# .moltke.local.md describes, against 28.50 s for the whole fast label before
# it. Folded into test_engine that would take its line from 3.19 s to 21.5 s
# with nothing in the ctest output to say where the time went. A cost this size
# is reported rather than absorbed.
#
# The timeout is its own for the same reason, and the Debug figure is why it
# has to be: the same binary is 697 s in Debug, 38 times the Release one, so
# the shared 60 s of add_doctest_target would report a failure that is the
# clock. 1500 s is a little over twice the measured Debug run, which is the
# rule the timeout block at the top of this file states, and 120 s is over six
# times the Release one.
add_doctest_target(test_mate_breadth)
target_compile_definitions(test_mate_breadth
     PRIVATE CHESSO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
     set(CHESSO_MATE_BREADTH_TIMEOUT 120)
else()
     set(CHESSO_MATE_BREADTH_TIMEOUT 1500)
endif()
set_tests_properties(test_mate_breadth
     PROPERTIES LABELS "fast" TIMEOUT ${CHESSO_MATE_BREADTH_TIMEOUT})

# S147: every info line claiming a mate carries a line that reaches it. Reads
# both S145 sets out of the source tree, so it needs the same definition
# test_mate_breadth does.
#
# Its own binary for the same reason and at a fraction of the cost: 400
# positions at depth 8, which is enough for both sets to report mates and does
# not buy mate finding this file asserts nothing about. The Debug ratio
# test_mate_breadth measured (38x) is what sets the second timeout; neither is
# tight.
add_doctest_target(test_mate_pv)
target_compile_definitions(test_mate_pv
     PRIVATE CHESSO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
     set(CHESSO_MATE_PV_TIMEOUT 120)
else()
     set(CHESSO_MATE_PV_TIMEOUT 1500)
endif()
set_tests_properties(test_mate_pv
     PROPERTIES LABELS "fast" TIMEOUT ${CHESSO_MATE_PV_TIMEOUT})

# S170: a mate score a search inherited -- from an entry an earlier move's
# search wrote, or from a proof this search then overwrote -- still carries a
# line that reaches it. Reads adocs/data/S170_cases.tsv out of the source tree,
# so it needs the same definition test_mate_pv does.
#
# Its own binary because it is the only test here that replays whole games
# through one process without a `ucinewgame` between the plies, which is the
# only shape that reproduces the defect at all: test_mate_pv gives every case
# a cold table and cannot see it. 146 searches over four games at their own
# node budgets, 6.3 s in Release when the cases were chosen. The Debug ratio
# test_mate_breadth measured (38x) is what sets the second timeout; neither is
# tight.
add_doctest_target(test_mate_carry)
target_compile_definitions(test_mate_carry
     PRIVATE CHESSO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
     set(CHESSO_MATE_CARRY_TIMEOUT 120)
else()
     set(CHESSO_MATE_CARRY_TIMEOUT 1500)
endif()
set_tests_properties(test_mate_carry
     PROPERTIES LABELS "fast" TIMEOUT ${CHESSO_MATE_CARRY_TIMEOUT})

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

# S174: the two tools that sit on algebraic_to_move(), run on fixture PGNs.
# 2026-09-03_adversarial-F02 -- the parser's failure paths were `assert(false)`
# with no return, so in Release an unparseable SAN token became a fabricated
# move that make_move() applied, and make_book wrote a loadable book from a
# rewritten board while pgn_to_positions -- the DEC-023 board tool -- printed
# positions that never occurred, both with exit 0. Nothing in tests/ ran either
# tool. Same shape as the script tests above: fixtures in a throwaway directory,
# no engine searched. The binaries come from tools/, hence the target files.
add_test(NAME test_make_book_tools
         COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_make_book_tools.sh
                      $<TARGET_FILE:make_book> $<TARGET_FILE:pgn_to_positions>)
set_tests_properties(test_make_book_tools PROPERTIES LABELS "fast" TIMEOUT 60)

# S177: rating.sh's terminal marker, and the GNU tools both it and
# build_release.sh assumed. 2026-09-03_adversarial-F03 -- on a machine without
# coreutils `./rating.sh --bracket` died at `$(nproc)` with exit 127 before the
# RATING-RUN-* trap was armed, so a detached run would have left its watcher
# spinning (the class S167 fixed for fastchess.sh). Same shape as the fastchess
# smoke run: no game, a throwaway directory, and a PATH holding only the
# utilities the test links in, so the result does not depend on the machine.
add_test(NAME test_rating_script
         COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_rating_script.sh
                      ${CMAKE_SOURCE_DIR}/rating.sh ${CMAKE_SOURCE_DIR}/build_release.sh)
set_tests_properties(test_rating_script PROPERTIES LABELS "fast" TIMEOUT 60)

# S189: the commit gate. Nine cases over the one thing gate.sh decides -- is a
# signature owed, is the one in the message the one the binary prints -- in a
# throwaway git repository whose PATH holds stubs for cmake, ctest and
# clang-format.sh. git is real, because the touched-paths and message reading
# is the half of the logic a stub would hide. No suite runs and no game is
# played, so it costs nothing and does not depend on the machine.
add_test(NAME test_gate_script
         COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_gate_script.sh
                      ${CMAKE_SOURCE_DIR}/tools/gate.sh)
set_tests_properties(test_gate_script PROPERTIES LABELS "fast" TIMEOUT 60)

# S084: the SPSA driver. Python, and the first test in this directory that is
# neither doctest nor shell -- the driver is a tool and the objective it has to
# be right about is a noisy quadratic, not a chess position.
#
# It plays no games. The gate is a synthetic objective with a known optimum,
# because a driver tested by playing games cannot tell a bug in itself from
# noise in the objective at any budget this machine can afford: a sign error and
# a parameter that does not matter produce the same flat trajectory. The
# sign-flipped run is asserted to FAIL the criterion the honest one passes.
#
# ~4 s, which is a third of the fast suite as it stands, so the convergence runs
# are sized by measurement rather than by preference: 5000 iterations x 4 pairs
# converges the same objective as 20000 x 1 and costs a quarter of the wall
# time. The system python3 is enough -- stdlib only, no python-chess.
add_test(NAME test_spsa_driver
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_spsa_driver.py)
set_tests_properties(test_spsa_driver PROPERTIES LABELS "fast" TIMEOUT 120)

# S141: `--touches`, the one mode of tools/plan_prose_check.py that is in the
# suite. The other two are not, and DEV_MANUAL.md says why: any source commit
# shifts lines under fifty step files at once, so gating on citation freshness
# would make red the normal state and this the check that gets weakened to clear
# it. That reason does not reach here. `--touches` holds no line numbers: it
# fires only when a step's goal names a code symbol that no file the step may
# touch carries, which changes when a step file is written or a symbol changes
# file, and both are rare and both are one line to fix.
#
# 0.2 s, no build products read, and it skips itself with exit 0 outside a git
# checkout. 2026-08-20_plan_review-F06 is the finding; S039 is the case, and it
# was observed red on that step before the repair.
add_test(NAME test_plan_touches
         COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/plan_prose_check.py
                         --touches)
set_tests_properties(test_plan_touches PROPERTIES LABELS "fast" TIMEOUT 60)

# S150: `--params`, the second mode in the suite. Same reasoning as `--touches`
# and for the same reason it is not `--citations`: it holds no line numbers, so
# an unrelated source commit cannot make it red. It fires only when a document
# states a different number for a search parameter than src/search_params.hpp
# compiles, which is exactly the class 2026-08-21_adversarial-F02 found -- three
# values S085 retuned and the prose did not follow, in specs.md and MANUAL.md,
# with specs.md first in the reading order. Observed red on all three before the
# repair, the third against `eaad88b^:adocs/specs.md` where its wording still
# lives.
#
# It can also go red on a rewrite rather than on a drift: PARAM_PHRASES keys on
# the wording of sentences that never name their parameter, and a rule that
# matches nothing reports STALE. That is the check's maintenance cost and it is
# meant to be visible rather than silent.
#
# S184 widened what it reads: the four documents above **plus every file in
# adocs/plan_todo/ and adocs/plan_current/**, matched on the C++ symbol as well
# as the UCI name, because a step file names `ASPIRATION_DELTA` where a manual
# names `AspirationDelta` and the four-document set could not see the class at
# all. It found four on the day it was written -- S082 and S115 at pre-S085
# values, S127, and S184's own text. adocs/plan_done/ stays out: it is history.
# Cost on the workstation, 68 pending files: 0.37 s over the four documents,
# 1.27 s over the whole set, once per build.
add_test(NAME test_plan_params
         COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/plan_prose_check.py
                         --params)
set_tests_properties(test_plan_params PROPERTIES LABELS "fast" TIMEOUT 60)

# S187: the planted-case gate on `--citations` itself. Not a check over the
# plan -- that is test_plan_citations' sibling below -- but a check that the
# checker still recognises a citation. Each case writes one step file into a
# temporary directory and runs the mode over that file alone, so a plan edit
# cannot turn it red and a recogniser that stopped firing cannot hide behind a
# green real set.
#
# It resolves the symbols its passing cases name against this checkout:
# `negamax` in src/search.cpp, "pruning does not hide a forced mate" in
# tests/test_search.cpp, `GAME_PHASE_MAX` in the comment in src/search_params.hpp
# that must stay comment-only for the negative case to mean anything. Renaming
# one turns this red, and the rename is the fix. 0.6 s, stdlib only.
add_test(NAME test_plan_citations
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_plan_citations.py)
set_tests_properties(test_plan_citations PROPERTIES LABELS "fast" TIMEOUT 120)

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
