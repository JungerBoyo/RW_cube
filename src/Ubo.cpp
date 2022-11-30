#include "Ubo.hpp"

#include <glad/glad.h>

using namespace rw_cube;

UBO::UBO(std::uint32_t binding_location, std::int32_t size) : size_(size) {
	glCreateBuffers(1, &ubo_id_);
	glNamedBufferStorage(ubo_id_, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_location, ubo_id_);
}
void UBO::sendData(const void *data) const {
	glNamedBufferSubData(ubo_id_, 0, size_, data);
}
void UBO::sendData(const void *data, std::int32_t offset, std::int32_t size) const {
	glNamedBufferSubData(ubo_id_, offset, size > size_ ? size_ : size, data);
}
void UBO::deinit() {
	glDeleteBuffers(1, &ubo_id_);
}