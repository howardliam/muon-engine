> [!WARNING]
> This is very early in development and probably impractical to use in its current state.

# Muon Engine
Muon is an experimental game engine built for curiosity and learning graphics programming.

## Features
- Bindless textures
- Dynamic rendering
- Custom render graph

## Usage
Add as a git submodule and link to the required libraries.

## Project Conventions
- use cstdint integer types: `int32_t`, `int64_t`, etc.,
- where possible, forward declare if includes are massive,
- classes, structs are named in PascalCase,
- functions, methods, variables, members are named in camelCase,
- abbreviations and initialisations count as a single word; `initSdl`, `loadGltfModel`, `encodePng`,
- American English throughout code,
- Comments and logs may use whatever dialect of English.

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
