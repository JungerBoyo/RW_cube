add_library(wrappers_INC INTERFACE Shader.hpp Window.hpp Cube.hpp Ubo.hpp)
target_include_directories(wrappers_INC INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})