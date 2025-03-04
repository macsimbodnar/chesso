file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_library(pixello_fix_version STATIC ${SRCS})

# SDL2
find_package(SDL2 REQUIRED)
target_include_directories(pixello_fix_version SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(pixello_fix_version PRIVATE ${SDL2_LIBRARIES})

# SDL2_Image
find_package(SDL2_image REQUIRED)
target_include_directories(pixello_fix_version SYSTEM PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(pixello_fix_version PRIVATE ${SDL2_IMAGE_LIBRARIES})

# SDL2_ttf
find_package(SDL2_ttf REQUIRED)
target_include_directories(pixello_fix_version SYSTEM PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(pixello_fix_version PRIVATE ${SDL2_TTF_LIBRARIES})

# SDL2_mixer
find_package(SDL2_mixer REQUIRED)
target_include_directories(pixello_fix_version SYSTEM PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(pixello_fix_version PRIVATE ${SDL2_MIXER_LIBRARIES})

# SDL2_gfx
find_package(SDL2_gfx REQUIRED)
target_include_directories(pixello_fix_version SYSTEM PRIVATE ${SDL2_GFX_INCLUDE_DIRS})
target_link_libraries(pixello_fix_version PRIVATE ${SDL2_GFX_LIBRARIES})
