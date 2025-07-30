#pragma once

#include "muon/asset/asset_loader.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/texture.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <deque>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace muon::asset {

class AssetManager {
public:
    struct Spec {
        const graphics::Context &context;
        std::vector<AssetLoader *> loaders{};

        Spec(const graphics::Context &context) : context{context} {}
    };

public:
    AssetManager(const Spec &spec);
    ~AssetManager();

    auto registerAssetLoader(AssetLoader *loader) -> void;

    auto beginLoading() -> void;
    auto endLoading() -> void;

    auto loadFromMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void;
    auto loadFromFile(const std::filesystem::path &path) -> void;

public:
    auto getCommandBuffer() -> vk::raii::CommandBuffer &;
    auto getCommandBuffer() const -> const vk::raii::CommandBuffer &;

    auto getUploadBuffers() -> std::deque<graphics::Buffer> *;

    auto getContext() const -> const graphics::Context &;

    auto getTextures() -> std::vector<graphics::Texture> &;

private:
    auto getAssetLoader(const std::string_view fileType) -> AssetLoader *;

private:
    const graphics::Context &m_context;
    const graphics::Queue &m_transferQueue;

    bool m_loadingInProgress{false};

    vk::raii::CommandBuffer m_commandBuffer{nullptr};
    vk::raii::Fence m_uploadFence{nullptr};
    std::deque<graphics::Buffer> m_uploadBuffers{};

    std::unordered_map<std::string, AssetLoader *> m_fileTypes{};
    std::vector<std::unique_ptr<AssetLoader>> m_loaders{};

    std::vector<graphics::Texture> m_textures;
};

} // namespace muon::asset
