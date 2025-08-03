set(VULKAN_TAG vulkan-sdk-1.4.313.0)

FetchContent_Declare(
    spirv-headers
    GIT_REPOSITORY  https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG         ${VULKAN_TAG}
)
FetchContent_MakeAvailable(spirv-headers)

FetchContent_Declare(
    spirv-reflect
    GIT_REPOSITORY  https://github.com/KhronosGroup/SPIRV-Reflect.git
    GIT_TAG         ${VULKAN_TAG}
)
set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)
set(SPIRV_REFLECT_BUILD_TESTS OFF)
FetchContent_MakeAvailable(spirv-reflect)

FetchContent_Declare(
    spirv-tools
    GIT_REPOSITORY  https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG         ${VULKAN_TAG}
)
set(SPIRV_SKIP_EXECUTABLES ON)
FetchContent_MakeAvailable(spirv-tools)

FetchContent_Declare(
    glslang
    GIT_REPOSITORY  https://github.com/KhronosGroup/glslang.git
    GIT_TAG         ${VULKAN_TAG}
)
set(BUILD_EXTERNAL OFF)
set(ENABLE_HLSL OFF)
set(ENABLE_GLSLANG_BINARIES OFF)
FetchContent_MakeAvailable(glslang)

FetchContent_Declare(
    vulkan-headers
    GIT_REPOSITORY  https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG         ${VULKAN_TAG}
)
FetchContent_MakeAvailable(vulkan-headers)

# FetchContent_Declare(
#     vulkan-loader
#     GIT_REPOSITORY  https://github.com/KhronosGroup/Vulkan-Loader.git
#     GIT_TAG         ${VULKAN_TAG}
# )
# FetchContent_MakeAvailable(vulkan-loader)

FetchContent_Declare(
    vulkan-hpp
    GIT_REPOSITORY  https://github.com/KhronosGroup/Vulkan-Hpp.git
    GIT_TAG         v1.4.313
)
set(VULKAN_HPP_SAMPLES_BUILD OFF)
set(VULKAN_HPP_TESTS_BUILD OFF)
set(VULKAN_HPP_NO_EXCEPTIONS ON)
FetchContent_MakeAvailable(vulkan-hpp)

FetchContent_Declare(
    vulkan-memory-allocator
    GIT_REPOSITORY  https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG         v3.3.0
)
FetchContent_MakeAvailable(vulkan-memory-allocator)

FetchContent_Declare(
    vulkan-memory-allocator-hpp
    GIT_REPOSITORY  https://github.com/YaaZ/VulkanMemoryAllocator-Hpp.git
    GIT_TAG         v3.2.1
)
FetchContent_MakeAvailable(vulkan-memory-allocator-hpp)
