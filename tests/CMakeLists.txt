file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Test chesso board
add_executable(test_chesso test_chesso.cpp)

target_include_directories(test_chesso SYSTEM PRIVATE doctest/doctest)
target_include_directories(test_chesso SYSTEM PRIVATE json/single_include/nlohmann)
target_include_directories(test_chesso PRIVATE ../src)

target_link_libraries(test_chesso PRIVATE chesso_engine)

add_test(NAME test_chesso COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_chesso)

# # Test openings
# add_executable(test_openings test_openings.cpp)

# target_include_directories(test_openings SYSTEM PRIVATE doctest/doctest)
# target_include_directories(test_openings PRIVATE ../src)

# target_link_libraries(test_openings PRIVATE chesso_engine)

# add_test(NAME test_openings COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_openings)

# Perft
add_executable(test_perft test_perft.cpp)
target_include_directories(test_perft SYSTEM PRIVATE json/single_include/nlohmann)
target_include_directories(test_perft PRIVATE ../src)
target_link_libraries(test_perft PRIVATE chesso_engine)
add_test(NAME test_perft COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_perft)

# # Test gui
if(CHESSO_BUILD_GUI)
     message("-- Debug gui build enabled")
     set(PIXELLO_ENABLE_TESTS OFF) # Disable pixello tests
     add_subdirectory(pixello)
     add_executable(debug_gui debug_gui.cpp)

     target_include_directories(debug_gui SYSTEM PRIVATE pixello/src)
     target_include_directories(debug_gui PRIVATE ../src)

     target_link_libraries(debug_gui pixello)
     target_link_libraries(debug_gui chesso_engine)
else()
     message("-- Debug gui build disabled")
endif()

# add_test(NAME debug_gui COMMAND ${CMAKE_CURRENT_BINARY_DIR}/debug_gui)

# Perft generation program
add_executable(debug_perft_app debug_perft_app.cpp)
target_include_directories(debug_perft_app PRIVATE ../src)
target_link_libraries(debug_perft_app chesso_engine)

# Debug main
add_executable(debug_main debug_main.cpp)
target_include_directories(debug_main PRIVATE ../src)
target_link_libraries(debug_main chesso_engine)
