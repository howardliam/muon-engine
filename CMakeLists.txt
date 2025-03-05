cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(muon VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp

    src/core/compression.cpp
    src/core/logging.cpp
    src/core/window.cpp
    src/core/assets/image.cpp

    src/engine/vulkan/device.cpp
    src/engine/vulkan/renderer.cpp
    src/engine/vulkan/swapchain.cpp
)

find_package(spdlog CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(tomlplusplus REQUIRED IMPORTED_TARGET tomlplusplus)
find_package(zstd CONFIG REQUIRED)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)

find_package(PNG CONFIG REQUIRED)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src/)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    fmt::fmt spdlog::spdlog
    PkgConfig::tomlplusplus
    zstd::libzstd

    SDL3::SDL3
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp

    png
)

if (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()
