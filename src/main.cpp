#include <Cube.hpp>
#include <Shader.hpp>
#include <Ubo.hpp>
#include <Window.hpp>

#include <array>
#include <numbers>

#include <fmt/format.h>
#include <linmath.h>
#include <spdlog/spdlog.h>

using namespace rw_cube;

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

		const std::array<std::filesystem::path, 2> shbin_paths{
			{"shaders/bin/vert.spv", "shaders/bin/frag.spv"}};
		Shader sh(shbin_paths);

		constexpr std::array<AttribConfig, 2> attrib_configs{
			{{SHCONFIG_IN_POSITION_LOCATION, 3 * sizeof(float)},
			 {SHCONFIG_IN_COLOR_LOCATION, 3 * sizeof(float)}}};
		Cube cube(0U, attrib_configs);

		UBO ubo(SHCONFIG_MVP_UBO_BINDING, sizeof(mat4x4)); // NOLINT

		sh.bind();
		cube.bind();

		float last_time{0.F};
		while (!win.shouldClose()) {
			const auto [w, h] = win.size();
			win.setViewport(w, h);

			// NOLINTBEGIN
			mat4x4 proj_mat;
			mat4x4_perspective(proj_mat, std::numbers::pi_v<float> / 4.f,
							   static_cast<float>(w) / static_cast<float>(h),
							   .1F, 100.F);

			const auto time = win.time();
			const float timestep = time - last_time;
			last_time = time;

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
			mat4x4_translate(model_mat, 0.F + x_pos, 0.F + y_pos, -7.F + z_pos);
			mat4x4_rotate_X(model_mat, model_mat, x_angle);
			mat4x4_rotate_Y(model_mat, model_mat, y_angle);
			mat4x4_rotate_Z(model_mat, model_mat, z_angle);
			mat4x4_translate_in_place(model_mat, -.5F, -.5F, -.5F);
			mat4x4 mvp;
			mat4x4_mul(mvp, proj_mat, model_mat);
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