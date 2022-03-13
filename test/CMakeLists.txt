include_directories(.)

link_libraries(libdragon)

add_executable(test_ir test_ir.cpp)
add_executable(test_node test_node.cpp)
