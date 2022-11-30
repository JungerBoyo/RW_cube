#include <Cube.hpp>
#include <Shader.hpp>
#include <Ubo.hpp>
#include <Window.hpp>
#include <Camera.hpp>

#include <array>
#include <numbers>

#include <fmt/format.h>
#include <glad/glad.h>
#include <linmath.h>
#include <spdlog/spdlog.h>

using namespace rw_cube;

struct WinData {
	Camera& camera;
	float prev_mouse_x_position{ 0.F };
	float prev_mouse_y_position{ 0.F };
	float timestep{ 0.F };
	float fov{ std::numbers::pi_v<float>/4.F };
	bool light_move_mode{ false };
	float* light_pos;
};

struct UboData {
    mat4x4 vp;
    mat4x4 m_position;
    vec3 light_pos;
    float ambient_light;
	vec3 camera_pos;
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

		Camera camera;
		UboData ubo_data{ .light_pos = {3.5F, 0.F, 5.F}, .ambient_light = 0.2F};
		WinData win_data = {.camera = camera, .light_pos = ubo_data.light_pos};
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
				case GLFW_KEY_W: 
						if (!win_data_ptr->light_move_mode) {
							camera.move(0.F, 0.F, step * timestep); 
						} else {
							win_data_ptr->light_pos[2] += step * timestep;
						}
					break;
				case GLFW_KEY_A: 
						if  (!win_data_ptr->light_move_mode) {
							camera.move(-step * timestep, 0.F, 0.F); break;
						} else {
							win_data_ptr->light_pos[0] += step * timestep;
						}
					break;
				case GLFW_KEY_S: 
						if  (!win_data_ptr->light_move_mode) {
							camera.move(0.F, 0.F,-step * timestep); 
						} else {
							win_data_ptr->light_pos[2] -= step * timestep;
						}
					break;
				case GLFW_KEY_D: 
						if  (!win_data_ptr->light_move_mode) {
							camera.move(step * timestep, 0.F, 0.F); 
						} else {
							win_data_ptr->light_pos[0] -= step * timestep;
						}
					break;
				case GLFW_KEY_SPACE:
						win_data_ptr->light_pos[1] += step * timestep;
					break;
				case GLFW_KEY_C:
						win_data_ptr->light_pos[1] -= step * timestep;
					break;
				case GLFW_KEY_K: 
						win_data_ptr->fov = std::clamp(win_data_ptr->fov - fov_step, fov_low_lim, fov_high_lim);
					break;
				case GLFW_KEY_J: 
						win_data_ptr->fov = std::clamp(win_data_ptr->fov + fov_step, fov_low_lim, fov_high_lim);
					break;
				case GLFW_KEY_L:
						win_data_ptr->light_move_mode = !win_data_ptr->light_move_mode;
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

		int ext_num{0};
		bool is_arb_spirv_supported{false};
		glGetIntegerv(GL_NUM_EXTENSIONS, &ext_num);
		for (int i{0}; i<ext_num; ++i) {
			const auto ext_str = std::string_view(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
			if (ext_str == "GL_ARB_gl_spirv") {
				is_arb_spirv_supported = true;
				break;
			}
		}

		Cube cube(
			0U, 
			{
				{SHCONFIG_IN_POSITION_LOCATION, 3},
				{SHCONFIG_IN_TEXCOORD_LOCATION, 3},
				{SHCONFIG_IN_NORMAL_LOCATION, 3}
			}, 
			{
				Shader(is_arb_spirv_supported, {
					is_arb_spirv_supported ? "shaders/bin/diffuse_shader/vert.spv" : "shaders/src/diffuse_shader/shader.vert", 
					is_arb_spirv_supported ? "shaders/bin/diffuse_shader/frag.spv" : "shaders/src/diffuse_shader/shader.frag"
				}),
				Shader(is_arb_spirv_supported, {
					is_arb_spirv_supported ? "shaders/bin/light_source_shader/vert.spv" : "shaders/src/light_source_shader/shader.vert", 
					is_arb_spirv_supported ? "shaders/bin/light_source_shader/frag.spv" : "shaders/src/light_source_shader/shader.frag"
				}),
				Shader(is_arb_spirv_supported, {
					is_arb_spirv_supported ? "shaders/bin/specular_shader/vert.spv" : "shaders/src/specular_shader/shader.vert", 
					is_arb_spirv_supported ? "shaders/bin/specular_shader/frag.spv" : "shaders/src/specular_shader/shader.frag"
				}),
				Shader(is_arb_spirv_supported, {
					is_arb_spirv_supported ? "shaders/bin/phong_shader/vert.spv" : "shaders/src/phong_shader/shader.vert", 
					is_arb_spirv_supported ? "shaders/bin/phong_shader/frag.spv" : "shaders/src/phong_shader/shader.frag"
				}),
				Shader(is_arb_spirv_supported, {
					is_arb_spirv_supported ? "shaders/bin/texture_shader/vert.spv" : "shaders/src/texture_shader/shader.vert", 
					is_arb_spirv_supported ? "shaders/bin/texture_shader/frag.spv" : "shaders/src/texture_shader/shader.frag"
				})
			},
			{
				{{0.F, 0.F, 0.F}, {0.F, 0.F, 0.F}, {3.F, 0.F, 0.F}, {-3.F, 0.F, 0.F}, {6.F, 0.F, 0.F}}
			},
			"assets/textures/mcgrasstexture.png"
		);

		UBO ubo(SHCONFIG_MVP_UBO_BINDING, sizeof(UboData)); // NOLINT

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
			mat4x4 view_mat;
			camera.lookAt(view_mat);

			mat4x4 vp;
			mat4x4_mul(vp, proj_mat, view_mat);

			// ubo data update camera position + vp
			vec3_dup(ubo_data.camera_pos, camera.position_);
			mat4x4_dup(ubo_data.vp, vp);

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

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (std::size_t i{0}; i<cube.cube_count; ++i) {
				const auto shader = cube.shaders[i];
				const auto offset = cube.offsets[i];

				mat4x4 model_mat;
				mat4x4_translate(model_mat, 
					0.F /*+ x_pos*/ + offset[0], 
					0.F /*+ y_pos*/ + offset[1], 
					7.F /*+ z_pos*/ + offset[2]
				);				
				// mat4x4_rotate_X(model_mat, model_mat, x_angle);
				// mat4x4_rotate_Y(model_mat, model_mat, y_angle);
				// mat4x4_rotate_Z(model_mat, model_mat, z_angle);
				mat4x4_translate_in_place(model_mat, -.5F, -.5F, -.5F);

				// ubo data update model mat
				mat4x4_dup(ubo_data.m_position, model_mat);

				ubo.sendData(static_cast<const void *>(&ubo_data));
				// NOLINTEND

				shader.bind();
				cube.draw();
			}

			win.swapBuffers();
			win.pollEvents();
		}

		ubo.deinit();
		cube.deinit();
		win.deinit();

	} catch (std::exception &e) {
		spdlog::critical("{}", e.what());
	}
}
