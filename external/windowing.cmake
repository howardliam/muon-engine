FetchContent_Declare(
    sdl
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
    GIT_TAG         release-3.2.18
)
set(SDL_TEST_LIBRARY OFF)
FetchContent_MakeAvailable(sdl)
