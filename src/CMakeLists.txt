find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(spdlog REQUIRED)
find_package(stb REQUIRED)

add_library(wrappers_IMPL STATIC Shader.cpp Window.cpp Cube.cpp Ubo.cpp Camera.cpp CubeTexture.cpp)
target_link_libraries(wrappers_IMPL PUBLIC wrappers_INC)
target_link_system_libraries(wrappers_IMPL
  PRIVATE
		fmt::fmt
    glad::glad
    linmath
    SHCONFIG
)

# MAIN EXE
add_executable(${PROJECT_NAME} 
  main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE 
    project_options
    project_warnings
    wrappers_IMPL
    SHCONFIG
)

target_link_system_libraries(${PROJECT_NAME}
  PRIVATE 
    glad::glad
    fmt::fmt
    spdlog::spdlog
    linmath
)

add_dependencies(${PROJECT_NAME} compile_and_copy_shaders_dirs)

target_include_directories(${PROJECT_NAME} 
  PRIVATE 
    "${CMAKE_BINARY_DIR}/config"
)
