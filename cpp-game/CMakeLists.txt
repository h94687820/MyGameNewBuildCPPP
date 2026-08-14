cmake_minimum_required(VERSION 3.20)

project(third_person_cube_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)

add_executable(third_person_cube
    src/main.cpp
)

target_link_libraries(third_person_cube PRIVATE PkgConfig::RAYLIB)

if (UNIX AND NOT APPLE)
    target_link_libraries(third_person_cube PRIVATE m)
endif()