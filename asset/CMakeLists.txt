# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-asset)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADER_FILES
    # ${INCLUDE_DIR}/muon/asset/audio/opus.hpp
    # ${INCLUDE_DIR}/muon/asset/audio/wav.hpp
    # ${INCLUDE_DIR}/muon/asset/audio.hpp

    # ${INCLUDE_DIR}/muon/asset/image/jpeg.hpp
    # ${INCLUDE_DIR}/muon/asset/image/png.hpp
    ${INCLUDE_DIR}/muon/asset/image.hpp

    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/camera.hpp
    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/image.hpp
    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/material.hpp
    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/mesh.hpp
    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/node.hpp
    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/sampler.hpp
    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/scene.hpp
    # ${INCLUDE_DIR}/muon/asset/model/scene-graph/texture.hpp
    # ${INCLUDE_DIR}/muon/asset/model/gltf.hpp
)

set(SRC_FILES
    # ${SRC_DIR}/asset/audio/opus.cpp
    # ${SRC_DIR}/asset/audio/wav.cpp

    # ${SRC_DIR}/asset/image/jpeg.cpp
    ${SRC_DIR}/asset/image/png.cpp

    # ${SRC_DIR}/asset/model/gltf.cpp

    # ${SRC_DIR}/asset/assetloader.cpp
    # ${SRC_DIR}/asset/image.cpp
)

add_library(${LIB_NAME} STATIC
    ${HEADER_FILES}
    ${SRC_FILES}
)

target_include_directories(${LIB_NAME} PRIVATE ${OPUS_INCLUDE_DIRS} ${OPUSFILE_INCLUDE_DIRS})
target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spng nlohmann_json turbojpeg ${OPUS_LIBRARIES} ${OPUSFILE_LIBRARIES})
target_link_libraries(${LIB_NAME} PUBLIC)

add_library(muon::asset ALIAS ${LIB_NAME})

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

add_executable(asset-test
    ${TEST_DIR}/test.cpp
)

target_link_libraries(asset-test PRIVATE muon::asset)
