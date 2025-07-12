#pragma once

#include "muon/asset/loader.hpp"
#include "muon/graphics/context.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::asset {

class Manager {
public:
    struct Spec {
        const graphics::Context *context{nullptr};
        std::vector<Loader> loaders{};
    };

public:
    Manager(const Spec &spec);
    ~Manager();

    auto RegisterLoader(const Loader &loader) -> void;

    auto BeginLoading() -> void;
    auto EndLoading() -> void;

    auto LoadFile(const std::filesystem::path &path) -> void;
    auto LoadMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void;

public:
    [[nodiscard]] auto GetCommandBuffer() -> VkCommandBuffer;

private:
    const graphics::Context &m_context;
    const graphics::Queue &m_transferQueue;

    VkCommandBuffer m_cmd{nullptr};
    bool m_loadingInProgress{false};

    std::unordered_map<std::string, FileLoadFn> m_fileLoaders;
    std::unordered_map<std::string, MemoryLoadFn> m_memoryLoaders;
};

} // namespace muon::asset
