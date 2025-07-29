#pragma once

#include "muon/asset/manager.hpp"
#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/renderer.hpp"

#include <filesystem>
#include <memory>
#include <string_view>

namespace muon {

class Application : NoCopy, NoMove {
public:
    struct Spec {
        std::string name{"Muon Application"};
        std::filesystem::path workingDirectory;
        std::vector<const char *> args;
    };

public:
    Application(const Spec &spec);
    virtual ~Application();

    auto run() -> void;

public:
    auto getName() -> const std::string_view;
    static auto get() -> Application &;

protected:
    std::string m_name;

    std::unique_ptr<event::Dispatcher> m_dispatcher{nullptr};
    event::Dispatcher::Handle m_onWindowClose{};

    std::unique_ptr<Window> m_window{nullptr};
    std::unique_ptr<graphics::Context> m_context{nullptr};
    std::unique_ptr<graphics::Renderer> m_renderer{nullptr};
    std::unique_ptr<asset::Manager> m_assetManager{nullptr};

    bool m_running{true};

    static inline Application *s_instance{nullptr};
};

auto createApplication(const std::vector<const char *> &args) -> Application *;

} // namespace muon
