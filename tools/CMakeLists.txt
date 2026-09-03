# Analysis tooling. Not part of the engine and not covered by ctest, but built
# with it so it cannot rot: it links the engine library and would stop
# compiling the moment a signature it uses changes.
add_executable(pgn_to_positions pgn_to_positions.cpp)
target_include_directories(pgn_to_positions PRIVATE ../src)
target_link_libraries(pgn_to_positions PRIVATE chesso_engine)

# Self-play data generation for S028. Threads are its own, not the engine's:
# the search is single threaded and datagen runs one game per thread.
find_package(Threads REQUIRED)
add_executable(datagen datagen.cpp)
target_include_directories(datagen PRIVATE ../src)
target_link_libraries(datagen PRIVATE chesso_engine Threads::Threads)

# The evaluation tuner for S028. Reads eval_tables.hpp for its starting point
# and never links the search: the model it fits is evaluate(), in floating
# point, which is what makes a pass over a million positions cheap.
add_executable(tuner tuner.cpp)
target_include_directories(tuner PRIVATE ../src)
target_link_libraries(tuner PRIVATE chesso_engine Threads::Threads)

# S077. The commit an emitted table names is stamped at **build** time, not at
# configure time: a sha captured by `cmake -S . -B build` goes stale on the next
# commit, and a table carrying the wrong commit is worse than one carrying none.
# The script rewrites the header only when the value changes, so this costs one
# `git rev-parse` per build and a recompile only when the commit moves.
set(CHESSO_BUILD_INFO_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(CHESSO_BUILD_INFO ${CHESSO_BUILD_INFO_DIR}/chesso_build_info.hpp)

add_custom_target(chesso_build_info ALL
     COMMAND ${CMAKE_COMMAND}
          -DOUTPUT=${CHESSO_BUILD_INFO}
          -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
          -P ${CMAKE_SOURCE_DIR}/cmake/build_info.cmake
     BYPRODUCTS ${CHESSO_BUILD_INFO}
     COMMENT "Stamping the engine commit for tools/tuner")

add_dependencies(tuner chesso_build_info)
target_include_directories(tuner PRIVATE ${CHESSO_BUILD_INFO_DIR})

# S076. One row per distinct position in a tuning corpus. Links the engine
# because the key it dedupes on is the engine's own zobrist key: a second
# hashing scheme here would dedupe by a definition of "the same position" the
# rest of the project does not use.
add_executable(corpus_dedupe corpus_dedupe.cpp)
target_include_directories(corpus_dedupe PRIVATE ../src)
target_link_libraries(corpus_dedupe PRIVATE chesso_engine)

# S034 feasibility probe. Prices a table access against the cost of calling
# evaluate(), which is the question that decides whether caching the evaluation
# can pay for itself. Kept rather than thrown away: the answer changes every
# time the evaluation gets more expensive, and it will.
add_executable(probe_cost probe_cost.cpp)
target_include_directories(probe_cost PRIVATE ../src)
target_link_libraries(probe_cost PRIVATE chesso_engine)

# What the lazy shortcut's margin is worth, measured rather than sampled. Links
# the engine because the correction it reports has to be the one the search
# computes; the clamp inside evaluate_expensive() is why a copy of the model
# could not answer this.
add_executable(eval_spread eval_spread.cpp)
target_include_directories(eval_spread PRIVATE ../src)
target_link_libraries(eval_spread PRIVATE chesso_engine)

# S076. The positions that make evaluate()'s taper truncate hardest, which
# tests/test_eval_model.cpp pins and which belong to the weights rather than to
# the positions -- so the list is re-measured on every refit (DEC-057). Twice
# done by hand from a scratchpad script that no longer exists; tracked from
# here.
add_executable(truncation_scan truncation_scan.cpp)
target_include_directories(truncation_scan PRIVATE ../src)
target_link_libraries(truncation_scan PRIVATE chesso_engine)

# S100. What the tuning corpus contains per evaluation feature: the stored
# columns against the features recomputed from the FEN text, the two exact
# piece-square dependencies the rook-seventh and seventh-rank-passer columns
# have by construction, occurrence by phase band, and the R^2 of each term
# column on the tables it overlaps. Tracked rather than a scratchpad script
# because S082 and S083 replace the corpus and every one of these has to be
# re-run on the new one -- the S072 lesson.
add_executable(feature_audit feature_audit.cpp)
target_include_directories(feature_audit PRIVATE ../src)
target_link_libraries(feature_audit PRIVATE chesso_engine Threads::Threads)

# S171. What the table holds along a line the engine reported, after a game has
# been replayed into it. Links the engine because the entry that answered is
# the engine's own, in the state its own searches left it: nothing outside the
# process can see it, which is why the reproduction that found the defect could
# only say that the score was wrong and not where it came from.
add_executable(mate_trace mate_trace.cpp)
target_include_directories(mate_trace PRIVATE ../src)
target_link_libraries(mate_trace PRIVATE chesso_engine)

# S172. Builds the Polyglot book the engine embeds, and reads one back. Links
# the engine because the key it writes has to be the key the engine probes with
# and the moves have to come from the engine's own parser: a book built against
# a second definition of "the same position" loads and then finds nothing.
add_executable(make_book make_book.cpp)
target_include_directories(make_book PRIVATE ../src)
target_link_libraries(make_book PRIVATE chesso_engine)
