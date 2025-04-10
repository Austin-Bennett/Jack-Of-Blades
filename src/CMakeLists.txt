cmake_minimum_required(VERSION 3.30)
project(Main)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(
        -Wall
        -Werror
        -g
)

set(
        IMGUI_SOURCE_FILES
        imgui-1.91.9b/imgui.cpp
        imgui-1.91.9b/imgui_draw.cpp
        imgui-1.91.9b/imgui_tables.cpp
        imgui-1.91.9b/imgui_widgets.cpp
        imgui-1.91.9b/imgui_demo.cpp
        imgui-1.91.9b/rlImGui.cpp
        imgui-1.91.9b/rlImGui.h
        imgui-1.91.9b/imconfig.h
        imgui-1.91.9b/imgui.h
        imgui-1.91.9b/imgui_impl_raylib.h
        imgui-1.91.9b/imgui_internal.h
        imgui-1.91.9b/imstb_rectpack.h
        imgui-1.91.9b/imstb_textedit.h
        imgui-1.91.9b/imstb_truetype.h
        imgui-1.91.9b/rlImGuiColors.h
        game/lib/enums.hpp

)

add_executable(Main main.cpp
        game/lib/globals.hpp
        game.cpp
        game.hpp
        game/lib/utils.hpp
        game/lib/JOB.hpp
        game/sprites/sprite.hpp
        game/sprites/player.hpp
        game/settings.hpp
)

add_executable(
        LevelEditor
        level_editor.cpp
        game/lib/globals.hpp
        game.cpp
        game.hpp
        game/lib/utils.hpp
        game/lib/JOB.hpp
        game/sprites/sprite.hpp
        game/sprites/player.hpp
        game/settings.hpp
        ${IMGUI_SOURCE_FILES}
        Editor/LevelEditor.hpp
        Editor/tinyfiledialogs.c
        Editor/tinyfiledialogs.h
        Editor/utils.hpp
)

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

target_link_libraries(
        LevelEditor
        AustinUtils
        raylib
        opengl32
        gdi32
        winmm
)
