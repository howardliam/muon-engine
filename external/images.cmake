FetchContent_Declare(
    libspng
    GIT_REPOSITORY  https://github.com/randy408/libspng.git
    GIT_TAG         v0.7.4
)
set(BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(libspng)

# FetchContent_Declare(
#     libktx
#     GIT_REPOSITORY  https://github.com/KhronosGroup/KTX-Software.git
#     GIT_TAG         v4.4.0
# )
# set(KTX_FEATURE_TESTS OFF)
# set(KTX_FEATURE_TOOLS OFF)
# set(KTX_FEATURE_GL_UPLOAD OFF)
# FetchContent_MakeAvailable(libktx)
