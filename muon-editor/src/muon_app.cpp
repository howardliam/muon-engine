#include "fmt/ranges.h"
#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/layer.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/event/event.hpp"
#include "muon/input/input_state.hpp"

namespace muon {

class TestLayer final : public Layer {
public:
    TestLayer() {}

    auto onAttach() -> void override { core::debug("attached test layer"); }
    auto onDetach() -> void override { core::debug("detached test layer"); }
    auto onUpdate() -> void override {}
};

class MuonEditor final : public Application {
public:
    MuonEditor(const std::string_view name, const vk::Extent2D &extent, const bool vsync, const WindowMode mode)
        : Application{name, extent, vsync, mode} {
        // override default window close handler if you need to
        auto success = m_dispatcher->unsubscribe<event::WindowCloseEvent>(m_onWindowClose);
        client::expect(success, "failed to unsubscribe from default window close event handler");
        m_onWindowClose = m_dispatcher->subscribe<event::WindowCloseEvent>([&](const auto &event) { m_running = false; });

        m_dispatcher->subscribe<event::WindowResizeEvent>([&](const auto &event) {
            m_context->getGraphicsQueue().get().waitIdle();
            m_renderer->rebuildSwapchain();
        });

        m_dispatcher->subscribe<event::MouseButtonEvent>([](const auto &event) {
            if (event.inputState == input::InputState::Pressed && event.button == input::MouseButton::Left) {
                core::info("hello!");
            }
        });

        m_dispatcher->subscribe<event::KeyEvent>([&](const auto &event) {
            if (event.keycode == input::KeyCode::V && event.mods.isCtrlDown() && event.inputState == input::InputState::Pressed) {
                core::info("{}", m_window->getClipboardContents());
            }
        });

        m_dispatcher->subscribe<event::FileDropEvent>([](const auto &event) { core::info("{}", fmt::join(event.paths, ", ")); });

        pushLayer(new TestLayer{});
    }

private:
};

auto createApplication(size_t argCount, char **argArray) -> Application * {
    return new MuonEditor{
        "Muon Editor", {1920, 1080},
         false, WindowMode::Windowed
    };
}

} // namespace muon
