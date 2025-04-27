#include "muon/asset/image/jpeg.hpp"

#include <turbojpeg.h>

namespace muon::asset {

    std::optional<Image> decodeJpeg(const std::vector<uint8_t> &encodedData) {
        tjhandle handle = tjInitDecompress();

        int32_t width;
        int32_t height;
        int32_t subsamp;
        int32_t colorspace;

        if (tjDecompressHeader3(handle, encodedData.data(), encodedData.size(), &width, &height, &subsamp, &colorspace) != 0) {
            tjDestroy(handle);
            return {};
        }

        int32_t totalPixels = width * height;
        const int32_t bytesPerPixel = 3;
        std::vector<uint8_t> decodedData(totalPixels * bytesPerPixel);

        if (tjDecompress2(handle, encodedData.data(), encodedData.size(), decodedData.data(), width, 0, height, TJPF_RGB, TJFLAG_FASTDCT) != 0) {
            tjDestroy(handle);
            return {};
        }

        return Image{
            {static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
            ColorFormat::Rgb,
            8,
            decodedData,
        };
    }

}
