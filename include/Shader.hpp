#ifndef RW_CUBE_SHADER_HPP
#define RW_CUBE_SHADER_HPP

#include <array>
#include <cinttypes>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace rw_cube {

struct Shader {
	std::uint32_t prog_id_;
	std::array<std::uint32_t, 2> shader_ids_;

	Shader(bool is_spirv, const std::vector<std::filesystem::path>& paths);
	static std::vector<char> parseAsSpirv(const std::filesystem::path &path);
	static void compileShader(const std::filesystem::path &path, std::uint32_t shader_id);

	void bind() const;

	void deinit();
};

} // namespace rw_cube

#endif