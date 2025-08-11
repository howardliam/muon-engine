#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/layer.hpp"
#include "muon/core/log.hpp"
#include "muon/core/types.hpp"
#include "muon/core/window.hpp"
#include "muon/event/event.hpp"
#include "muon/input/key.hpp"

#include <string_view>

namespace muon {

class TestLayer final : public Layer {
public:
    TestLayer() {}

    auto on_attach() -> void override { client::debug("attached test layer"); }
    auto on_detach() -> void override { client::debug("detached test layer"); }
    auto on_update() -> void override {}
};

class MuonEditor final : public Application {
public:
    MuonEditor(
        Extent2D extent,
        bool v_sync,
        WindowMode mode
    ) : Application{"Muon Editor", extent, v_sync, mode} {
        // override default window close handler if you need to
        auto success = dispatcher_->unsubscribe<event::WindowQuit>(on_window_close_);
        client::expect(success, "failed to unsubscribe from default window close event handler");
        on_window_close_ = dispatcher_->subscribe<event::WindowQuit>([&](const auto &event) { running_ = false; });

        dispatcher_->subscribe<event::MouseButton>([](const auto &event) {
            if (event.down && event.button == input::MouseButton::Left) {
                client::info("hello!");
            }
        });

        fullscreen_ = mode == WindowMode::BorderlessFullscreen;

        dispatcher_->subscribe<event::Keyboard>([&](const auto &event) {
            if (event.scancode == input::Scancode::KeyC && event.mods.is_ctrl_down() && event.down) {
                window_->set_clipboard_text("foobar");
            }

            if (event.scancode == input::Scancode::KeyV && event.mods.is_ctrl_down() && event.down) {
                if (const auto text = window_->get_clipboard_text(); text) {
                    window_->set_title(*text);
                }
            }

            if (event.scancode == input::Scancode::Function11 && event.down) {
                window_->set_mode(fullscreen_ ? WindowMode::Windowed : WindowMode::BorderlessFullscreen);
                fullscreen_ = !fullscreen_;
            }

            if (event.scancode == input::Scancode::Function2 && event.down) {
                window_->begin_text_input();
            }

            if (event.scancode == input::Scancode::Function3 && event.down) {
                window_->end_text_input();
            }

            if (event.scancode == input::Scancode::KeyQ && event.mods.is_ctrl_down() && event.down) {
                running_ = false;
            }
        });

        dispatcher_->subscribe<event::DropFile>([](const auto &event) {
            client::info("{}", event.path);
        });

        dispatcher_->subscribe<event::DropText>([](const auto &event) {
            client::info("{}", event.text);
        });

        dispatcher_->subscribe<event::TextInput>([](const auto &event) {
            client::info("{}", event.text);
        });

        push_layer(new TestLayer);

        client::info("created {}", name_);
    }

    ~MuonEditor() {
        client::info("destroyed {}", name_);
    }

private:
    bool fullscreen_{false};
};

auto create_application(size_t count, char **arguments) -> Application::Pointer {
    return new MuonEditor{
        {1920, 1080},
        false,
        WindowMode::Windowed
    };
}

} // namespace muon
