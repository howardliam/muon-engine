#include "muon/core/window.hpp"

#include "GLFW/glfw3.h"
#include "muon/core/event/event.hpp"
#include "muon/core/event/data.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/input.hpp"
#include "muon/core/log.hpp"

namespace muon {

    Window::Window(const WindowProperties &props, EventDispatcher *dispatcher) {
        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;
        m_data.dispatcher = dispatcher;

        Init();
        ConfigureDispatcher();
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);

        glfwTerminate();
    }

    void Window::PollEvents() const {
        glfwPollEvents();
    }

    vk::Result Window::CreateSurface(vk::Instance instance, vk::SurfaceKHR *surface) {
        auto result = glfwCreateWindowSurface(
            static_cast<VkInstance>(instance),
            m_window,
            nullptr,
            reinterpret_cast<VkSurfaceKHR *>(surface)
        );
        return static_cast<vk::Result>(result);
    }

    std::vector<const char *> Window::RequiredExtensions() const {
        uint32_t count{0};
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + count);
        return extensions;
    }

    void *Window::Get() const {
        return m_window;
    }

    const char *Window::ClipboardContents() const {
        return glfwGetClipboardString(m_window);
    }

    vk::Extent2D Window::Extent() const {
        return {
            m_data.width,
            m_data.height
        };
    }

    uint32_t Window::Width() const {
        return m_data.width;
    }

    uint32_t Window::Height() const {
        return m_data.height;
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

    void Window::ConfigureDispatcher() {
        glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(Event{
                .type = EventType::WindowClose,
                .data = WindowCloseEventData {}
            });
        });

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, int width, int height) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(Event{
                .type = EventType::WindowResize,
                .data = WindowResizeEventData {
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height),
                }
            });
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xOffset, double yOffset) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(Event{
                .type = EventType::MouseScroll,
                .data = MouseScrollEventData {
                    .xOffset = xOffset,
                    .yOffset = yOffset,
                }
            });
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(Event{
                .type = EventType::Key,
                .data = KeyEventData {
                    .key = key,
                    .scancode = scancode,
                    .action = static_cast<Action>(action),
                    .mods = mods,
                }
            });
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int32_t button, int32_t action, int32_t mods) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(Event{
                .type = EventType::MouseButton,
                .data = MouseButtonEventData {
                    .button = button,
                    .action = static_cast<Action>(action),
                    .mods = mods,
                }
            });
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y) {
            WindowData &data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
            data.dispatcher->dispatch(Event{
                .type = EventType::CursorPosition,
                .data = CursorPositionEventData {
                    .x = x,
                    .y = y,
                }
            });
        });
    }

}
