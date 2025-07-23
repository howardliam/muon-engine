# Engine Library

add_library(muon-engine STATIC)
add_library(muon::engine ALIAS muon-engine)

target_sources(
    muon-engine
    PRIVATE
        src/asset/loaders/png.cpp
        src/asset/manager.cpp

        src/core/application.cpp
        src/core/arena.cpp
        src/core/hash.cpp
        src/core/layer_stack.cpp
        src/core/log.cpp
        src/core/project.cpp
        src/core/window.cpp

        src/fs/fs.cpp

        src/graphics/buffer.cpp
        src/graphics/context.cpp
        src/graphics/gpu.cpp
        src/graphics/image.cpp
        src/graphics/mesh.cpp
        src/graphics/pipeline_base.cpp
        src/graphics/pipeline_compute.cpp
        src/graphics/pipeline_graphics.cpp
        src/graphics/pipeline_layout.cpp
        src/graphics/pipeline_meshlet.cpp
        src/graphics/queue.cpp
        src/graphics/queue_info.cpp
        src/graphics/renderer.cpp
        src/graphics/shader_compiler.cpp
        src/graphics/shader_reflection.cpp
        src/graphics/swapchain.cpp
        src/graphics/texture.cpp

        src/input/modifier.cpp

        src/profiling/profiler.cpp

        src/utils/color.cpp
        src/utils/literals.cpp
        src/utils/pretty_print.cpp

    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src
    FILES
        src/asset/loaders/png.hpp
        src/asset/loader.hpp
        src/asset/manager.hpp

        src/core/arena.hpp
        src/core/application.hpp
        src/core/assert.hpp
        src/core/base.hpp
        src/core/entry_point.hpp
        src/core/errors.hpp
        src/core/hash.hpp
        src/core/layer.hpp
        src/core/layer_stack.hpp
        src/core/log.hpp
        src/core/no_copy.hpp
        src/core/no_move.hpp
        src/core/platform_detection.hpp
        src/core/project.hpp
        src/core/window.hpp

        src/event/dispatcher.hpp
        src/event/event.hpp

        src/fs/fs.hpp

        src/graphics/buffer.hpp
        src/graphics/context.hpp
        src/graphics/device_extensions.hpp
        src/graphics/gpu.hpp
        src/graphics/image.hpp
        src/graphics/mesh.hpp
        src/graphics/instance_extensions.hpp
        src/graphics/pipeline_base.hpp
        src/graphics/pipeline_compute.hpp
        src/graphics/pipeline_graphics.hpp
        src/graphics/pipeline_layout.hpp
        src/graphics/pipeline_meshlet.hpp
        src/graphics/queue.hpp
        src/graphics/queue_info.hpp
        src/graphics/renderer.hpp
        src/graphics/shader_compiler.hpp
        src/graphics/shader_reflection.hpp
        src/graphics/swapchain.hpp
        src/graphics/texture.hpp

        src/input/input_state.hpp
        src/input/key_code.hpp
        src/input/modifier.hpp
        src/input/mouse.hpp

        src/profiling/profiler.hpp

        src/schematic/pipeline/color_blend_attachment_info.hpp
        src/schematic/pipeline/color_blend_state_info.hpp
        src/schematic/pipeline/depth_stencil_state_info.hpp
        src/schematic/pipeline/input_assembly_state_info.hpp
        src/schematic/pipeline/multisample_state_info.hpp
        src/schematic/pipeline/pipeline_info.hpp
        src/schematic/pipeline/pipeline_state_info.hpp
        src/schematic/pipeline/rasterization_state_info.hpp
        src/schematic/pipeline/shader_info.hpp
        src/schematic/pipeline/stencil_op_state_info.hpp
        src/schematic/pipeline/viewport_state_info.hpp

        src/utils/alignment.hpp
        src/utils/color.hpp
        src/utils/literals.hpp
        src/utils/pretty_print.hpp
)

# target_include_directories(muon-engine PUBLIC ${INCLUDE_DIR})

target_link_libraries(muon-engine PRIVATE
    spirv-reflect-static SPIRV-Tools glslang
)

target_link_libraries(muon-engine PUBLIC
    fmt::fmt spdlog::spdlog_header_only
    tomlplusplus::tomlplusplus nlohmann_json::nlohmann_json
    glfw eventpp::eventpp
    Tracy::TracyClient
    Vulkan::Headers Vulkan::Loader VulkanMemoryAllocator
    glm
    magic_enum::magic_enum
    spng_static
    libtomcrypt
    SQLiteCpp
)

add_compile_definitions(
    $<$<CONFIG:Debug>:MU_DEBUG_ENABLED>
)

option(MUON_ENGINE_TESTS "Enable Muon Engine tests" ON)
if(MUON_ENGINE_TESTS)

    set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    set(TEST_FILES
        ${TEST_DIR}/main.cpp

        ${TEST_DIR}/core/arena.cpp

        ${TEST_DIR}/schematic/pipeline/pipeline_info.cpp
        ${TEST_DIR}/schematic/pipeline/shader_info.cpp

        ${TEST_DIR}/utils/alignment.cpp
        ${TEST_DIR}/utils/literals.cpp
    )

    add_executable(muon-tests ${TEST_FILES})
    target_link_libraries(muon-tests PRIVATE
        Catch2
        muon::engine
    )

endif()
