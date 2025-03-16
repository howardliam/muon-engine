cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

project(muon VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp

    src/core/compression.cpp
    src/core/compression.hpp
    src/core/logging.cpp
    src/core/logging.hpp
    src/core/window.cpp
    src/core/window.hpp

    src/core/assets/image.cpp
    src/core/assets/image.hpp

    src/engine/vulkan/device.cpp
    src/engine/vulkan/device.hpp
    src/engine/vulkan/renderer.cpp
    src/engine/vulkan/renderer.hpp
    src/engine/vulkan/swapchain.cpp
    src/engine/vulkan/swapchain.hpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src/)

set(SPDLOG_USE_STD_FORMAT ON FORCE)
add_subdirectory(external/spdlog)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE external/tomlplusplus/include/)
find_package(zstd REQUIRED)

set(SDL_SHARED OFF FORCE)
set(SDL_STATIC ON FORCE)
set(SDL_TEST_LIBRARY OFF FORCE)
set(SDL_TESTS OFF FORCE)
set(SDL_INSTALL_TESTS OFF FORCE)
add_subdirectory(external/sdl3)

find_package(Vulkan REQUIRED)
add_subdirectory(external/vulkan-memory-allocator)
add_subdirectory(external/vulkan-memory-allocator-hpp)

find_package(PNG REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    spdlog::spdlog
    zstd::libzstd

    SDL3::SDL3
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp

    png
)

if (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()
