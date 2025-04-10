#ifndef ENUMS_HPP
#define ENUMS_HPP
#include <AustinUtils.hpp>
using namespace AustinUtils;


enum class collisionType : u32 {
    NO_COLLISION,//no collision at all
    EVENTS_ONLY,//only calls collision-related events
    AI_OBSTACLE_MARKER,//only used for AI, no hit events or collision
    BLOCK_ALL//blocks everything without prejudice, can be used for AI and creates hit events
};

inline const char* collisionTypeToString(void* data) {
    switch (*static_cast<collisionType*>(data)) {
        case collisionType::NO_COLLISION:
            return "No Collision";
        case collisionType::EVENTS_ONLY:
            return "Events Only";
        case collisionType::AI_OBSTACLE_MARKER:
            return "AI Obstacle Marker";
        case collisionType::BLOCK_ALL:
            return "Block All";
    }

    return "unknown";
}

enum class animation_type : u32 {
    NONE,
    LOOP,
    ONCE
};

inline const char* animation_typeToString(void* data) {
    switch (*cast(data, animation_type*)) {
        case animation_type::NONE:
            return "Null";
        case animation_type::LOOP:
            return "Loop";
        case animation_type::ONCE:
            return "Once";
    }
    return "Unknown";
}


#endif
