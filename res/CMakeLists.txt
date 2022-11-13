set(RESOURCE_FILES_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_library(linmath STATIC linmath.h linmath.c)
target_include_directories(linmath
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)