#include "muon/assets/image.hpp"

#include <print>
#include <spng.h>
#include <vector>

namespace muon::assets {

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

}
