#include "muon/assets/image.hpp"
#include "muon/assets/file.hpp"
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

    ImageData loadImagePng(std::vector<char> imageData) {
        spng_ctx *ctx = spng_ctx_new(0);

        spng_set_png_buffer(ctx, imageData.data(), imageData.size());

        spng_ihdr ihdr;
        spng_get_ihdr(ctx, &ihdr);

        size_t imageSize;
        spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &imageSize);

        std::vector<char> decodedImage(imageSize);
        spng_decode_image(ctx, decodedImage.data(), decodedImage.size(), SPNG_FMT_RGBA8, 0);

        spng_ctx_free(ctx);

        return {
            ihdr.width,
            ihdr.height,
            ihdr.bit_depth,
            decodedImage,
        };
    }

    std::vector<uint8_t> encodeImagePng(const ImageData &imageData) {
        spng_ctx *ctx = spng_ctx_new(SPNG_CTX_ENCODER);
        spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

        spng_ihdr ihdr{};
        ihdr.width = imageData.width;
        ihdr.height = imageData.height;
        ihdr.bit_depth = imageData.bitDepth;
        ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;

        int32_t result = spng_set_ihdr(ctx, &ihdr);
        if (result > 0) {
            std::println("spng_set_ihdr() error: {}", spng_strerror(result));
        }

        result = spng_encode_image(ctx, imageData.data.data(), imageData.data.size(), SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
        if (result > 0) {
            std::println("spng_encode_image() error: {}", spng_strerror(result));
        }

        size_t encodedSize{0};
        int32_t error{0};
        void *data = spng_get_png_buffer(ctx, &encodedSize, &error);
        if (error > 0) {
            std::println("spng_get_png_buffer() error: {}", spng_strerror(result));
        }

        std::vector<uint8_t> pngData(static_cast<uint8_t *>(data), static_cast<uint8_t *>(data) + encodedSize);

        free(data);
        spng_ctx_free(ctx);

        return pngData;
    }
}
