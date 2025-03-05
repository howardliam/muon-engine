#include "image.hpp"

#include <fstream>
#include <spdlog/spdlog.h>
#include <png.h>

namespace muon::assets {

    void pngReader(png_structp png, png_bytep data, png_size_t length) {
        std::istream *stream = static_cast<std::istream *>(png_get_io_ptr(png));
        if(!stream->read(reinterpret_cast<char *>(data), length)) {
            png_error(png, "Error reading PNG file");
        }
    }

    void readPngFile(std::filesystem::path path, std::vector<uint8_t> &imageData, PngProperties &properties) {
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            spdlog::error("Failed to create PNG read struct");
            return;
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, &info, nullptr);
            spdlog::error("Failed to create PNG info struct");
            return;
        }

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &info, nullptr);
            spdlog::error("Error during PNG read");
            return;
        }

        std::ifstream imageFile{path, std::ios::binary};
        if (!imageFile) {
            spdlog::error("Failed to open file: {}", path.string());
            return;
        }

        png_set_read_fn(png, static_cast<void *>(&imageFile), pngReader);
        png_read_info(png, info);

        properties.width = png_get_image_width(png, info);
        properties.height = png_get_image_height(png, info);
        properties.color_type = png_get_color_type(png, info);
        properties.bit_depth = png_get_bit_depth(png, info);

        png_read_update_info(png, info);

        png_size_t rowBytes = png_get_rowbytes(png, info);
        imageData.resize(rowBytes * properties.height);
        std::vector<png_bytep> rowPointers(properties.height);

        for (png_uint_32 i = 0; i < properties.height; ++i) {
            rowPointers[i] = imageData.data() + i * rowBytes;
        }

        png_read_image(png, rowPointers.data());
        png_destroy_read_struct(&png, &info, nullptr);
    }
}
