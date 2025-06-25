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

set(HEADER_FILES
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
    ${INCLUDE_DIR}/muon/graphics/frame_manager.hpp
    ${INCLUDE_DIR}/muon/graphics/gpu.hpp
    ${INCLUDE_DIR}/muon/graphics/queue.hpp
    ${INCLUDE_DIR}/muon/graphics/queue_info.hpp
    ${INCLUDE_DIR}/muon/graphics/swapchain.hpp

    ${INCLUDE_DIR}/muon/input/key.hpp
    ${INCLUDE_DIR}/muon/input/modifier.hpp
    ${INCLUDE_DIR}/muon/input/mouse.hpp

    ${INCLUDE_DIR}/muon/scripting/script_manager.hpp

    ${INCLUDE_DIR}/muon/utils/color.hpp
    ${INCLUDE_DIR}/muon/utils/nocopy.hpp
    ${INCLUDE_DIR}/muon/utils/nomove.hpp
    ${INCLUDE_DIR}/muon/utils/pretty_print.hpp
)

set(SRC_FILES
    ${SRC_DIR}/core/application.cpp
    ${SRC_DIR}/core/log.cpp
    ${SRC_DIR}/core/window.cpp

    ${SRC_DIR}/debug/profiler.cpp

    ${SRC_DIR}/graphics/device_context.cpp
    ${SRC_DIR}/graphics/frame_manager.cpp
    ${SRC_DIR}/graphics/gpu.cpp
    ${SRC_DIR}/graphics/queue.cpp
    ${SRC_DIR}/graphics/queue_info.cpp
    ${SRC_DIR}/graphics/swapchain.cpp

    ${SRC_DIR}/input/modifier.cpp

    ${SRC_DIR}/scripting/script_manager.cpp

    ${SRC_DIR}/utils/color.cpp
    ${SRC_DIR}/utils/pretty_print.cpp
)

add_library(${LIB_NAME} STATIC
    ${HEADER_FILES}
    ${SRC_FILES}
)
add_library(muon::engine ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PRIVATE
    Vulkan::Headers
)

target_link_libraries(${LIB_NAME} PUBLIC
    fmt::fmt spdlog::spdlog_header_only
    yaml-cpp::yaml-cpp
    glfw eventpp::eventpp
    Tracy::TracyClient
    Vulkan::Loader VulkanMemoryAllocator
    glm
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(MU_DEBUG_ENABLED)
endif()
