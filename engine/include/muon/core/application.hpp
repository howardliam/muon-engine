#pragma once

#include "muon/core/assert.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/graphics/device_context.hpp"
#include "muon/graphics/renderer.hpp"
#include "muon/scripting/script_manager.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>

auto main(int32_t argc, char **argv) -> int32_t;

namespace muon {

    struct ApplicationCommandLineArgs {
        int32_t count;
        char **args{nullptr};

        const char *operator[](int32_t index) const {
            MU_CORE_ASSERT(index < count);
            return args[index];
        }
    };

    struct ApplicationSpecification {
        std::string name = "Muon Application";
        std::filesystem::path workingDirectory;
        ApplicationCommandLineArgs cliArgs;
    };

    class Application {
    public:
        Application(const ApplicationSpecification &spec);
        virtual ~Application();

        [[nodiscard]] Window &GetWindow() const;
        [[nodiscard]] graphics::DeviceContext &GetDeviceContext() const;

        [[nodiscard]] static Application &Get();

    private:
        void Run();

        friend auto ::main(int32_t argc, char **argv) -> int32_t;

    protected:
        std::unique_ptr<event::Dispatcher> m_dispatcher{nullptr};
        std::optional<event::Dispatcher::Handle> m_onWindowClose{std::nullopt};

        std::unique_ptr<Window> m_window{nullptr};
        std::unique_ptr<graphics::DeviceContext> m_deviceContext{nullptr};
        std::unique_ptr<graphics::Renderer> m_renderer{nullptr};

        std::unique_ptr<ScriptManager> m_scriptManager{nullptr};

        bool m_running{true};

        static inline Application *s_instance{nullptr};
    };

    auto CreateApplication(ApplicationCommandLineArgs args) -> Application *;

}
