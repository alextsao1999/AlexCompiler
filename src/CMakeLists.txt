set(SUB_DIRS ir optimizer utils semantic target parser)

include_directories(${SUB_DIRS})
foreach(DIR ${SUB_DIRS})
    aux_source_directory(${DIR} SOURCES)
endforeach()

add_library(libdragon STATIC ${SOURCES})
target_include_directories(libdragon PUBLIC ${SUB_DIRS})

add_executable(compiler main.cpp)
target_link_libraries(compiler libdragon)
