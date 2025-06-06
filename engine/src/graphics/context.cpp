#include "muon/graphics/context.hpp"

#include "muon/core/application.hpp"
#include "muon/core/window.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/gpu.hpp"
#include <bitset>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vulkan/vulkan_core.h>

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
    }

    Context::~Context() {

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

            uint32_t propertyCount{0};
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
        MU_CORE_TRACE("created instance");
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

        // std::unordered_map<GpuSuitability, VkPhysicalDevice> physicalDeviceSuitabilities{};

        auto determineSuitability = [](VkPhysicalDevice physicalDevice, const std::vector<const char *> &deviceExtensions) -> GpuSuitability {
            GpuSuitability suitability;

            VkPhysicalDeviceProperties2 deviceProperties{};
            deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);

            if (deviceProperties.properties.apiVersion >= VK_API_VERSION_1_3) {
                suitability.SetFlag(SuitabilityFlagBits::ApiVersion13);
            }

            if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                suitability.SetFlag(SuitabilityFlagBits::DiscreteGPU);
            }

            if (deviceProperties.properties.limits.maxPushConstantsSize >= 128) {
                suitability.SetFlag(SuitabilityFlagBits::MinimumPushConstantSize);
            }

            uint32_t availableExtensionCount{0};
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());

            std::unordered_set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
            for (const auto &extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            if (requiredExtensions.empty()) {
                suitability.SetFlag(SuitabilityFlagBits::HasRequiredExtensions);
            }

            VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
            indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

            VkPhysicalDeviceFeatures2 deviceFeatures{};
            deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures.pNext = &indexingFeatures;
            vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

            uint32_t queueFamilyPropertyCount{0};
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

            if (queueFamilyPropertyCount == 1) {
                suitability.SetFlag(SuitabilityFlagBits::UnifiedQueue);
            }

            for (const auto &props : queueFamilyProperties) {
                MU_CORE_INFO("queue count: {}", props.queueCount);
                MU_CORE_INFO("supports graphics {}", (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0);
                MU_CORE_INFO("supports compute {}", (props.queueFlags & VK_QUEUE_COMPUTE_BIT) > 0);
                MU_CORE_INFO("supports transfer {}", (props.queueFlags & VK_QUEUE_TRANSFER_BIT) > 0);
            }

            MU_CORE_INFO("{}", deviceProperties.properties.deviceName);

            return suitability;
        };

        for (const auto &physicalDevice : physicalDevices) {
            MU_CORE_INFO("{}", determineSuitability(physicalDevice, m_deviceExtensions).ToString());
            // physicalDeviceSuitabilities[determineSuitability(physicalDevice, m_deviceExtensions)] = physicalDevice;
        }

        MU_CORE_INFO("{}", 1ul << 47);

        MU_CORE_ASSERT(m_physicalDevice, "unable to select a suitable GPU");

        VkPhysicalDeviceProperties2 deviceProperties{};
        deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);
        MU_CORE_TRACE("selected GPU: {}", deviceProperties.properties.deviceName);
    }

    void Context::CreateLogicalDevice() {

    }

    void Context::CreateQueues() {

    }

    void Context::CreateAllocator() {

    }

    void Context::CreateCommandPool() {

    }

    void Context::CreateProfiler() {

    }

}
