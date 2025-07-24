#include "muon/asset/manager.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <algorithm>
#include <fmt/ranges.h>
#include <memory>

namespace muon::asset {

Manager::Manager(const Spec &spec) : m_context{*spec.context}, m_transferQueue{spec.context->GetTransferQueue()} {
    vk::CommandBufferAllocateInfo commandBufferAi;
    commandBufferAi.commandPool = m_transferQueue.GetCommandPool();
    commandBufferAi.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAi.commandBufferCount = 1;

    auto commandBufferResult = m_context.GetDevice().allocateCommandBuffers(commandBufferAi);
    MU_CORE_ASSERT(commandBufferResult, "failed to allocate command buffers");
    m_commandBuffer = std::move((*commandBufferResult)[0]);

    vk::FenceCreateInfo fenceCi;
    auto fenceResult = m_context.GetDevice().createFence(fenceCi);
    MU_CORE_ASSERT(fenceResult, "failed to create upload fence");
    m_uploadFence = std::move(*fenceResult);

    for (auto &loader : spec.loaders) {
        RegisterLoader(loader);
    }
}

Manager::~Manager() {}

auto Manager::RegisterLoader(Loader *loader) -> void {
    auto it = std::ranges::find_if(m_loaders, [&l = loader](const std::unique_ptr<Loader> &loader) -> bool {
        return loader->GetFileTypes() == l->GetFileTypes();
    });

    if (it != m_loaders.end()) {
        MU_CORE_WARN("loader already exists for: {} files, skipping", fmt::join(loader->GetFileTypes(), ", "));
        return;
    }

    auto &l = m_loaders.emplace_back(loader);

    for (const auto fileType : l->GetFileTypes()) {
        m_fileTypes[fileType.data()] = l.get();
    }

    MU_CORE_DEBUG("registered loader for: {} files", fmt::join(loader->GetFileTypes(), ", "));
}

auto Manager::BeginLoading() -> void {
    MU_CORE_ASSERT(!m_loadingInProgress, "cannot begin loading while loading is in progress");

    vk::CommandBufferBeginInfo commandBufferBi;
    m_commandBuffer.begin(commandBufferBi);

    m_loadingInProgress = true;
}

auto Manager::EndLoading() -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot end loading if loading has not been started");

    m_commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(*m_commandBuffer);
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.signalSemaphoreCount = 0;
    m_transferQueue.Get().submit({submitInfo}, m_uploadFence);

    m_loadingInProgress = false;

    auto waitResult = m_context.GetDevice().waitForFences({m_uploadFence}, true, 30'000'000);
    MU_CORE_ASSERT(waitResult == vk::Result::eSuccess, "failed to wait for upload fence to be signalled");

    m_context.GetDevice().resetFences({m_uploadFence});

    m_uploadBuffers.clear();
}

auto Manager::LoadFromMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot load from memory if loading hasn't begun");

    auto loader = GetLoader(fileType);
    MU_CORE_ASSERT(loader != nullptr, "no loader found");

    loader->FromMemory(data);
}

auto Manager::LoadFromFile(const std::filesystem::path &path) -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot load from file if loading hasn't begun");

    MU_CORE_ASSERT(path.has_extension(), "file must have an extension");
    auto extension = path.extension();

    auto loader = GetLoader(extension.c_str());
    MU_CORE_ASSERT(loader != nullptr, "no loader found");

    loader->FromFile(path);
}

auto Manager::GetCommandBuffer() -> vk::raii::CommandBuffer & { return m_commandBuffer; }
auto Manager::GetCommandBuffer() const -> const vk::raii::CommandBuffer & { return m_commandBuffer; }

auto Manager::GetUploadBuffers() -> std::deque<graphics::Buffer> * { return &m_uploadBuffers; }

auto Manager::GetLoader(const std::string_view fileType) -> Loader * {
    auto it = m_fileTypes.find(fileType.data());
    if (it == m_fileTypes.end()) {
        return nullptr;
    }
    return it->second;
}

} // namespace muon::asset
