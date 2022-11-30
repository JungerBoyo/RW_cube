#include "CubeTexture.hpp"
#include <lodepng.h>
#include <fmt/core.h>

using namespace rw_cube;

CubeTexture::CubeTexture(const std::filesystem::path& tex_path) {
    std::vector<unsigned char> img;
    std::uint32_t width{0U};
    std::uint32_t height{0U};

    lodepng::decode(img, width, height, tex_path.string());

    constexpr std::uint32_t channel_num{ 4 };
    const auto stride = width * channel_num;

    const auto face_stride = width;
    const auto face_width = width/channel_num;
    const auto face_height = height/3;

    std::array<std::uint32_t, 6> face_offsets {{
        face_height * stride + 2*face_stride, // +x
        face_height * stride,                 // -x
        face_stride,                          // +y
        2*face_height * stride + face_stride, // -y
        face_height * stride + face_stride,   // +z
        face_height * stride + 3*face_stride, // -z
    }};

    const auto face_capacity = face_stride * face_height;

    std::size_t i{0};
    for (const auto face_offset : face_offsets) {
        auto& face_texture = face_textures[i];
        face_texture.resize(face_stride * face_height);

        std::size_t h_i{0};
        while (h_i < face_height) {
            std::copy(
                std::next(img.cbegin(), face_offset + h_i * stride),
                std::next(img.cbegin(), face_offset + h_i * stride + face_stride),
                std::next(face_texture.begin(), (face_height - 1 - h_i) * face_stride)
            );
            ++h_i;
        }

        ++i;
    }

    this->width = static_cast<std::int32_t>(face_width);
    this->height = static_cast<std::int32_t>(face_height);
}