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
     * @brief   attempts to parse the format type from the extension.
     *
     * @param   extension   the file extension.
     *
     * @return  optional format type if parsing was successful.
     */
    template<typename T>
    [[nodiscard]] std::optional<T> parseFormat(const std::string &extension);

    /**
     * @brief   attempts to parse the media type from the file with path.
     *
     * @param   path                the path of the file.
     * @param   expectedFileType    the expected type of the file.
     *
     * @return  optional media type if parsing was successful.
     */
    [[nodiscard]] std::optional<MediaType> parseMediaType(const std::filesystem::path &path, FileType expectedFileType);

    /**
     * @brief   reads the bytes of the file at the given path.
     *
     * @param   path    the path of the file.
     *
     * @return  optional vector of bytes if reading was successful.
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> readFile(const std::filesystem::path &path);

}
