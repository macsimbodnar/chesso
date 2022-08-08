file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable(Chesso ${SRCS})

target_link_libraries(Chesso pixello)
target_include_directories(Chesso PRIVATE ../pixello/src)
