#include "muon/graphics/device_context.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/window.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/gpu.hpp"
#include "muon/graphics/queue.hpp"
#include "muon/graphics/queue_info.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <set>
#include <vector>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace {

    #ifdef MU_DEBUG_ENABLED
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void *userData
    ) {
        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
                MU_VK_DEBUG(callbackData->pMessage);
                break;
            }

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
                MU_VK_INFO(callbackData->pMessage);
                break;
            }

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
                MU_VK_WARN(callbackData->pMessage);
                break;
            }

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
                MU_VK_ERROR(callbackData->pMessage);
                break;
            }

            default: {
                break;
            }
        }

        return false;
    }

    VkResult CreateDebugUtilsMessenger(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *createInfo,
        const VkAllocationCallbacks *allocator,
        VkDebugUtilsMessengerEXT *debugMessenger
    ) {
        auto procAddr = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(procAddr);
        if (vkCreateDebugUtilsMessengerEXT == nullptr) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        return vkCreateDebugUtilsMessengerEXT(instance, createInfo, allocator, debugMessenger);
    }

    void DestroyDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *allocator
    ) {
        auto procAddr = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(procAddr);
        if (vkDestroyDebugUtilsMessengerEXT == nullptr) {
            return;
        }

        return vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocator);
    }
    #endif

}

namespace muon::gfx {

    DeviceContext::DeviceContext(const DeviceContextSpecification &spec) {
        MU_CORE_ASSERT(spec.window, "a window must be present");

        CreateInstance(*spec.window);
        #ifdef MU_DEBUG_ENABLED
        CreateDebugMessenger();
        #endif
        CreateSurface(*spec.window);
        SelectPhysicalDevice();
        CreateLogicalDevice();
        CreateAllocator();

        MU_CORE_DEBUG("created device");
    }

    DeviceContext::~DeviceContext() {
        vmaDestroyAllocator(m_allocator);
        m_graphicsQueue.reset();
        m_computeQueue.reset();
        m_transferQueue.reset();
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        #ifdef MU_DEBUG_ENABLED
        DestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
        #endif
        vkDestroyInstance(m_instance, nullptr);

        MU_CORE_DEBUG("destroyed device");
    }

    VkInstance DeviceContext::GetInstance() const {
        return m_instance;
    }

    VkSurfaceKHR DeviceContext::GetSurface() const {
        return m_surface;
    }

    VkPhysicalDevice DeviceContext::GetPhysicalDevice() const {
        return m_physicalDevice;
    }

    VkDevice DeviceContext::GetDevice() const {
        return m_device;
    }

    Queue &DeviceContext::GetGraphicsQueue() const {
        return *m_graphicsQueue;
    }

    Queue &DeviceContext::GetComputeQueue() const {
        return *m_computeQueue;
    }

    Queue &DeviceContext::GetTransferQueue() const {
        return *m_transferQueue;
    }

    VmaAllocator DeviceContext::GetAllocator() const {
        return m_allocator;
    }

    void DeviceContext::CreateInstance(const Window &window) {
        auto extensions = window.GetRequiredExtensions();

        #ifdef MU_DEBUG_ENABLED
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        #endif

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Muon";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Muon";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        #ifdef MU_DEBUG_ENABLED
        const char *validationLayer = "VK_LAYER_KHRONOS_validation";

        auto checkValidationLayerSupport = [&validationLayer]() -> bool {
            uint32_t propertyCount = 0;
            vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(propertyCount);
            vkEnumerateInstanceLayerProperties(&propertyCount, availableLayers.data());

            bool layerFound = false;
            for (const auto &layerProperties : availableLayers) {
                if (std::strcmp(validationLayer, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) { return false; }
            return true;
        };

        if (checkValidationLayerSupport()) {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &validationLayer;
        } else {
            MU_CORE_WARN("the validation layer is not available");
        }
        #endif

        auto result = vkCreateInstance(&createInfo, nullptr, &m_instance);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create instance");
    }

    void DeviceContext::CreateDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;

        auto result = CreateDebugUtilsMessenger(m_instance, &createInfo, nullptr, &m_debugMessenger);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create debug messenger");
    }

    void DeviceContext::CreateSurface(const Window &window) {
        auto result = window.CreateSurface(m_instance, &m_surface);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create window surface");
    }

    void DeviceContext::SelectPhysicalDevice() {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());
        MU_CORE_ASSERT(physicalDevices.size() > 0, "no GPUs available with Vulkan support");

        if (physicalDevices.size() == 1) {
            auto suitability = GpuSuitability::DetermineSuitability(
                physicalDevices[0],
                m_surface,
                { constants::requiredDeviceExtensions.begin(), constants::requiredDeviceExtensions.end() }
            );

            if (suitability.IsSuitable()) {
                m_physicalDevice = physicalDevices[0];
            }
        } else {
            std::vector<std::pair<GpuSuitability, VkPhysicalDevice>> suitabilities{};

            for (const auto &physicalDevice : physicalDevices) {
                auto suitability = GpuSuitability::DetermineSuitability(
                    physicalDevice,
                    m_surface,
                    { constants::requiredDeviceExtensions.begin(), constants::requiredDeviceExtensions.end() }
                );

                if (suitability.IsSuitable()) {
                    suitabilities.push_back({ suitability, physicalDevice });
                }
            }

            auto sort = [](const std::pair<GpuSuitability, VkPhysicalDevice> &a, const std::pair<GpuSuitability, VkPhysicalDevice> &b) {
                return a.first.memorySize > b.first.memorySize;
            };
            std::sort(suitabilities.begin(), suitabilities.end(), sort);

            if (suitabilities.size() >= 1) {
                auto &[_, physicalDevice] = suitabilities.front();
                m_physicalDevice = physicalDevice;
            }
        }

        MU_CORE_ASSERT(m_physicalDevice, "unable to select a suitable GPU");
    }

    void DeviceContext::CreateLogicalDevice() {
        const QueueInfo queueInfo(m_physicalDevice, m_surface);
        MU_CORE_ASSERT(queueInfo.GetFamilyInfo().size() >= 1, "there must be at least one queue family");
        MU_CORE_ASSERT(queueInfo.GetTotalQueueCount() >= 3, "there must be at least three queues available");

        const auto queueFamilies = queueInfo.GetFamilyInfo();

        auto graphicsFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsGraphicsCapable(); });
        MU_CORE_ASSERT(graphicsFamily != queueFamilies.end(), "there must be a graphics capable queue family");
        MU_CORE_ASSERT(graphicsFamily->IsPresentCapable(), "the graphics capable queue family must support presentation");

        auto computeFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsComputeDedicated(); });
        if (computeFamily == queueFamilies.end()) {
            computeFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsComputeCapable(); });
        }
        MU_CORE_ASSERT(computeFamily != queueFamilies.end(), "there must be a compute capable queue family");

        auto transferFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsTransferDedicated(); });
        if (transferFamily == queueFamilies.end()) {
            transferFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsTransferCapable(); });
        }
        MU_CORE_ASSERT(transferFamily != queueFamilies.end(), "there must be a transfer capable queue family");

        std::map<uint32_t, uint32_t> queueCounts;
        queueCounts[graphicsFamily->index] += 1;
        queueCounts[computeFamily->index] += 1;
        queueCounts[transferFamily->index] += 1;
        std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily->index, computeFamily->index, transferFamily->index };

        std::vector<float> queuePriorities(uniqueQueueFamilies.size(), 1.0);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());
        uint32_t index = 0;
        for (const auto family : uniqueQueueFamilies) {
            auto &createInfo = queueCreateInfos[index];
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            createInfo.queueFamilyIndex = family;
            createInfo.queueCount = queueCounts[family];
            createInfo.pQueuePriorities = queuePriorities.data();
            index += 1;
        }

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
        meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
        meshShaderFeatures.meshShader = true;
        meshShaderFeatures.taskShader = true;

        VkPhysicalDeviceSynchronization2Features syncFeatures{};
        syncFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        syncFeatures.synchronization2 = true;
        syncFeatures.pNext = &meshShaderFeatures;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.dynamicRendering = true;
        dynamicRenderingFeatures.pNext = &syncFeatures;

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        indexingFeatures.descriptorBindingPartiallyBound = true;
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
        indexingFeatures.runtimeDescriptorArray = true;
        indexingFeatures.descriptorBindingVariableDescriptorCount = true;
        indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
        indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = true;
        indexingFeatures.descriptorBindingStorageImageUpdateAfterBind = true;
        indexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
        indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;
        indexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
        indexingFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
        indexingFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
        indexingFeatures.shaderStorageImageArrayNonUniformIndexing = true;
        indexingFeatures.pNext = &dynamicRenderingFeatures;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        vkGetPhysicalDeviceFeatures2(m_physicalDevice, &deviceFeatures);
        deviceFeatures.pNext = &indexingFeatures;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = constants::requiredDeviceExtensions.size();
        createInfo.ppEnabledExtensionNames = constants::requiredDeviceExtensions.data();
        createInfo.pNext = &deviceFeatures;

        auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create a logical device");

        std::map<uint32_t, uint32_t> nextQueueIndices;
        nextQueueIndices[graphicsFamily->index] = 0;
        nextQueueIndices[computeFamily->index] = 0;
        nextQueueIndices[transferFamily->index] = 0;

        QueueSpecification graphicsSpec{};
        graphicsSpec.device = m_device;
        graphicsSpec.queueFamilyIndex = graphicsFamily->index;
        graphicsSpec.queueIndex = nextQueueIndices[graphicsFamily->index]++;
        graphicsSpec.name = "graphics";
        m_graphicsQueue = std::make_unique<Queue>(graphicsSpec);
        MU_CORE_ASSERT(m_graphicsQueue, "graphics queue must not be null");

        QueueSpecification computeSpec{};
        computeSpec.device = m_device;
        computeSpec.queueFamilyIndex = computeFamily->index;
        computeSpec.queueIndex = nextQueueIndices[computeFamily->index]++;
        computeSpec.name = "compute";
        m_computeQueue = std::make_unique<Queue>(computeSpec);
        MU_CORE_ASSERT(m_computeQueue, "compute queue must not be null");

        QueueSpecification transferSpec{};
        transferSpec.device = m_device;
        transferSpec.queueFamilyIndex = transferFamily->index;
        transferSpec.queueIndex = nextQueueIndices[transferFamily->index]++;
        transferSpec.name = "transfer";
        m_transferQueue = std::make_unique<Queue>(transferSpec);
        MU_CORE_ASSERT(m_transferQueue, "transfer queue must not be null");
    }

    void DeviceContext::CreateAllocator() {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.instance = m_instance;
        createInfo.physicalDevice = m_physicalDevice;
        createInfo.device = m_device;

        auto result = vmaCreateAllocator(&createInfo, &m_allocator);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create allocator");
    }

}
