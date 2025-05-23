#include "muon/asset/image.hpp"

#include <fstream>
#include <optional>
#include <spng.h>

namespace mu::asset {

    std::shared_ptr<Image> decodePng(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::binary | std::ios::ate};
        std::vector<uint8_t> encodedData(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(encodedData.data()), encodedData.size());

        spng_ctx *ctx = spng_ctx_new(0);

        spng_set_png_buffer(ctx, encodedData.data(), encodedData.size());

        spng_ihdr ihdr;
        spng_get_ihdr(ctx, &ihdr);

        spng_format format;
        uint8_t channels{0};
        switch (ihdr.color_type) {
        case SPNG_COLOR_TYPE_TRUECOLOR:
            format = SPNG_FMT_RGB8;
            channels = 3;
            break;

        case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
            format = SPNG_FMT_RGBA8;
            channels = 4;
            break;

        default:
            return nullptr;
        }

        size_t imageSize;
        spng_decoded_image_size(ctx, format, &imageSize);

        std::vector<uint8_t> decodedData(imageSize);
        spng_decode_image(ctx, decodedData.data(), decodedData.size(), format, 0);

        spng_ctx_free(ctx);

        std::shared_ptr asset = std::make_shared<Image>();
        asset->width = ihdr.width;
        asset->height = ihdr.height;
        asset->channels = channels;
        asset->bitDepth = ihdr.bit_depth;
        asset->data = decodedData;

        return asset;
    }

    std::optional<std::vector<uint8_t>> encodePng(const Image &image) {
        spng_ctx *ctx = spng_ctx_new(SPNG_CTX_ENCODER);
        spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

        spng_ihdr ihdr{};
        ihdr.width = image.width;
        ihdr.height = image.height;
        ihdr.bit_depth = image.bitDepth;

        switch (image.channels) {
        case 4:
            ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
            break;

        case 3:
            ihdr.color_type =  SPNG_COLOR_TYPE_TRUECOLOR;
            break;

        default:
            return {};
        }


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
