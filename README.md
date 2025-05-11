# Muon Engine
Muon Engine, or simply, Muon, is an amateur game engine being developed as a
personal project.

## Usage
Add as a git submodule and link to the required libraries.

## Project Conventions
- classes, structs are named in PascalCase
- functions, methods, variables, members are named in camelCase
- abbreviations and initialisations count as a single word; `initSdl`, `loadGltfModel`, `encodePng`
- American English throughout for consistency sake
- use cstdint integer types: `int32_t`, `int64_t`, etc

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
