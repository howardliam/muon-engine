#include "muon/core/window.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace muon {

    struct Window::Impl {
        GLFWwindow *window;
    };

    Window::Window(const Properties &props) {
        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;

        glfwSetErrorCallback([](int32_t code, const char *message) {
            MU_CORE_ERROR(message);
        });

        auto initialized = glfwInit();
        MU_CORE_ASSERT(initialized == GLFW_TRUE, "GLFW must be initialised");

        auto vkSupported = glfwVulkanSupported();
        MU_CORE_ASSERT(vkSupported == GLFW_TRUE, "GLFW must support Vulkan");

        m_handle = std::make_unique<Impl>();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_handle->window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
        MU_CORE_ASSERT(m_handle->window, "window must exist");
    }

    Window::~Window() {
        glfwDestroyWindow(m_handle->window);

        glfwTerminate();
    }

    vk::Result Window::createSurface(vk::Instance instance, vk::SurfaceKHR *surface) {
        auto result = glfwCreateWindowSurface(
            static_cast<VkInstance>(instance),
            m_handle->window,
            nullptr,
            reinterpret_cast<VkSurfaceKHR *>(surface)
        );
        return static_cast<vk::Result>(result);
    }

    std::vector<const char *> Window::requiredExtensions() const {
        uint32_t count{0};
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + count);
        return extensions;
    }

    void *Window::window() const {
        return m_handle->window;
    }

    vk::Extent2D Window::extent() const {
        return {
            m_data.width,
            m_data.height
        };
    }

    uint32_t Window::width() const {
        return m_data.width;
    }

    uint32_t Window::height() const {
        return m_data.height;
    }

}
