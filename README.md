> [!WARNING]
> This is very early in development and probably impractical to use in its current state.

# Muon Engine
Muon is an experimental game engine built for curiosity and learning graphics programming.

## Features
- Bindless textures
- Dynamic rendering
- In-progress render graph

## Usage
> [!IMPORTANT]
> Project uses CMake 4.0.0, C++23, and targets Vulkan 1.3.

Add as a git submodule and link to the required libraries.

## Project Conventions
| item | convention |
| ---- | ---- |
| integers | use `cstdint`/`stdint.h` integer types: `int32_t`, `int64_t`, etc. |
| classes, structs | named in PascelCase: `FileManager`, `MaterialInfo` |
| functions, methods, variables | named in camelCase: `renderModel()` |
| files | open compound named in snake_case: `frame_handler.hpp` |
| headers | always use `.hpp` extension over `.h` to be C++ specific |
| abbreviations, initialisations | treated as single words: `Hdr`, `Ssao`, `Gltf` |
| code spelling | follow American English |
| non-code spelling | may follow preferred English dialect |

## Attributions
### Engine
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [VulkanMemoryAllocator-Hpp](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp)
- [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect)
- [glslang](https://github.com/KhronosGroup/glslang)
- [SDL3](https://wiki.libsdl.org/SDL3/FrontPage)
- [glm](https://github.com/g-truc/glm)

### Asset
- [libspng](https://libspng.org/)
- [libjpeg-turbo](https://libjpeg-turbo.org/)

### Test
- [spdlog](https://github.com/gabime/spdlog)
