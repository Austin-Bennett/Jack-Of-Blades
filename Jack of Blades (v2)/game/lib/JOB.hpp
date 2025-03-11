#ifndef GAMELIB_HPP
#define GAMELIB_HPP

#include "utils.hpp"

using namespace AustinUtils;
using namespace std;





enum class collisionType : u32 {
    NO_COLLISION = 0,//no collision at all
    EVENTS_ONLY = 1,//only calls collision-related events
    SPRITES_ONLY = 2,//only blocks sprites, does not block LevelObjects
    LEVEL_OBJECTS_ONLY = 3,//only blocks LevelObjects
    AI_OBSTACLE_MARKER = 4,//only used for AI, no hit events or collision
    BLOCK_ALL = 5//blocks everything without prejudice, can be used for AI and creates hit events
};

struct collision_hit {
    Object* collidingObject;
};

class level;


#define GENERATE_LEVEL_OBJECT() friend level;\

template<typename T>
concept ObjectFactory = requires(T x, json& data)
{
    typename T::factory_type;
    {T::createFromJson(data)} -> same_as<unique_ptr<typename T::factory_type>>;
};

//static level object, does not move
struct LevelObject : public Object {
private:
    static bool registered;
protected:
    rect collision;//position and also collision
    collisionType eCollision = collisionType::BLOCK_ALL;
    /*
     *if a sprite is on top of the object and walkable is false, the sprite will fall
     *if collides is true then the value of walkable is ignored
     *if collides is false and walkable is true sprites will be able to walk on top of the object
     */
    bool walkable = false;


public:

    virtual ~LevelObject() = default;

    GENERATE_LEVEL_OBJECT();

    LevelObject(): collision({0, 0, 0, 0}) {
        eCollision = collisionType::NO_COLLISION;
        walkable = true;
    }

    explicit LevelObject(const rect &bounding_box) : collision(bounding_box) {};

    LevelObject(const rect &r, const collisionType eC, const bool walkable) : collision(r), eCollision(eC), walkable(walkable) {}

    LevelObject& CollisionType(const collisionType eCollision) {
        this->eCollision = eCollision;
        return *this;
    }

    NODISCARD collisionType CollisionType() const {
        return eCollision;
    }

    LevelObject& Walkable(const bool val) {
        walkable = val;
        return *this;
    }

    NODISCARD bool Walkable() const {
        return walkable;
    }

    virtual void debugDrawCollision(const dvec2 offset) {
        DrawRectangleLinesEx(collision-offset, 5, debug_colors[cast(eCollision, u32)]);
    }

    virtual void OnCollision(collision_hit hit) {

    }
};


class LevelObjectRegistry {
    unordered_map<str, function<unique_ptr<LevelObject>(json& dat)>> factories;
public:
    static LevelObjectRegistry& instance() {
        static LevelObjectRegistry inst;
        return inst;
    }

    bool addEntry(const str& type_id, const function<unique_ptr<LevelObject>(json& dat)>& factory) {
        //returns true if the registry was succesfully added
        if (factories.contains(type_id)) {
            return false;
        }
        factories[type_id] = factory;
        return true;
    }


    unique_ptr<LevelObject> create(const str& type_id, json& data) {
        if (!factories.contains(type_id)) throw Exception("Cannot create type ", type_id);
        return factories[type_id](data);
    }

};



struct LevelObjectFactory {
    virtual ~LevelObjectFactory() = default;


    using factory_type = LevelObject;

    NODISCARD static unique_ptr<factory_type> createFromJson(json& data) {
        rect c = rect();
        auto eC = collisionType::NO_COLLISION;
        bool w = true;
        if (validateJsonData(data, "collision", json::value_t::array)) {
            auto vals = data["collision"].get<vector<double>>();
            if (vals.size() != 4) {
                throw Exception("Cannot create collision from array of size ", vals.size());
            }
            c.x = vals[0];
            c.y = vals[1];
            c.w = vals[2];
            c.h = vals[3];
        }
        if (validateJsonData(data, "collision_type", json::value_t::number_unsigned)) {
            eC = data["collision_type"].get<collisionType>();
        }
        if (validateJsonData(data, "walkable", json::value_t::boolean)) {
            w = data["walkable"].get<bool>();
        }

        return make_unique<factory_type>(c, eC, w);
    }
};

inline bool LevelObject::registered = LevelObjectRegistry::instance().addEntry("level_object", LevelObjectFactory::createFromJson);

struct LevelFloor : public LevelObject {
private:
    static bool registered;
protected:
    Texture2D floor_texture;
public:
    GENERATE_LEVEL_OBJECT()
    LevelFloor(const rect &area, Texture2D floor_texture) : LevelObject(area, collisionType::NO_COLLISION, true),
    floor_texture(std::move(floor_texture)) {}

    void draw(const dvec2 offset) override {
        DrawTexturePro(floor_texture, collision.pure(), collision-offset, {0, 0}, 0, GRAY);
    }


};

struct LevelFloorFactory {

    using factory_type = LevelFloor;

    NODISCARD static unique_ptr<factory_type> createFromJson(json& data) {
        rect c = rect();
        Texture2D t;
        if (validateJsonData(data, "area", json::value_t::array)) {
            auto vals = data["area"].get<vector<double>>();
            if (vals.size() != 4) {
                throw Exception("Cannot create collision from array of size ", vals.size());
            }
            c.x = vals[0];
            c.y = vals[1];
            c.w = vals[2];
            c.h = vals[3];
        } else {
            throw Exception("Could not make Level Floor from json data");
        }
        if (validateJsonData(data, "texture", json::value_t::string)) {
            t = Allocator::instance().allocateTexture(("resources/"_str + data["texture"].get<string>()).data());
        }

        return make_unique<factory_type>(c, t);
    }
};

inline bool LevelFloor::registered = LevelObjectRegistry::instance().addEntry("level_floor", LevelFloorFactory::createFromJson);

struct LevelProp : public LevelObject {
private:
    static bool registered;
protected:
    animation texture;

public:
    explicit LevelProp(const animation& anim, rect r = {0, 0, 0, 0}, collisionType eCollision = collisionType::BLOCK_ALL, bool w = false) :
    LevelObject(r, eCollision, w)
    {
        texture = anim;
    }

    void update(seconds delta) override {
        texture.update(delta);
    }

    void draw(dvec2 offset) override {
        DrawAnimation(texture, collision-offset);
    }

    GENERATE_LEVEL_OBJECT();
};

struct LevelPropFactory {
    using factory_type = LevelProp;

    NODISCARD static unique_ptr<factory_type> createFromJson(json& data) {
        rect c = rect();
        auto eC = collisionType::NO_COLLISION;
        bool w = true;
        shared_ptr<animation> anim;
        if (validateJsonData(data, "collision", json::value_t::array)) {
            auto vals = data["collision"].get<vector<double>>();
            if (vals.size() != 4) {
                throw Exception("Cannot create collision from array of size ", vals.size());
            }
            c.x = vals[0];
            c.y = vals[1];
            c.w = vals[2];
            c.h = vals[3];
        }
        if (validateJsonData(data, "collision_type", json::value_t::number_unsigned)) {
            eC = data["collision_type"].get<collisionType>();
        }
        if (validateJsonData(data, "walkable", json::value_t::boolean)) {
            w = data["walkable"].get<bool>();
        }
        if (validateJsonData(data, "texture", json::value_t::string)) {
            anim = AnimationRegistry::Instance().get(Identifier("animation", data["texture"].get<string>()));
        } else {
            throw Exception("Cannot create level prop from json data");
        }

        return make_unique<factory_type>(*anim, c, eC, w);
    }
};

inline bool LevelProp::registered = LevelObjectRegistry::instance().addEntry("level_prop", LevelPropFactory::createFromJson);

class level : public Object {
private:
    vector<unique_ptr<LevelObject>> objects;
    str name;
    dvec2 scroll = {0, 0};
public:
    explicit level(const char* level_json) {
        ifstream file(level_json);
        if (!file.is_open()) {
            throw Exception("Could not create level from file ", level_json);
        }
        json data;
        file >> data;

        assertJsonData(data, "name", json::value_t::string);
        name = data["name"].get<string>();

        assertJsonData(data, "objects", json::value_t::array);
        vector<json> objs = data["objects"].get<vector<json>>();
        for (auto& obj: objs) {
            assertJsonData(obj, "type", json::value_t::string);
            objects.push_back(LevelObjectRegistry::instance().create(obj["type"].get<string>(), obj));
        }
    }

    void addScroll(dvec2 amt) {
        scroll += amt;
    }

    void focusScroll(dvec2 pos) {
        pos.x -= GetScreenWidth()/2.0;
        pos.y -= GetScreenHeight()/2.0;

        scroll = pos;
    }

    void update(seconds delta) override {
        for (const auto& obj: objects) {
            obj->update(delta);
        }
    }

    void draw(dvec2 offset) override {
        for (const auto& obj: objects) {
            obj->draw(scroll);
        }
    }

    void debugDrawCollision() {
        for (const auto& obj: objects) {
            obj->debugDrawCollision(scroll);
        }
    }
};

#endif