cmake_minimum_required(VERSION 3.18)

project(Chesso)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add extra compiler checks
if(MSVC)
     add_compile_options(/W4 /WX)
else()
     add_compile_options(-Wall -Wextra -Werror)

     if(CMAKE_BUILD_TYPE STREQUAL "Debug")
          add_compile_options(-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter)
     endif()
endif()

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
