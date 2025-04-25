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

        std::vector<uint8_t> decodedImage(imageSize);
        spng_decode_image(ctx, decodedImage.data(), decodedImage.size(), SPNG_FMT_RGBA8, 0);

        spng_ctx_free(ctx);

        ColorFormat format = ColorFormat::Rgba;
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
            decodedImage,
        };
    }

}
