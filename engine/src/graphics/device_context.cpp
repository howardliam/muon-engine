#include "muon/graphics/device_context.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/window.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/gpu.hpp"
#include "muon/graphics/queue.hpp"
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
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
        MU_CORE_ASSERT(queueFamilyCount >= 1, "there must be at least one queue family");

        uint32_t queueCount = 0;
        for (const auto &family : queueFamilies) {
            queueCount += family.queueCount;
        }
        MU_CORE_ASSERT(queueCount >= 3, "there must be at least three queues available");
        MU_CORE_INFO("there are {} available queues", queueCount);

        uint32_t graphicsFamily = 0;
        uint32_t computeFamily = 0;
        uint32_t transferFamily = 0;
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            const auto &family = queueFamilies[i];
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = i;
            }
            if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                computeFamily = i;
            }
            if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                transferFamily = i;
            }
        }

        std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily, computeFamily, transferFamily };

        float queuePriority = 1.0;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());
        uint32_t index = 0;
        for (const auto family : uniqueQueueFamilies) {
            queueCreateInfos[index].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[index].queueFamilyIndex = family;
            queueCreateInfos[index].queueCount = 1;
            queueCreateInfos[index].pQueuePriorities = &queuePriority;

            index += 1;
        }

        VkPhysicalDeviceSynchronization2Features syncFeatures{};
        syncFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        syncFeatures.synchronization2 = true;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.pNext = &syncFeatures;
        dynamicRenderingFeatures.dynamicRendering = true;

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        indexingFeatures.pNext = &dynamicRenderingFeatures;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = &indexingFeatures;

        vkGetPhysicalDeviceFeatures2(m_physicalDevice, &deviceFeatures);

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = constants::requiredDeviceExtensions.size();
        createInfo.ppEnabledExtensionNames = constants::requiredDeviceExtensions.data();
        createInfo.pNext = &deviceFeatures;

        auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create a logical device");

        QueueSpecification graphicsSpec{};
        graphicsSpec.device = m_device;
        graphicsSpec.queueFamilyIndex = graphicsFamily;
        graphicsSpec.queueIndex = 0;
        graphicsSpec.name = "graphics";
        m_graphicsQueue = std::make_unique<Queue>(graphicsSpec);
        MU_CORE_ASSERT(m_graphicsQueue, "graphics queue must not be null");

        QueueSpecification computeSpec{};
        computeSpec.device = m_device;
        computeSpec.queueFamilyIndex = computeFamily;
        computeSpec.queueIndex = 0;
        computeSpec.name = "compute";
        m_computeQueue = std::make_unique<Queue>(computeSpec);
        MU_CORE_ASSERT(m_computeQueue, "compute queue must not be null");

        QueueSpecification transferSpec{};
        transferSpec.device = m_device;
        transferSpec.queueFamilyIndex = transferFamily;
        transferSpec.queueIndex = 0;
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
