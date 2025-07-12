#pragma once

#include "muon/asset/loader.hpp"
#include "muon/graphics/context.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::asset {

class Manager {
public:
    struct Spec {
        const graphics::Context *context{nullptr};
        std::vector<Loader *> loaders{};
    };

public:
    Manager(const Spec &spec);
    ~Manager();

    auto RegisterLoader(Loader *loader) -> void;

    auto BeginLoading() -> void;
    auto EndLoading() -> void;

    auto FromMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void;
    auto FromFile(const std::filesystem::path &path) -> void;

public:
    [[nodiscard]] auto GetCommandBuffer() -> VkCommandBuffer;

private:
    [[nodiscard]] auto GetLoader(const std::string_view fileType) -> Loader *;

private:
    const graphics::Context &m_context;
    const graphics::Queue &m_transferQueue;

    VkCommandBuffer m_cmd{nullptr};
    bool m_loadingInProgress{false};

    std::unordered_map<std::string, std::unique_ptr<Loader>> m_loaders;
};

} // namespace muon::asset
