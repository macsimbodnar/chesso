file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable(Chesso ${SRCS})

target_link_libraries(Chesso pixello)
target_include_directories(Chesso PRIVATE ../pixello/src)

# Add the binary as test so we can run it with ctest --verbose
add_test(NAME Chesso
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/Chesso
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)
