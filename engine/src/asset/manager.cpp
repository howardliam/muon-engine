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

    for (auto &loader : spec.loaders) {
        RegisterLoader(loader);
    }
}

Manager::~Manager() { vkFreeCommandBuffers(m_context.GetDevice(), m_transferQueue.GetCommandPool(), 1, &m_cmd); }

auto Manager::RegisterLoader(Loader *loader) -> void {
    auto key = loader->GetFileType();
    MU_CORE_ASSERT(key.starts_with('.'), "file type must begin with a fullstop, e.g.: .png, .jxl");

    if (m_loaders.find(key.data()) == m_loaders.end()) {
        std::unique_ptr<Loader> loader2(loader);
        loader2->SetManager(this);
        m_loaders[key.data()] = std::move(loader2);
        MU_CORE_DEBUG("registered file loader for: {} files", key);
    } else {
        MU_CORE_WARN("loader already exists for: {} files", key);
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

auto Manager::FromMemory(const std::vector<uint8_t> &data, const std::string_view fileType) -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot load from memory if loading hasn't begun");

    auto loader = GetLoader(fileType);
    MU_CORE_ASSERT(loader != nullptr, "no loader found");

    loader->FromMemory(data);
}

auto Manager::FromFile(const std::filesystem::path &path) -> void {
    MU_CORE_ASSERT(m_loadingInProgress, "cannot load from file if loading hasn't begun");

    MU_CORE_ASSERT(path.has_extension(), "file must have an extension");
    auto extension = path.extension();

    auto loader = GetLoader(extension.c_str());
    MU_CORE_ASSERT(loader != nullptr, "no loader found");

    loader->FromFile(path);
}

auto Manager::GetCommandBuffer() -> VkCommandBuffer { return m_cmd; }

auto Manager::GetLoader(const std::string_view fileType) -> Loader * {
    auto it = m_loaders.find(fileType.data());
    if (it == m_loaders.end()) {
        return nullptr;
    }
    return it->second.get();
}

} // namespace muon::asset
