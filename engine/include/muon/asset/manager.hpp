#pragma once

#include "muon/asset/loader.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/context.hpp"

#include <deque>
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

    auto LoadFromMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void;
    auto LoadFromFile(const std::filesystem::path &path) -> void;

public:
    auto GetCommandBuffer() -> VkCommandBuffer;
    auto GetUploadBuffers() -> std::deque<graphics::Buffer> *;

private:
    auto GetLoader(const std::string_view fileType) -> Loader *;

private:
    const graphics::Context &m_context;
    const graphics::Queue &m_transferQueue;

    bool m_loadingInProgress{false};

    VkCommandBuffer m_cmd{nullptr};
    VkFence m_uploadFence{nullptr};
    std::deque<graphics::Buffer> m_uploadBuffers{};

    std::unordered_map<std::string, Loader *> m_fileTypes{};
    std::vector<std::unique_ptr<Loader>> m_loaders{};
};

} // namespace muon::asset
