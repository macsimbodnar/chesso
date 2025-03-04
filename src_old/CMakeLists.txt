add_subdirectory(pixello_fix_version)

file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable(OLD_Chesso ${SRCS})

target_link_libraries(OLD_Chesso pixello_fix_version)
target_include_directories(OLD_Chesso PRIVATE pixello_fix_version/src)

# Add the binary as test so we can run it with ctest --verbose
add_test(NAME OLD_Chesso
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/OLD_Chesso
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)
