#include "muon/core/window.hpp"

#include "GLFW/glfw3.h"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

namespace muon {

    Window::Window(const WindowSpecification &props) {
        m_data.dispatcher = props.dispatcher;
        m_data.title = props.title;

        glfwSetErrorCallback([](int32_t code, const char *message) {
            MU_CORE_ERROR(message);
        });

        auto initialized = glfwInit();
        MU_CORE_ASSERT(initialized, "GLFW must be initialised");

        auto vkSupported = glfwVulkanSupported();
        MU_CORE_ASSERT(vkSupported, "GLFW must support Vulkan");

        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        m_data.refreshRate = mode->refreshRate;
        if (props.width == std::numeric_limits<uint32_t>().max() || props.height == std::numeric_limits<uint32_t>().max()) {
            m_data.width = (mode->width * 0.75);
            m_data.height = (mode->height * 0.75);
        } else {
            m_data.width = props.width;
            m_data.height = props.height;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
        MU_CORE_ASSERT(m_window, "window must exist");

        glfwSetWindowUserPointer(m_window, &m_data);

        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, true);
            m_data.rawMouseMotion = true;
        }

        ConfigureDispatchers();

        MU_CORE_DEBUG("created window with dimensions: {}x{}", m_data.width, m_data.height);
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        MU_CORE_DEBUG("destroyed window");
    }

    void Window::PollEvents() const {
        glfwPollEvents();
    }

    VkResult Window::CreateSurface(VkInstance instance, VkSurfaceKHR *surface) const {
        return glfwCreateWindowSurface(instance, m_window, nullptr, surface);
    }

    GLFWwindow *Window::Get() const {
        return m_window;
    }

    VkExtent2D Window::GetExtent() const {
        return {
            m_data.width,
            m_data.height
        };
    }

    uint32_t Window::GetWidth() const {
        return m_data.width;
    }

    uint32_t Window::GetHeight() const {
        return m_data.height;
    }

    uint32_t Window::GetRefreshRate() const {
        return m_data.refreshRate;
    }

    const char *Window::GetClipboardContents() const {
        return glfwGetClipboardString(m_window);
    }

    std::vector<const char *> Window::GetRequiredExtensions() const {
        uint32_t count = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + count);
        return extensions;
    }

    void Window::ConfigureDispatchers() {
        /* window events */
        glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window) {
            const auto &data = static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data->dispatcher->Dispatch<event::WindowCloseEvent>({});
        });

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, int width, int height) {
            const auto &data = static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data->dispatcher->Dispatch<event::WindowResizeEvent>({
                .width = static_cast<uint32_t>(width),
                .height = static_cast<uint32_t>(height),
            });
        });

        /* keyboard events */
        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
            const auto &data = static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data->dispatcher->Dispatch<event::KeyEvent>({
                .key = key,
                .scancode = scancode,
                .action = action,
                .mods = mods,
            });
        });

        /* mouse events */
        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int32_t button, int32_t action, int32_t mods) {
            const auto &data = static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data->dispatcher->Dispatch<event::MouseButtonEvent>({
                .button = button,
                .action = action,
                .mods = mods,
            });
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y) {
            const auto &data = static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data->dispatcher->Dispatch<event::CursorPositionEvent>({
                .x = x,
                .y = y,
            });
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xOffset, double yOffset) {
            const auto &data = static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data->dispatcher->Dispatch<event::MouseScrollEvent>({
                .xOffset = xOffset,
                .yOffset = yOffset,
            });
        });

    }

}
