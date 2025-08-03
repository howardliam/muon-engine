# Engine Library

add_library(muon-engine STATIC)
add_library(muon::engine ALIAS muon-engine)

target_sources(
    muon-engine
    PRIVATE
        src/muon/asset/loaders/png.cpp
        src/muon/asset/asset_manager.cpp

        src/muon/config/config_manager.cpp

        src/muon/core/application.cpp
        src/muon/core/layer_stack.cpp
        src/muon/core/log.cpp
        src/muon/core/window.cpp

        src/muon/format/bytes.cpp

        src/muon/fs/fs.cpp

        src/muon/graphics/buffer.cpp
        src/muon/graphics/context.cpp
        src/muon/graphics/descriptor_pool.cpp
        src/muon/graphics/descriptor_set_layout.cpp
        src/muon/graphics/descriptor_writer.cpp
        src/muon/graphics/gpu.cpp
        src/muon/graphics/image.cpp
        src/muon/graphics/mesh.cpp
        src/muon/graphics/pipeline_base.cpp
        src/muon/graphics/pipeline_compute.cpp
        src/muon/graphics/pipeline_graphics.cpp
        src/muon/graphics/pipeline_layout.cpp
        src/muon/graphics/pipeline_meshlet.cpp
        src/muon/graphics/queue.cpp
        src/muon/graphics/queue_family_info.cpp
        src/muon/graphics/renderer.cpp
        src/muon/graphics/shader_compiler.cpp
        src/muon/graphics/swapchain.cpp
        src/muon/graphics/texture.cpp

        src/muon/input/modifier.cpp

        src/muon/project/project.cpp

    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src/
    FILES
        src/muon/asset/loaders/png.hpp
        src/muon/asset/asset_loader.hpp
        src/muon/asset/asset_manager.hpp

        src/muon/config/application.hpp
        src/muon/config/config_manager.hpp

        src/muon/core/application.hpp
        src/muon/core/debug.hpp
        src/muon/core/entry_point.hpp
        src/muon/core/expect.hpp
        src/muon/core/layer.hpp
        src/muon/core/layer_stack.hpp
        src/muon/core/log.hpp
        src/muon/core/no_copy.hpp
        src/muon/core/no_move.hpp
        src/muon/core/platform.hpp
        src/muon/core/serialization.hpp
        src/muon/core/window.hpp

        src/muon/crypto/crypto.hpp

        src/muon/event/dispatcher.hpp
        src/muon/event/event.hpp

        src/muon/format/bytes.hpp

        src/muon/fs/fs.hpp

        src/muon/graphics/buffer.hpp
        src/muon/graphics/constants.hpp
        src/muon/graphics/context.hpp
        src/muon/graphics/descriptor_pool.hpp
        src/muon/graphics/descriptor_set_layout.hpp
        src/muon/graphics/descriptor_writer.hpp
        src/muon/graphics/extensions.hpp
        src/muon/graphics/gpu.hpp
        src/muon/graphics/image.hpp
        src/muon/graphics/mesh.hpp
        src/muon/graphics/pipeline_base.hpp
        src/muon/graphics/pipeline_compute.hpp
        src/muon/graphics/pipeline_graphics.hpp
        src/muon/graphics/pipeline_layout.hpp
        src/muon/graphics/pipeline_meshlet.hpp
        src/muon/graphics/queue.hpp
        src/muon/graphics/queue_family_info.hpp
        src/muon/graphics/renderer.hpp
        src/muon/graphics/shader_compiler.hpp
        src/muon/graphics/swapchain.hpp
        src/muon/graphics/texture.hpp

        src/muon/input/input_state.hpp
        src/muon/input/key_code.hpp
        src/muon/input/modifier.hpp
        src/muon/input/mouse.hpp

        src/muon/project/project.hpp

        src/muon/utils/alignment.hpp
        src/muon/utils/library.hpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    target_sources(
        muon-engine
        PRIVATE
            src/muon/core/platform_posix.cpp
            src/muon/crypto/crypto_posix.cpp
            src/muon/utils/library_posix.cpp
    )

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    target_sources(
        muon-engine
        PRIVATE
            src/muon/core/platform_win32.cpp
            src/muon/utils/library_win32.cpp
    )

endif()

target_link_libraries(muon-engine PRIVATE
    spirv-reflect-static SPIRV-Tools glslang
)

target_link_libraries(muon-engine PUBLIC
    argparse
    fmt::fmt spdlog::spdlog_header_only
    tomlplusplus::tomlplusplus nlohmann_json::nlohmann_json
    glfw eventpp::eventpp
    Vulkan::Headers Vulkan::Hpp
    VulkanMemoryAllocator VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
    glm
    magic_enum::magic_enum
    spng_static
    SQLiteCpp
)

add_compile_definitions(
    $<$<CONFIG:Debug>:MU_DEBUG>
)

option(MUON_ENGINE_TESTS "Enable Muon Engine tests" ON)
if(MUON_ENGINE_TESTS)

    add_executable(muon-tests)

    target_sources(
        muon-tests
        PRIVATE
            tests/main.cpp

            tests/utils/alignment.cpp
    )

    target_link_libraries(muon-tests PRIVATE
        Catch2
        muon::engine
    )

endif()
