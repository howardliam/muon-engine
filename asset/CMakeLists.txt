# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-asset)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADER_FILES
    ${INCLUDE_DIR}/muon/assets/image/jpeg.hpp
    ${INCLUDE_DIR}/muon/assets/image/png.hpp
    ${INCLUDE_DIR}/muon/assets/model/gltf.hpp
    ${INCLUDE_DIR}/muon/assets/model/obj.hpp
    ${INCLUDE_DIR}/muon/assets/file.hpp
    ${INCLUDE_DIR}/muon/assets/image.hpp
    ${INCLUDE_DIR}/muon/assets/model.hpp
    ${INCLUDE_DIR}/muon/assets/scene.hpp
)

set(SRC_FILES
    ${SRC_DIR}/assets/image/jpeg.cpp
    ${SRC_DIR}/assets/image/png.cpp
    ${SRC_DIR}/assets/model/gltf.cpp
    ${SRC_DIR}/assets/model/obj.cpp
    ${SRC_DIR}/assets/file.cpp
    ${SRC_DIR}/assets/image.cpp
    ${SRC_DIR}/assets/model.cpp
)

add_library(${LIB_NAME} SHARED
    ${HEADER_FILES}
    ${SRC_FILES}
)

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})
target_include_directories(${LIB_NAME} PRIVATE)

target_link_libraries(${LIB_NAME} PRIVATE spng nlohmann_json turbojpeg)
target_link_libraries(${LIB_NAME} PUBLIC)

add_library(muon::asset ALIAS ${LIB_NAME})
