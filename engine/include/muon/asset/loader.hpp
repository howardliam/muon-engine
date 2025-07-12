#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace muon::asset {

class Manager;

using FileLoadFn = std::function<void(Manager *, const std::filesystem::path &)>;
using MemoryLoadFn = std::function<void(Manager *, const std::vector<uint8_t> &)>;

struct Loader {
    std::string fileType; // the fullstop must be included: .png, .jxl, .opus, .wav, .ktx, .obj, etc.

    FileLoadFn fileLoad;
    MemoryLoadFn memoryLoad;
};

} // namespace muon::asset
