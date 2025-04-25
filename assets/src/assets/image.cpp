#include "muon/assets/image.hpp"
#include "muon/assets/image/png.hpp"

#include <print>
#include <vector>
#include <spng.h>

namespace muon::assets {

    std::optional<Image> loadImage(const std::filesystem::path &path) {
        auto encodedData = readFile(path);
        if (!encodedData.has_value()) {
            return {};
        }

        // todo libmagic

        return loadImage(*encodedData, ImageFormat::Png);
    }

    std::optional<Image> loadImage(const std::vector<uint8_t> &encodedData, ImageFormat format) {
        switch (format) {
        case ImageFormat::Png:
            return decodePng(encodedData);

        case ImageFormat::Jpeg:
        default:
            return {};
        }
    }

    std::optional<std::vector<uint8_t>> encodeImage(const Image &image, ImageFormat format) {
        switch (format) {
        case ImageFormat::Png:
            return encodePng(image);

        case ImageFormat::Jpeg:
        default:
            return {};
        }
    }

}
