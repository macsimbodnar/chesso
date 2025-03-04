cmake_minimum_required(VERSION 3.18)

project(pixello_fix_version)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

option(PIXELLO_ENABLE_TESTS "Enable testing" ON)

# Add extra compiler checks
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

add_subdirectory(src)

if(PIXELLO_ENABLE_TESTS)
    message("Pixello tests enabled")
    enable_testing()
    add_subdirectory(tests)
else()
    message("Pixello tests disabled")
endif()
