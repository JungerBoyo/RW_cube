#include "Cube.hpp"

#include <glad/glad.h>

using namespace rw_cube;

Cube::Cube(std::uint32_t attrib_binding,
		   std::span<const AttribConfig> attrib_configs) {
	glCreateBuffers(1, &vbo_id_);

	std::array<Vertex, INDICES.size()> vertices;
	std::size_t i{0};
	for (auto &vertex : vertices) {
		vertex = VERTICES.at(static_cast<std::size_t>(INDICES.at(i++)));
	}
	glNamedBufferStorage(vbo_id_, sizeof(vertices), vertices.data(), 0);

	glCreateVertexArrays(1, &vao_id_);

	std::int32_t offset{0};
	for (const auto attrib_config : attrib_configs) {
		glEnableVertexArrayAttrib(vao_id_, attrib_config.index);
		glVertexArrayAttribFormat(vao_id_, attrib_config.index,
								  attrib_config.size / 4, GL_FLOAT, GL_FALSE,
								  offset);
		glVertexArrayAttribBinding(vao_id_, attrib_config.index,
								   attrib_binding);
		offset += attrib_config.size;
	}
	glVertexArrayBindingDivisor(vao_id_, attrib_binding, 0);
	glVertexArrayVertexBuffer(vao_id_, attrib_binding, vbo_id_, 0,
							  sizeof(Vertex));
}
void Cube::draw() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, INDICES.size());
}
void Cube::bind() const {
	glBindVertexArray(vao_id_);
}
void Cube::deinit() {
	glDeleteBuffers(1, &vbo_id_);
	glDeleteVertexArrays(1, &vao_id_);
}