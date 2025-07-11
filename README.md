> [!WARNING]
> This is very early in development and probably impractical to use in its current state.

# Muon Engine
Muon is an experimental game engine built for curiosity and learning graphics programming.

## Features

## Usage
> [!IMPORTANT]
> Project uses CMake 3.30.0, C++23, and targets Vulkan 1.3.

## Project Conventions
| item | convention |
| ---- | ---- |
| integers | use `cstdint`/`stdint.h` integer types: `int32_t`, `int64_t`, etc. |
| classes, structs | named in PascalCase, e.g.: `FileManager`, `MaterialInfo` |
| functions, methods | named in PascalCase with trailing return type, e.g.: `auto RenderModel() -> void` |
| variables, lambdas | named in camelCase, e.g.: `fileIndex` |
| global constants | name begins with `k_`, e.g.: `constexpr uint64_t k_version = 3.14;` |
| static variables | name begins with `s_`, e.g.: `static tracy::VkCtx *s_tracyContext;` |
| member variables | name begins with `m_`, e.g.: `Device &m_device;` |
| struct, class members | default values within curly braces, e.g.: `size_t m_size{100};` |
| pointers, references | `*` and `&` must be on the variable name, e.g.: `const std::vector<uint8_t> &data;` |
| files | open compound named in snake_case, e.g.: `frame_handler.hpp` |
| class/struct visibility | group visibility by methods and members separately |
| headers | always use `.hpp` extension over `.h` when the code is C++ specific |
| abbreviations, initialisations | treated as single words, e.g.: `Hdr`, `Ssao`, `Gltf` |
| code spelling | follow American English |
| non-code spelling | may follow preferred English dialect |
| commits | commits should follow the [Conventional Commits Spec](https://www.conventionalcommits.org/en/v1.0.0/), post 2025-05-24 |

## Attributions
### Compression
- madler/zlib
- facebook/zstd

### Configuration
- nlohmann/json
- jbeder/yaml-cpp

### Events
- wqking/eventpp

### Game
- skypjack/entt

### Graphics
- g-truc/glm
- KhronosGroup/SPIRV-Reflect
- KhronosGroup/Vulkan-Headers
- KhronosGroup/Vulkan-Loader
- GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator

### Images
- randy408/libspng
- KhronosGroup/KTX-Software

### Logging
- fmtlib/fmt
- gabime/spdlog

### Profiling
- wolfpld/tracy

### Testing
- catchorg/catch2

### Windowing
- glfw/glfw
