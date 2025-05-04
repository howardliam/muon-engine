#include "muon/asset/image/jpeg.hpp"
#include "muon/asset/image.hpp"

#include <turbojpeg.h>
#include <fstream>

namespace muon::asset {

    bool JpegHandler::supports(const std::string &extension) const {
        return extension == "jpeg" || extension == "jpg";
    }

    std::shared_ptr<Asset> JpegHandler::load(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::binary | std::ios::ate};
        std::vector<uint8_t> encodedData(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(encodedData.data()), encodedData.size());

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

        std::shared_ptr asset = std::make_shared<ImageAsset>();
        asset->width = static_cast<uint32_t>(width);
        asset->height = static_cast<uint32_t>(height);
        asset->channels = 3;
        asset->bitDepth = 8;
        asset->data = decodedData;

        return asset;
    }

}
