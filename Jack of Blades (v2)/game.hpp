#ifndef GAME_HPP
#define GAME_HPP
#include "game/lib/JOB.hpp"
#include "game/lib/utils.hpp"
#include "game/sprites/sprite.hpp"
#include "game/sprites/player.hpp"

class Game {
    public:

    double instant_fps = 0.0;
    double smoothed_fps = 0.0;
    double average_fps = 0.0;
    double low_1fps = 0.0;
    double high_1fps = 0.0;
    double runtime = 0.0;
    double delta = 0.0;


    level test_level;

    Game();

    ~Game();

    void beginPlay();

    void endPlay();

    void update(double delta);

    void draw();

    void drawUI();

    void update_fps(double delta)  {
        instant_fps = 1/delta;

        if (fmod(runtime, 0.2) <= 0.001) smoothed_fps = (smoothed_fps + instant_fps)/2;

        if (fmod(runtime, 1) <= 0.001) {
            high_1fps = average_fps;
            low_1fps = average_fps;
        }
        if (std::isinf(smoothed_fps)) smoothed_fps = 0.0;
        average_fps = (average_fps + smoothed_fps)/2.0;
        high_1fps = max(high_1fps, smoothed_fps);
        low_1fps = min(low_1fps, smoothed_fps);
    }
};

#endif
