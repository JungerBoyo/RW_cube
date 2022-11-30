#ifndef RW_CUBE_CUBE_TEXTURE_HPP
#define RW_CUBE_CUBE_TEXTURE_HPP

#include <filesystem>
#include <vector>
#include <array>

namespace rw_cube {

struct CubeTexture {
    std::array<std::vector<unsigned char>, 6> face_textures;      

    std::int32_t width;
    std::int32_t height;

    explicit CubeTexture(const std::filesystem::path& tex_path);
};

}

#endif