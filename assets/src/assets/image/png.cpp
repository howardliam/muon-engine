#include "muon/assets/image/png.hpp"
#include "muon/assets/image.hpp"

#include <spng.h>

namespace muon::assets {

    std::optional<Image> decodePng(const std::vector<uint8_t> &encodedData) {
        spng_ctx *ctx = spng_ctx_new(0);

        spng_set_png_buffer(ctx, encodedData.data(), encodedData.size());

        spng_ihdr ihdr;
        spng_get_ihdr(ctx, &ihdr);

        size_t imageSize;
        spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &imageSize);

        std::vector<uint8_t> decodedData(imageSize);
        spng_decode_image(ctx, decodedData.data(), decodedData.size(), SPNG_FMT_RGBA8, 0);

        spng_ctx_free(ctx);

        ColorFormat format;
        if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR) {
            format = ColorFormat::Rgb;
        } else if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) {
            format = ColorFormat::Rgba;
        } else {
            return {};
        }

        return Image{
            {ihdr.width, ihdr.height},
            format,
            ihdr.bit_depth,
            decodedData,
        };
    }

    std::optional<std::vector<uint8_t>> encodePng(const Image &image) {
        auto colorType = [](const ColorFormat &format) {
            switch(format) {
            case ColorFormat::Rgb:
                return SPNG_COLOR_TYPE_TRUECOLOR;
            case ColorFormat::Rgba:
                return SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
            }
            return SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
        };

        spng_ctx *ctx = spng_ctx_new(SPNG_CTX_ENCODER);
        spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

        spng_ihdr ihdr{};
        ihdr.width = image.size.width;
        ihdr.height = image.size.height;
        ihdr.bit_depth = image.bitDepth;
        ihdr.color_type = colorType(image.format);

        int32_t result = spng_set_ihdr(ctx, &ihdr);
        if (result != 0) {
            return {};
        }

        result = spng_encode_image(ctx, image.data.data(), image.data.size(), SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
        if (result != 0) {
            return {};
        }

        size_t encodedSize{0};
        int32_t error{0};
        void *data = spng_get_png_buffer(ctx, &encodedSize, &error);
        if (error != 0) {
            return {};
        }

        std::vector<uint8_t> encodedData(static_cast<uint8_t *>(data), static_cast<uint8_t *>(data) + encodedSize);

        free(data);
        spng_ctx_free(ctx);

        return encodedData;
    }

}
