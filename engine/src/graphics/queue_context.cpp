#include "muon/graphics/queue_context.hpp"

#include "muon/core/application.hpp"
#include "muon/graphics/queue.hpp"

namespace muon::gfx {

    QueueContext::QueueContext() {
        auto &context = Application::Get().GetDeviceContext();
        auto device = context.GetDevice();
        auto queueAllocator = context.GetQueueAllocator();

        m_renderQueue = std::make_unique<Queue>(QueueSpecification{
            QueueType::Graphics,
            device,
            queueAllocator.GetQueueFamilyIndex(QueueType::Graphics),
            queueAllocator.GetNextQueueIndex(QueueType::Graphics),
            "render"
        });

        m_postFxQueue = std::make_unique<Queue>(QueueSpecification{
            QueueType::Compute,
            device,
            queueAllocator.GetQueueFamilyIndex(QueueType::Compute),
            queueAllocator.GetNextQueueIndex(QueueType::Compute),
            "post-processing fx"
        });

        m_presentQueue = std::make_unique<Queue>(QueueSpecification{
            QueueType::Present,
            device,
            queueAllocator.GetQueueFamilyIndex(QueueType::Present),
            queueAllocator.GetNextQueueIndex(QueueType::Present),
            "present"
        });

        m_computeQueue = std::make_unique<Queue>(QueueSpecification{
            QueueType::Compute,
            device,
            queueAllocator.GetQueueFamilyIndex(QueueType::Compute),
            queueAllocator.GetNextQueueIndex(QueueType::Compute),
            "compute"
        });

        m_transferQueue = std::make_unique<Queue>(QueueSpecification{
            QueueType::Transfer,
            device,
            queueAllocator.GetQueueFamilyIndex(QueueType::Transfer),
            queueAllocator.GetNextQueueIndex(QueueType::Transfer),
            "transfer"
        });
    }

    QueueContext::~QueueContext() {
        m_renderQueue.reset();
        m_postFxQueue.reset();
        m_presentQueue.reset();
        m_computeQueue.reset();
        m_transferQueue.reset();
    }

    Queue &QueueContext::GetRenderQueue() const {
        return *m_renderQueue;
    }

    Queue &QueueContext::GetPostFxQueue() const {
        return *m_postFxQueue;
    }

    Queue &QueueContext::GetPresentQueue() const {
        return *m_presentQueue;
    }

    Queue &QueueContext::GetComputeQueue() const {
        return *m_computeQueue;
    }

    Queue &QueueContext::GetTransferQueue() const {
        return *m_transferQueue;
    }

    QueueRequestInfo QueueContext::GetRequestInfo() {
        return {
            .graphicsCount = 1,
            .computeCount = 2,
            .transferCount = 1,
            .presentCount = 1,
        };
    }

}
