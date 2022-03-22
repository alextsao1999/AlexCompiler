include_directories(.)

link_libraries(libdragon)

add_executable(test_ir test_ir.cpp)
add_executable(test_node test_node.cpp)
add_executable(test_grammar test_grammar.cpp)
add_executable(test_lexer test_lexer.cpp)
