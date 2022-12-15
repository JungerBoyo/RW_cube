#ifndef RW_CUBE_UTILS_HPP
#define RW_CUBE_UTILS_HPP

#include <cinttypes>
#include <vector>
#include <array>

namespace rw_cube {

struct AttribConfig {
	std::int32_t index;
	std::int32_t size_in_dwords;
};

void setVertexArrayLayout(
    std::uint32_t vao_id,
    std::uint32_t vbo_id,
    std::uint32_t attrib_binding,
    const std::vector<AttribConfig>& attrib_configs
);

}

#endif