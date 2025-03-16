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
add_executable(test_perft test_perft.cpp)
target_include_directories(test_perft SYSTEM PRIVATE json/single_include/nlohmann)
target_include_directories(test_perft PRIVATE ../src)
target_link_libraries(test_perft PRIVATE chesso)
add_test(NAME test_perft COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_perft)

# Test gui
set(PIXELLO_ENABLE_TESTS OFF) # Disable pixello tests
add_subdirectory(pixello)
add_executable(debug_gui debug_gui.cpp)

target_include_directories(debug_gui SYSTEM PRIVATE pixello/src)
target_include_directories(debug_gui PRIVATE ../src)

target_link_libraries(debug_gui pixello)
target_link_libraries(debug_gui chesso)

# add_test(NAME debug_gui COMMAND ${CMAKE_CURRENT_BINARY_DIR}/debug_gui)


# Perft generation program
add_executable(debug_perft_app debug_perft_app.cpp)
target_include_directories(debug_perft_app PRIVATE ../src)
target_link_libraries(debug_perft_app chesso)
