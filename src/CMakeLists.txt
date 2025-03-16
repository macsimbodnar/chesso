file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_library(chesso STATIC ${SRCS})

add_executable(chesso_engine chesso.cpp)
target_link_libraries(chesso_engine PRIVATE chesso)
