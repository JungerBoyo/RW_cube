find_package(glfw3 REQUIRED)
find_package(lodepng REQUIRED)

add_library(wrappers_INC 
    INTERFACE 
        Shader.hpp 
        Window.hpp 
        Cube.hpp 
        Ubo.hpp 
        Camera.hpp 
        CubeTexture.hpp
        Model.hpp
        utils.hpp
        PseudoQuadTree.hpp
)
target_link_system_libraries(wrappers_INC INTERFACE glfw::glfw lodepng::lodepng)
target_include_directories(wrappers_INC INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(wrappers_INC PUBLIC ${RESOURCE_FILES_DIR})
