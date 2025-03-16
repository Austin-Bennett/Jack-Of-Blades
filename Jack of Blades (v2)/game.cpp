#include "game.hpp"
#include <raylib.h>
#include <str.hpp>
#include "game/settings.hpp"
#include "game/lib/globals.hpp"


Game::Game() : test_level("data/level/example.json") {
    //all keybindings should be registered here, after the game is made,
    //settings will be initialized and adding new keybind registries will not work

    settings::KeybindRegistry::instance().registerKeybind("move_up", KEY_W, -1, "Moves the player up");
    settings::KeybindRegistry::instance().registerKeybind("move_left", KEY_A, -1, "Moves the player left");
    settings::KeybindRegistry::instance().registerKeybind("move_right", KEY_D, -1, "Moves the player right");
    settings::KeybindRegistry::instance().registerKeybind("move_down", KEY_S, -1, "Moves the player down");
}

Game::~Game() = default;

void Game::beginPlay() {

}

void Game::endPlay() {

}


void Game::draw() {
    test_level.draw({});
    test_level.debugDrawCollision();
}

void Game::drawUI() {
    //draw the frame profiler
    DrawText(("FPS |"_str + " current: " + str(smoothed_fps, 0) + " average: " +
        str(average_fps, 0) + " low 1%: " + str(low_1fps, 0) + " high 1%: " + str(high_1fps, 0))
        .data(), 20, 20, 20, MAGENTA);
    DrawText(("Object Count: "_str + test_level.objects.size()).data(), 20, 37, 20, MAGENTA);
}


void Game::update(const double delta) {
    test_level.update(delta);
}


