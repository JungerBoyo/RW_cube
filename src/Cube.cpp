#include "Cube.hpp"

#include <cstring>
#include <algorithm>
#include <random>

#include <fmt/core.h>
#include <glad/glad.h>

using namespace rw_cube;

auto wrap(auto value, auto abs_limit, auto wrap_to) {
	return std::abs(value) > abs_limit ? wrap_to : value;
}

Cube::Cube(
	std::uint32_t attrib_binding, 
	std::span<const AttribConfig> attrib_configs,
	std::optional<std::uint32_t> instance_attrib_binding,
	std::optional<std::span<const AttribConfig>> instance_attrib_configs,
	const void* instance_data,
	std::uint32_t instance_data_size
) {
	auto is_instanced = instance_attrib_binding && instance_attrib_configs && instance_data != nullptr && instance_data_size != 0;

	glCreateBuffers(1, &vbo_id_);

	std::array<Vertex, INDICES.size()> vertices;
	
	std::array<float, 3> random_color{{0, 0, 0}};
	std::mt19937 gen32{ static_cast<unsigned int>(time(nullptr)) }; // NOLINT
	std::uniform_real_distribution<float> fdist(0.F, 1.F);
	std::size_t i{0};
	for (auto &vertex : vertices) {
		if (i % 6 == 0) {
			random_color[0] = fdist(gen32);
			random_color[1] = fdist(gen32);
			random_color[2] = fdist(gen32);
		}

		const auto VERTEX = VERTICES.at(static_cast<std::size_t>(INDICES.at(i++)));
		vertex[0] = VERTEX[0];
		vertex[1] = VERTEX[1];
		vertex[2] = VERTEX[2];
		vertex[3] = random_color[0];
		vertex[4] = random_color[1];
		vertex[5] = random_color[2];
	}
	if (!is_instanced) {
		glNamedBufferStorage(vbo_id_, sizeof(vertices), vertices.data(), 0);
	} else {
		using byte = unsigned char;
		byte* tmp_data = new byte[sizeof(vertices) * instance_data_size];
		memcpy(tmp_data, vertices.data(), sizeof(vertices));
		memcpy(tmp_data + sizeof(vertices), instance_data, instance_data_size);

		glNamedBufferStorage(vbo_id_, sizeof(vertices) + instance_data_size, tmp_data, 0);

		delete[] tmp_data;
	}

	glCreateVertexArrays(1, &vao_id_);

	std::int32_t offset{ 0 };
	for (const auto attrib_config : attrib_configs) {
		glEnableVertexArrayAttrib(vao_id_, attrib_config.index);
		glVertexArrayAttribFormat(vao_id_, attrib_config.index,
								  attrib_config.size_in_dwords, GL_FLOAT, GL_FALSE,
								  offset);
		glVertexArrayAttribBinding(vao_id_, attrib_config.index,
								   attrib_binding);
		offset += attrib_config.size_in_dwords * 4;
	}
	glVertexArrayBindingDivisor(vao_id_, attrib_binding, 0);
	glVertexArrayVertexBuffer(vao_id_, attrib_binding, vbo_id_, 0, offset);

	if (is_instanced) {
		std::uint32_t instance_offset{ 0 };
		for (const auto instance_attrib_config : instance_attrib_configs.value()) {
			glEnableVertexArrayAttrib(vao_id_, instance_attrib_config.index);
			glVertexArrayAttribFormat(vao_id_, instance_attrib_config.index, instance_attrib_config.size_in_dwords,
									  GL_FLOAT, GL_FALSE, instance_offset);
			glVertexArrayAttribBinding(vao_id_, instance_attrib_config.index, instance_attrib_binding.value());
			instance_offset += instance_attrib_config.size_in_dwords * 4;
		}
		glVertexArrayBindingDivisor(vao_id_, instance_attrib_binding.value(), 1);
		glVertexArrayVertexBuffer(vao_id_, instance_attrib_binding.value(), vbo_id_, INDICES.size() * offset, instance_offset);
		instance_count_ = instance_data_size / instance_offset;
	}
}

std::tuple<float, float, float> Cube::rotate(float x, float y, float z) {
	rot_xyz_[0] = wrap(rot_xyz_[0] + x, 2.F * std::numbers::pi_v<float>, 0.F);
	rot_xyz_[1] = wrap(rot_xyz_[1] + y, 2.F * std::numbers::pi_v<float>, 0.F);
	rot_xyz_[2] = wrap(rot_xyz_[2] + z, 2.F * std::numbers::pi_v<float>, 0.F);
	return std::make_tuple(rot_xyz_[0], rot_xyz_[1], rot_xyz_[2]);
}
std::tuple<float, float, float> Cube::move(float x, float y, float z) {
	pos_xyz_[0] += x;
	pos_xyz_[1] += y;
	pos_xyz_[2] += z;
	return std::make_tuple(pos_xyz_[0], pos_xyz_[1], pos_xyz_[2]);
}

void Cube::draw() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArraysInstanced(GL_TRIANGLES, 0, INDICES.size(), instance_count_);
}
void Cube::bind() const {
	glBindVertexArray(vao_id_);
}
void Cube::deinit() {
	glDeleteBuffers(1, &vbo_id_);
	glDeleteVertexArrays(1, &vao_id_);
}