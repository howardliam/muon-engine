#include "muon/engine/device.hpp"

#include "muon/engine/window.hpp"
#include <SDL3/SDL_vulkan.h>
#include <print>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

namespace muon::engine {

    /**
     * @brief   Callback function for Vulkan validation layers.
     *
     * @param   messageSeverity the severity of the message.
     * @param   messageType     what type is the message.
     * @param   callbackData    callback parameters.
     * @param   userData        arbitrary data to be loaded into the callback.
     *
     * @return  false.
    */
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT *callbackData,
        void *userData
    ) {
        std::println("{}", callbackData->pMessage);

        return false;
    }

    /**
     * @brief   Creates the debug utils messenger.
     *
     * @param   instance        the instance to create debug messenger for.
     * @param   createInfo      what info to create messenger with.
     * @param   allocator       pointer to an allocator.
     * @param   debugMessenger  handle to a debug messenger to be created.
     *
     * @return  result based on the function being executed, otherwise an error.
    */
    vk::Result createDebugUtilsMessenger(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *createInfo,
        const VkAllocationCallbacks *allocator,
        VkDebugUtilsMessengerEXT *debugMessenger
    ) {
        auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (function == nullptr) {
            return vk::Result::eErrorExtensionNotPresent;
        }

        return static_cast<vk::Result>(function(instance, createInfo, allocator, debugMessenger));
    }


    /**
     * @brief   Destroys the debug utils messenger.
     *
     * @param   instance        the instance to destroy debug messenger for.
     * @param   debugMessenger  handle to a debug messenger to be destroyed.
     * @param   allocator       pointer to an allocator.
    */
    void destroyDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *allocator
    ) {
        auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (function == nullptr) {
            return;
        }

        return function(instance, debugMessenger, allocator);
    }

    Device::Device(window::Window &window) : window(window) {
        createInstance();
        createDebugMessenger();
        createSurface();
        // selectPhysicalDevice();
        // createLogicalDevice();
        // createAllocator();
    }

    Device::~Device() {
        allocator.destroy();

        device.destroy(nullptr);

        instance.destroySurfaceKHR(surface, nullptr);

        destroyDebugUtilsMessenger(instance, debugMessenger, nullptr);

        instance.destroy(nullptr);
    }


    vk::Instance Device::getInstance() {
        return instance;
    }

    vk::SurfaceKHR Device::getSurface() {
        return surface;
    }

    vk::PhysicalDevice Device::getPhysicalDevice() {
        return physicalDevice;
    }

    vk::Device Device::getDevice() {
        return device;
    }

    vk::Queue Device::getGraphicsQueue() {
        return graphicsQueue;
    }

    vk::Queue Device::getPresentQueue() {
        return presentQueue;
    }

    vma::Allocator Device::getAllocator() {
        return allocator;
    }

    void Device::createInstance() {
        auto checkValidationLayerSupport = [&]() -> bool {
            std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

            for (const auto layerName : validationLayers) {
                bool layerFound = false;
                for (const auto &layerProperties : availableLayers) {
                    if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        };

        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers were requested but are not available");
        }

        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "Muon";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Muon";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.pNext = nullptr;

        auto getRequiredExtensions = [](bool enableValidationLayers) -> std::vector<const char *> {
            uint32_t sdlExtensionCount = 0;
            const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

            if (sdlExtensions == nullptr) {
                throw std::runtime_error(std::format("failed to get instance extensions from SDL: {}", SDL_GetError()));
            }

            std::vector<const char *> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        };

        auto extensions = getRequiredExtensions(enableValidationLayers);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        auto result = vk::createInstance(&createInfo, nullptr, &instance);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create instance");
        }
    }

    void Device::createDebugMessenger() {
        if (!enableValidationLayers) {
            return;
        }

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        createInfo.messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        createInfo.pfnUserCallback = debugCallback;

        auto result = createDebugUtilsMessenger(
            instance,
            reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(&createInfo),
            nullptr,
            reinterpret_cast<VkDebugUtilsMessengerEXT *>(&debugMessenger)
        );

        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create debug messenger");
        }
    }

    void Device::createSurface() {
        auto success = window.createSurface(instance, reinterpret_cast<VkSurfaceKHR *>(&surface));
        if (!success) {
            throw std::runtime_error("failed to create surface");
        }
    }
}
