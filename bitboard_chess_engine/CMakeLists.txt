file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable(BB_Chesso ${SRCS})

target_link_libraries(BB_Chesso pixello)
target_include_directories(BB_Chesso PRIVATE ../pixello/src)

# Add the binary as test so we can run it with ctest --verbose
add_test(NAME BB_Chesso
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/BB_Chesso
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)
