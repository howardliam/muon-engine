FetchContent_Declare(
    libsodium
    GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
    GIT_TAG 260622e5b69bce9b955603a98e46354125a932a4
)
set(SODIUM_DISABLE_TESTS ON)
FetchContent_MakeAvailable(libsodium)
