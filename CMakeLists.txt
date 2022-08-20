cmake_minimum_required(VERSION 3.18)

project(Chesso)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PIXELLO_ENABLE_TESTS OFF) # Disable pixello tests

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
add_subdirectory(src)
add_subdirectory(pixello)
