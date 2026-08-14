cmake_minimum_required(VERSION 3.22)

project(third_person_cube_android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PLATFORM Android CACHE STRING "" FORCE)
set(ANDROID_NDK "${CMAKE_ANDROID_NDK}" CACHE PATH "" FORCE)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_library(main SHARED
    ../../../../../src/main.cpp
)

target_link_libraries(main PRIVATE raylib)
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)