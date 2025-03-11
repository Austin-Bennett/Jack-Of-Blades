#ifndef GAME_HPP
#define GAME_HPP
#include "game/lib/JOB.hpp"
#include "game/lib/utils.hpp"

class Game {
    public:

    double instant_fps = 0.0;
    double smoothed_fps = 0.0;
    double runtime = 0.0;
    double delta = 0.0;


    level test_level;

    Game();

    ~Game();

    void beginPlay();

    void endPlay();

    void update(double delta);

    void draw();

    void update_fps(double delta)  {
        instant_fps = 1/delta;
        if (fmod(runtime, 0.2) <= 0.001) smoothed_fps = (smoothed_fps + instant_fps)/2;
        if (std::isinf(smoothed_fps)) smoothed_fps = 0.0;
    }
};

#endif
