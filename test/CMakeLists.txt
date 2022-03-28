set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest)

include_directories(.)

link_libraries(libdragon gtest_main gmock_main)

add_executable(test_ir test_ir.cpp)
add_executable(test_node test_node.cpp)
add_executable(test_grammar test_grammar.cpp)
add_executable(test_lexer test_lexer.cpp)
add_executable(test_SysY test_SysY.cpp)
