cmake_minimum_required(VERSION 4.0.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

project(muon)

add_subdirectory(asset)
add_subdirectory(engine)
add_subdirectory(common)

find_package(PkgConfig REQUIRED)

set(SPDLOG_USE_STD_FORMAT ON FORCE)
add_subdirectory(external/spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_package(zstd REQUIRED)

option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(external/tracy)

set(SDL_SHARED OFF FORCE)
set(SDL_STATIC ON FORCE)
set(SDL_TEST_LIBRARY OFF FORCE)
set(SDL_TESTS OFF FORCE)
set(SDL_INSTALL_TESTS OFF FORCE)
add_subdirectory(external/sdl)

find_package(Vulkan REQUIRED)
find_package(SPIRV-Tools REQUIRED)
add_subdirectory(external/vulkan-memory-allocator)
add_subdirectory(external/vulkan-memory-allocator-hpp)

set(SPIRV_REFLECT_EXECUTABLE OFF FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON FORCE)
add_subdirectory(external/spirv-reflect)

set(ALLOW_EXTERNAL_SPIRV_TOOLS ON FORCE)
add_subdirectory(external/glslang)

set(GLM_ENABLE_CXX_20 ON FORCE)
set(GLM_ENABLE_SIMD_AVX2 ON FORCE)
add_subdirectory(external/glm)

pkg_check_modules(SPNG REQUIRED spng)
find_package(libjpeg-turbo REQUIRED)
pkg_check_modules(OPUS REQUIRED opus)
pkg_check_modules(OPUSFILE REQUIRED opusfile)

add_subdirectory(external/json)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()

add_subdirectory(test)
add_subdirectory(editor)
