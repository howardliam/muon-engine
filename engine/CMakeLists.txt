# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(${LIB_NAME} SHARED
    ${INCLUDE_DIR}/muon/engine/device.hpp
    ${SRC_DIR}/device.cpp

    ${INCLUDE_DIR}/muon/engine/swapchain.hpp

    ${INCLUDE_DIR}/muon/engine/window.hpp
    ${SRC_DIR}/window.cpp
)

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spdlog)
target_link_libraries(${LIB_NAME} PUBLIC SDL3 vulkan VulkanMemoryAllocator-Hpp VulkanMemoryAllocator)
