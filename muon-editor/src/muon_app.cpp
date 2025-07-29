#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"
#include "muon/core/expect.hpp"
#include "muon/event/event.hpp"
#include "fmt/ranges.h"

#include <filesystem>

namespace muon {

class MuonEditor final : public Application {
public:
    MuonEditor(const Spec &spec) : Application(spec) {
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
            if (event.keycode == input::KeyCode::V && event.mods.isCtrlDown()) {
                core::info("{}", m_window->getClipboardContents());
            }
        });

        m_dispatcher->subscribe<event::FileDropEvent>([](const auto &event) { core::info("{}", fmt::join(event.paths, ", ")); });
    }

private:
};

auto createApplication(const std::vector<const char *> &args) -> Application * {
    Application::Spec spec{};
    spec.name = "Muon Editor";
    spec.workingDirectory = std::filesystem::current_path();
    spec.args = args;

    return new MuonEditor(spec);
}

} // namespace muon
