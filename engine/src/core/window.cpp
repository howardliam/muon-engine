#include "muon/core/window.hpp"

#include "GLFW/glfw3.h"
#include "muon/event/event.hpp"
#include "muon/event/data.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

namespace muon {

    Window::Window(const WindowProperties &props, event::EventDispatcher *dispatcher) {
        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;
        m_data.dispatcher = dispatcher;

        Init();
        ConfigureDispatchers();
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void Window::PollEvents() const {
        glfwPollEvents();
    }

    VkResult Window::CreateSurface(VkInstance instance, VkSurfaceKHR *surface) {
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

    const char *Window::GetClipboardContents() const {
        return glfwGetClipboardString(m_window);
    }

    std::vector<const char *> Window::GetRequiredExtensions() const {
        uint32_t count = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + count);
        return extensions;
    }

    void Window::Init() {
        glfwSetErrorCallback([](int32_t code, const char *message) {
            MU_CORE_ERROR(message);
        });

        auto initialized = glfwInit();
        MU_CORE_ASSERT(initialized == GLFW_TRUE, "GLFW must be initialised");

        auto vkSupported = glfwVulkanSupported();
        MU_CORE_ASSERT(vkSupported == GLFW_TRUE, "GLFW must support Vulkan");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
        MU_CORE_ASSERT(m_window, "window must exist");

        glfwSetWindowUserPointer(m_window, &m_data);

        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            m_data.rawMouseMotion = true;
        } else {
            m_data.rawMouseMotion = false;
        }
    }

    void Window::ConfigureDispatchers() {

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(event::Event{
                .type = event::EventType::WindowClose,
                .data = event::WindowCloseData {}
            });
        });

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, int width, int height) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(event::Event{
                .type = event::EventType::WindowResize,
                .data = event::WindowResizeData {
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height),
                }
            });
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xOffset, double yOffset) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(event::Event{
                .type = event::EventType::MouseScroll,
                .data = event::MouseScrollData {
                    .xOffset = xOffset,
                    .yOffset = yOffset,
                }
            });
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(event::Event{
                .type = event::EventType::Key,
                .data = event::KeyData {
                    .key = key,
                    .scancode = scancode,
                    .action = action,
                    .mods = mods,
                }
            });
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int32_t button, int32_t action, int32_t mods) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(event::Event{
                .type = event::EventType::MouseButton,
                .data = event::MouseButtonData {
                    .button = button,
                    .action = action,
                    .mods = mods,
                }
            });
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(event::Event{
                .type = event::EventType::CursorPosition,
                .data = event::CursorPositionData {
                    .x = x,
                    .y = y,
                }
            });
        });
    }

}
