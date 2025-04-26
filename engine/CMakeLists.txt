# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADER_FILES
    ${INCLUDE_DIR}/muon/engine/pipeline/compute.hpp
    ${INCLUDE_DIR}/muon/engine/pipeline/graphics.hpp
    ${INCLUDE_DIR}/muon/engine/buffer.hpp
    ${INCLUDE_DIR}/muon/engine/computesystem.hpp
    ${INCLUDE_DIR}/muon/engine/descriptor.hpp
    ${INCLUDE_DIR}/muon/engine/device.hpp
    ${INCLUDE_DIR}/muon/engine/framebuffer.hpp
    ${INCLUDE_DIR}/muon/engine/framegraph.hpp
    ${INCLUDE_DIR}/muon/engine/framehandler.hpp
    ${INCLUDE_DIR}/muon/engine/image.hpp
    ${INCLUDE_DIR}/muon/engine/model.hpp
    ${INCLUDE_DIR}/muon/engine/pipeline.hpp
    ${INCLUDE_DIR}/muon/engine/renderpass.hpp
    ${INCLUDE_DIR}/muon/engine/rendersystem.hpp
    ${INCLUDE_DIR}/muon/engine/swapchain.hpp
    ${INCLUDE_DIR}/muon/engine/texture.hpp
    ${INCLUDE_DIR}/muon/engine/vertex.hpp
    ${INCLUDE_DIR}/muon/engine/window.hpp
    ${INCLUDE_DIR}/muon/misc/logger.hpp
)

set(SRC_FILES
    ${SRC_DIR}/engine/descriptor/pool.cpp
    ${SRC_DIR}/engine/descriptor/setlayout.cpp
    ${SRC_DIR}/engine/descriptor/writer.cpp
    ${SRC_DIR}/engine/pipeline/compute.cpp
    ${SRC_DIR}/engine/pipeline/graphics.cpp
    ${SRC_DIR}/engine/buffer.cpp
    ${SRC_DIR}/engine/computesystem.cpp
    ${SRC_DIR}/engine/device.cpp
    ${SRC_DIR}/engine/framebuffer.cpp
    ${SRC_DIR}/engine/framegraph.cpp
    ${SRC_DIR}/engine/framehandler.cpp
    ${SRC_DIR}/engine/image.cpp
    ${SRC_DIR}/engine/model.cpp
    ${SRC_DIR}/engine/pipeline.cpp
    ${SRC_DIR}/engine/renderpass.cpp
    ${SRC_DIR}/engine/rendersystem.cpp
    ${SRC_DIR}/engine/swapchain.cpp
    ${SRC_DIR}/engine/texture.cpp
    ${SRC_DIR}/engine/window.cpp
    ${SRC_DIR}/misc/logger.cpp
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


add_library(${LIB_NAME} SHARED
    ${HEADER_FILES}
    ${SRC_FILES}

    ${IMGUI_FILES}
)

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR} ${IMGUI_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spng)
target_link_libraries(${LIB_NAME} PUBLIC SDL3 vulkan VulkanMemoryAllocator-Hpp VulkanMemoryAllocator glm)

add_library(muon::engine ALIAS ${LIB_NAME})
