file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_library(pixello STATIC ${SRCS})

# SDL2
find_package(SDL2 REQUIRED)
target_include_directories(pixello SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(pixello PRIVATE ${SDL2_LIBRARIES})