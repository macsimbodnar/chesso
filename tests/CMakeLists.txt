add_executable(chesso_board_test test.cpp)

target_include_directories(chesso_board_test SYSTEM PRIVATE ../src)

add_test(NAME chesso_board_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/chesso_board_test)
