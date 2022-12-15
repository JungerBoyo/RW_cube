#ifndef RW_CUBE_MODEL_HPP
#define RW_CUBE_MODEL_HPP

#include <array>
#include <filesystem>

#include <Shader.hpp>

namespace rw_cube {

struct Model {
	struct Material {
		float ambient[3]{0.F, 0.F, 0.F};
		float shininess{ 0.F };
		float diffuse[3]{0.F, 0.F, 0.F};
		float alpha{ 1.F };
		alignas(16) float specular[3]{0.F, 0.F, 0.F};
	};

	std::uint32_t vbo_id_{ 0 };
	std::uint32_t vao_id_{ 0 };
    std::uint32_t ebo_id_{ 0 };
	std::uint32_t tex_id_{ 0 };

	Material model_material_{};

	std::uint32_t indices_count_{ 0 };
	
	Shader model_shader_;

    Model(bool is_spirv, const std::filesystem::path& obj_path, const std::filesystem::path& tex_path);
	void draw() const;
	void bind() const;
	void deinit();
};

}

#endif