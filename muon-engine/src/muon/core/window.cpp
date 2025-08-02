#include "muon/core/window.hpp"

#include "GLFW/glfw3.h"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/input/input_state.hpp"
#include "muon/input/key_code.hpp"
#include "muon/input/mouse.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace muon {

Window::Window(
    const std::string_view title,
    const vk::Extent2D &extent,
    const WindowMode mode,
    const event::Dispatcher &dispatcher
) : m_mode{mode}, m_data{dispatcher} {
    m_data.title = title;
    m_data.extent = extent;

    glfwSetErrorCallback([](int32_t, const char *message) { core::error(message); });

    auto initialized = glfwInit();
    core::expect(initialized, "GLFW must be initialised");

    auto vkSupported = glfwVulkanSupported();
    core::expect(vkSupported, "GLFW must support Vulkan");

    m_monitor = glfwGetPrimaryMonitor();
    const auto *videoMode = glfwGetVideoMode(m_monitor);
    m_data.refreshRate = videoMode->refreshRate;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(m_data.extent.width, m_data.extent.height, m_data.title.c_str(), nullptr, nullptr);
    core::expect(m_window, "window must exist");

    glfwSetWindowSizeLimits(m_window, 1280, 720, GLFW_DONT_CARE, GLFW_DONT_CARE);

    setMode(m_mode);

    glfwSetWindowUserPointer(m_window, &m_data);

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, true);
        m_data.rawMouseMotion = true;
    }

    configureDispatchers();

    core::debug("created window with dimensions: {}x{}", m_data.extent.width, m_data.extent.height);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
    core::debug("destroyed window");
}

void Window::pollEvents() const { glfwPollEvents(); }

void Window::requestAttention() const { glfwRequestWindowAttention(m_window); }

auto Window::createSurface(const vk::raii::Instance &instance) const -> std::expected<vk::raii::SurfaceKHR, vk::Result> {
    VkSurfaceKHR rawSurface;

    auto result = static_cast<vk::Result>(glfwCreateWindowSurface(*instance, m_window, nullptr, &rawSurface));

    if (result != vk::Result::eSuccess) {
        return std::unexpected(result);
    }

    return vk::raii::SurfaceKHR{instance, rawSurface};
}

void Window::setMode(WindowMode mode) {
    GLFWmonitor *monitor = nullptr;

    switch (mode) {
        case WindowMode::Windowed:
            glfwWindowHint(GLFW_DECORATED, true);
            break;

        case WindowMode::Fullscreen:
            monitor = m_monitor;
            break;

        case WindowMode::BorderlessFullscreen:
            const auto *mode = glfwGetVideoMode(m_monitor);
            m_data.extent.width = mode->width;
            m_data.extent.height = mode->height;
            glfwWindowHint(GLFW_DECORATED, false);
            break;
    }

    glfwSetWindowMonitor(m_window, monitor, 0, 0, m_data.extent.width, m_data.extent.height, m_data.refreshRate);
}

auto Window::getExtent() const -> VkExtent2D { return m_data.extent; }
auto Window::getWidth() const -> uint32_t { return m_data.extent.width; }
auto Window::getHeight() const -> uint32_t { return m_data.extent.height; }
auto Window::getRefreshRate() const -> uint32_t { return m_data.refreshRate; }

auto Window::getRequiredExtensions() const -> std::vector<const char *> {
    uint32_t count = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + count);
    return extensions;
}

auto Window::getClipboardContents() const -> const char * { return glfwGetClipboardString(m_window); }

void Window::configureDispatchers() {
    /* window events */
    glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::WindowCloseEvent>({});
    });

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, int width, int height) {
        auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));

        data.extent.width = static_cast<uint32_t>(width);
        data.extent.height = static_cast<uint32_t>(height);

        data.dispatcher.dispatch<event::WindowResizeEvent>({
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
        });
    });

    glfwSetWindowFocusCallback(m_window, [](GLFWwindow *window, int32_t focused) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::WindowFocusEvent>({
            .focused = focused > 0,
        });
    });

    /* keyboard events */
    glfwSetKeyCallback(m_window, [](GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::KeyEvent>({
            .keycode = input::KeyCode(key),
            .scancode = input::KeyCode(scancode),
            .inputState = input::InputState(action),
            .mods = input::Modifier(mods),
        });
    });

    /* mouse events */
    glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int32_t button, int32_t action, int32_t mods) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::MouseButtonEvent>({
            .button = input::MouseButton(button),
            .inputState = input::InputState(action),
            .mods = input::Modifier(mods),
        });
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::CursorPositionEvent>({
            .x = x,
            .y = y,
        });
    });

    glfwSetCursorEnterCallback(m_window, [](GLFWwindow *window, int32_t entered) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::CursorEnterEvent>({
            .entered = entered > 0,
        });
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xOffset, double yOffset) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::MouseScrollEvent>({
            .xOffset = xOffset,
            .yOffset = yOffset,
        });
    });

    /* misc events */
    glfwSetDropCallback(m_window, [](GLFWwindow *window, int32_t pathCount, const char **paths) {
        const auto &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
        data.dispatcher.dispatch<event::FileDropEvent>({
            .paths = std::vector<const char *>(paths, paths + pathCount),
        });
    });
}

} // namespace muon
