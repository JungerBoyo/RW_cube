#include "Cube.hpp"
#include "CubeTexture.hpp"

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
	const std::vector<AttribConfig>& attrib_configs,
	const std::vector<Shader>& shaders,
	const std::vector<std::array<float, 3>>& offsets,
	const std::filesystem::path& tex_path
) : shaders(shaders), offsets(offsets) {
	cube_count = static_cast<std::uint32_t>(std::min(this->shaders.size(), this->offsets.size()));

	glCreateBuffers(1, &vbo_id_);

	using Vertex = std::array<float, 3 + 3 + 3>; // position + texcoord + normal
	std::array<Vertex, INDICES.size()> vertices;
	
	std::size_t i{0};
	for (auto &vertex : vertices) {
		const auto base_vertex_index = static_cast<std::size_t>(INDICES.at(i)) * 3;
		// set position
		vertex[0] = VERTICES[base_vertex_index + 0];
		vertex[1] = VERTICES[base_vertex_index + 1];
		vertex[2] = VERTICES[base_vertex_index + 2];

		const auto in_side_texcoord_i = 2*(i % 6);
		vertex[3] = TEX_COORDS[in_side_texcoord_i + 0];
		vertex[4] = TEX_COORDS[in_side_texcoord_i + 1];
		const auto face_index = i / 6;
		vertex[5] = static_cast<float>(face_index);

		vertex[6] = NORMALS[face_index * 3 + 0];
		vertex[7] = NORMALS[face_index * 3 + 1];
		vertex[8] = NORMALS[face_index * 3 + 2];
		++i;
	}
	glNamedBufferStorage(vbo_id_, sizeof(vertices), vertices.data(), 0);

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

	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &tex_id_);
	CubeTexture texture(tex_path);

	const auto num_levels = static_cast<int>(std::ceil(std::log2(std::min(
		static_cast<float>(texture.width), 
		static_cast<float>(texture.height)
	))));
	glTextureStorage3D(tex_id_, num_levels, GL_RGBA8, texture.width, texture.height, 6);
    glTextureParameteri(tex_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(tex_id_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTextureParameteri(tex_id_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	i = 0;
	for (const auto& face_texture : texture.face_textures) {
		glTextureSubImage3D(
			tex_id_, 0, 0, 0, static_cast<int>(i),
			texture.width, texture.height, 1,
			GL_RGBA, GL_UNSIGNED_BYTE,
			static_cast<const void*>(face_texture.data())
		);
		++i;
	}
	glGenerateTextureMipmap(tex_id_);
	glBindTextureUnit(SHCONFIG_2D_TEX_ARRAY_BINDING, tex_id_);
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
	glDrawArrays(GL_TRIANGLES, 0, INDICES.size());
}
void Cube::bind() const {
	glBindVertexArray(vao_id_);
}
void Cube::deinit() {
	for (auto shader : shaders) {
		shader.deinit();
	}
	glDeleteBuffers(1, &vbo_id_);
	glDeleteVertexArrays(1, &vao_id_);
}