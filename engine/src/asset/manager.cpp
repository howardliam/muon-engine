#include "muon/asset/manager.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::asset {

Manager::Manager(const Spec &spec) : m_context{*spec.context}, m_transferQueue{spec.context->GetTransferQueue()} {
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = m_transferQueue.GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(m_context.GetDevice(), &allocInfo, &m_cmd);

    for (const auto &loader : spec.loaders) {
        RegisterLoader(loader);
    }
}

Manager::~Manager() { vkFreeCommandBuffers(m_context.GetDevice(), m_transferQueue.GetCommandPool(), 1, &m_cmd); }

auto Manager::RegisterLoader(const Loader &loader) -> void {
    MU_CORE_ASSERT(loader.fileType.starts_with('.'), "file type must begin with a fullstop, e.g.: .png, .jxl");

    if (m_fileLoaders.find(loader.fileType) != m_fileLoaders.end()) {
        m_fileLoaders[loader.fileType] = loader.fileLoad;
    } else {
        MU_CORE_WARN("file loader already exists for: {} files", loader.fileType);
    }

    if (m_memoryLoaders.find(loader.fileType) != m_memoryLoaders.end()) {
        m_memoryLoaders[loader.fileType] = loader.memoryLoad;
    } else {
        MU_CORE_WARN("memory loader already exists for: {} files", loader.fileType);
    }
}

auto Manager::BeginLoading() -> void {
    MU_CORE_ASSERT(!m_loadingInProgress, "cannot begin loading while loading is in progress");

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    auto result = vkBeginCommandBuffer(m_cmd, &beginInfo);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to begin recording command buffer");

    m_loadingInProgress = true;
}

auto Manager::EndLoading() -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot end loading if loading has not been started");

    auto result = vkEndCommandBuffer(m_cmd);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to end recording command buffer");

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_cmd;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.signalSemaphoreCount = 0;
    result = vkQueueSubmit(m_transferQueue.Get(), 1, &submitInfo, nullptr);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to submit command buffer to queue");

    m_loadingInProgress = false;
}

auto Manager::LoadFile(const std::filesystem::path &path) -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot load from file if loading hasn't begun");

    MU_CORE_ASSERT(path.has_extension(), "file must have an extension");
    auto extension = path.extension();

    auto it = m_fileLoaders.find(extension);
    MU_CORE_ASSERT(it != m_fileLoaders.end(), "no loader found");

    it->second(this, path);
}

auto Manager::LoadMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot load from memory if loading hasn't begun");

    auto it = m_memoryLoaders.find(fileType.data());
    MU_CORE_ASSERT(it != m_memoryLoaders.end(), "no loader found");

    it->second(this, data);
}

auto Manager::GetCommandBuffer() -> VkCommandBuffer { return m_cmd; }

} // namespace muon::asset
