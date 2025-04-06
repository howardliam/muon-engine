# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADER_FILES
    ${INCLUDE_DIR}/muon/engine/assets/image.hpp
    ${INCLUDE_DIR}/muon/engine/misc/logger.hpp
    ${INCLUDE_DIR}/muon/engine/buffer.hpp
    ${INCLUDE_DIR}/muon/engine/device.hpp
    ${INCLUDE_DIR}/muon/engine/renderer.hpp
    ${INCLUDE_DIR}/muon/engine/swapchain.hpp
    ${INCLUDE_DIR}/muon/engine/window.hpp
)

set (SRC_FILES
    ${SRC_DIR}/assets/image.cpp
    ${SRC_DIR}/buffer.cpp
    ${SRC_DIR}/device.cpp
    ${SRC_DIR}/renderer.cpp
    ${SRC_DIR}/swapchain.cpp
    ${SRC_DIR}/window.cpp
)

add_library(${LIB_NAME} SHARED
    ${HEADER_FILES}
    ${SRC_FILES}
)

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spng)
target_link_libraries(${LIB_NAME} PUBLIC SDL3 vulkan VulkanMemoryAllocator-Hpp VulkanMemoryAllocator)
