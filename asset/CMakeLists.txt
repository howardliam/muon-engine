# Engine Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-asset)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADER_FILES
    ${INCLUDE_DIR}/muon/asset/audio/opus.hpp
    ${INCLUDE_DIR}/muon/asset/audio/wav.hpp
    ${INCLUDE_DIR}/muon/asset/audio.hpp

    ${INCLUDE_DIR}/muon/asset/image/jpeg.hpp
    ${INCLUDE_DIR}/muon/asset/image/png.hpp
    ${INCLUDE_DIR}/muon/asset/image.hpp

    ${INCLUDE_DIR}/muon/asset/model/gltf.hpp
    ${INCLUDE_DIR}/muon/asset/model/obj.hpp
    ${INCLUDE_DIR}/muon/asset/model/scene.hpp
    ${INCLUDE_DIR}/muon/asset/model.hpp

    ${INCLUDE_DIR}/muon/asset/file.hpp
    ${INCLUDE_DIR}/muon/asset/error.hpp
)

set(SRC_FILES
    ${SRC_DIR}/asset/audio/opus.cpp
    ${SRC_DIR}/asset/audio/wav.cpp

    ${SRC_DIR}/asset/image/jpeg.cpp
    ${SRC_DIR}/asset/image/png.cpp
    ${SRC_DIR}/asset/image.cpp

    ${SRC_DIR}/asset/model/gltf.cpp
    ${SRC_DIR}/asset/model/obj.cpp
    ${SRC_DIR}/asset/model.cpp

    ${SRC_DIR}/asset/file.cpp
)

add_library(${LIB_NAME} SHARED
    ${HEADER_FILES}
    ${SRC_FILES}
)

target_include_directories(${LIB_NAME} PRIVATE ${OPUS_INCLUDE_DIRS} ${OPUSFILE_INCLUDE_DIRS})
target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PRIVATE spng nlohmann_json turbojpeg ${OPUS_LIBRARIES} ${OPUSFILE_LIBRARIES})
target_link_libraries(${LIB_NAME} PUBLIC)

add_library(muon::asset ALIAS ${LIB_NAME})
