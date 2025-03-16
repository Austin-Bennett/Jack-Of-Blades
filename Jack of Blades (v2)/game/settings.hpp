#include <unordered_set>
#include "lib/utils.hpp"

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

struct keybind {
    i32 keyboard{};
    i32 mouse{};
    str description;
};

inline json makeJObjectFromKeybind(str name, const keybind& k) {
    json ret;
    ret["id"] = name.stdStr();
    ret["description"] = k.description;
    ret["key"] = k.keyboard;
    ret["mouse"] = k.mouse;

    return ret;
}


class settings {
private:

    explicit settings() {
        LSettings.info("Reading settings.json...");
        ifstream infile("settings.json");
        json data;
        if (infile.is_open()) {
            infile >> data;
            infile.close();
            if (!validateJsonFile(data)) {
                LSettings.warn("Could not validate settings.json, creating default settings!");
                createDefaultSettings(data);
            }
        } else {
            LSettings.warn("No settings.json file found, creating new one...");
            createDefaultSettings(data);
        }
        //set the keybinds
        for (auto& obj: data["keybindings"]) {
            LSettings.info("Keybind ID: ", obj["id"], " set to key binding ", KeyToString(obj["key"].get<KeyboardKey>()));
            str name = obj["id"].get<string>();
            KeybindRegistry::instance()[name].description = obj["description"].get<string>();
            KeybindRegistry::instance()[name].keyboard = obj["key"].get<i32>();
            KeybindRegistry::instance()[name].mouse = obj["mouse"].get<i32>();
        }


        //output the json again
        ofstream outfile("settings.json");
        outfile << data.dump(4);
        outfile.close();
    }

    inline static auto LSettings = logger("player-settings");

    inline static bool initialized = false;
public:

    class KeybindRegistry {
        private:
        unordered_map<str, keybind> keybindings{};

        KeybindRegistry() = default;

        logger LKeybindRegistry = logger("keybind-registry");

        public:
        friend settings;

        static KeybindRegistry& instance() {
            static KeybindRegistry k{};
            return k;
        }

        void registerKeybind(const str& id, const i32 default_keyboard, const i32 default_mouse, const str& default_description) {
            keybindings[id] = {
                .keyboard = default_keyboard,
                .mouse = default_mouse,
                .description = default_description,
            };
            LKeybindRegistry.info("Registered new keybind: ", id);
        }

        unordered_map<str, keybind> getKeybindings() {
            return keybindings;
        }

        bool contains(const str& id) const {
            return keybindings.contains(id);
        }

        keybind& get(const str& id) {
            return keybindings[id];
        }

        keybind& operator[](const str& id) {
            return keybindings[id];
        }
    };

    static settings& instance() {
        if (!initialized) {
            throw Exception("Cannot use settings until it is initialized");
        }
        static auto s = settings();
        return s;
    }

    static void initialize() {
        LSettings.info("Initialized settings");
        initialized = true;
    }

    static keybind& get_kb(const str& id) {
        return KeybindRegistry::instance()[id];
    }

    static bool validateJsonFile(json& data) {
        if (validateJsonData(data, "keybindings", json::value_t::array)) {
            for (const auto& obj: data["keybindings"].get<vector<json>>()) {
                if (!validateJsonData(obj, "id", json::value_t::string)) {
                    LSettings.warn("keybind does not have an \"ID\" field, validation failed");
                    goto false_;
                }
                if (!KeybindRegistry::instance().contains(obj["id"].get<string>())) {
                    LSettings.warn("keybind has unrecognized ID, validation failed");
                    goto false_;
                }
                str id = obj["id"].get<string>();
                if (!validateJsonData(obj, "key", JSON_INTEGERS)) {
                    LSettings.warn("keybind ", id, " does not have a \"key\" field, validation failed");
                    goto false_;
                }
                if (!validateJsonData(obj, "mouse", JSON_INTEGERS)) {
                    LSettings.warn("keybind ", id, "  does not have a \"mouse\" field, validation failed");
                    goto false_;
                }
                if (!validateJsonData(obj, "description", json::value_t::string)) {
                    LSettings.warn("keybind ", id, "  does not have a \"description\" field, validation failed");
                    goto false_;
                }
                LSettings.info("Validated keybind ", obj["id"].get<string>());
            }
            //check that we got all the keybindings
            for (const auto& [name, keybind] : KeybindRegistry::instance().getKeybindings()) {
                bool found = false;

                for (auto& obj : data["keybindings"]) {
                    if (obj["id"] == name.data()) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    LSettings.warn("Could not find keybind by ID lookup: ", name);
                    goto false_;
                }
            }
        }
        LSettings.info("Validated settings successfully");
        return true;

        false_:
            return false;
    }

    static void createDefaultSettings(json& data) {
        data.clear();

        data["keybindings"] = json::array();
        for (auto&[name, keybind]: KeybindRegistry::instance().getKeybindings()) {
            data["keybindings"].push_back(makeJObjectFromKeybind(name, keybind));
        }
    }
};

inline bool IsKeybindDown(const keybind& key) {
    return IsKeyDown(key.keyboard) || (IsMouseButtonPressed(key.mouse) && key.mouse != -1);
}

inline bool IsKeybindPressed(const keybind& key) {
    return IsKeyPressed(key.keyboard) || (IsMouseButtonPressed(key.mouse) && key.mouse != -1);
}

inline bool IsKeybindReleased(const keybind& key) {
    return IsKeyReleased(key.keyboard) || (IsMouseButtonPressed(key.mouse) && key.mouse != -1);
}

inline bool IsKeybindUp(const keybind& key) {
    return IsKeyUp(key.keyboard) || (IsMouseButtonPressed(key.mouse) && key.mouse != -1);
}

#endif
