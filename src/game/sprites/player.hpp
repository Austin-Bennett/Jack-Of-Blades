#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "sprite.hpp"
#include "../settings.hpp"

struct player : sprite {
    GENERATE_DYNAMIC_LEVEL_OBJECT(player)
    using animation_t = shared_ptr<animation>;
    animation_t current_animation;

#define SWITCH_ANIMATION(s) current_animation.reset();\
                               current_animation = animations[#s];\

#define ANIM(s) animations[#s]

    inline static unordered_map<str, shared_ptr<animation>> animations;
    inline static bool initialized = false;
    double speed = 90;

public:
    explicit player(spriteSpawnPoint* spawn_point) : sprite(
        Settings().Collision(rect(spawn_point->getCollision().pos()+dvec2{5, 25}, 24, 7))
        .collisionType(collisionType::BLOCK_ALL).maxHealth(100).
        startingHealth(100)) {
        spawn = spawn_point;
        if (!initialized) {
            initialized = true;
            animations["fall"] = AnimationRegistry::Instance().get("player_fall");
            animations["front_idle"] = AnimationRegistry::Instance().get("player_front_idle");
            animations["front_walking"] = AnimationRegistry::Instance().get("player_front_walking");
            animations["side_idle"] = AnimationRegistry::Instance().get("side_idle");
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
                ANIM(fall)->reset();
                health -= 1;
                setPosition(last_valid_pos, false, false);
                move(-last_movement*500, false);
                Level->focusScroll(collision.center());
                on_ground = true;
                goto movement_;
            }
            return;
        }

        movement_:
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
            movement *= speed*delta;
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
        Level->focusScroll(collision.center());
    }

    bool OnDeath() override {
        setPosition(last_valid_pos, false, true);
        return false;
    }

    void draw(const dvec2 offset) override {
        sprite::drawShadow({collision.center().x-offset.x-1, collision.y+collision.h-offset.y}, collision.w/2.5f, 0.8f*collision.h);
        DrawAnimation(*current_animation, collision.pos()-offset-dvec2{5, 25});
    }
};

GENERATE_SPAWN_POINT(player, "player_spawn_point");


#endif