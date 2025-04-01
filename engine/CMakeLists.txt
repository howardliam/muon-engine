# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-engine)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(${LIB_NAME} SHARED
    ${INCLUDE_DIR}/muon/engine/version.hpp
)

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spdlog::spdlog)

add_library(muon::engine ALIAS ${LIB_NAME})
