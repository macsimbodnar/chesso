file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# main.cpp is the only translation unit holding main(). Keeping it out of the
# library lets the test binaries link chesso.cpp (the UCI layer) and supply
# doctest's own main.
list(REMOVE_ITEM SRCS main.cpp)

add_library(chesso_engine STATIC ${SRCS})
target_include_directories(chesso_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(chesso main.cpp)
target_link_libraries(chesso PRIVATE chesso_engine)
