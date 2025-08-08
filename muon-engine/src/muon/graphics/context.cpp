#include "muon/graphics/context.hpp"

#include "muon/core/application.hpp"
#include "muon/core/debug.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/constants.hpp"
#include "muon/graphics/extensions.hpp"
#include "muon/graphics/gpu.hpp"
#include "muon/graphics/queue.hpp"
#include "muon/graphics/queue_family_info.hpp"
#include "vulkan/vulkan.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <memory>
#include <numeric>
#include <set>
#include <vector>

namespace muon::graphics {

Context::Context(const Window &window) {
    m_context = vk::raii::Context();

    createInstance(window);
    createDebugMessenger();
    createSurface(window);
    selectPhysicalDevice();
    createLogicalDevice();
    createAllocator();

    core::debug("created graphics context");
}

Context::~Context() {
    m_allocator.destroy();
    m_graphicsQueue.reset();
    m_computeQueue.reset();
    m_transferQueue.reset();

    core::debug("destroyed graphics context");
}

auto Context::getVramUsage() const -> uint64_t {
    auto chain =
        m_physicalDevice.getMemoryProperties2<vk::PhysicalDeviceMemoryProperties2, vk::PhysicalDeviceMemoryBudgetPropertiesEXT>();

    auto memoryBudgetProps = chain.get<vk::PhysicalDeviceMemoryBudgetPropertiesEXT>();

    return std::accumulate(memoryBudgetProps.heapUsage.begin(), memoryBudgetProps.heapUsage.end(), 0);
}
auto Context::getVramCapacity() const -> uint64_t { return m_vramCapacity; }

auto Context::getInstance() -> vk::raii::Instance & { return m_instance; }
auto Context::getInstance() const -> const vk::raii::Instance & { return m_instance; }

auto Context::getSurface() -> vk::raii::SurfaceKHR & { return m_surface; }
auto Context::getSurface() const -> const vk::raii::SurfaceKHR & { return m_surface; }

auto Context::getPhysicalDevice() -> vk::raii::PhysicalDevice & { return m_physicalDevice; }
auto Context::getPhysicalDevice() const -> const vk::raii::PhysicalDevice & { return m_physicalDevice; }

auto Context::getDevice() -> vk::raii::Device & { return m_device; }
auto Context::getDevice() const -> const vk::raii::Device & { return m_device; }

auto Context::getGraphicsQueue() -> Queue & { return *m_graphicsQueue; }
auto Context::getGraphicsQueue() const -> const Queue & { return *m_graphicsQueue; }

auto Context::getComputeQueue() -> Queue & { return *m_computeQueue; }
auto Context::getComputeQueue() const -> const Queue & { return *m_computeQueue; }

auto Context::getTransferQueue() -> Queue & { return *m_transferQueue; }
auto Context::getTransferQueue() const -> const Queue & { return *m_transferQueue; }

auto Context::getAllocator() -> vma::Allocator & { return m_allocator; }
auto Context::getAllocator() const -> const vma::Allocator & { return m_allocator; }

void Context::createInstance(const Window &window) {
    std::vector extensions = window.getRequiredExtensions();
    extensions.insert(extensions.end(), k_instanceRequiredExtensions.begin(), k_instanceRequiredExtensions.end());

    std::vector<const char *> layers;

    if constexpr (k_debugEnabled) {
        extensions.push_back("VK_EXT_debug_utils");

        auto checkValidationLayerSupport = [](const vk::raii::Context &context, const char *layer) -> bool {
            auto availableLayers = context.enumerateInstanceLayerProperties();

            auto it = std::ranges::find_if(availableLayers, [&](const VkLayerProperties &props) -> bool {
                return std::strcmp(props.layerName, layer) == 0;
            });

            if (it == availableLayers.end()) {
                return false;
            }

            return true;
        };

        const char *validationLayer = "VK_LAYER_KHRONOS_validation";
        if (checkValidationLayerSupport(m_context, validationLayer)) {
            layers.push_back(validationLayer);
        } else {
            core::warn("`{}` is unavailable", validationLayer);
        }
    }

    vk::ApplicationInfo appInfo;
    appInfo.apiVersion = k_vulkanApiVersion;

    appInfo.pApplicationName = Application::get().getName().data();
    appInfo.applicationVersion = vk::makeApiVersion(1, 0, 0, 0);

    appInfo.pEngineName = "Muon";
    appInfo.engineVersion = vk::makeApiVersion(1, 0, 0, 0);

    vk::InstanceCreateInfo instanceCi;
    instanceCi.enabledExtensionCount = extensions.size();
    instanceCi.ppEnabledExtensionNames = extensions.data();

    instanceCi.enabledLayerCount = layers.size();
    instanceCi.ppEnabledLayerNames = layers.data();

    instanceCi.pApplicationInfo = &appInfo;

    auto instanceResult = m_context.createInstance(instanceCi);
    core::expect(instanceResult, "failed to create instance");
    m_instance = std::move(*instanceResult);
}

void Context::createDebugMessenger() {
    if constexpr (k_debugEnabled) {
        auto debugCallback = [](
            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
            vk::DebugUtilsMessageTypeFlagsEXT type,
            const vk::DebugUtilsMessengerCallbackDataEXT *callbackData,
            void *userData
        ) -> vk::Bool32 {
            switch (severity) {
                case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
                    muon::core::debug(callbackData->pMessage);
                    break;

                case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
                    muon::core::info(callbackData->pMessage);
                    break;

                case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
                    muon::core::warn(callbackData->pMessage);
                    break;

                case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
                    muon::core::error(callbackData->pMessage);
                    break;

                default:
                    break;
            }

            return false;
        };

        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCi;
        debugUtilsMessengerCi.pfnUserCallback = debugCallback;

        debugUtilsMessengerCi.messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        debugUtilsMessengerCi.messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        auto debugMessengerResult = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCi);
        core::expect(debugMessengerResult, "failed to create debug messenger");
        m_debugMessenger = std::move(*debugMessengerResult);
    }
}

void Context::createSurface(const Window &window) {
    auto surfaceResult = window.createSurface(m_instance);
    core::expect(surfaceResult, "failed to create window surface");
    m_surface = std::move(*surfaceResult);
    core::expect(*m_surface, "surface must not be null");
}

void Context::selectPhysicalDevice() {
    auto physicalDevicesResult = m_instance.enumeratePhysicalDevices();
    core::expect(physicalDevicesResult, "failed to get available GPUs");
    auto physicalDevices = *physicalDevicesResult;

    std::vector<Gpu> gpus;

    for (const auto &physicalDevice : physicalDevices) {
        Gpu gpu{&physicalDevice};

        if (gpu.isSuitable()) {
            gpus.push_back(gpu);
        }
    }

    std::sort(gpus.begin(), gpus.end(), [](const Gpu &lhs, const Gpu &rhs) -> bool {
        return lhs.getMemorySize() > rhs.getMemorySize();
    });

    bool gpuSelected = false;

    if (gpus.size() >= 1) {
        auto &gpu = gpus.front();
        m_physicalDevice = gpu.getPhysicalDevice();
        m_vramCapacity = gpu.getMemorySize();
        gpuSelected = true;
    }

    core::expect(gpuSelected, "failed to select a suitable GPU");
}

void Context::createLogicalDevice() {
    std::optional<uint32_t> graphicsIndex;
    std::optional<uint32_t> computeIndex;
    std::optional<uint32_t> transferIndex;

    for (const auto &family : generateQueueFamilyInfo(m_physicalDevice, m_surface)) {
        uint32_t requestedQueueCount = 0;

        if (!graphicsIndex) {
            if (family.isCapable(vk::QueueFlagBits::eGraphics) && family.supportsPresent()) {
                graphicsIndex.emplace(family.getIndex());
                requestedQueueCount += 1;

                core::trace("selected graphics queue family with index: {}", family.getIndex());
            }
        }

        if (!computeIndex) {
            if (family.isCapable(vk::QueueFlagBits::eCompute) && requestedQueueCount + 1 <= family.getQueueCount()) {
                computeIndex.emplace(family.getIndex());
                requestedQueueCount += 1;

                core::trace("selected compute queue family with index: {}", family.getIndex());
            }
        }

        if (!transferIndex) {
            if (family.isCapable(vk::QueueFlagBits::eTransfer) && requestedQueueCount + 1 <= family.getQueueCount()) {
                transferIndex.emplace(family.getIndex());
                requestedQueueCount += 1;

                core::trace("selected transfer queue family with index: {}", family.getIndex());
            }
        }
    }

    core::expect(graphicsIndex, "there must be a graphics capable queue family");
    core::expect(computeIndex, "there must be a compute capable queue family");
    core::expect(transferIndex, "there must be a transfer capable queue family");

    std::map<uint32_t, uint32_t> queueCounts;
    queueCounts[*graphicsIndex] += 1;
    queueCounts[*computeIndex] += 1;
    queueCounts[*transferIndex] += 1;

    std::set<uint32_t> uniqueIndices = {*graphicsIndex, *computeIndex, *transferIndex};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    std::vector<float> queuePriorities(uniqueIndices.size(), 1.0);
    for (const auto index : uniqueIndices) {
        vk::DeviceQueueCreateInfo deviceQueueCi;
        deviceQueueCi.queueFamilyIndex = index;
        deviceQueueCi.queueCount = queueCounts[index];
        deviceQueueCi.pQueuePriorities = queuePriorities.data();

        queueCreateInfos.push_back(deviceQueueCi);
    }

    auto features = vk::StructureChain<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceVulkan14Features, vk::PhysicalDeviceMeshShaderFeaturesEXT,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceExtendedDynamicState2FeaturesEXT,
        vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT, vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>{};

    auto &vidsFeatures = features.get<vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>();
    vidsFeatures.vertexInputDynamicState = true;

    auto &ds3Features = features.get<vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    ds3Features.extendedDynamicState3PolygonMode = true;

    auto &ds2Features = features.get<vk::PhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    ds2Features.extendedDynamicState2 = true;

    auto &dsFeatures = features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    dsFeatures.extendedDynamicState = true;

    auto &msFeatures = features.get<vk::PhysicalDeviceMeshShaderFeaturesEXT>();
    msFeatures.meshShader = true;
    msFeatures.taskShader = true;

    auto &vk14Features = features.get<vk::PhysicalDeviceVulkan14Features>();
    vk14Features.pushDescriptor = true;

    auto &vk13Features = features.get<vk::PhysicalDeviceVulkan13Features>();
    vk13Features.synchronization2 = true;
    vk13Features.dynamicRendering = true;

    auto &vk12Features = features.get<vk::PhysicalDeviceVulkan12Features>();
    vk12Features.bufferDeviceAddress = true;
    vk12Features.timelineSemaphore = true;
    vk12Features.scalarBlockLayout = true;

    vk12Features.shaderInputAttachmentArrayDynamicIndexing = true;
    vk12Features.shaderUniformTexelBufferArrayDynamicIndexing = true;
    vk12Features.shaderStorageTexelBufferArrayDynamicIndexing = true;
    vk12Features.shaderUniformBufferArrayNonUniformIndexing = true;
    vk12Features.shaderSampledImageArrayNonUniformIndexing = true;
    vk12Features.shaderStorageBufferArrayNonUniformIndexing = true;
    vk12Features.shaderStorageImageArrayNonUniformIndexing = true;
    vk12Features.shaderInputAttachmentArrayNonUniformIndexing = true;
    vk12Features.shaderUniformTexelBufferArrayNonUniformIndexing = true;
    vk12Features.shaderStorageTexelBufferArrayNonUniformIndexing = true;
    vk12Features.descriptorBindingUniformBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingSampledImageUpdateAfterBind = true;
    vk12Features.descriptorBindingStorageImageUpdateAfterBind = true;
    vk12Features.descriptorBindingStorageBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingUpdateUnusedWhilePending = true;
    vk12Features.descriptorBindingPartiallyBound = true;
    vk12Features.descriptorBindingVariableDescriptorCount = true;
    vk12Features.runtimeDescriptorArray = true;

    auto &deviceFeatures = features.get<vk::PhysicalDeviceFeatures2>().features;
    deviceFeatures = m_physicalDevice.getFeatures();

    vk::DeviceCreateInfo deviceCi;
    deviceCi.queueCreateInfoCount = queueCreateInfos.size();
    deviceCi.pQueueCreateInfos = queueCreateInfos.data();
    deviceCi.enabledExtensionCount = k_deviceRequiredExtensions.size();
    deviceCi.ppEnabledExtensionNames = k_deviceRequiredExtensions.data();
    deviceCi.pNext = &features.get<vk::PhysicalDeviceFeatures2>();

    auto deviceResult = m_physicalDevice.createDevice(deviceCi);
    core::expect(deviceResult, "failed to create device");
    m_device = std::move(*deviceResult);

    std::map<uint32_t, uint32_t> nextQueueIndices;
    nextQueueIndices[*graphicsIndex] = 0;
    nextQueueIndices[*computeIndex] = 0;
    nextQueueIndices[*transferIndex] = 0;

    Queue::Spec graphicsSpec{m_device};
    graphicsSpec.queueFamilyIndex = *graphicsIndex;
    graphicsSpec.queueIndex = nextQueueIndices[*graphicsIndex]++;
    graphicsSpec.name = "graphics";
    m_graphicsQueue = std::make_unique<Queue>(graphicsSpec);
    core::expect(m_graphicsQueue, "graphics queue must not be null");

    Queue::Spec computeSpec{m_device};
    computeSpec.queueFamilyIndex = *computeIndex;
    computeSpec.queueIndex = nextQueueIndices[*computeIndex]++;
    computeSpec.name = "compute";
    m_computeQueue = std::make_unique<Queue>(computeSpec);
    core::expect(m_computeQueue, "compute queue must not be null");

    Queue::Spec transferSpec{m_device};
    transferSpec.queueFamilyIndex = *transferIndex;
    transferSpec.queueIndex = nextQueueIndices[*transferIndex]++;
    transferSpec.name = "transfer";
    m_transferQueue = std::make_unique<Queue>(transferSpec);
    core::expect(m_transferQueue, "transfer queue must not be null");
}

void Context::createAllocator() {
    vma::AllocatorCreateInfo createInfo{};
    createInfo.instance = m_instance;
    createInfo.physicalDevice = m_physicalDevice;
    createInfo.device = m_device;
    createInfo.flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress;

    auto [result, allocator] = vma::createAllocator(createInfo);
    core::expect(result == vk::Result::eSuccess, "failed to create allocator");
    m_allocator = allocator;
}

} // namespace muon::graphics
