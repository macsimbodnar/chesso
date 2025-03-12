file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Test chesso board
add_executable(test_chesso_board test_chesso_board.cpp)

target_include_directories(test_chesso_board SYSTEM PRIVATE doctest/doctest)
target_include_directories(test_chesso_board SYSTEM PRIVATE json/single_include/nlohmann)
target_include_directories(test_chesso_board PRIVATE ../src)

target_link_libraries(test_chesso_board PRIVATE chesso)

add_test(NAME test_chesso_board COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_chesso_board)

# Perft
add_executable(perft perft.cpp)
target_include_directories(perft SYSTEM PRIVATE json/single_include/nlohmann)
target_include_directories(perft PRIVATE ../src)
target_link_libraries(perft PRIVATE chesso)
add_test(NAME perft COMMAND ${CMAKE_CURRENT_BINARY_DIR}/perft)

# Test gui
set(PIXELLO_ENABLE_TESTS OFF) # Disable pixello tests
add_subdirectory(pixello)
add_executable(test_gui test_gui.cpp)

target_include_directories(test_gui SYSTEM PRIVATE pixello/src)
target_include_directories(test_gui PRIVATE ../src)

target_link_libraries(test_gui pixello)
target_link_libraries(test_gui chesso)

# Perft generation program
add_executable(chesso_perft_generator chesso_perft_generator.cpp)
target_include_directories(chesso_perft_generator PRIVATE ../src)
target_link_libraries(chesso_perft_generator chesso)
