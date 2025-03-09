file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Include dependencies
include_directories(SYSTEM doctest/doctest)
include_directories(SYSTEM json/single_include/nlohmann)

add_executable(test_chesso_board test_chesso_board.cpp)
add_executable(test_gui test_gui.cpp)

target_include_directories(test_chesso_board SYSTEM PRIVATE ../src)

target_link_libraries(test_chesso_board PRIVATE chesso)
add_test(NAME test_chesso_board COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_chesso_board)
