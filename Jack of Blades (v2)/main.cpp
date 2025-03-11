#include "AustinUtils.hpp"
#include "raylib.h"
#include <chrono>

#include "game.hpp"
#include "game/lib/globals.hpp"

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



int main() {


    INIT();

    Game game = Game();

    Shader post = Allocator::instance().allocateShader(nullptr, "resources/shaders/post.fsh");
    game.beginPlay();

    while (running) {

        UPDATE_DELTA();


        //updates
        game.update_fps(delta);
        game.update(delta);

        //draw to buffer
        BeginTextureMode(rbuf);

        //clear the background
        ClearBackground(BLACK);
        //draw the game
        game.draw();

        EndTextureMode();

        //calculate window scaling
        CALCULATE_WINDOW_SCALING();

        //draw to the screen
        BeginDrawing();
        BeginShaderMode(post);

        ClearBackground(BLACK);
        DRAW_GAME_CONTENT(rbuf.texture);

        EndShaderMode();
        EndDrawing();

        frame_end = high_resolution_clock::now();
    }

    game.endPlay();
    Allocator::instance().free();
    CloseWindow();
}