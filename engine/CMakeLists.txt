# Engine ImGui Library

set(IMGUI_DIR ../external/imgui)
add_library(muon-imgui STATIC
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.h
)
add_library(muon::imgui ALIAS muon-imgui)
target_include_directories(muon-imgui PUBLIC ${IMGUI_DIR})

# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(ENGINE_HEADER_FILES
    ${INCLUDE_DIR}/muon/core/application.hpp
    ${INCLUDE_DIR}/muon/core/assert.hpp
    ${INCLUDE_DIR}/muon/core/base.hpp
    ${INCLUDE_DIR}/muon/core/entry_point.hpp
    ${INCLUDE_DIR}/muon/core/log.hpp
    ${INCLUDE_DIR}/muon/core/platform_detection.hpp
    ${INCLUDE_DIR}/muon/core/window.hpp

    ${INCLUDE_DIR}/muon/debug/profiler.hpp

    ${INCLUDE_DIR}/muon/event/dispatcher.hpp
    ${INCLUDE_DIR}/muon/event/event.hpp

    ${INCLUDE_DIR}/muon/graphics/device_context.hpp
    ${INCLUDE_DIR}/muon/graphics/device_extensions.hpp
    ${INCLUDE_DIR}/muon/graphics/gpu.hpp
    ${INCLUDE_DIR}/muon/graphics/instance_extensions.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_compute.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_graphics.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_layout.hpp
    ${INCLUDE_DIR}/muon/graphics/pipeline_mesh.hpp
    ${INCLUDE_DIR}/muon/graphics/queue.hpp
    ${INCLUDE_DIR}/muon/graphics/queue_info.hpp
    ${INCLUDE_DIR}/muon/graphics/renderer.hpp
    ${INCLUDE_DIR}/muon/graphics/shader_reflection.hpp
    ${INCLUDE_DIR}/muon/graphics/swapchain.hpp

    ${INCLUDE_DIR}/muon/input/input_state.hpp
    ${INCLUDE_DIR}/muon/input/key_code.hpp
    ${INCLUDE_DIR}/muon/input/modifier.hpp
    ${INCLUDE_DIR}/muon/input/mouse.hpp

    ${INCLUDE_DIR}/muon/schematic/pipeline.hpp
    ${INCLUDE_DIR}/muon/schematic/pipeline_state.hpp
    ${INCLUDE_DIR}/muon/schematic/shader.hpp

    ${INCLUDE_DIR}/muon/scripting/script_manager.hpp

    ${INCLUDE_DIR}/muon/utils/color.hpp
    ${INCLUDE_DIR}/muon/utils/fs.hpp
    ${INCLUDE_DIR}/muon/utils/nocopy.hpp
    ${INCLUDE_DIR}/muon/utils/nomove.hpp
    ${INCLUDE_DIR}/muon/utils/pretty_print.hpp
)

set(ENGINE_SRC_FILES
    ${SRC_DIR}/core/application.cpp
    ${SRC_DIR}/core/log.cpp
    ${SRC_DIR}/core/window.cpp

    ${SRC_DIR}/debug/profiler.cpp

    ${SRC_DIR}/graphics/device_context.cpp
    ${SRC_DIR}/graphics/gpu.cpp
    ${SRC_DIR}/graphics/pipeline_compute.cpp
    ${SRC_DIR}/graphics/pipeline_graphics.cpp
    ${SRC_DIR}/graphics/pipeline_layout.cpp
    ${SRC_DIR}/graphics/pipeline_mesh.cpp
    ${SRC_DIR}/graphics/queue.cpp
    ${SRC_DIR}/graphics/queue_info.cpp
    ${SRC_DIR}/graphics/renderer.cpp
    ${SRC_DIR}/graphics/shader_reflection.cpp
    ${SRC_DIR}/graphics/swapchain.cpp

    ${SRC_DIR}/input/modifier.cpp

    ${SRC_DIR}/schematic/pipeline.cpp
    ${SRC_DIR}/schematic/shader.cpp

    ${SRC_DIR}/scripting/script_manager.cpp

    ${SRC_DIR}/utils/color.cpp
    ${SRC_DIR}/utils/fs.cpp
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
    yaml-cpp::yaml-cpp nlohmann_json::nlohmann_json
    glfw eventpp::eventpp
    Tracy::TracyClient
    Vulkan::Headers Vulkan::Loader VulkanMemoryAllocator
    spirv-reflect-static
    glm
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(MU_DEBUG_ENABLED)
endif()

option(MUON_ENGINE_TESTS "Enable Muon Engine tests" ON)
if(MUON_ENGINE_TESTS)

    set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    set(TEST_FILES
        ${TEST_DIR}/schematic/pipeline.cpp
        ${TEST_DIR}/schematic/shader.cpp
    )

    add_executable(muon-tests ${TEST_FILES})
    target_link_libraries(muon-tests PRIVATE
        Catch2WithMain
        muon::engine
    )

endif()
