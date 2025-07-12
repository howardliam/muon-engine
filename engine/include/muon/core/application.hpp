#pragma once

#include "muon/core/assert.hpp"
#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/renderer.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>

auto main(int32_t argc, char **argv) -> int32_t;

namespace muon {

struct ApplicationCommandLineArgs {
    int32_t count;
    char **args{nullptr};

    auto operator[](int32_t index) const -> const char * {
        MU_CORE_ASSERT(index < count);
        return args[index];
    }
};

class Application : NoCopy, NoMove {
public:
    struct Spec {
        std::string name = "Muon Application";
        std::filesystem::path workingDirectory;
        ApplicationCommandLineArgs cliArgs;
    };

public:
    Application(const Spec &spec);
    virtual ~Application();

    [[nodiscard]] static auto Get() -> Application &;

private:
    auto Run() -> void;

    friend auto ::main(int32_t argc, char **argv) -> int32_t;

protected:
    std::unique_ptr<event::Dispatcher> m_dispatcher{nullptr};
    std::optional<event::Dispatcher::Handle> m_onWindowClose{std::nullopt};

    std::unique_ptr<Window> m_window{nullptr};
    std::unique_ptr<graphics::Context> m_context{nullptr};
    std::unique_ptr<graphics::Renderer> m_renderer{nullptr};

    bool m_running{true};

    static inline Application *s_instance{nullptr};
};

auto CreateApplication(ApplicationCommandLineArgs args) -> Application *;

} // namespace muon
