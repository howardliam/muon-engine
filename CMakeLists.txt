cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(muon)

add_subdirectory(engine)
add_subdirectory(editor)

# logging
add_subdirectory(external/fmt)
add_subdirectory(external/spdlog)

# config formats
add_subdirectory(external/tomlplusplus)
add_subdirectory(external/json)

# compression
set(ZLIB_LIBRARY zlib)
set(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/zlib)
add_subdirectory(external/zlib)
add_subdirectory(external/zstd/build/cmake zstd)
target_include_directories(zstd PUBLIC external/zstd/lib/)

# profiling
option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(external/tracy)

# windowing
add_subdirectory(external/glfw)

# event dispatcher
add_subdirectory(external/eventpp)

# graphics
add_subdirectory(external/spirv-headers)

set(SPIRV_REFLECT_EXECUTABLE OFF FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON FORCE)
add_subdirectory(external/spirv-reflect)

add_subdirectory(external/spirv-tools)

set(BUILD_EXTERNAL OFF FORCE)
set(ENABLE_HLSL OFF FORCE)
set(ENABLE_GLSLANG_BINARIES OFF FORCE)
add_subdirectory(external/glslang)

add_subdirectory(external/vulkan-headers)
add_subdirectory(external/vulkan-loader)
add_subdirectory(external/vulkan-hpp)
add_subdirectory(external/vulkan-memory-allocator)

# maths
set(GLM_ENABLE_CXX_20 ON FORCE)
set(GLM_ENABLE_SIMD_AVX2 ON FORCE)
add_subdirectory(external/glm)

# assets
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/libspng)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF FORCE)
add_subdirectory(external/libktx)

# misc
add_subdirectory(external/entt)
add_subdirectory(external/magic-enum)
add_subdirectory(external/sqlitecpp)

# testing
add_subdirectory(external/catch2)

# crypto
add_subdirectory(external/libtommath)
add_subdirectory(external/libtomcrypt)

if(MSVC)
    #add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -fdiagnostics-color=always -O2)
endif()
