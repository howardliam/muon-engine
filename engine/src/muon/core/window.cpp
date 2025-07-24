#include "muon/core/window.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/input/input_state.hpp"
#include "muon/input/key_code.hpp"
#include "muon/input/mouse.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace muon {

struct Window::WindowHandle {
    GLFWwindow *window;
};

Window::Window(const Spec &spec) : m_handle(std::make_unique<WindowHandle>()) {
    m_data.dispatcher = spec.dispatcher;
    m_data.title = spec.title;

    glfwSetErrorCallback([](int32_t code, const char *message) { MU_CORE_ERROR(message); });

    auto initialized = glfwInit();
    MU_CORE_ASSERT(initialized, "GLFW must be initialised");

    auto vkSupported = glfwVulkanSupported();
    MU_CORE_ASSERT(vkSupported, "GLFW must support Vulkan");

    const auto *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    m_data.refreshRate = mode->refreshRate;
    if (spec.width == std::numeric_limits<uint32_t>().max() || spec.height == std::numeric_limits<uint32_t>().max()) {
        m_data.width = (mode->width * 0.75);
        m_data.height = (mode->height * 0.75);
    } else {
        m_data.width = spec.width;
        m_data.height = spec.height;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_handle->window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
    MU_CORE_ASSERT(m_handle->window, "window must exist");
    glfwSetWindowSizeLimits(m_handle->window, 1280, 720, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetWindowUserPointer(m_handle->window, &m_data);

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_handle->window, GLFW_RAW_MOUSE_MOTION, true);
        m_data.rawMouseMotion = true;
    }

    ConfigureDispatchers();

    MU_CORE_DEBUG("created window with dimensions: {}x{}", m_data.width, m_data.height);
}

Window::~Window() {
    glfwDestroyWindow(m_handle->window);
    glfwTerminate();
    MU_CORE_DEBUG("destroyed window");
}

auto Window::PollEvents() const -> void { glfwPollEvents(); }

auto Window::RequestAttention() const -> void { glfwRequestWindowAttention(m_handle->window); }

auto Window::CreateSurface(const vk::raii::Instance &instance) const -> std::expected<vk::raii::SurfaceKHR, vk::Result> {
    VkSurfaceKHR rawSurface;

    auto result = static_cast<vk::Result>(glfwCreateWindowSurface(*instance, m_handle->window, nullptr, &rawSurface));

    if (result != vk::Result::eSuccess) {
        return std::unexpected(result);
    }

    return vk::raii::SurfaceKHR{instance, rawSurface};
}

auto Window::GetClipboardContents() const -> const char * { return glfwGetClipboardString(m_handle->window); }

auto Window::GetExtent() const -> VkExtent2D { return {m_data.width, m_data.height}; }

auto Window::GetWidth() const -> uint32_t { return m_data.width; }

auto Window::GetHeight() const -> uint32_t { return m_data.height; }

auto Window::GetRefreshRate() const -> uint32_t { return m_data.refreshRate; }

auto Window::GetRequiredExtensions() const -> std::vector<const char *> {
    uint32_t count = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + count);
    return extensions;
}

auto Window::ConfigureDispatchers() -> void {
    /* window events */
    glfwSetWindowCloseCallback(m_handle->window, [](GLFWwindow *window) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::WindowCloseEvent>({});
    });

    glfwSetWindowSizeCallback(m_handle->window, [](GLFWwindow *window, int width, int height) {
        auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

        data.width = static_cast<uint32_t>(width);
        data.height = static_cast<uint32_t>(height);

        data.dispatcher->Dispatch<event::WindowResizeEvent>({
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
        });
    });

    glfwSetWindowFocusCallback(m_handle->window, [](GLFWwindow *window, int32_t focused) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::WindowFocusEvent>({
            .focused = focused > 0,
        });
    });

    /* keyboard events */
    glfwSetKeyCallback(m_handle->window, [](GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::KeyEvent>({
            .keycode = input::KeyCode(key),
            .scancode = input::KeyCode(scancode),
            .inputState = input::InputState(action),
            .mods = input::Modifier(mods),
        });
    });

    /* mouse events */
    glfwSetMouseButtonCallback(m_handle->window, [](GLFWwindow *window, int32_t button, int32_t action, int32_t mods) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::MouseButtonEvent>({
            .button = input::MouseButton(button),
            .inputState = input::InputState(action),
            .mods = input::Modifier(mods),
        });
    });

    glfwSetCursorPosCallback(m_handle->window, [](GLFWwindow *window, double x, double y) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::CursorPositionEvent>({
            .x = x,
            .y = y,
        });
    });

    glfwSetCursorEnterCallback(m_handle->window, [](GLFWwindow *window, int32_t entered) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::CursorEnterEvent>({
            .entered = entered > 0,
        });
    });

    glfwSetScrollCallback(m_handle->window, [](GLFWwindow *window, double xOffset, double yOffset) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::MouseScrollEvent>({
            .xOffset = xOffset,
            .yOffset = yOffset,
        });
    });

    /* misc events */
    glfwSetDropCallback(m_handle->window, [](GLFWwindow *window, int32_t pathCount, const char **paths) {
        const auto &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.dispatcher->Dispatch<event::FileDropEvent>({
            .paths = std::vector<const char *>(paths, paths + pathCount),
        });
    });
}

} // namespace muon
