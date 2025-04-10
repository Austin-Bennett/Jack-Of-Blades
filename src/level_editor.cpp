#define LEVEL_EDITOR

#include "AustinUtils.hpp"
#include "raylib.h"
#include <chrono>
#include <rlgl.h>

#include "game.hpp"
#include "game/settings.hpp"
#include "game/lib/globals.hpp"
#include "imgui-1.91.9b/imgui.h"
#include "imgui-1.91.9b/rlImGui.h"
#include "Editor/LevelEditor.hpp"


using namespace AustinUtils;
using namespace std::chrono;

logger L_raylib = logger("raylib");

void raylibLogCallback(const int logLevel, const char* fmt, va_list args) {
    try {
        //convert the raylib log into my own logging enum
        switch (logLevel) {
            case LOG_WARNING:
                L_raylib.c_log(LOG_TYPE::LOG_WARN, fmt, args);
            case TraceLogLevel::LOG_ERROR:
                L_raylib.c_log(LOG_TYPE::LOG_ERROR, fmt, args);
            case TraceLogLevel::LOG_DEBUG:
                L_raylib.c_log(LOG_TYPE::LOG_DEBUG, fmt, args);
            default:
                L_raylib.c_log(LOG_TYPE::LOG_INFO, fmt, args);
        }
    } catch ([[maybe_unused]] const std::exception& e) {
        L_raylib.log(LOG_TYPE::LOG_ERROR, "Error logging");
    }
}



int main(int rargc, char** rargv) {

    unordered_set<str> argv = unordered_set<str>(rargv, rargv+rargc);
    bool log_raylib_stuff = true;
    if (unordered_set<str>::iterator i; (i = argv.find("--noraylib")) != argv.end()) {
        log_raylib_stuff = false;
    }
    auto LMain = logger("main");

    for (const auto &s: LevelObjectRegistry::instance().factories | views::keys) {
        LMain.info("Found registered type: ", s);
    }

    INIT(log_raylib_stuff ? LOG_ALL:LOG_NONE);
    MaximizeWindow();

    RenderTexture2D light_texture = Allocator::allocateRenderTexture(base_resolution.x, base_resolution.y);

    rlImGuiSetup(true);
    AnimationRegistry::Instance();

    auto editor = LevelEditor(&running);
    settings::initialize();
    //ensure we pre-process the settings
    settings::instance();

    const Shader post = Allocator::allocateShader(nullptr, "resources/shaders/post.fsh");

    while (running) {

        if (WindowShouldClose()) {
            editor.levelEditorQuit();
        }
        UPDATE_DELTA_EDITOR();


        //updates
        editor.update(delta);


        //draw to buffer
        BeginTextureMode(rbuf);

        //clear the background
        ClearBackground(BLACK);
        //draw the game
        editor.draw();

        EndTextureMode();

        //draw the lighting
        BeginTextureMode(light_texture);
        BeginBlendMode(BLEND_CUSTOM);
        rlSetBlendFactors(RL_ONE, RL_ZERO, RL_FUNC_ADD);

        ClearBackground(BLANK);
        editor.drawLighting();
        EndBlendMode();

        EndTextureMode();

        //wont work as a macro :/
        //calculate screen scaling
        win_scale = cast(fmin(cast(GetScreenWidth(), float) / base_resolution.x, cast(GetScreenHeight(), float)/base_resolution.y), float) * zoom;
        win_res = {win_scale*base_resolution.x, win_scale*base_resolution.y};
        win_pos = {-(win_res.x - cast(GetScreenWidth(), float)) / 2.0f, -(win_res.y - cast(GetScreenHeight(), float)) / 2.0f};

        //draw to the screen
        BeginDrawing();
        BeginShaderMode(post);

        ClearBackground(BLACK);


        DRAW_GAME_CONTENT(rbuf.texture)
        EndShaderMode();

        DRAW_GAME_CONTENT(light_texture.texture);

        rlImGuiBegin();
        editor.drawUI();
        rlImGuiEnd();
        EndDrawing();

        frame_end = high_resolution_clock::now();
    }


    Allocator::free();
    CloseWindow();
}