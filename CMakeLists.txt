cmake_minimum_required(VERSION 3.18)

project(Chesso)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CHESSO_BUILD_GUI "Build the debug gui. Requires SDL2" OFF)

# Add extra compiler checks
if(MSVC)
     add_compile_options(/W4 /WX)
else()
     add_compile_options(-Wall -Wextra -Werror)

     if(CMAKE_BUILD_TYPE STREQUAL "Debug")
          add_compile_options(-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter)
     endif()

     # TODO: Check if this will speed it up. We don't use floating points anyway
     # string(REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
