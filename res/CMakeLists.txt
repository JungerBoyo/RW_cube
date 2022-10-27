add_library(linmath STATIC linmath.h linmath.c)
target_include_directories(linmath
    PUBLIC  
        ${CMAKE_CURRENT_SOURCE_DIR}
)