#pragma once

#include <cstdint>
#include <filesystem>
#include "muon/core/assert.hpp"
#include "muon/core/event/dispatcher.hpp"

int main(int argc, char **argv);

namespace muon {

    class Window;
    class Device;
    class FrameHandler;
    class ScriptManager;

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

        [[nodiscard]] Window &GetWindow();

        [[nodiscard]] static Application &Get();

    private:
        void run();

        friend int ::main(int argc, char **argv);

    private:
        EventDispatcher m_dispatcher;

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Device> m_device;
        std::unique_ptr<FrameHandler> m_frameHandler;
        std::unique_ptr<ScriptManager> m_scriptManager;

        bool m_running{true};

        static inline Application *s_instance{nullptr};
    };

    Application *createApplication(ApplicationCommandLineArgs args);

}
