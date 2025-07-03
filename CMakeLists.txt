cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

project(muon)

add_subdirectory(engine)
add_subdirectory(editor)

# logging
add_subdirectory(external/fmt)
add_subdirectory(external/spdlog)

# config formats
add_subdirectory(external/yaml-cpp)
add_subdirectory(external/json)

# compression
set(ZLIB_LIBRARY zlib)
set(ZLIB_INCLUDE_DIR external/zlib)
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
add_subdirectory(external/vulkan-headers)
add_subdirectory(external/vulkan-loader)
add_subdirectory(external/vulkan-memory-allocator)

set(SPIRV_REFLECT_EXECUTABLE OFF FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON FORCE)
add_subdirectory(external/spirv-reflect)

set(SLANG_ENABLE_PREBUILT_BINARIES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_SLANGD OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_SLANGC OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_SLANGI OFF CACHE BOOL "" FORCE)
set(SLANG_OVERRIDE_VULKAN_HEADERS_PATH ON CACHE BOOL "" FORCE)
set(SLANG_OVERRIDE_GLM_PATH ON CACHE BOOL "" FORCE)
add_subdirectory(external/slang)

# maths
set(GLM_ENABLE_CXX_20 ON FORCE)
set(GLM_ENABLE_SIMD_AVX2 ON FORCE)
add_subdirectory(external/glm)

# assets
add_subdirectory(external/libspng)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF FORCE)
add_subdirectory(external/libktx)

# misc
add_subdirectory(external/entt)
add_subdirectory(external/magic-enum)

# testing
add_subdirectory(external/catch2)

if (MSVC)
    #add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -fdiagnostics-color=always -O2)
endif()
