#ifndef RW_CUBE_SHADER_HPP
#define RW_CUBE_SHADER_HPP

#include <array>
#include <cinttypes>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace rw_cube {

struct Shader {
	std::uint32_t prog_id_;
	std::array<std::uint32_t, 2> shader_ids_;

	Shader(std::span<const std::filesystem::path> paths);
	static std::vector<char> parse(const std::filesystem::path &path);

	void bind() const;

	void deinit();
};

} // namespace rw_cube

#endif