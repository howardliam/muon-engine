cmake_minimum_required(VERSION 4.0.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

project(muon)

add_subdirectory(asset)
add_subdirectory(engine)

find_package(PkgConfig REQUIRED)

# logging
set(SPDLOG_USE_STD_FORMAT ON FORCE)
add_subdirectory(external/spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)

# config formats
add_subdirectory(external/yaml-cpp)
add_subdirectory(external/json)

# compression
find_package(zstd REQUIRED)

# profiling
option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(external/tracy)

# windowing
add_subdirectory(external/glfw)

# graphics
find_package(Vulkan REQUIRED)
find_package(SPIRV-Tools REQUIRED)
add_subdirectory(external/vulkan-memory-allocator)
add_subdirectory(external/vulkan-memory-allocator-hpp)

set(SPIRV_REFLECT_EXECUTABLE OFF FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON FORCE)
add_subdirectory(external/spirv-reflect)

set(ALLOW_EXTERNAL_SPIRV_TOOLS ON FORCE)
add_subdirectory(external/glslang)

# event dispatcher
add_subdirectory(external/eventpp)

# scripting
find_package(Lua REQUIRED)
add_subdirectory(external/sol2)

# maths
set(GLM_ENABLE_CXX_20 ON FORCE)
set(GLM_ENABLE_SIMD_AVX2 ON FORCE)
add_subdirectory(external/glm)

# assets
pkg_check_modules(SPNG REQUIRED spng)
find_package(libjpeg-turbo REQUIRED)
pkg_check_modules(OPUS REQUIRED opus)
pkg_check_modules(OPUSFILE REQUIRED opusfile)

# misc
add_subdirectory(external/entt)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()

add_subdirectory(editor)
