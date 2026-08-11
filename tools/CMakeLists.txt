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

# S034 feasibility probe. Prices a table access against the cost of calling
# evaluate(), which is the question that decides whether caching the evaluation
# can pay for itself. Kept rather than thrown away: the answer changes every
# time the evaluation gets more expensive, and it will.
add_executable(probe_cost probe_cost.cpp)
target_include_directories(probe_cost PRIVATE ../src)
target_link_libraries(probe_cost PRIVATE chesso_engine)
