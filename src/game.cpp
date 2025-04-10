#include "game.hpp"
#include <raylib.h>
#include <str.hpp>
#include <utility>
#include "game/settings.hpp"
#include "game/lib/globals.hpp"
#include "imgui-1.91.9b/imgui.h"
#include "imgui-1.91.9b/rlImGui.h"

void register_keybinds() {
    settings::KeybindRegistry::instance().registerKeybind("move_up", KEY_W, -1, "Moves the player up");
    settings::KeybindRegistry::instance().registerKeybind("move_left", KEY_A, -1, "Moves the player left");
    settings::KeybindRegistry::instance().registerKeybind("move_right", KEY_D, -1, "Moves the player right");
    settings::KeybindRegistry::instance().registerKeybind("move_down", KEY_S, -1, "Moves the player down");
    settings::KeybindRegistry::instance().registerKeybind("debug_mode", KEY_F3, -1, "Switches debug mode on/off");
}

Game::Game() {
    //all keybindings should be registered here, after the game is made,
    //settings will be initialized and adding new keybind registries will not work
    register_keybinds();
    current_level = make_unique<level>("data/level/example.json");
}

Game::Game(const bool editor) : editor_mode(editor) {
    register_keybinds();
}

Game::~Game() = default;

void Game::beginPlay() {

}

void Game::endPlay() {

}


void Game::draw() {
    if (current_level) current_level->draw({});

    if (debug) debug_draw();
}

void Game::debug_draw() {
    if (current_level) current_level->debugDrawCollision();
}

void Game::editor_update(double delta) {
    if (IsKeybindPressed(settings::get_kb("debug_mode"))) {
        debug = !debug;
    }
}


void Game::drawUI() {

    if (debug) {
        //draw the frame profiler
        DrawText(("FPS |"_str + " current: " + str(smoothed_fps, 0) + " average: " +
            str(average_fps, 0) + " low 1%: " + str(low_1fps, 0) + " high 1%: " + str(high_1fps, 0))
            .data(), 20, 20, 20, MAGENTA);
        //draw level scrolling
        if (current_level) DrawText(("Level scroll | "_str + current_level->Scroll()).data(), 20, 37, 20, MAGENTA);
    }




}


void Game::update(const double delta) {
    if (current_level) current_level->update(delta);
    if (IsKeybindPressed(settings::get_kb("debug_mode"))) {
        debug = !debug;
    }
}


void Game::change_level(const char* new_json) {
    if (new_json[0] == '\0') {
        current_level.reset();
        return;
    }
    current_level = make_unique<level>(new_json);
}
