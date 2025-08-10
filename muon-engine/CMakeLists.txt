# Engine Library

add_library(muon-engine STATIC)
add_library(muon::engine ALIAS muon-engine)

target_sources(
    muon-engine
    PRIVATE
        src/muon/config/config_manager.cpp

        src/muon/core/application.cpp
        src/muon/core/buffer.cpp
        src/muon/core/layer_stack.cpp
        src/muon/core/log.cpp
        src/muon/core/uuid.cpp
        src/muon/core/window.cpp

        src/muon/crypto/hash.cpp

        src/muon/format/bytes.cpp

        src/muon/fs/fs.cpp

        src/muon/input/modifier.cpp

        src/muon/maths/alignment.cpp

        src/muon/project/project.cpp

    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src/
    FILES
        src/muon/config/config_manager.hpp

        src/muon/core/application.hpp
        src/muon/core/buffer.hpp
        src/muon/core/debug.hpp
        src/muon/core/entry_point.hpp
        src/muon/core/expect.hpp
        src/muon/core/layer.hpp
        src/muon/core/layer_stack.hpp
        src/muon/core/log.hpp
        src/muon/core/types.hpp
        src/muon/core/uuid.hpp
        src/muon/core/window.hpp

        src/muon/crypto/hash.hpp

        src/muon/event/dispatcher.hpp
        src/muon/event/event.hpp

        src/muon/format/bytes.hpp

        src/muon/fs/fs.hpp

        src/muon/input/key.hpp
        src/muon/input/modifier.hpp
        src/muon/input/mouse.hpp

        src/muon/maths/alignment.hpp

        src/muon/project/project.hpp

        src/muon/serde/binary.hpp
        src/muon/serde/serde.hpp
        src/muon/serde/toml.hpp

        src/muon/utils/no_copy.hpp
        src/muon/utils/no_move.hpp
        src/muon/utils/platform.hpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    target_sources(
        muon-engine
        PRIVATE
            src/muon/utils/platform_posix.cpp
    )

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    target_sources(
        muon-engine
        PRIVATE
            src/muon/utils/platform_win32.cpp
    )

endif()

target_link_libraries(muon-engine PRIVATE
    spirv-reflect-static SPIRV-Tools glslang
    sodium
)

target_link_libraries(muon-engine PUBLIC
    fmt::fmt spdlog::spdlog_header_only
    tomlplusplus::tomlplusplus nlohmann_json::nlohmann_json
    SDL3::SDL3-static
    eventpp::eventpp
    Vulkan::Headers Vulkan::Hpp
    VulkanMemoryAllocator VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
    glm
    magic_enum::magic_enum
    spng_static
    SQLiteCpp
)

add_compile_definitions(
    $<$<CONFIG:Debug>:MU_DEBUG>
)

option(MUON_ENGINE_TESTS "Enable Muon Engine tests" ON)
if(MUON_ENGINE_TESTS)

    add_executable(muon-tests)

    target_sources(
        muon-tests
        PRIVATE
            tests/main.cpp

            tests/core/uuid.cpp

            tests/maths/alignment.cpp
    )

    target_link_libraries(muon-tests PRIVATE
        Catch2
        muon::engine
    )

endif()
