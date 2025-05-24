# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADER_FILES
    ${INCLUDE_DIR}/muon/engine/core/assert.hpp
    ${INCLUDE_DIR}/muon/engine/core/base.hpp
    ${INCLUDE_DIR}/muon/engine/core/entry_point.hpp
    ${INCLUDE_DIR}/muon/engine/core/log.hpp

    ${INCLUDE_DIR}/muon/engine/renderer/descriptor/pool.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/descriptor/set_layout.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/descriptor/writer.hpp

    ${INCLUDE_DIR}/muon/engine/renderer/frame_graph/blackboard.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/frame_graph/blackboard.inl
    ${INCLUDE_DIR}/muon/engine/renderer/frame_graph/frame_graph.hpp

    ${INCLUDE_DIR}/muon/engine/renderer/pipeline/compute.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/pipeline/graphics.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/pipeline/layout.hpp

    ${INCLUDE_DIR}/muon/engine/renderer/buffer.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/debug_ui.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/device.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/frame_handler.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/image.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/material.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/mesh.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/mesh.inl
    ${INCLUDE_DIR}/muon/engine/renderer/model.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/shader.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/swapchain.hpp
    ${INCLUDE_DIR}/muon/engine/renderer/texture.hpp

    ${INCLUDE_DIR}/muon/engine/platform/window.hpp

    ${INCLUDE_DIR}/muon/engine/utils/color.hpp
    ${INCLUDE_DIR}/muon/engine/utils/nocopy.hpp
    ${INCLUDE_DIR}/muon/engine/utils/nomove.hpp
    ${INCLUDE_DIR}/muon/engine/utils/pretty_print.hpp
)

set(SRC_FILES
    ${SRC_DIR}/engine/core/log.cpp

    ${SRC_DIR}/engine/renderer/descriptor/pool.cpp
    ${SRC_DIR}/engine/renderer/descriptor/set_layout.cpp
    ${SRC_DIR}/engine/renderer/descriptor/writer.cpp

    ${SRC_DIR}/engine/renderer/frame_graph/frame_graph.cpp

    ${SRC_DIR}/engine/renderer/pipeline/compute.cpp
    ${SRC_DIR}/engine/renderer/pipeline/graphics.cpp
    ${SRC_DIR}/engine/renderer/pipeline/layout.cpp

    ${SRC_DIR}/engine/renderer/buffer.cpp
    ${SRC_DIR}/engine/renderer/debug_ui.cpp
    ${SRC_DIR}/engine/renderer/device.cpp
    ${SRC_DIR}/engine/renderer/frame_handler.cpp
    ${SRC_DIR}/engine/renderer/image.cpp
    ${SRC_DIR}/engine/renderer/mesh.cpp
    ${SRC_DIR}/engine/renderer/model.cpp
    ${SRC_DIR}/engine/renderer/shader.cpp
    ${SRC_DIR}/engine/renderer/swapchain.cpp
    ${SRC_DIR}/engine/renderer/texture.cpp

    ${SRC_DIR}/engine/platform/window.cpp

    ${SRC_DIR}/engine/utils/color.cpp
    ${SRC_DIR}/engine/utils/pretty_print.cpp
)

set(IMGUI_DIR ../external/imgui)
set(IMGUI_FILES
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
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.h
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.h
)

add_library(${LIB_NAME} STATIC
    ${HEADER_FILES}
    ${SRC_FILES}
    ${IMGUI_FILES}
)
add_library(muon::engine ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR} ${IMGUI_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spirv-reflect-static glslang SPIRV muon::common spdlog::spdlog_header_only)
target_link_libraries(${LIB_NAME} PUBLIC SDL3 vulkan VulkanMemoryAllocator-Hpp VulkanMemoryAllocator glm)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(${LIB_NAME} PUBLIC MU_DEBUG_ENABLED)
endif()
