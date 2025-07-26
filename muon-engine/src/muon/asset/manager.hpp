#pragma once

#include "muon/asset/loader.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/context.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <deque>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

    auto registerLoader(Loader *loader) -> void;

    auto beginLoading() -> void;
    auto endLoading() -> void;

    auto loadFromMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void;
    auto loadFromFile(const std::filesystem::path &path) -> void;

public:
    auto getCommandBuffer() -> vk::raii::CommandBuffer &;
    auto getCommandBuffer() const -> const vk::raii::CommandBuffer &;

    auto getUploadBuffers() -> std::deque<graphics::Buffer> *;

private:
    auto getLoader(const std::string_view fileType) -> Loader *;

private:
    const graphics::Context &m_context;
    const graphics::Queue &m_transferQueue;

    bool m_loadingInProgress{false};

    vk::raii::CommandBuffer m_commandBuffer{nullptr};
    vk::raii::Fence m_uploadFence{nullptr};
    std::deque<graphics::Buffer> m_uploadBuffers{};

    std::unordered_map<std::string, Loader *> m_fileTypes{};
    std::vector<std::unique_ptr<Loader>> m_loaders{};
};

} // namespace muon::asset
