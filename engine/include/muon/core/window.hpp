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
        event::Dispatcher *dispatcher{nullptr};
        uint32_t width{std::numeric_limits<uint32_t>().max()};
        uint32_t height{std::numeric_limits<uint32_t>().max()};
        std::string_view title{};
        std::filesystem::path icon{};
    };

    class Window {
    public:
        Window(const WindowSpecification &spec);
        ~Window();

        auto PollEvents() const -> void;

        auto RequestAttention() const -> void;

        [[nodiscard]] auto CreateSurface(VkInstance instance, VkSurfaceKHR *surface) const -> VkResult;
        [[nodiscard]] auto GetClipboardContents() const -> const char *;

    public:
        [[nodiscard]] auto GetExtent() const -> VkExtent2D;
        [[nodiscard]] auto GetWidth() const -> uint32_t;
        [[nodiscard]] auto GetHeight() const -> uint32_t;
        [[nodiscard]] auto GetRefreshRate() const -> uint32_t;
        [[nodiscard]] auto GetRequiredExtensions() const -> std::vector<const char *>;

    private:
        auto ConfigureDispatchers() -> void;

    private:
        struct WindowHandle;
        std::unique_ptr<WindowHandle> m_handle;

        struct WindowData {
            const event::Dispatcher *dispatcher{nullptr};

            std::string title{};
            uint32_t width{};
            uint32_t height{};
            uint32_t refreshRate{};
            bool rawMouseMotion{false};
        };
        WindowData m_data{};
    };

}

namespace YAML {

    template<>
    struct convert<muon::WindowSpecification> {
        static auto encode(const auto &spec) -> Node {
            Node node;
            node["title"] = spec.title;
            node["dimensions"].push_back(spec.width);
            node["dimensions"].push_back(spec.height);
            return node;
        }

        static auto decode(const Node &node, auto &spec) -> bool {
            if (!node.IsMap()) {
                return false;
            }

            spec.width = node["dimensions"][0].as<uint32_t>();
            spec.height = node["dimensions"][1].as<uint32_t>();
            return true;
        }
    };

}
