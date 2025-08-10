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

class test_layer final : public layer {
public:
    test_layer() {}

    auto on_attach() -> void override { client::debug("attached test layer"); }
    auto on_detach() -> void override { client::debug("detached test layer"); }
    auto on_update() -> void override {}
};

class muon_editor final : public application {
public:
    muon_editor(
        extent2d extent,
        const bool v_sync,
        const window_mode mode
    ) : application{"Muon Editor", extent, v_sync, mode} {
        // override default window close handler if you need to
        auto success = dispatcher_->unsubscribe<event::window_quit_event>(on_window_close_);
        client::expect(success, "failed to unsubscribe from default window close event handler");
        on_window_close_ = dispatcher_->subscribe<event::window_quit_event>([&](const auto &event) { running_ = false; });

        dispatcher_->subscribe<event::window_resize_event>([&](const auto &event) {
        });

        dispatcher_->subscribe<event::mouse_button_event>([](const auto &event) {
            if (event.down && event.button == input::MouseButton::Left) {
                client::info("hello!");
            }
        });

        fullscreen_ = mode == window_mode::borderless_fullscreen;

        dispatcher_->subscribe<event::keyboard_event>([&](const auto &event) {
            if (event.scancode == input::Scancode::KeyC && event.mods.isCtrlDown() && event.down) {
                window_->set_clipboard_text("foobar");
            }

            if (event.scancode == input::Scancode::KeyV && event.mods.isCtrlDown() && event.down) {
                if (const auto text = window_->get_clipboard_text(); text) {
                    window_->set_title(*text);
                }
            }

            if (event.scancode == input::Scancode::Function11 && event.down) {
                window_->set_mode(fullscreen_ ? window_mode::windowed : window_mode::borderless_fullscreen);
                fullscreen_ = !fullscreen_;
            }

            if (event.scancode == input::Scancode::Function2 && event.down) {
                window_->begin_text_input();
            }

            if (event.scancode == input::Scancode::Function3 && event.down) {
                window_->end_text_input();
            }

            if (event.scancode == input::Scancode::KeyQ && event.mods.isCtrlDown() && event.down) {
                running_ = false;
            }
        });

        dispatcher_->subscribe<event::drop_file_event>([](const auto &event) {
            client::info("{}", event.path);
        });

        dispatcher_->subscribe<event::drop_text_event>([](const auto &event) {
            client::info("{}", event.text);
        });

        dispatcher_->subscribe<event::text_input_event>([](const auto &event) {
            client::info("{}", event.text);
        });

        push_layer(new test_layer);

        client::info("created {}", name_);
    }

    ~muon_editor() {
        client::info("destroyed {}", name_);
    }

private:
    bool fullscreen_{false};
};

auto create_application(size_t count, char **arguments) -> application::pointer {
    return new muon_editor{
        {1920, 1080},
        false,
        window_mode::windowed
    };
}

} // namespace muon
