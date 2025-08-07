#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/layer.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/event/event.hpp"
#include "muon/input/key.hpp"

#include <string_view>

namespace muon {

class TestLayer final : public Layer {
public:
    TestLayer() {}

    auto onAttach() -> void override { client::debug("attached test layer"); }
    auto onDetach() -> void override { client::debug("detached test layer"); }
    auto onUpdate() -> void override {}
};

class MuonEditor final : public Application {
public:
    MuonEditor(
        const std::string_view name,
        const vk::Extent2D &extent,
        const bool vSync,
        const WindowMode mode
    ) : Application{name, extent, vSync, mode} {
        // override default window close handler if you need to
        auto success = m_dispatcher->unsubscribe<event::WindowQuitEvent>(m_onWindowClose);
        client::expect(success, "failed to unsubscribe from default window close event handler");
        m_onWindowClose = m_dispatcher->subscribe<event::WindowQuitEvent>([&](const auto &event) { m_running = false; });

        m_dispatcher->subscribe<event::WindowResizeEvent>([&](const auto &event) {
            m_renderer->rebuildSwapchain();
        });

        m_dispatcher->subscribe<event::MouseButtonEvent>([](const auto &event) {
            if (event.down && event.button == input::MouseButton::Left) {
                client::info("hello!");
            }
        });

        m_fullscreen = mode == WindowMode::BorderlessFullscreen;

        m_dispatcher->subscribe<event::KeyboardEvent>([&](const auto &event) {
            if (event.scancode == input::Scancode::KeyC && event.mods.isCtrlDown() && event.down) {
                m_window->setClipboardText("foobar");
            }

            if (event.scancode == input::Scancode::KeyV && event.mods.isCtrlDown() && event.down) {
                if (const auto text = m_window->getClipboardText(); text) {
                    m_window->setTitle(*text);
                }
            }

            if (event.scancode == input::Scancode::Function11 && event.down) {
                m_window->setMode(m_fullscreen ? WindowMode::Windowed : WindowMode::BorderlessFullscreen);
                m_fullscreen = !m_fullscreen;
            }

            if (event.scancode == input::Scancode::Function2 && event.down) {
                m_window->beginTextInput();
            }

            if (event.scancode == input::Scancode::Function3 && event.down) {
                m_window->endTextInput();
            }

            if (event.scancode == input::Scancode::KeyQ && event.mods.isCtrlDown() && event.down) {
                m_running = false;
            }
        });

        m_dispatcher->subscribe<event::DropFileEvent>([](const auto &event) {
            client::info("{}", event.path);
        });

        m_dispatcher->subscribe<event::DropTextEvent>([](const auto &event) {
            client::info("{}", event.text);
        });

        m_dispatcher->subscribe<event::TextInputEvent>([](const auto &event) {
            client::info("{}", event.text);
        });

        pushLayer(new TestLayer{});
    }

private:
    bool m_fullscreen{false};
};

auto createApplication(size_t argCount, char **argArray) -> Application * {
    return new MuonEditor{
        "Muon Editor",
        {1920, 1080},
        false,
        WindowMode::Windowed
    };
}

} // namespace muon
