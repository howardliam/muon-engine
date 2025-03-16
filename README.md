# Muon
An amateur game engine being developed to pursue an interest in graphics programming.

## Running
```bash
git clone https://github.com/howardliam/muon-engine
cd muon-engine
git submodule init
git submodule update
cmake -B build -G 'Ninja'
cmake --build build && ./build/muon
```

## Attributions {#attr}
- [SDL3](https://wiki.libsdl.org/SDL3/FrontPage)
- [spdlog](https://github.com/gabime/spdlog)
- [toml++](https://marzer.github.io/tomlplusplus/)
- [Vulkan](https://www.vulkan.org/)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [zstd](https://github.com/facebook/zstd)
