# Analysis tooling. Not part of the engine and not covered by ctest, but built
# with it so it cannot rot: it links the engine library and would stop
# compiling the moment a signature it uses changes.
add_executable(pgn_to_positions pgn_to_positions.cpp)
target_include_directories(pgn_to_positions PRIVATE ../src)
target_link_libraries(pgn_to_positions PRIVATE chesso_engine)
