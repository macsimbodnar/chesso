cmake_minimum_required(VERSION 3.18)

project(Chesso)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PIXELLO_ENABLE_TESTS OFF) # Disable pixello tests

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Add extra compiler checks
if(MSVC)
     add_compile_options(/W4 /WX)
else()
     add_compile_options(-Wall -Wextra -Werror)

     if(CMAKE_BUILD_TYPE STREQUAL "Debug")
          add_compile_options(-Wno-unused-variable -Wno-unused-but-set-variable)
     endif()
endif()

enable_testing()
add_subdirectory(src)
add_subdirectory(pixello)
