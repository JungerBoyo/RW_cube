find_package(glfw3 REQUIRED)

add_library(wrappers_INC INTERFACE Shader.hpp Window.hpp Cube.hpp Ubo.hpp Camera.hpp)
target_link_system_libraries(wrappers_INC INTERFACE glfw::glfw)
target_include_directories(wrappers_INC INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(wrappers_INC PUBLIC ${RESOURCE_FILES_DIR})
