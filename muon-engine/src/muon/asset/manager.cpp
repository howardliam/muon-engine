#include "muon/asset/manager.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <algorithm>
#include <fmt/ranges.h>
#include <memory>

namespace muon::asset {

Manager::Manager(const Spec &spec) : m_context{spec.context}, m_transferQueue{m_context.getTransferQueue()} {
    vk::CommandBufferAllocateInfo commandBufferAi;
    commandBufferAi.commandPool = m_transferQueue.getCommandPool();
    commandBufferAi.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAi.commandBufferCount = 1;

    auto commandBufferResult = m_context.getDevice().allocateCommandBuffers(commandBufferAi);
    core::expect(commandBufferResult, "failed to allocate command buffers");
    m_commandBuffer = std::move((*commandBufferResult)[0]);

    vk::FenceCreateInfo fenceCi;
    auto fenceResult = m_context.getDevice().createFence(fenceCi);
    core::expect(fenceResult, "failed to create upload fence");
    m_uploadFence = std::move(*fenceResult);

    for (auto &loader : spec.loaders) {
        registerLoader(loader);
    }
}

Manager::~Manager() {}

auto Manager::registerLoader(Loader *loader) -> void {
    auto it = std::ranges::find_if(m_loaders, [&l = loader](const std::unique_ptr<Loader> &loader) -> bool {
        return loader->getFileTypes() == l->getFileTypes();
    });

    if (it != m_loaders.end()) {
        core::warn("loader already exists for: {} files, skipping", fmt::join(loader->getFileTypes(), ", "));
        return;
    }

    auto &l = m_loaders.emplace_back(loader);

    for (const auto fileType : l->getFileTypes()) {
        m_fileTypes[fileType.data()] = l.get();
    }

    core::debug("registered loader for: {} files", fmt::join(loader->getFileTypes(), ", "));
}

auto Manager::beginLoading() -> void {
    core::expect(!m_loadingInProgress, "cannot begin loading while loading is in progress");

    vk::CommandBufferBeginInfo commandBufferBi;
    m_commandBuffer.begin(commandBufferBi);

    m_loadingInProgress = true;
}

auto Manager::endLoading() -> void {
    core::expect(m_loadingInProgress, "cannot end loading if loading has not been started");

    m_commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(*m_commandBuffer);
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.signalSemaphoreCount = 0;
    m_transferQueue.get().submit({submitInfo}, m_uploadFence);

    m_loadingInProgress = false;

    auto waitResult = m_context.getDevice().waitForFences({m_uploadFence}, true, 30'000'000);
    core::expect(waitResult == vk::Result::eSuccess, "failed to wait for upload fence to be signalled");

    m_context.getDevice().resetFences({m_uploadFence});

    m_uploadBuffers.clear();
}

auto Manager::loadFromMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void {
    core::expect(m_loadingInProgress, "cannot load from memory if loading hasn't begun");

    auto loader = getLoader(fileType);
    core::expect(loader, "no loader found");

    loader->fromMemory(data);
}

auto Manager::loadFromFile(const std::filesystem::path &path) -> void {
    core::expect(m_loadingInProgress, "cannot load from file if loading hasn't begun");

    core::expect(path.has_extension(), "file must have an extension");
    auto extension = path.extension();

    auto loader = getLoader(extension.c_str());
    core::expect(loader, "no loader found");

    loader->fromFile(path);
}

auto Manager::getCommandBuffer() -> vk::raii::CommandBuffer & { return m_commandBuffer; }
auto Manager::getCommandBuffer() const -> const vk::raii::CommandBuffer & { return m_commandBuffer; }

auto Manager::getUploadBuffers() -> std::deque<graphics::Buffer> * { return &m_uploadBuffers; }

auto Manager::getLoader(const std::string_view fileType) -> Loader * {
    auto it = m_fileTypes.find(fileType.data());
    if (it == m_fileTypes.end()) {
        return nullptr;
    }
    return it->second;
}

} // namespace muon::asset
