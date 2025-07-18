# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(ENGINE_HEADER_FILES
    ${INCLUDE_DIR}/muon/asset/loaders/png.hpp
    ${INCLUDE_DIR}/muon/asset/loader.hpp
    ${INCLUDE_DIR}/muon/asset/manager.hpp

    ${INCLUDE_DIR}/muon/core/arena.hpp
    ${INCLUDE_DIR}/muon/core/application.hpp
    ${INCLUDE_DIR}/muon/core/assert.hpp
    ${INCLUDE_DIR}/muon/core/base.hpp
    ${INCLUDE_DIR}/muon/core/entry_point.hpp
    ${INCLUDE_DIR}/muon/core/errors.hpp
    ${INCLUDE_DIR}/muon/core/hash.hpp
    ${INCLUDE_DIR}/muon/core/layer.hpp
    ${INCLUDE_DIR}/muon/core/layer_stack.hpp
    ${INCLUDE_DIR}/muon/core/log.hpp
    ${INCLUDE_DIR}/muon/core/no_copy.hpp
    ${INCLUDE_DIR}/muon/core/no_move.hpp
    ${INCLUDE_DIR}/muon/core/platform_detection.hpp
    ${INCLUDE_DIR}/muon/core/project.hpp
    ${INCLUDE_DIR}/muon/core/window.hpp

    ${INCLUDE_DIR}/muon/event/dispatcher.hpp
    ${INCLUDE_DIR}/muon/event/event.hpp

    ${INCLUDE_DIR}/muon/fs/fs.hpp

    ${INCLUDE_DIR}/muon/graphics/buffer.hpp
    ${INCLUDE_DIR}/muon/graphics/context.hpp
    ${INCLUDE_DIR}/muon/graphics/device_extensions.hpp
    ${INCLUDE_DIR}/muon/graphics/gpu.hpp
    ${INCLUDE_DIR}/muon/graphics/image.hpp
    ${INCLUDE_DIR}/muon/graphics/mesh.hpp
    ${INCLUDE_DIR}/muon/graphics/instance_extensions.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_base.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_compute.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_graphics.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_layout.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_meshlet.hpp
    ${INCLUDE_DIR}/muon/graphics/queue.hpp
    ${INCLUDE_DIR}/muon/graphics/queue_info.hpp
    ${INCLUDE_DIR}/muon/graphics/renderer.hpp
    ${INCLUDE_DIR}/muon/graphics/shader_compiler.hpp
    ${INCLUDE_DIR}/muon/graphics/shader_reflection.hpp
    ${INCLUDE_DIR}/muon/graphics/swapchain.hpp
    ${INCLUDE_DIR}/muon/graphics/texture.hpp

    ${INCLUDE_DIR}/muon/input/input_state.hpp
    ${INCLUDE_DIR}/muon/input/key_code.hpp
    ${INCLUDE_DIR}/muon/input/modifier.hpp
    ${INCLUDE_DIR}/muon/input/mouse.hpp

    ${INCLUDE_DIR}/muon/profiling/profiler.hpp

    ${INCLUDE_DIR}/muon/schematic/pipeline/color_blend_attachment_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/color_blend_state_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/depth_stencil_state_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/input_assembly_state_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/multisample_state_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/pipeline_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/pipeline_state_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/rasterization_state_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/shader_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/stencil_op_state_info.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline/viewport_state_info.hpp

    ${INCLUDE_DIR}/muon/utils/alignment.hpp
    ${INCLUDE_DIR}/muon/utils/color.hpp
    ${INCLUDE_DIR}/muon/utils/literals.hpp
    ${INCLUDE_DIR}/muon/utils/pretty_print.hpp
)

set(ENGINE_SRC_FILES
    ${SRC_DIR}/asset/loaders/png.cpp
    ${SRC_DIR}/asset/manager.cpp

    ${SRC_DIR}/core/application.cpp
    ${SRC_DIR}/core/arena.cpp
    ${SRC_DIR}/core/hash.cpp
    ${SRC_DIR}/core/layer_stack.cpp
    ${SRC_DIR}/core/log.cpp
    ${SRC_DIR}/core/project.cpp
    ${SRC_DIR}/core/window.cpp

    ${SRC_DIR}/fs/fs.cpp

    ${SRC_DIR}/graphics/buffer.cpp
    ${SRC_DIR}/graphics/context.cpp
    ${SRC_DIR}/graphics/gpu.cpp
    ${SRC_DIR}/graphics/image.cpp
    ${SRC_DIR}/graphics/mesh.cpp
    ${SRC_DIR}/graphics/pipeline_base.cpp
    ${SRC_DIR}/graphics/pipeline_compute.cpp
    ${SRC_DIR}/graphics/pipeline_graphics.cpp
    ${SRC_DIR}/graphics/pipeline_layout.cpp
    ${SRC_DIR}/graphics/pipeline_meshlet.cpp
    ${SRC_DIR}/graphics/queue.cpp
    ${SRC_DIR}/graphics/queue_info.cpp
    ${SRC_DIR}/graphics/renderer.cpp
    ${SRC_DIR}/graphics/shader_compiler.cpp
    ${SRC_DIR}/graphics/shader_reflection.cpp
    ${SRC_DIR}/graphics/swapchain.cpp
    ${SRC_DIR}/graphics/texture.cpp

    ${SRC_DIR}/input/modifier.cpp

    ${SRC_DIR}/profiling/profiler.cpp

    ${SRC_DIR}/utils/color.cpp
    ${SRC_DIR}/utils/literals.cpp
    ${SRC_DIR}/utils/pretty_print.cpp
)

add_library(${LIB_NAME} STATIC
    ${ENGINE_HEADER_FILES}
    ${ENGINE_SRC_FILES}
)
add_library(muon::engine ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PUBLIC
    fmt::fmt spdlog::spdlog_header_only
    tomlplusplus::tomlplusplus nlohmann_json::nlohmann_json
    glfw eventpp::eventpp
    Tracy::TracyClient
    Vulkan::Headers Vulkan::Loader VulkanMemoryAllocator
    spirv-reflect-static SPIRV-Tools
    glslang
    glm
    magic_enum::magic_enum
    spng
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
