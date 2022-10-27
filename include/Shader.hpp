#ifndef RW_CUBE_SHADER_HPP
#define RW_CUBE_SHADER_HPP

#include <cinttypes>
#include <array>
#include <vector>
#include <span>
#include <string>
#include <string_view>
#include <filesystem>

namespace rw_cube {

struct Shader {
	std::uint32_t prog_id_;
	std::array<std::uint32_t, 2> shader_ids_;

	Shader(std::span<const std::filesystem::path> paths);
	static std::vector<char> parse(const std::filesystem::path& path);

	void bind() const;

	void deinit();
};

}

#endif