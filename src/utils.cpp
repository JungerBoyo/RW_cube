#include <utils.hpp>

#include <glad/glad.h>

using namespace rw_cube;

void rw_cube::setVertexArrayLayout(
    std::uint32_t vao_id,
    std::uint32_t vbo_id,
    std::uint32_t attrib_binding,
    const std::vector<AttribConfig>& attrib_configs
) {
	std::int32_t offset{ 0 };
	for (const auto attrib_config : attrib_configs) {
		glEnableVertexArrayAttrib(vao_id, attrib_config.index);
		glVertexArrayAttribFormat(vao_id, attrib_config.index,
								  attrib_config.size_in_dwords, GL_FLOAT, GL_FALSE,
								  offset);
		glVertexArrayAttribBinding(vao_id, attrib_config.index,
								   attrib_binding);
		offset += attrib_config.size_in_dwords * 4;
	}
	glVertexArrayBindingDivisor(vao_id, attrib_binding, 0);
	glVertexArrayVertexBuffer(vao_id, attrib_binding, vbo_id, 0, offset);
} 