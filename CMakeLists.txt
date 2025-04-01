cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

project(muon)

add_subdirectory(engine)
add_subdirectory(common)

set(SPDLOG_USE_STD_FORMAT ON FORCE)
add_subdirectory(external/spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()

add_subdirectory(test)
