#pragma once

#include <filesystem>
#include <optional>
#include <variant>
#include <vector>

namespace muon::assets {

    enum class FileType {
        Text,
        Image,
        Model,
        Audio,
        Font,
    };

    enum class TextFormat {
        Plain,
    };

    enum class ImageFormat {
        Png,
        Jpeg,
    };

    enum class ModelFormat {
        GltfBinary,
        GltfJson,
        Obj,
    };

    enum class AudioFormat {
        Opus,
        Wav,
    };

    enum class FontFormat {
        Otf,
        Ttf,
    };

    enum class OtherFormat {
        Unknown,
    };

    using FileFormat = std::variant<TextFormat, ImageFormat, ModelFormat, AudioFormat, FontFormat>;

    struct MediaType {
        FileType type;
        FileFormat format;
    };

    std::optional<MediaType> getMediaType(const std::vector<uint8_t> &fileData);
    std::optional<MediaType> getMediaType(const std::filesystem::path &path);

    std::optional<std::vector<uint8_t>> readFile(const std::filesystem::path &path);

}
