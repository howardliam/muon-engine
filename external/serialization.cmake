FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY  https://github.com/marzer/tomlplusplus.git
    GIT_TAG         v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_Declare(
    json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
    GIT_TAG         v3.12.0
)
FetchContent_MakeAvailable(json)
