#include "muon/graphics/context.hpp"

#include "muon/core/application.hpp"
#include "muon/core/window.hpp"
#include "muon/core/log.hpp"
#include "muon/debug/profiler.hpp"
#include "muon/graphics/gpu.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#ifdef MU_DEBUG_ENABLED

namespace {

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

}

#endif

namespace muon::gfx {

    Context::Context() {
        CreateInstance();
        #ifdef MU_DEBUG_ENABLED
        CreateDebugMessenger();
        #endif
        CreateSurface();
        SelectPhysicalDevice();
        CreateLogicalDevice();
        CreateAllocator();
        CreateCommandPool();
        CreateProfiler();

        MU_CORE_DEBUG("created device");
    }

    Context::~Context() {
        #ifdef MU_DEBUG_ENABLED
        Profiler::DestroyContext();
        #endif
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        #ifdef MU_DEBUG_ENABLED
        DestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
        #endif
        vkDestroyInstance(m_instance, nullptr);

        MU_CORE_DEBUG("destroyed device");
    }

    VkInstance Context::GetInstance() const {
        return m_instance;
    }

    VkSurfaceKHR Context::GetSurface() const {
        return m_surface;
    }

    VkPhysicalDevice Context::GetPhysicalDevice() const {
        return m_physicalDevice;
    }

    VkDevice Context::GetDevice() const {
        return m_device;
    }

    VkQueue Context::GetGraphicsQueue() const {
        return m_graphicsQueue;
    }

    VkQueue Context::GetComputeQueue() const {
        return m_computeQueue;
    }

    VkQueue Context::GetPresentQueue() const {
        return m_presentQueue;
    }

    VmaAllocator Context::GetAllocator() const {
        return m_allocator;
    }

    VkCommandPool Context::GetCommandPool() const {
        return m_commandPool;
    }

    void Context::CreateInstance() {
        auto extensions = Application::Get().GetWindow().RequiredExtensions();

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

            if (!layerFound) {
                return false;
            }

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

    void Context::CreateDebugMessenger() {
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
        MU_CORE_TRACE("created debug messenger");
    }

    void Context::CreateSurface() {
        auto result = Application::Get().GetWindow().CreateSurface(m_instance, &m_surface);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create window surface");
        MU_CORE_TRACE("created surface");
    }

    void Context::SelectPhysicalDevice() {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());
        MU_CORE_ASSERT(physicalDevices.size() > 0, "no GPUs available with Vulkan support");

        for (const auto &physicalDevice : physicalDevices) {
            auto suitability = GpuSuitability::DetermineSuitability(physicalDevice, m_surface, m_deviceExtensions);

            if (suitability.IsSuitable()) {
                m_physicalDevice = physicalDevice;
                break;
            }
        }

        MU_CORE_ASSERT(m_physicalDevice, "unable to select a suitable GPU");

        VkPhysicalDeviceProperties2 deviceProperties{};
        deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);
    }

    void Context::CreateLogicalDevice() {
        float queuePriority = 1.0;

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = 0;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

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
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
        createInfo.pNext = &deviceFeatures;

        auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create a logical device");

        vkGetDeviceQueue(m_device, 0, 0, &m_graphicsQueue);
    }

    void Context::CreateAllocator() {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.instance = m_instance;
        createInfo.physicalDevice = m_physicalDevice;
        createInfo.device = m_device;

        auto result = vmaCreateAllocator(&createInfo, &m_allocator);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create allocator");
    }

    void Context::CreateCommandPool() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = 0;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create command pool");
    }

    void Context::CreateProfiler() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        auto result = vkAllocateCommandBuffers(m_device, &allocateInfo, &cmd);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate profiler creation command buffer");

        Profiler::CreateContext(m_physicalDevice, m_device, m_graphicsQueue, cmd);
    }

}
