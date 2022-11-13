#ifndef RW_CUBE_CUBE_HPP
#define RW_CUBE_CUBE_HPP

#include <array>
#include <numbers>
#include <span>
#include <optional>

#include <Shader.hpp>

namespace rw_cube {

struct AttribConfig {
	std::int32_t index;
	std::int32_t size_in_dwords;
};

struct Cube {
#include "Indices.hpp"
#include "Vertices.hpp"

	std::uint32_t vbo_id_;
	std::uint32_t vao_id_;
	std::uint32_t instance_count_{ 1 };
	std::array<float, 3> rot_xyz_{{0.F, 0.F, 0.F}};
	std::array<float, 3> pos_xyz_{{0.F, 0.F, 0.F}};

	Cube(
		std::uint32_t attrib_binding, 
		std::span<const AttribConfig> attrib_configs,
		std::optional<std::uint32_t> instance_attrib_binding,
		std::optional<std::span<const AttribConfig>> instance_attrib_configs,
		const void* instance_data,
		std::uint32_t instance_data_size
	);
	void draw() const;

	std::tuple<float, float, float> rotate(float x, float y, float z);
	std::tuple<float, float, float> move(float x, float y, float z);

	void bind() const;
	void deinit();
};

} // namespace rw_cube

#endif