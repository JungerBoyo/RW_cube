#ifndef RW_CUBE_CUBE_HPP
#define RW_CUBE_CUBE_HPP

#include <array>
#include <span>
#include <numbers>
#include <algorithm>

#include <Shader.hpp>

namespace rw_cube {

struct AttribConfig {
    std::int32_t index;
    std::int32_t size;
};

struct Cube {
    #include "Vertices.hpp"
    #include "Indices.hpp"

    std::uint32_t vbo_id_;
    std::uint32_t vao_id_;
    std::array<float, 3> rot_xyz_{{0.F, 0.F, 0.F}};

    Cube(std::uint32_t attrib_binding, std::span<const AttribConfig> attrib_configs);
    void draw() const;
    auto rotate(float x, float y, float z) {
        rot_xyz_[0] = rot_xyz_[0] + x > 2.F * std::numbers::pi_v<float> ? 0.F : rot_xyz_[0] + x;
        rot_xyz_[1] = rot_xyz_[1] + y > 2.F * std::numbers::pi_v<float> ? 0.F : rot_xyz_[1] + y;
        rot_xyz_[2] = rot_xyz_[2] + z > 2.F * std::numbers::pi_v<float> ? 0.F : rot_xyz_[2] + z;
        return std::make_tuple(rot_xyz_[0], rot_xyz_[1], rot_xyz_[2]);
    }
    void bind() const;
    void deinit();
};

}

#endif