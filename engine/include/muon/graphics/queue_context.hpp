#pragma once

#include "muon/graphics/queue.hpp"
#include "muon/graphics/queue_info.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    class QueueContext {
    public:
        QueueContext();
        ~QueueContext();

    public:
        [[nodiscard]] Queue &GetRenderQueue() const;
        [[nodiscard]] Queue &GetPostFxQueue() const;
        [[nodiscard]] Queue &GetPresentQueue() const;
        [[nodiscard]] Queue &GetComputeQueue() const;
        [[nodiscard]] Queue &GetTransferQueue() const;

        [[nodiscard]] static QueueRequestInfo GetRequestInfo();

    private:


    private:
        std::unique_ptr<Queue> m_presentQueue;
        std::unique_ptr<Queue> m_renderQueue;
        std::unique_ptr<Queue> m_postFxQueue;
        std::unique_ptr<Queue> m_computeQueue;
        std::unique_ptr<Queue> m_transferQueue;
    };

}
