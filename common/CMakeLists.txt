# Common Library

set(LIB_NAME ${CMAKE_PROJECT_NAME}-common)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(${LIB_NAME} SHARED

    ${INCLUDE_DIR}/muon/common/compress.hpp
    ${SRC_DIR}/compress.cpp

    ${INCLUDE_DIR}/muon/common/fs.hpp
    ${SRC_DIR}/fs.cpp

    ${INCLUDE_DIR}/muon/common/log.hpp
    ${SRC_DIR}/log.cpp

)

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PRIVATE zstd)

add_library(muon::common ALIAS ${LIB_NAME})
