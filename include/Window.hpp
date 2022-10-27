#ifndef RW_CUBE_WINDOW_HPP
#define RW_CUBE_WINDOW_HPP

#include <string_view>
#include <memory>

namespace rw_cube {

struct Window {
    struct WinNative;
    std::shared_ptr<WinNative> win_handle_;

    Window(std::string_view title, int w, int h,   
        void(*win_error_callback)(int, const char*),
        void(*gl_error_callback)(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, int, const char*, const void*));

    [[nodiscard]] std::pair<int, int> size() const;
    [[nodiscard]] float time() const;
    void setViewport(int w, int h) const;
    void swapBuffers() const;
    bool shouldClose() const;
    void pollEvents() const;

    void deinit();
};

}

#endif