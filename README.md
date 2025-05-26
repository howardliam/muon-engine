> [!WARNING]
> This is very early in development and probably impractical to use in its current state.

# Muon Engine
Muon is an experimental game engine built for curiosity and learning graphics programming.

![Progress 2025-05-19](./2025-05-19-progress.png)

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
| static variables | name begins with `s_`, e.g.: `static tracy::VkCtx *s_tracyContext;` |
| member variables | name begins with `m_`, e.g.: `Device &m_device;` |
| pointers, references | `*` and `&` must be on the variable name, e.g.: `const std::vector<uint8_t> &data;` |
| files | open compound named in snake_case: `frame_handler.hpp` |
| class/struct visibility | group visibility by methods and members separately |
| headers | always use `.hpp` extension over `.h` to be C++ specific |
| abbreviations, initialisations | treated as single words: `Hdr`, `Ssao`, `Gltf` |
| code spelling | follow American English |
| non-code spelling | may follow preferred English dialect |
| commits | commits should follow the [Conventional Commits Spec](https://www.conventionalcommits.org/en/v1.0.0/), post 2025-05-24 |

## Attributions
### Engine
| library | licences |
| ----- | ----- |
| [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) | ... |
| [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | MIT |
| [VulkanMemoryAllocator-Hpp](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp) | CC0-1.0 |
| [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect) | Apache-2.0 |
| [glslang](https://github.com/KhronosGroup/glslang) | 3-Clause BSD, 2-Clause BSD, MIT, Apache-2.0 |
| [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) | zlib |
| [glm](https://github.com/g-truc/glm) | The Happy Bunny License, MIT |

### Asset
| library | licences |
| ----- | ----- |
| [libspng](https://libspng.org/) | 2-Clause BSD |
| [libjpeg-turbo](https://libjpeg-turbo.org/) | IJG, 3-Clause BSD |

### Test
| library | licences |
| ----- | ----- |
| [spdlog](https://github.com/gabime/spdlog) | MIT |
