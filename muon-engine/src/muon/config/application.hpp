#pragma once

#include "muon/core/serialization.hpp"

#include <cstdint>

namespace muon::config {

struct Application {
    uint32_t width{0};
    uint32_t height{0};
    bool vsync{false};

    static auto serialize(const Application &application) -> SerializationResult {
        toml::table table;

        table.insert("width", application.width);
        table.insert("height", application.height);
        table.insert("vsync", application.vsync);

        return table;
    }

    static auto deserialize(const toml::table &table) -> DeserializationResult<Application> {
        Application application;

        auto width = table["width"].value<uint32_t>();
        if (!width) {
            std::unexpected(DeserializationError::FieldNotPresent);
        }

        auto height = table["height"].value<uint32_t>();
        if (!height) {
            std::unexpected(DeserializationError::FieldNotPresent);
        }

        auto vsync = table["vsync"].value<bool>();
        if (!vsync) {
            std::unexpected(DeserializationError::FieldNotPresent);
        }

        application.width = *width;
        application.height = *height;
        application.vsync = *vsync;

        return application;
    }
};

} // namespace muon::config
