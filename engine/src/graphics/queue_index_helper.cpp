#include "muon/graphics/queue_index_helper.hpp"

#include "muon/core/assert.hpp"
#include <optional>

namespace muon::gfx {

    QueueIndexHelper::QueueIndexHelper(const QueueInfo &queueInfo, const QueueRequestInfo &requestInfo) {
        const auto families = queueInfo.GetFamilyInfo();

        auto presentIt = std::ranges::find_if(families, [](const QueueFamilyInfo &el) { return el.IsPresentCapable(); });
        MU_CORE_ASSERT(presentIt != families.end(), "there must be a present capable queue family");
        auto &presentFamily = *presentIt;

        auto graphicsIt = std::ranges::find_if(families, [](const QueueFamilyInfo &el) { return el.IsGraphicsCapable(); });
        MU_CORE_ASSERT(graphicsIt != families.end(), "there must be a graphics capable queue family");
        auto &graphicsFamily = *graphicsIt;

        std::optional<QueueFamilyInfo> computeFamily;
        auto computeIt = std::ranges::find_if(families, [](const QueueFamilyInfo &el) { return el.IsComputeDedicated(); });
        if (computeIt != families.end()) { computeFamily.emplace(*computeIt); }
        if (!computeFamily) {
            auto computeIt = std::ranges::find_if(families, [](const QueueFamilyInfo &el) { return el.IsComputeCapable(); });
            if (computeIt != families.end()) { computeFamily.emplace(*computeIt); }
        }
        MU_CORE_ASSERT(computeFamily, "there must be a compute capable queue family");

        std::optional<QueueFamilyInfo> transferFamily;
        auto transferIt = std::ranges::find_if(families, [](const QueueFamilyInfo &el) { return el.IsTransferDedicated(); });
        if (transferIt != families.end()) { transferFamily.emplace(*transferIt); }
        if (!transferFamily) {
            auto transferIt = std::ranges::find_if(families, [](const QueueFamilyInfo &el) { return el.IsTransferCapable(); });
            if (transferIt != families.end()) { transferFamily.emplace(*transferIt); }
        }
        MU_CORE_ASSERT(transferFamily, "there must be a transfer capable queue family");

        m_aliases[QueueType::Graphics] = QueueType::Graphics;

        if (*computeFamily == graphicsFamily) {
            m_aliases[QueueType::Compute] = QueueType::Graphics;
        } else {
            m_aliases[QueueType::Compute] = QueueType::Compute;
        }

        if (*transferFamily == graphicsFamily) {
            m_aliases[QueueType::Transfer] = QueueType::Graphics;
        } else {
            m_aliases[QueueType::Transfer] = QueueType::Transfer;
        }

        if (presentFamily == graphicsFamily) {
            m_aliases[QueueType::Present] = QueueType::Graphics;
        } else if (presentFamily == *computeFamily) {
            m_aliases[QueueType::Present] = QueueType::Compute;
        } else if (presentFamily == *computeFamily) {
            m_aliases[QueueType::Present] = QueueType::Transfer;
        } else {
            m_aliases[QueueType::Present] = QueueType::Present;
        }

        for (auto &[_, type] : m_aliases) {
            m_nextQueueIndex[type] = 0;
            m_maxQueues[type] = 0;
            m_familyIndices[type] = 0;
        }

        m_familyIndices[m_aliases[QueueType::Graphics]] = graphicsFamily.familyIndex;
        m_familyIndices[m_aliases[QueueType::Compute]] = computeFamily->familyIndex;
        m_familyIndices[m_aliases[QueueType::Transfer]] = transferFamily->familyIndex;
        m_familyIndices[m_aliases[QueueType::Present]] = presentFamily.familyIndex;

        m_maxQueues[m_aliases[QueueType::Graphics]] += requestInfo.graphicsCount;
        m_maxQueues[m_aliases[QueueType::Compute]] += requestInfo.computeCount;
        m_maxQueues[m_aliases[QueueType::Transfer]] += requestInfo.transferCount;
        m_maxQueues[m_aliases[QueueType::Present]] += requestInfo.presentCount;
    }

    std::vector<VkDeviceQueueCreateInfo> QueueIndexHelper::GenerateCreateInfos() {
        std::vector<VkDeviceQueueCreateInfo> createInfos{};

        createInfos.reserve(m_familyIndices.size());
        std::vector<float> priorities(5, 1.0);
        for (const auto &[type, index] : m_familyIndices) {
            VkDeviceQueueCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            createInfo.queueFamilyIndex = index;
            createInfo.queueCount = m_maxQueues[type];
            createInfo.pQueuePriorities = priorities.data();

            createInfos.push_back(createInfo);
        }

        return createInfos;
    }

    uint32_t QueueIndexHelper::GetQueueFamilyIndex(const QueueType &queueType) const {
        auto real = m_aliases.find(queueType);
        MU_CORE_ASSERT(real != m_aliases.end(), "there must be an alias");

        auto index = m_familyIndices.find(real->second);
        MU_CORE_ASSERT(index != m_familyIndices.end(), "there must be a queue family index");

        return index->second;
    }

    uint32_t QueueIndexHelper::GetNextQueueIndex(const QueueType &queueType) {
        auto real = m_aliases.find(queueType);
        MU_CORE_ASSERT(real != m_aliases.end(), "there must be an alias");

        auto index = m_nextQueueIndex.find(real->second);
        MU_CORE_ASSERT(index != m_familyIndices.end(), "there must be a next queue index");

        auto max = m_maxQueues.find(real->second);
        MU_CORE_ASSERT(max != m_maxQueues.end(), "there must be a max queues value");

        MU_CORE_ASSERT(index->second < max->second, "not enough queue indices left");

        uint32_t queueIndex = index->second;
        m_nextQueueIndex[real->second] += 1;

        return queueIndex;
    }

}
