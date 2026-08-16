file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# main.cpp is the only translation unit holding main(). Keeping it out of the
# library lets the test binaries link chesso.cpp (the UCI layer) and supply
# doctest's own main.
list(REMOVE_ITEM SRCS main.cpp)

add_library(chesso_engine STATIC ${SRCS})
target_include_directories(chesso_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# PUBLIC, and it has to be: search_params.hpp declares a parameter as
# `inline constexpr int` without the definition and as `extern int` with it, so
# a tool or a test compiled the other way from the library it links would be an
# ODR violation the linker is not obliged to notice. S073.
if(CHESSO_TUNE)
     message("-- Tune build ON: the search parameters are UCI options. NOT the release binary")
     target_compile_definitions(chesso_engine PUBLIC CHESSO_TUNE)
else()
     message("-- Tune build OFF")
endif()

add_executable(chesso main.cpp)
target_link_libraries(chesso PRIVATE chesso_engine)
