#include "muon/assets/image.hpp"
#include "muon/assets/file.hpp"
#include "muon/assets/image/jpeg.hpp"
#include "muon/assets/image/png.hpp"

#include <print>
#include <variant>
#include <vector>
#include <spng.h>

namespace muon::assets {

    std::optional<Image> loadImage(const std::filesystem::path &path) {
        auto mediaType = parseMediaType(path, FileType::Image);
        if (!mediaType) {
            return {};
        }

        if (!std::holds_alternative<ImageFormat>(mediaType->format)) {
            return {};
        }

        auto encodedData = readFile(path);
        if (!encodedData) {
            return {};
        }

        return loadImage(*encodedData, std::get<ImageFormat>(mediaType->format));
    }

    std::optional<Image> loadImage(const std::vector<uint8_t> &encodedData, ImageFormat format) {
        switch (format) {
        case ImageFormat::Png:
            return decodePng(encodedData);

        case ImageFormat::Jpeg:
            return decodeJpeg(encodedData);

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
