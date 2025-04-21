file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_library(chesso_engine STATIC ${SRCS})

add_executable(chesso chesso.cpp)
target_link_libraries(chesso PRIVATE chesso_engine)
