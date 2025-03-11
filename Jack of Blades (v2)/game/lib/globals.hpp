#ifndef GLOBALS_HPP
#define GLOBALS_HPP
#include <math.hpp>
#include "raylib.h"
#include <chrono>
using namespace std::chrono;

constexpr AustinUtils::ivec2 base_resolution = {1280, 720};

#define INIT() logger main_log = logger("main");\
               bool running = true;\
               bool paused = false;\
               SetTraceLogLevel(LOG_ALL);\
               SetTraceLogCallback(raylibLogCallback);\
               SetConfigFlags(FLAG_WINDOW_RESIZABLE);\
               InitWindow(base_resolution.x, base_resolution.y, "Game");\
               SetWindowSize(1280, 720);\
               SetWindowPosition(GetMonitorWidth(0)/2-1280/2, GetMonitorHeight(0)/2-720/2);\
               SetExitKey(0);\
               RenderTexture2D rbuf = Allocator::instance().allocateRenderTexture(base_resolution.x, base_resolution.y);\
               double delta = 0;\
               high_resolution_clock::time_point frame_start;\
               high_resolution_clock::time_point frame_end;\

#define UPDATE_DELTA() running = !WindowShouldClose();\
                       delta = duration_cast<nanoseconds>(frame_end-frame_start).count()/1e9;\
                       frame_start = high_resolution_clock::now();\
                       if (paused) delta = 0.0;\
                       game.delta = delta;\
                       game.runtime += delta;\

#define CALCULATE_WINDOW_SCALING() float scale = fmax(GetScreenWidth() / base_resolution.x, GetScreenHeight()/base_resolution.y);\
                                   fvec2 res = {scale*base_resolution.x, scale*base_resolution.y};\
                                   fvec2 pos = {-((res.x-GetScreenWidth())/2), -((res.y-GetScreenHeight())/2)};\

#define DRAW_GAME_CONTENT(texture) DrawTexturePro(texture,\
                                   {0, 0, base_resolution.x, -base_resolution.y},\
                                   {pos.x, pos.y, res.x, res.y}, {0, 0}, 0, WHITE);\

#endif
