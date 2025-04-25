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

    /**
     * @brief   gets the media type from the file data.
     *
     * @param   fileData    vector of bytes of file data.
     *
     * @return  optional media type if parsing was successful.
     */
    [[nodiscard]] std::optional<MediaType> getMediaType(const std::vector<uint8_t> &fileData);

    /**
     * @brief   gets the media type from the file data with path.
     *
     * @param   path    the path of the file.
     *
     * @return  optional media type if parsing was successful.
     */
    [[nodiscard]] std::optional<MediaType> getMediaType(const std::filesystem::path &path);

    /**
     * @brief   reads the bytes of the file at the given path.
     *
     * @param   path    the path of the file.
     *
     * @return  optional vector of bytes if reading was successful.
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> readFile(const std::filesystem::path &path);

}
