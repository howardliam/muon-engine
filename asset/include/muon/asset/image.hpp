#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

namespace mu::asset {

    struct Image {
        uint32_t width;
        uint32_t height;

        uint8_t channels;
        uint8_t bitDepth;

        std::vector<uint8_t> data{};
    };

    std::shared_ptr<Image> decodePng(const std::filesystem::path &path);
    std::optional<std::vector<uint8_t>> encodePng(const Image &image);

}
