#include "muon/asset/manager.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <algorithm>
#include <fmt/ranges.h>
#include <limits>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::asset {

Manager::Manager(const Spec &spec) : m_context{*spec.context}, m_transferQueue{spec.context->GetTransferQueue()} {
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = m_transferQueue.GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_context.GetDevice(), &allocInfo, &m_cmd);

    VkFenceCreateInfo createInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkCreateFence(m_context.GetDevice(), &createInfo, nullptr, &m_uploadFence);

    for (auto &loader : spec.loaders) {
        RegisterLoader(loader);
    }
}

Manager::~Manager() {
    vkDestroyFence(m_context.GetDevice(), m_uploadFence, nullptr);
    vkFreeCommandBuffers(m_context.GetDevice(), m_transferQueue.GetCommandPool(), 1, &m_cmd);
}

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
    result = vkQueueSubmit(m_transferQueue.Get(), 1, &submitInfo, m_uploadFence);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to submit command buffer to queue");

    m_loadingInProgress = false;

    result = vkWaitForFences(m_context.GetDevice(), 1, &m_uploadFence, true, std::numeric_limits<uint64_t>().max());
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to wait for upload fence to be signalled");

    result = vkResetFences(m_context.GetDevice(), 1, &m_uploadFence);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to reset upload fence");

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

auto Manager::GetCommandBuffer() -> VkCommandBuffer { return m_cmd; }

auto Manager::GetUploadBuffers() -> std::deque<graphics::Buffer> * { return &m_uploadBuffers; }

auto Manager::GetLoader(const std::string_view fileType) -> Loader * {
    auto it = m_fileTypes.find(fileType.data());
    if (it == m_fileTypes.end()) {
        return nullptr;
    }
    return it->second;
}

} // namespace muon::asset
