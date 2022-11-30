#include "Shader.hpp"

#include <exception>
#include <fstream>

#include <fmt/format.h>
#include <glad/glad.h>

using namespace rw_cube;

enum : std::size_t { VERTEX_SHADER, FRAGMENT_SHADER };

Shader::Shader(bool is_spirv, const std::vector<std::filesystem::path>& paths) {
	prog_id_ = glCreateProgram();

	shader_ids_.at(VERTEX_SHADER) = glCreateShader(GL_VERTEX_SHADER);
	shader_ids_.at(FRAGMENT_SHADER) = glCreateShader(GL_FRAGMENT_SHADER);

	for (std::size_t i{0}; i < 2; ++i) {
		const auto sh_id = shader_ids_.at(i);
		if (is_spirv) {
			const auto sh_binary = parseAsSpirv(paths[i]);
			glShaderBinary(1, static_cast<const GLuint *>(&sh_id),
						GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
						static_cast<const void *>(sh_binary.data()),
						static_cast<GLsizei>(sh_binary.size()));
			glSpecializeShaderARB(sh_id, "main", 0, nullptr, nullptr);
		} else {
			compileShader(paths[i], sh_id);
		}
		glAttachShader(prog_id_, sh_id);
	}

	glValidateProgram(prog_id_);
	glLinkProgram(prog_id_);
}

std::vector<char> Shader::parseAsSpirv(const std::filesystem::path &path) {
	std::ifstream stream(path.c_str(), std::ios::binary | std::ios::ate);

	if (!stream.good()) {
		throw std::runtime_error(fmt::format(
			"failed to create stream from shader file {}", path.string()));
	}

	const auto size = static_cast<std::size_t>(stream.tellg());
	std::vector<char> code(size);

	stream.seekg(0);
	stream.read(code.data(), static_cast<std::streamsize>(size));

	stream.close();

	return code;
}

void Shader::compileShader(const std::filesystem::path &path, std::uint32_t shader_id) {
	std::ifstream stream(path.c_str());
	std::stringstream sstream;
	for (std::string line; std::getline(stream, line);) {
		sstream << line << '\n';
	}
	const auto shader_src = sstream.str();

	const char* shader_src_cstr = shader_src.c_str();
	const auto len = static_cast<std::int32_t>(shader_src.length());

	glShaderSource(shader_id, 1, &shader_src_cstr, &len);
	glCompileShader(shader_id);
}
void Shader::bind() const {
	glUseProgram(prog_id_);
}

void Shader::deinit() {
	for (std::size_t i{0}; i < 2; ++i) {
		auto &sh_id = shader_ids_.at(i);
		glDetachShader(prog_id_, sh_id);
		glDeleteShader(sh_id);
		sh_id = 0;
	}
	glDeleteProgram(prog_id_);
	prog_id_ = 0;
}