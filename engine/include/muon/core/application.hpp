#pragma once

#include "muon/core/assert.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/graphics/device_context.hpp"
#include "muon/graphics/renderer.hpp"
#include "muon/scripting/script_manager.hpp"
#include <cstdint>
#include <filesystem>

int main(int argc, char **argv);

namespace muon {

    struct ApplicationCommandLineArgs {
        int32_t count;
        char **args = nullptr;

        const char *operator[](int32_t index) const {
            MU_CORE_ASSERT(index < count);
            return args[index];
        }
    };

    struct ApplicationSpecification {
        std::string name{"Muon Application"};
        std::filesystem::path workingDirectory;
        ApplicationCommandLineArgs cliArgs;
    };

    class Application {
    public:
        Application(const ApplicationSpecification &spec);
        virtual ~Application();

        [[nodiscard]] Window &GetWindow() const;
        [[nodiscard]] gfx::DeviceContext &GetDeviceContext() const;

        [[nodiscard]] static Application &Get();

    private:
        void Run();

        friend int ::main(int argc, char **argv);

    private:
        event::Dispatcher m_dispatcher;

        std::unique_ptr<Window> m_window = nullptr;
        std::unique_ptr<gfx::DeviceContext> m_deviceContext = nullptr;
        std::unique_ptr<gfx::Renderer> m_renderer = nullptr;

        std::unique_ptr<ScriptManager> m_scriptManager = nullptr;

        bool m_running = true;

        static inline Application *s_instance = nullptr;
    };

    Application *CreateApplication(ApplicationCommandLineArgs args);

}
