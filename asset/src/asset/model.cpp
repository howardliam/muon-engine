#include "muon/asset/model.hpp"
#include "muon/asset/error.hpp"
#include "muon/asset/file.hpp"
#include "muon/asset/model/gltf.hpp"

namespace muon::asset {


    std::expected<Scene, AssetLoadError> loadGltf(const std::filesystem::path &path) {
        auto mediaType = parseMediaType(path, FileType::Model);
        if (!mediaType) {
            return std::unexpected(AssetLoadError::InvalidFormat);
        }

        if (!std::holds_alternative<ModelFormat>(mediaType->format)) {
            return std::unexpected(AssetLoadError::InvalidFormat);
        }

        std::expected<GltfIntermediate, AssetLoadError> intermediate;

        switch (std::get<ModelFormat>(mediaType->format)) {
        case ModelFormat::GltfBinary:
            intermediate = intermediateFromGlb(path);
            break;

        case ModelFormat::GltfJson:
            intermediate = intermediateFromGltf(path);
            break;

        default:
            return std::unexpected(AssetLoadError::InvalidFormat);
        }

        if (!intermediate) {
            return std::unexpected(intermediate.error());
        }

        return parseGltf(*intermediate);
    }

}
