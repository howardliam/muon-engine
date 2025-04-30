#include "muon/asset/model.hpp"
#include "muon/asset/file.hpp"
#include "muon/asset/model/gltf.hpp"

namespace muon::asset {


    std::optional<Scene> loadGltf(const std::filesystem::path &path) {
        auto mediaType = parseMediaType(path, FileType::Model);
        if (!mediaType) {
            return {};
        }

        if (!std::holds_alternative<ModelFormat>(mediaType->format)) {
            return {};
        }

        std::optional<GltfIntermediate> intermediate;

        switch (std::get<ModelFormat>(mediaType->format)) {
        case ModelFormat::GltfBinary:
            intermediate = intermediateFromGlb(path);
            break;

        case ModelFormat::GltfJson:
            intermediate = intermediateFromGltf(path);
            break;

        default:
            return {};
        }

        if (!intermediate) {
            return {};
        }

        return parseGltf(*intermediate);
    }

}
