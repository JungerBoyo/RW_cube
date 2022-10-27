#ifndef RW_CUBE_UBO_HPP
#define RW_CUBE_UBO_HPP

#include <cinttypes>

namespace rw_cube {

struct UBO {
	std::uint32_t ubo_id_;
	std::int32_t size_;

	UBO(std::uint32_t binding_location, std::int32_t size);
	void sendData(const void *data) const;
	void deinit();
};

} // namespace rw_cube

#endif