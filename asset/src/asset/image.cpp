#include "muon/asset/image.hpp"
#include "muon/asset/file.hpp"
#include "muon/asset/image/jpeg.hpp"
#include "muon/asset/image/png.hpp"

#include <print>
#include <variant>
#include <vector>
#include <spng.h>

namespace muon::asset {

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
