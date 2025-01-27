#pragma once

#include "lib/lib.hpp"
#include <fstream>


//a structure for describing keybinds
struct keybind {
    i32 keycode;
    string name;
    string description;
};

class player {
    
    public:
    struct settings {
        unordered_map<std::string, keybind> keybinds;
    };

    protected:
    //non-gameplaye variables
    JoB::animation* current_animation;
    unordered_map<string, JoB::animation> animations;
    settings player_settings;

    //gameplay variables
    JoB::levelEntity selfSpr;
    bool is_falling = 0;
    bool is_moving = 0;
    double current_velocity = 0.0;

    //allocator
    JoB::Allocator& allocator;
    //player logging
    JoB::logger log = JoB::logger("player");

    public:

    //max velocity and acceleration of the player
    const double max_velocity = 180;//in pixels-per-second
    const double max_acceleration = 30.0;//still pps

    

    player(JoB::Allocator& allocator) : selfSpr(this, {0, 0}, {0, 0, 64, 64}), allocator(allocator) {
        
        //load animations
        animations["falling"] = JoB::animation(allocator, "resources\\Characters\\Player\\Fall.png",
        {32, 32},
        0.055,
        JoB::animation::ONCE);

        //front animations
        animations["front_idle"] = JoB::animation(allocator, "resources\\Characters\\Player\\Front\\FrontIdle.png", {32, 32}, 0.75);
        animations["front_walking"] = JoB::animation(
            allocator,
            "resources\\Characters\\Player\\Front\\FrontWalking.png",
            {32, 32},
            0.1
        );

        

        
        //start out front facing
        current_animation = &animations["front_idle"];

        
        //load settings in
        setup_settings();
        
        //setup the draw command
        selfSpr.draw = this->draw;
        selfSpr.update = this->update;
        //TODO: add an update function the sprite so levels can update sprites automatically
    }

    void setup_settings() {
        //setup settings
        //first create an abstract description of settings to be filled with values later
        player_settings.keybinds["move_up"] = {};
        player_settings.keybinds["move_left"] = {};
        player_settings.keybinds["move_right"] = {};
        player_settings.keybinds["move_down"] = {};

        player_settings.keybinds["ui_interact"] = {};
        player_settings.keybinds["pause_game"] = {};
        player_settings.keybinds["fullscreen"] = {};


        //load from the settings.json file
        std::ifstream settings_file("settings.json");
        json dSettings;
        if (!settings_file.is_open()) {
            settings_file.close();
            
            dSettings = create_default_settings();
            write_default_settings(dSettings);

            return;
        }
        settings_file >> dSettings;

        //if we cant verify the settings we just roll with default settings
        if (!verify_settings(dSettings)) {
            log.log(JoB::logger::WARN, "Could not verify settings!");
            dSettings = create_default_settings();
            write_default_settings(dSettings);
        }

        //setup keybinds
        for (const auto& keybind: dSettings["keybinds"].items()) {
            player_settings.keybinds[keybind.key()] = {
                .keycode = keybind.value()["keycode"],
                .name = keybind.value()["name"],
                .description = keybind.value()["description"]
            };
        }

        settings_file.close();
    }

    JoB::levelEntity& getSpriteComponent() {
        return selfSpr;
    }
    
    //sets up default settings
    json create_default_settings() {
        log.log(JoB::logger::INFO, "Creating default settings");
        json dSettings;

        dSettings["keybinds"] = json::object();

        dSettings["keybinds"]["move_up"] = {
            {"keycode", KEY_W},
            {"name","Move up"},
            {"description", "Move up the screen"}
        };

        dSettings["keybinds"]["move_left"] = {
            {"keycode", KEY_A},
            {"name","Move left"},
            {"description", "Move left on the screen."}
        };

        dSettings["keybinds"]["move_right"] = {
            {"keycode", KEY_D},
            {"name","Move right"},
            {"description", "Move right on the screen."}
        };

        dSettings["keybinds"]["move_down"] = {
            {"keycode", KEY_S},
            {"name","Move down"},
            {"description", "Move down the screen."}
        };

        dSettings["keybinds"]["ui_interact"] = {
            {"keycode", MOUSE_LEFT_BUTTON},
            {"name","UI Interaction"},
            {"description", "The button for pressing UI buttons, or interacting similarly with other UI."}
        };

        dSettings["keybinds"]["pause_game"] = {
            {"keycode", KEY_ESCAPE},
            {"name","Pause Game"},
            {"description", "Pauses the game and shows the paused menu."}
        };

        dSettings["keybinds"]["fullscreen"] = {
            {"keycode", KEY_F11},
            {"name","Switch Fullscreen"},
            {"description", "Sets the screen mode to fullscreen."}
        };

        return dSettings;
    }

    bool verify_settings(json& settings) {
        //check keybinds
        if (!settings.contains("keybinds")) {
            log.log(JoB::logger::WARN, "Settings does not contain keybinds!");
            return false;
        };
        //check that each keybind exists, has the right fields and types
        for (auto const&[name, kb]: player_settings.keybinds) {
            //check that keybinds has all the required keybinds
            if (!settings["keybinds"].contains(name)) {
                log.log(JoB::logger::WARN, "Settings does not have keybind {}", name);
                return false;
            }

            //check that each kebind has valid values that can be used
            if (!settings["keybinds"][name].contains("keycode") || !settings["keybinds"][name]["keycode"].is_number_integer()) {
                log.log(JoB::logger::WARN, "keybind {} does not have a keycode or the keycode is invalid!", name);
                return false;
            }
            if (!settings["keybinds"][name].contains("name") || !settings["keybinds"][name]["name"].is_string()) {
                log.log(JoB::logger::WARN, "keybind {} does not have a name or the name is invalid!", name);
                return false;
            }
            if (!settings["keybinds"][name].contains("description") || !settings["keybinds"][name]["description"].is_string()) {
                log.log(JoB::logger::WARN, "keybind {} does not have a description or the description is invalid!", name);
                return false;
            }
        }


        //if we find no preoblems the settings are valid
        return true;
    }

    //helper function to write the settings out
    void write_default_settings(json& settings) {
        std::ofstream out_settings("settings.json");

        settings = create_default_settings();

        out_settings << settings.dump(4);
        out_settings.close();
    }

    //debug function
    void debugPrintKeybinds() {
        for (auto&[name, keybind]: player_settings.keybinds) {
            log.log(JoB::logger::DEBUG, "\n[keybind][{}] -> {{\n\tkeycode: {},\n\tname: {},\n\tdescription: {}\n}",
             name, keybind.keycode, keybind.name, keybind.description);
        }
    }

    keybind getKeybind(string name) {
        if (player_settings.keybinds.find(name) == player_settings.keybinds.end()) {
            log.log(JoB::logger::ERROR, "Could not find keybind by name {}", name);
            throw runtime_error("");
        }
        return player_settings.keybinds[name];
    }

    static void update(JoB::levelComponent& self, t_seconds delta);

    static void draw(JoB::levelComponent& self_comp, dvec2 scroll);

    void respawn() {
        //for now, set position to 0
        //TODO: make the player respawn at a spawn point
        selfSpr.setPosition({0, 0});
    }

    void start_falling() {
        is_falling = 1;
        current_animation = &animations["falling"];
    }

};