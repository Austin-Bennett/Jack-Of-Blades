cmake_minimum_required(VERSION 3.30)
project(Main)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Werror -Wno-class-memaccess -g -Wno-delete-incomplete)

add_executable(Main main.cpp
        game/lib/globals.hpp
        game.cpp
        game.hpp
        game/lib/utils.hpp
        game/lib/JOB.hpp)


# Include directories (-I flag)
include_directories(
        C:/msys64/mingw64/include
)

# Link directories (-L flag)
link_directories(
        C:/msys64/mingw64/lib
)

target_link_libraries(
        Main
        AustinUtils
        raylib
        opengl32
        gdi32
        winmm
)

