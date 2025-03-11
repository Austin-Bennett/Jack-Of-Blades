#include "game.hpp"
#include <raylib.h>
#include <str.hpp>


Game::Game() : test_level("data/level/example.json") {
}

Game::~Game() = default;

void Game::beginPlay() {

}

void Game::endPlay() {

}


void Game::draw() {
    test_level.draw({});
    test_level.debugDrawCollision();
    DrawText(("FPS: "_str + smoothed_fps).data(), 20, 20, 20, {55, 155, 255, 255});
}

void Game::update(const double delta) {
    test_level.update(delta);
    static constexpr double scroll_speed = 100;
    if (IsKeyDown(KEY_W)) {
        test_level.addScroll({0, -scroll_speed*delta});
    }
    if (IsKeyDown(KEY_S)) {
        test_level.addScroll({0, scroll_speed*delta});
    }
    if (IsKeyDown(KEY_A)) {
        test_level.addScroll({-scroll_speed*delta, 0});
    }
    if (IsKeyDown(KEY_D)) {
        test_level.addScroll({scroll_speed*delta, 0});
    }
}


