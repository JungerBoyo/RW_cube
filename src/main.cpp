#include <Cube.hpp>
#include <Shader.hpp>
#include <Ubo.hpp>
#include <Window.hpp>
#include <Camera.hpp>

#include <array>
#include <numbers>

#include <fmt/format.h>
#include <linmath.h>
#include <spdlog/spdlog.h>

using namespace rw_cube;

struct WinData {
	Camera& camera;
	float prev_mouse_x_position{ 0.F };
	float prev_mouse_y_position{ 0.F };
	float timestep{ 0.F };
	float fov{ std::numbers::pi_v<float>/4.F };
};

int main() {
	try {

		Window win{
			"cube demo",
			640,
			480,
			[](int, const char *error_msg) {
				spdlog::error("[glfw3] {}", error_msg);
			},
			[](std::uint32_t, std::uint32_t, std::uint32_t id, std::uint32_t,
			   int, const char *message, const void *) {
				spdlog::error("[glad] {}, id = {}", message, id);
			},
		};

		/*

		*/

		Camera camera;
		WinData win_data = {.camera = camera };
		win.setWinUserDataPointer(static_cast<void*>(&win_data));
		win.setKeyCallback(+[](GLFWwindow* win_handle, int key, int, int action, int) {
			if (action == GLFW_PRESS or action == GLFW_REPEAT) {
				auto* win_data_ptr = static_cast<WinData*>(glfwGetWindowUserPointer(win_handle));
				const auto timestep = win_data_ptr->timestep;
				auto& camera = win_data_ptr->camera;
				static constexpr auto step{ 10.F };
				static constexpr auto fov_step{ std::numbers::pi_v<float> / 64.F };
				static constexpr auto fov_low_lim{ 10.F/180.F * std::numbers::pi_v<float> };
				static constexpr auto fov_high_lim{ 120.F/180.F * std::numbers::pi_v<float> };
				switch (key) {
				case GLFW_KEY_W: camera.move( 0.F, 0.F, step * timestep); break;
				case GLFW_KEY_A: camera.move(-step * timestep, 0.F, 0.F); break;
				case GLFW_KEY_S: camera.move( 0.F, 0.F,-step * timestep); break;
				case GLFW_KEY_D: camera.move( step * timestep, 0.F, 0.F); break;
				case GLFW_KEY_K: 
						win_data_ptr->fov = std::clamp(win_data_ptr->fov - fov_step, fov_low_lim, fov_high_lim);
					break;
				case GLFW_KEY_J: 
						win_data_ptr->fov = std::clamp(win_data_ptr->fov + fov_step, fov_low_lim, fov_high_lim);
					break;
				case GLFW_KEY_Q: glfwSetWindowShouldClose(win_handle, GLFW_TRUE); break;
				default: break;
				}
			}
		});
		win.setMousePositionCallback(+[](GLFWwindow* win_handle, double x_pos, double y_pos) {
			auto* win_data_ptr = static_cast<WinData*>(glfwGetWindowUserPointer(win_handle));
			const auto timestep = win_data_ptr->timestep;
			auto& camera = win_data_ptr->camera;

			int w{ 0 };
			int h{ 0 };
			glfwGetWindowSize(win_handle, &w, &h);
			const auto dw = static_cast<double>(w);
			const auto dh = static_cast<double>(h);
			glfwSetCursorPos(win_handle, dw/2.0, dh/2.0);

			const auto aspect_ratio = static_cast<float>(dw)/static_cast<float>(dh);

			const auto x_step = (static_cast<float>(x_pos) - win_data_ptr->prev_mouse_x_position);
			const auto y_step = aspect_ratio * (static_cast<float>(y_pos) - win_data_ptr->prev_mouse_y_position);

			static constexpr auto step{ 0.009F };
			camera.rotateXYPlane(timestep * step * x_step, timestep * step * y_step);
		});

		const std::array<std::filesystem::path, 2> shbin_paths{
			{"shaders/bin/vert.spv", "shaders/bin/frag.spv"}};
		Shader sh(shbin_paths);

		constexpr std::array<AttribConfig, 2> attrib_configs{
			{{SHCONFIG_IN_POSITION_LOCATION, 3},
			 {SHCONFIG_IN_COLOR_LOCATION, 3}}};
		constexpr std::array<AttribConfig, 1> instance_attrib_configs{
			{{SHCONFIG_IN_OFFSET_LOCATION, 3}}
		};
		constexpr std::array<float, static_cast<std::size_t>(19U * 3U)> instance_data{{
			 0.F, 0.F, 0.F,
			 2.F, 0.F, 0.F,
			-2.F, 0.F, 0.F,
			 0.F, 0.F, 2.F,
			 0.F, 0.F,-2.F,
			 0.F, 2.F, 0.F,
			 0.F,-2.F, 0.F,
			 4.F, 0.F, 0.F,
			-4.F, 0.F, 0.F,
			 0.F, 0.F, 4.F,
			 0.F, 0.F,-4.F,
			 0.F, 4.F, 0.F,
			 0.F,-4.F, 0.F,
			 6.F, 0.F, 0.F,
			-6.F, 0.F, 0.F,
			 0.F, 0.F, 6.F,
			 0.F, 0.F,-6.F,
			 0.F, 6.F, 0.F,
			 0.F,-6.F, 0.F,
		}};
		Cube cube(0U, attrib_configs, 1U, instance_attrib_configs, instance_data.data(), sizeof(instance_data));

		UBO ubo(SHCONFIG_MVP_UBO_BINDING, sizeof(mat4x4)); // NOLINT

		sh.bind();
		cube.bind();

		float last_time{0.F};
		while (!win.shouldClose()) {
			const auto [w, h] = win.size();
			win_data.prev_mouse_x_position = static_cast<float>(w)/2.F;			
			win_data.prev_mouse_y_position = static_cast<float>(h)/2.F;			
			win.setViewport(w, h);

			// NOLINTBEGIN
			mat4x4 proj_mat;
			mat4x4_perspective(proj_mat, win_data.fov,
							   static_cast<float>(w) / static_cast<float>(h),
							   .1F, 100.F);

			const auto time = win.time();
			const float timestep = time - last_time;
			last_time = time;
			win_data.timestep = timestep;

			const float x_rot = timestep * std::numbers::pi_v<float> / 12.F;
			const float y_rot = timestep * std::numbers::pi_v<float> / 24.F;
			const float z_rot = timestep * std::numbers::pi_v<float> / 7.F;
			const auto [x_angle, y_angle, z_angle] =
				cube.rotate(x_rot, y_rot, z_rot);

			const float x_mv = .2F * std::sin(y_angle) * timestep;
			const float y_mv = .2F * std::cos(x_angle) * timestep;
			const float z_mv = .2F * std::sin(z_angle) * timestep;
			const auto [x_pos, y_pos, z_pos] = cube.move(x_mv, y_mv, z_mv);

			mat4x4 model_mat;
			mat4x4_translate(model_mat, 0.F + x_pos, 0.F + y_pos, 7.F + z_pos);
			mat4x4_rotate_X(model_mat, model_mat, x_angle);
			mat4x4_rotate_Y(model_mat, model_mat, y_angle);
			mat4x4_rotate_Z(model_mat, model_mat, z_angle);
			mat4x4_translate_in_place(model_mat, -.5F, -.5F, -.5F);
			mat4x4 view_mat;
			camera.lookAt(view_mat);
			mat4x4 mvp;
			mat4x4_mul(mvp, proj_mat, view_mat);
			mat4x4_mul(mvp, mvp, model_mat);
			ubo.sendData(static_cast<const void *>(mvp));
			// NOLINTEND

			cube.draw();

			win.swapBuffers();
			win.pollEvents();
		}

		ubo.deinit();
		cube.deinit();
		sh.deinit();
		win.deinit();

	} catch (std::exception &e) {
		spdlog::critical("{}", e.what());
	}
}
