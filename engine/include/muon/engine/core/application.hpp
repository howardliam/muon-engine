#pragma once

#include <cstdint>
#include <filesystem>
#include "muon/engine/core/assert.hpp"

int main(int argc, char **argv);

namespace muon {

    class Application {
    public:
        struct CommandLineArgs {
            int32_t count;
            char **args = nullptr;

            const char *operator[](int32_t index) const {
                MU_CORE_ASSERT(index < count);
                return args[index];
            }
        };

        struct Specification {
            std::string name{"Muon Application"};
            std::filesystem::path workingDirectory;
            CommandLineArgs cliArgs;
        };

        Application(const Specification &spec);
        virtual ~Application();

    private:

        void run();

        friend int ::main(int argc, char **argv);
    };

    Application *createApplication(Application::CommandLineArgs args);

}
