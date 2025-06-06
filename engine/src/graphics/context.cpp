#include "muon/graphics/context.hpp"

#include "muon/core/application.hpp"
#include "muon/core/window.hpp"
#include "muon/core/log.hpp"

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
        auto extensions = Application::Get().GetWindow().requiredExtensions();

        #ifdef MU_DEBUG_ENABLED
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        const char *validationLayer = "VK_LAYER_KHRONOS_validation";

        auto checkValidationLayerSupport = [&validationLayer]() -> bool {

            std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

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

        bool support = checkValidationLayerSupport();

        MU_CORE_WARN("validation layers not available");
        #endif

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "Muon";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Muon";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        #ifdef MU_DEBUG_ENABLED
        if (support) {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &validationLayer;
        }
        #endif

        auto result = vk::createInstance(&createInfo, nullptr, &m_instance);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create instance");
    }

    void Context::CreateDebugMessenger() {

    }

    void Context::CreateSurface() {

    }

    void Context::SelectPhysicalDevice() {

    }

    void Context::CreateLogicalDevice() {

    }

    void Context::CreateAllocator() {

    }

    void Context::CreateCommandPool() {

    }

    void Context::CreateProfiler() {

    }

}
