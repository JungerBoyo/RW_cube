#ifndef RW_CUBE_CUBE_HPP
#define RW_CUBE_CUBE_HPP

#include <array>
#include <numbers>
#include <span>
#include <optional>
#include <filesystem>
#include <utils.hpp>

#include <Shader.hpp>

namespace rw_cube {

struct Cube {
#include "Indices.hpp"
#include "Vertices.hpp"
#include "TexCoords.hpp"
#include "Normals.hpp"

	std::uint32_t vbo_id_{ 0 };
	std::uint32_t vao_id_{ 0 };
	std::uint32_t tex_id_{ 0 };
	std::array<float, 3> rot_xyz_{{0.F, 0.F, 0.F}};
	std::array<float, 3> pos_xyz_{{0.F, 0.F, 0.F}};

	std::vector<Shader> shaders;
	std::vector<std::array<float, 3>> offsets;

	std::uint32_t cube_count{ 0 };

	Cube(
		std::uint32_t attrib_binding, 
		const std::vector<AttribConfig>& attrib_configs,
		const std::vector<Shader>& shaders,
		const std::vector<std::array<float, 3>>& offsets,
		const std::filesystem::path& tex_path
	);
	void draw() const;

	std::tuple<float, float, float> rotate(float x, float y, float z);
	std::tuple<float, float, float> move(float x, float y, float z);

	void bind() const;
	void deinit();
};

} // namespace rw_cube

#endif