#include "Window.hpp"

#include <stdexcept>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace rw_cube;

struct Window::WinNative {
	GLFWwindow *value{nullptr};
};

Window::Window(std::string_view title, int w, int h,
			   void (*win_error_callback)(int, const char *),
			   void (*gl_error_callback)(std::uint32_t, std::uint32_t,
										 std::uint32_t, std::uint32_t, int,
										 const char *, const void *))
	: win_handle_(std::make_shared<WinNative>()) {

	/// GLFW INIT
	if (glfwInit() != GLFW_TRUE) {
		throw std::runtime_error("glfw init failed");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SHCONFIG_GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SHCONFIG_GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	win_handle_->value = glfwCreateWindow(w, h, title.data(), nullptr, nullptr);
	if (win_handle_->value == nullptr) {
		throw std::runtime_error("window creation failed");
	}
	glfwMakeContextCurrent(win_handle_->value);

	if (win_error_callback != nullptr) {
		glfwSetErrorCallback(win_error_callback);
	}

	/// GLAD INIT
	if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) ==
		0) { // NOLINT
		throw std::runtime_error("glad loader failed");
	}
	if (gl_error_callback != nullptr) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
							  GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
							  GL_TRUE);
		glDebugMessageCallback(gl_error_callback, nullptr);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

float Window::time() const {
	return static_cast<float>(glfwGetTime());
}
std::pair<int, int> Window::size() const {
	int w{0};
	int h{0};
	glfwGetWindowSize(win_handle_->value, &w, &h);
	return {w, h};
}
void Window::setViewport(int w, int h) const {
	glViewport(0, 0, w, h);
}

void Window::swapBuffers() const {
	glfwSwapBuffers(win_handle_->value);
}
bool Window::shouldClose() const {
	return glfwWindowShouldClose(win_handle_->value) != 0;
}
void Window::pollEvents() const {
	glfwPollEvents();
}

void Window::deinit() {
	glfwDestroyWindow(win_handle_->value);
	win_handle_.reset();
	glfwTerminate();
}