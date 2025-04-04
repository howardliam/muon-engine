cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

project(muon)

add_subdirectory(engine)
add_subdirectory(common)

set(SPDLOG_USE_STD_FORMAT ON FORCE)
add_subdirectory(external/spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_package(zstd REQUIRED)

set(SDL_SHARED OFF FORCE)
set(SDL_STATIC ON FORCE)
set(SDL_TEST_LIBRARY OFF FORCE)
set(SDL_TESTS OFF FORCE)
set(SDL_INSTALL_TESTS OFF FORCE)
add_subdirectory(external/sdl)

find_package(Vulkan REQUIRED)
add_subdirectory(external/vulkan-memory-allocator)
add_subdirectory(external/vulkan-memory-allocator-hpp)

option(SPNG_SHARED OFF)
add_subdirectory(external/libspng)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()

add_subdirectory(test)
