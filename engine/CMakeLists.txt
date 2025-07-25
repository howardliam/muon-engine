# Engine Library

add_library(muon-engine STATIC)
add_library(muon::engine ALIAS muon-engine)

target_sources(
    muon-engine
    PRIVATE
        src/muon/asset/loaders/png.cpp
        src/muon/asset/manager.cpp

        src/muon/core/application.cpp
        src/muon/core/arena.cpp
        src/muon/core/layer_stack.cpp
        src/muon/core/log.cpp
        src/muon/core/window.cpp

        src/muon/fs/fs.cpp

        src/muon/graphics/buffer.cpp
        src/muon/graphics/context.cpp
        src/muon/graphics/gpu.cpp
        src/muon/graphics/image.cpp
        src/muon/graphics/mesh.cpp
        src/muon/graphics/pipeline_base.cpp
        src/muon/graphics/pipeline_compute.cpp
        src/muon/graphics/pipeline_graphics.cpp
        src/muon/graphics/pipeline_layout.cpp
        src/muon/graphics/pipeline_meshlet.cpp
        src/muon/graphics/queue.cpp
        src/muon/graphics/queue_info.cpp
        src/muon/graphics/renderer.cpp
        src/muon/graphics/shader_compiler.cpp
        src/muon/graphics/swapchain.cpp
        src/muon/graphics/texture.cpp

        src/muon/input/modifier.cpp

        src/muon/profiling/profiler.cpp

        src/muon/project/project.cpp

        src/muon/utils/color.cpp
        src/muon/utils/literals.cpp
        src/muon/utils/pretty_print.cpp

    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src/
    FILES
        src/muon/asset/loaders/png.hpp
        src/muon/asset/loader.hpp
        src/muon/asset/manager.hpp

        src/muon/core/arena.hpp
        src/muon/core/application.hpp
        src/muon/core/entry_point.hpp
        src/muon/core/errors.hpp
        src/muon/core/expect.hpp
        src/muon/core/layer.hpp
        src/muon/core/layer_stack.hpp
        src/muon/core/log.hpp
        src/muon/core/no_copy.hpp
        src/muon/core/no_move.hpp
        src/muon/core/signals.hpp
        src/muon/core/window.hpp

        src/muon/crypto/hash.hpp

        src/muon/event/dispatcher.hpp
        src/muon/event/event.hpp

        src/muon/fs/fs.hpp

        src/muon/graphics/buffer.hpp
        src/muon/graphics/context.hpp
        src/muon/graphics/device_extensions.hpp
        src/muon/graphics/gpu.hpp
        src/muon/graphics/image.hpp
        src/muon/graphics/mesh.hpp
        src/muon/graphics/instance_extensions.hpp
        src/muon/graphics/pipeline_base.hpp
        src/muon/graphics/pipeline_compute.hpp
        src/muon/graphics/pipeline_graphics.hpp
        src/muon/graphics/pipeline_layout.hpp
        src/muon/graphics/pipeline_meshlet.hpp
        src/muon/graphics/queue.hpp
        src/muon/graphics/queue_info.hpp
        src/muon/graphics/renderer.hpp
        src/muon/graphics/shader_compiler.hpp
        src/muon/graphics/swapchain.hpp
        src/muon/graphics/texture.hpp

        src/muon/input/input_state.hpp
        src/muon/input/key_code.hpp
        src/muon/input/modifier.hpp
        src/muon/input/mouse.hpp

        src/muon/profiling/profiler.hpp

        src/muon/project/project.hpp

        src/muon/utils/alignment.hpp
        src/muon/utils/color.hpp
        src/muon/utils/literals.hpp
        src/muon/utils/pretty_print.hpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    target_sources(
        muon-engine
        PRIVATE
            src/muon/core/signals_posix.cpp
    )

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    target_sources(
        muon-engine
        PRIVATE
            src/muon/core/signals_win32.cpp
    )

endif()

target_link_libraries(muon-engine PRIVATE
    spirv-reflect-static SPIRV-Tools glslang
)

target_link_libraries(muon-engine PUBLIC
    fmt::fmt spdlog::spdlog_header_only
    tomlplusplus::tomlplusplus nlohmann_json::nlohmann_json
    glfw eventpp::eventpp
    Tracy::TracyClient
    Vulkan::Headers Vulkan::Loader Vulkan::Hpp
    VulkanMemoryAllocator VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
    glm
    magic_enum::magic_enum
    spng_static
    libtomcrypt
    SQLiteCpp
)

add_compile_definitions(
    $<$<CONFIG:Debug>:MU_DEBUG_ENABLED>
)

option(MUON_ENGINE_TESTS "Enable Muon Engine tests" OFF)
if(MUON_ENGINE_TESTS)

    add_executable(muon-tests)

    target_sources(
        muon-tests
        PRIVATE
            tests/main.cpp

            tests/core/arena.cpp

            tests/utils/alignment.cpp
            tests/utils/literals.cpp
    )

    target_link_libraries(muon-tests PRIVATE
        Catch2
        muon::engine
    )

endif()
