# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADER_FILES
    ${INCLUDE_DIR}/muon/engine/descriptor/pool.hpp
    ${INCLUDE_DIR}/muon/engine/descriptor/set_layout.hpp
    ${INCLUDE_DIR}/muon/engine/descriptor/writer.hpp

    ${INCLUDE_DIR}/muon/engine/framegraph/blackboard.hpp
    ${INCLUDE_DIR}/muon/engine/framegraph/blackboard.inl
    ${INCLUDE_DIR}/muon/engine/framegraph/framegraph.hpp

    ${INCLUDE_DIR}/muon/engine/pipeline/compute.hpp
    ${INCLUDE_DIR}/muon/engine/pipeline/graphics.hpp
    ${INCLUDE_DIR}/muon/engine/pipeline/layout.hpp

    ${INCLUDE_DIR}/muon/engine/buffer.hpp
    ${INCLUDE_DIR}/muon/engine/debug_ui.hpp
    ${INCLUDE_DIR}/muon/engine/device.hpp
    ${INCLUDE_DIR}/muon/engine/frame_handler.hpp
    ${INCLUDE_DIR}/muon/engine/image.hpp
    ${INCLUDE_DIR}/muon/engine/material.hpp
    ${INCLUDE_DIR}/muon/engine/mesh.hpp
    ${INCLUDE_DIR}/muon/engine/mesh.inl
    ${INCLUDE_DIR}/muon/engine/shader.hpp
    ${INCLUDE_DIR}/muon/engine/swapchain.hpp
    ${INCLUDE_DIR}/muon/engine/texture.hpp
    ${INCLUDE_DIR}/muon/engine/window.hpp

    ${INCLUDE_DIR}/muon/log/logger.hpp

    ${INCLUDE_DIR}/muon/utils/color.hpp
    ${INCLUDE_DIR}/muon/utils/nocopy.hpp
    ${INCLUDE_DIR}/muon/utils/nomove.hpp
    ${INCLUDE_DIR}/muon/utils/pretty_print.hpp
)

set(SRC_FILES
    ${SRC_DIR}/engine/descriptor/pool.cpp
    ${SRC_DIR}/engine/descriptor/set_layout.cpp
    ${SRC_DIR}/engine/descriptor/writer.cpp

    ${SRC_DIR}/engine/framegraph/framegraph.cpp

    ${SRC_DIR}/engine/pipeline/compute.cpp
    ${SRC_DIR}/engine/pipeline/graphics.cpp
    ${SRC_DIR}/engine/pipeline/layout.cpp

    ${SRC_DIR}/engine/buffer.cpp
    ${SRC_DIR}/engine/debug_ui.cpp
    ${SRC_DIR}/engine/device.cpp
    ${SRC_DIR}/engine/frame_handler.cpp
    ${SRC_DIR}/engine/image.cpp
    ${SRC_DIR}/engine/mesh.cpp
    ${SRC_DIR}/engine/shader.cpp
    ${SRC_DIR}/engine/swapchain.cpp
    ${SRC_DIR}/engine/texture.cpp
    ${SRC_DIR}/engine/window.cpp

    ${SRC_DIR}/log/logger.cpp

    ${SRC_DIR}/utils/color.cpp
    ${SRC_DIR}/utils/pretty_print.cpp
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

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR} ${IMGUI_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spirv-reflect-static glslang SPIRV muon::common)
target_link_libraries(${LIB_NAME} PUBLIC SDL3 vulkan VulkanMemoryAllocator-Hpp VulkanMemoryAllocator glm)

add_library(muon::engine ALIAS ${LIB_NAME})
