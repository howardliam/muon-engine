#include "muon/assets/file.hpp"

#include <fstream>
#include <string>

namespace muon::assets {

    template<>
    std::optional<TextFormat> parseFormat(const std::string &extension) {

        if (extension == "txt") {
            return TextFormat::Plain;
        }

        return {};
    }

    template<>
    std::optional<ImageFormat> parseFormat(const std::string &extension) {

        if (extension == ".jpeg" || extension == ".jpg") {
            return ImageFormat::Jpeg;
        } else if (extension == ".png") {
            return ImageFormat::Png;
        }

        return {};
    }

    template<>
    std::optional<ModelFormat> parseFormat(const std::string &extension) {

        if (extension == ".gltf") {
            return ModelFormat::GltfJson;
        } else if (extension == ".glb") {
            return ModelFormat::GltfBinary;
        } else if (extension == ".obj") {
            return ModelFormat::Obj;
        }

        return {};
    }

    template<>
    std::optional<AudioFormat> parseFormat(const std::string &extension) {

        if (extension == ".opus") {
            return AudioFormat::Opus;
        } else if (extension == ".wav") {
            return AudioFormat::Wav;
        }

        return {};
    }

    template<>
    std::optional<FontFormat> parseFormat(const std::string &extension) {

        if (extension == ".otf") {
            return FontFormat::Otf;
        } else if (extension == ".ttf") {
            return FontFormat::Ttf;
        }

        return {};
    }

    std::optional<MediaType> parseMediaType(const std::filesystem::path &path, FileType expectedFileType) {
        std::string extension = path.filename().extension().string();

        std::optional<FileFormat> format;

        switch (expectedFileType) {
        case FileType::Text:
            format = parseFormat<TextFormat>(extension);
            break;
        case FileType::Image:
            format = parseFormat<ImageFormat>(extension);
            break;
        case FileType::Model:
            format = parseFormat<ModelFormat>(extension);
            break;
        case FileType::Audio:
            format = parseFormat<AudioFormat>(extension);
            break;
        case FileType::Font:
            format = parseFormat<FontFormat>(extension);
            break;
        }

        if (!format) {
            return {};
        }

        return MediaType{
            expectedFileType,
            *format,
        };
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
