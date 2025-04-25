#include "muon/assets/file.hpp"

#include <fstream>
#include <print>
#include <regex>
#include <string>
#include <magic.h>

namespace muon::assets {

    namespace constants {
        static const std::regex mimeRegex("(\\w+)/(\\w+)");
    }

    std::optional<std::string> getMimeType(const std::vector<uint8_t> &fileData) {
        magic_t magicHandle = magic_open(MAGIC_MIME_TYPE);
        if (magicHandle == nullptr) {
            return {};
        }

        if (magic_load(magicHandle, nullptr) != 0) {
            return {};
        }

        std::string mime = magic_buffer(magicHandle, fileData.data(), fileData.size());

        magic_close(magicHandle);

        return mime;
    }

    std::optional<std::string> getMimeType(const std::filesystem::path &path) {
        magic_t magicHandle = magic_open(MAGIC_MIME_TYPE);
        if (magicHandle == nullptr) {
            return {};
        }

        if (magic_load(magicHandle, nullptr) != 0) {
            return {};
        }

        std::string mime = magic_file(magicHandle, path.string().c_str());

        magic_close(magicHandle);

        return mime;
    }

    std::optional<MediaType> mimeToMediaType(const std::string &mime) {
        std::smatch match;

        if (!std::regex_match(mime, match, constants::mimeRegex)) {
            return {};
        }

        std::string type = match[1];
        std::string format = match[2];

        MediaType mediaType{};

        if (type == "text") {
            mediaType.type = FileType::Text;

            if (format == "plain") {
                mediaType.format = TextFormat::Plain;
            }

        } else if (type == "image") {
            mediaType.type = FileType::Image;

            if (format == "png") {
                mediaType.format = ImageFormat::Png;
            } else if (format == "jpeg") {
                mediaType.format = ImageFormat::Jpeg;
            }

        } else if (type == "model") {
            mediaType.type = FileType::Model;

            if (format == "gltf+json") {
                mediaType.format = ModelFormat::GltfJson;
            } else if (format == "gltf-binary") {
                mediaType.format = ModelFormat::GltfBinary;
            } else if (format == "obj") {
                mediaType.format = ModelFormat::Obj;
            }

        } else if (type == "audio") {
            mediaType.type = FileType::Audio;

            if (format == "opus") {
                mediaType.format = AudioFormat::Opus;
            } else if (format == "wav") {
                mediaType.format = AudioFormat::Wav;
            }

        } else if (type == "font") {
            mediaType.type = FileType::Font;

            if (format == "otf") {
                mediaType.format = FontFormat::Otf;
            } else if (format == "ttf") {
                mediaType.format = FontFormat::Ttf;
            }

        } else {
            return {};
        }

        return mediaType;
    }

    std::optional<MediaType> getMediaType(const std::vector<uint8_t> &fileData) {
        auto mime = getMimeType(fileData);
        if (!mime.has_value()) {
            return {};
        }

        return mimeToMediaType(*mime);

        return {};
    }

    std::optional<MediaType> getMediaType(const std::filesystem::path &path) {
        auto mime = getMimeType(path);
        if (!mime.has_value()) {
            return {};
        }

        return mimeToMediaType(*mime);

        return {};
    }

    std::optional<std::vector<uint8_t>> readFile(const std::filesystem::path &path) {
        if (!std::filesystem::is_regular_file(path)) {
            return {};
        }

        std::ifstream file{path, std::ios::binary};
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

}
