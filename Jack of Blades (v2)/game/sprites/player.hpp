#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "sprite.hpp"
#include "../settings.hpp"

struct player : sprite {
    using animation_t = shared_ptr<animation>;
    animation_t current_animation;

#define SWITCH_ANIMATION(s) current_animation.reset();\
                               current_animation = animations[#s];\

#define ANIM(s) animations[#s]

    inline static unordered_map<str, shared_ptr<animation>> animations;
    inline static bool initialized = false;

public:
    explicit player(spriteSpawnPoint* spawn_point) : sprite(
        sprite::Settings().Collision(rect(spawn_point->getCollision().pos(), 64, 64)).collisionType(collisionType::BLOCK_ALL).maxHealth(100).
        startingHealth(100)) {

        spawn = spawn_point;
        if (!initialized) {
            initialized = true;
            animations["fall"] = AnimationRegistry::Instance().get("player_fall");
            animations["front_idle"] = AnimationRegistry::Instance().get("player_front_idle");
            animations["front_walking"] = AnimationRegistry::Instance().get("player_front_walking");
        }

        for (const auto &anim: animations | views::values) {
            anim->reset();
        }

        current_animation = ANIM(front_idle);
    }

    void update(const seconds_t delta) override {
        if (delta == 0) return;
        current_animation->update(delta);
        sprite::update(delta);
        if (isDead()) return;
        if (!on_ground) {
            if (current_animation != ANIM(fall)) {
                SWITCH_ANIMATION(fall);
            }
            if (current_animation->isFinished()) {
                health = 0.0;
                return;
            }
            return;
        }

        //update movement if were not colliding
        dvec2 movement{};

        if (IsKeybindDown(settings::get_kb("move_up"))) {
            movement.y = -1;
        }
        if (IsKeybindDown(settings::get_kb("move_down"))) {
            movement.y = 1;
        }
        if (IsKeybindDown(settings::get_kb("move_left"))) {
            movement.x = -1;
        }
        if (IsKeybindDown(settings::get_kb("move_right"))) {
            movement.x = 1;
        }

        if (movement.length2() != 0) {
            movement.normalize();
            movement *= 180*delta;
            move(movement, true);
        } else {
            moving = false;
            move_dir = NONE;
        }


        if (moving && current_animation != ANIM(front_walking)) {
            SWITCH_ANIMATION(front_walking);
        }
        if (current_animation != ANIM(front_idle) && on_ground && !moving) {
            SWITCH_ANIMATION(front_idle);
        }
    }

    void OnSpawn() override {
        Level->focusScroll(collision.pos());
    }

    void OnDeath() override {
        if (spawn) {
            spawn->spawn();
        }
    }

    void draw(const dvec2 offset) override {
        DrawAnimation(*current_animation, collision-offset);
    }
};

//GENERATE_SPAWN_POINT(player, "player_spawn_point");
struct playerSpawnPoint : spriteSpawnPoint {
private:
    static bool registered;

public:
    friend level;

    playerSpawnPoint() = default;

    explicit playerSpawnPoint(const dvec2 pos) : spriteSpawnPoint(pos) {
    }

    shared_ptr<sprite> spawn() override {
        shared_ptr<player> ret = Level->spawnObject<player>(this);
        return ret;
    }
};

struct playerSpawnPointFactory {
    using factory_type = playerSpawnPoint;

    [[nodiscard]] static shared_ptr<factory_type> createFromJson(json &data) {
        if (validateJsonData(data, "position", json::value_t::array)) {
            auto pos = data["position"].get<vector<double> >();
            if (pos.size() != 2) { throw Exception("Cannot create type from json data!"); }
            return make_shared<factory_type>(dvec2{pos[0], pos[1]});
        }
        return make_shared<factory_type>();
    }
};

inline bool playerSpawnPoint::registered = LevelObjectRegistry::instance().addEntry(
    "player_spawn_point", playerSpawnPointFactory::createFromJson);

#endif