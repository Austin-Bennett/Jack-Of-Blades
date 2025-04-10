#ifndef GLOBALS_HPP
#define GLOBALS_HPP
#include <math.hpp>
#include "raylib.h"
#include <chrono>
using namespace std::chrono;
using namespace AustinUtils;

inline float win_scale;
inline fvec2 win_res;
inline fvec2 win_pos;
inline float zoom = 4.0;

inline RenderTexture2D rbuf;

#define UV_COLOR Color{55, 155, 255, 255}

const ivec2 base_resolution = {1280, 720};


#define INIT(LOG_LEVEL) logger main_log = logger("main");\
               bool running = true;\
               bool paused = false;\
               SetTraceLogLevel(LOG_LEVEL);\
               SetTraceLogCallback(raylibLogCallback);\
               SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);\
               InitWindow(base_resolution.x, base_resolution.y, "Game");\
               SetWindowSize(1280, 720);\
               SetWindowPosition(GetMonitorWidth(0)/2-1280/2, GetMonitorHeight(0)/2-720/2);\
               SetExitKey(0);\
               rbuf = Allocator::allocateRenderTexture(base_resolution.x, base_resolution.y);\
               double delta = 0;\
               high_resolution_clock::time_point frame_start;\
               high_resolution_clock::time_point frame_end;\

#define UPDATE_DELTA() running = !WindowShouldClose();\
                       delta = duration_cast<nanoseconds>(frame_end-frame_start).count()/1e9;\
                       frame_start = high_resolution_clock::now();\
                       if (paused) delta = 0.0;\
                       game.delta = delta;\
                       game.runtime += delta;\

#define UPDATE_DELTA_EDITOR() delta = duration_cast<nanoseconds>(frame_end-frame_start).count()/1e9;\
                              frame_start = high_resolution_clock::now();\
                              if (paused) delta = 0.0;\



#define DRAW_GAME_CONTENT(texture) DrawTexturePro(texture,\
                                   rect{0, 0, base_resolution.x, -base_resolution.y},\
                                   rect{win_pos.x, win_pos.y, win_res.x, win_res.y}, {0, 0}, 0, WHITE);\

#endif
