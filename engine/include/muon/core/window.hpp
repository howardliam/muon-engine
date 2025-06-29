#pragma once

#include "muon/event/dispatcher.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <yaml-cpp/yaml.h>

namespace muon {

    struct WindowSpecification {
        event::Dispatcher *dispatcher = nullptr;
        uint32_t width;
        uint32_t height;
        std::string_view title;
        std::filesystem::path icon;
    };

    class Window {
    public:
        Window(const WindowSpecification &spec);
        ~Window();

        void PollEvents() const;

        [[nodiscard]] VkResult CreateSurface(VkInstance instance, VkSurfaceKHR *surface) const;
        [[nodiscard]] const char *GetClipboardContents() const;
        void RequestAttention() const;

    public:
        [[nodiscard]] VkExtent2D GetExtent() const;
        [[nodiscard]] uint32_t GetWidth() const;
        [[nodiscard]] uint32_t GetHeight() const;
        [[nodiscard]] uint32_t GetRefreshRate() const;
        [[nodiscard]] std::vector<const char *> GetRequiredExtensions() const;

    private:
        void ConfigureDispatchers();

    private:
        struct WindowHandle;
        std::unique_ptr<WindowHandle> m_handle;

        struct WindowData {
            const event::Dispatcher *dispatcher;

            std::string title;
            uint32_t width;
            uint32_t height;
            uint32_t refreshRate;
            bool rawMouseMotion = false;
        };
        WindowData m_data{};
    };

}

namespace YAML {

    template <>
    struct convert<muon::WindowSpecification> {
        static Node encode(const muon::WindowSpecification &spec) {
            Node node;
            node["title"] = spec.title;
            node["dimensions"].push_back(spec.width);
            node["dimensions"].push_back(spec.height);
            return node;
        }

        static bool decode(const Node &node, muon::WindowSpecification &spec) {
            if (!node.IsMap()) {
                return false;
            }

            spec.width = node["dimensions"][0].as<uint32_t>();
            spec.height = node["dimensions"][1].as<uint32_t>();
            return true;
        }
    };

}
