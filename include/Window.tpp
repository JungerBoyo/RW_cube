#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

template<>
void rw_cube::Window::setKeyCallback
(void(*key_callback)(GLFWwindow*, int, int, int, int));

template<>
void rw_cube::Window::setMousePositionCallback
(void(*mouse_position_callback)(GLFWwindow*, double, double));