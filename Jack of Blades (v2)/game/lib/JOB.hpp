#ifndef GAMELIB_HPP
#define GAMELIB_HPP

#include <misc.hpp>

#include "utils.hpp"

struct LevelObject;
using namespace AustinUtils;
using namespace std;


enum class collisionType : u32 {
    NO_COLLISION = 0,//no collision at all
    EVENTS_ONLY = 1,//only calls collision-related events
    AI_OBSTACLE_MARKER = 2,//only used for AI, no hit events or collision
    BLOCK_ALL = 3//blocks everything without prejudice, can be used for AI and creates hit events
};



//a structure for passing around collision
struct collision {
    //most of the time, description will be valid since we manage the collision explicitly and removal of collision
    //will mean removal of these objects, also the pointer represents a pointer to some stack memory which means we dont have to worry about sudden
    //deallocation since most objects in the game will last the whole runtime, those that dont are explicitly removed, meaning we can explicitly remove these
    //pointer references
    rect* description = nullptr;
    collisionType typ = collisionType::NO_COLLISION;
    shared_ptr<LevelObject> owner = nullptr;
};

struct collision_hit {
    bool hit{};//if we hit anything at all
    bool walkable{};//if we hit anything we can walk on
    vector<shared_ptr<LevelObject>> objects = {};//all the things we hit
    collisionType max_collider_status = collisionType::NO_COLLISION;//the maximum colliding status of an object we hit
};


class level;


#define GENERATE_LEVEL_OBJECT() private:\
                                static bool registered;\
                                public:\
                                friend level;\

template<typename T>
concept ObjectFactory = requires(T x, json& data)
{
    typename T::factory_type;
    {T::createFromJson(data)} -> same_as<unique_ptr<typename T::factory_type>>;
};

struct DynamicLevelObject;

//static level object, does not move
struct LevelObject : public Object {
private:
    bool dynamic = false;
    GENERATE_LEVEL_OBJECT()
    friend DynamicLevelObject;
protected:
    rect collision;//position and also collision
    collisionType eCollision = collisionType::BLOCK_ALL;
    /*
     *if a sprite is on top of the object and walkable is false, the sprite will fall
     *if collides is true then the value of walkable is ignored
     *if collides is false and walkable is true sprites will be able to walk on top of the object
     */
    bool walkable = false;
    level* Level = nullptr;
    usize ID = -1;



public:

    ~LevelObject() override {
        //just to make sure it's not in the level anymore
        Level = nullptr;
        ID = -1;
    }



    LevelObject(): collision({0, 0, 0, 0}) {
        eCollision = collisionType::NO_COLLISION;
        walkable = true;
    }

    explicit LevelObject(const rect &bounding_box) : collision(bounding_box) {};

    LevelObject(const rect &r, const collisionType eC, const bool walkable) : collision(r), eCollision(eC), walkable(walkable) {

    }

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

    [[nodiscard]] rect getCollision() const {
        return collision;
    }

    virtual void debugDrawCollision(const dvec2 offset) {
        DrawRectangleLinesEx(collision-offset, 2, debug_colors[cast(eCollision, usize)]);
        DrawCircle(
            EXPAND_V((collision.pos()-offset).convert_data<i32>()), 2,
            debug_colors[cast(eCollision, usize)]);
    }

    virtual double depth() {
        return collision.y + collision.h;
    }

    //run when the sprite spawns
    virtual void OnSpawn() {

    }

    virtual void OnDeath() {

    }

    bool isDynamic() const {
        return dynamic;
    }
};


class LevelObjectRegistry {
public:
    unordered_map<str, function<shared_ptr<LevelObject>(json& dat)>> factories;

    static LevelObjectRegistry& instance() {
        static LevelObjectRegistry inst;
        return inst;
    }

    bool addEntry(const str& type_id, const function<shared_ptr<LevelObject>(json& dat)>& factory) {
        //returns true if the registry was succesfully added
        if (factories.contains(type_id)) {
            return false;
        }
        factories[type_id] = factory;
        return true;
    }


    shared_ptr<LevelObject> create(const str& type_id, json& data) {
        if (!factories.contains(type_id)) throw Exception("Cannot create type ", type_id);
        return factories[type_id](data);
    }

};



struct LevelObjectFactory {
    virtual ~LevelObjectFactory() = default;


    using factory_type = LevelObject;

    NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {
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

        return make_shared<factory_type>(c, eC, w);
    }
};

#define REGISTER(TYPE, ID) inline bool TYPE::registered = LevelObjectRegistry::instance().addEntry(ID, TYPE##Factory::createFromJson);

REGISTER(LevelObject, "level_object")

struct LevelFloor : public LevelObject {
    GENERATE_LEVEL_OBJECT()
protected:
    Texture2D floor_texture;
public:

    LevelFloor(const rect &area, Texture2D floor_texture) : LevelObject(area, collisionType::NO_COLLISION, true),
    floor_texture(floor_texture) {}

    void draw(const dvec2 offset) override {
        DrawTexturePro(floor_texture, collision.pure(), collision-offset, {0, 0}, 0, GRAY);
    }

    double depth() override {
        return -(1.0/0.0);
    }
};

struct LevelFloorFactory {

    using factory_type = LevelFloor;

    NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {
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
            t = Allocator::allocateTexture(("resources/"_str + data["texture"].get<string>()).data());
        }

        return make_shared<factory_type>(c, t);
    }
};

REGISTER(LevelFloor, "level_floor")

struct LevelProp : public LevelObject {
    GENERATE_LEVEL_OBJECT()
protected:
    animation texture;

public:
    explicit LevelProp(const animation& anim, rect r = {0, 0, 0, 0}, collisionType eCollision = collisionType::BLOCK_ALL, bool w = false) :
    LevelObject(r, eCollision, w)
    {
        texture = anim;
    }

    void update(seconds_t delta) override {
        texture.update(delta);
    }

    void draw(dvec2 offset) override {
        DrawAnimation(texture, collision-offset);
    }

};

struct LevelPropFactory {
    using factory_type = LevelProp;

    NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {
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
            anim = AnimationRegistry::Instance().get(data["texture"].get<string>());
        } else {
            throw Exception("Cannot create level prop from json data");
        }

        return make_shared<factory_type>(*anim, c, eC, w);
    }
};

REGISTER(LevelProp, "level_prop")

/*
 *a level object that can move, must be spawned by a static level object though or manually through the level, CANNOT be created through a factory
 *or through the level json
*/
struct DynamicLevelObject : public LevelObject {
private:

    GENERATE_LEVEL_OBJECT()
protected:

    bool on_ground = true;
    bool moving = false;
    dvec2 last_movement{};
    direction move_dir = NONE;

public:


    DynamicLevelObject(): LevelObject() {
        dynamic = true;
    }

    explicit DynamicLevelObject(const rect &bounding_box) : LevelObject(bounding_box) {
        dynamic = true;
    }

    DynamicLevelObject(const rect &r, const collisionType eC, const bool walkable) : LevelObject(r, eC, walkable) {
        dynamic = true;
    }


    virtual bool move(dvec2 amt, bool should_scroll);

    [[nodiscard]] bool isOnGround() const {
        return on_ground;
    }
    [[nodiscard]] bool isMoving() const {
        return moving;
    }

    [[nodiscard]] dvec2 getLastMovement() const {
        return last_movement;
    }
};


class Game;
class level : public Object {
private:
    vector<shared_ptr<LevelObject>> objects;
    vector<collision> level_collision;
    str name;
    dvec2 scroll = {0, 0};
    usize next_id = 0;

    logger LLevel;

public:

    friend Game;

    explicit level(const char* level_json) {
        ifstream file(level_json);
        if (!file.is_open()) {
            throw Exception("Could not create level from file ", level_json);
        }
        json data;
        file >> data;

        assertJsonData(data, "name", json::value_t::string);
        name = data["name"].get<string>();
        LLevel = logger(name.stdStr());

        LLevel.info("Successfully created level [", name, "] from json file");

        assertJsonData(data, "objects", json::value_t::array);
        vector<json> objs = data["objects"].get<vector<json>>();
        for (auto& obj: objs) {
            assertJsonData(obj, "type", json::value_t::string);
            objects.push_back(LevelObjectRegistry::instance().create(obj["type"].get<string>(), obj));
            objects.back()->Level = this;
            objects.back()->ID = next_id;
            next_id++;
            level_collision.emplace_back(
            &objects.back()->collision, objects.back()->eCollision, objects.back());

            objects.back()->OnSpawn();

            LLevel.info("Created object of type [registry name]: ", obj["type"].get<string>());
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

    [[nodiscard]] dvec2 Scroll() const {
        return scroll;
    }

    //THIS SETS THE SCROLL NOT ADDS!!!
    dvec2 Scroll(const dvec2 _new) {
        scroll = _new;
        return scroll;
    }

    //THIS SETS THE SCROLL NOT ADDS!!!
    dvec2 Scroll(double x, double y) {
        scroll = {x, y};
        return scroll;
    }



    template<class T, typename... Args, typename = enable_if_t<is_base_of_v<LevelObject, T>>>
    shared_ptr<T> spawnObject(Args... constructor) {
        objects.push_back(make_shared<T>(constructor...));
        objects.back()->ID = next_id;
        next_id++;
        objects.back()->Level = this;
        objects.back()->OnSpawn();

        level_collision.emplace_back(
            &objects.back()->collision, objects.back()->eCollision, objects.back());


        auto ret = dynamic_pointer_cast<T>(objects.back());
        if (!ret) throw Exception("Error creating object of type: ", getTypename<T>());
        LLevel.info("Created object of type: ", getTypename<T>());
        return ret;
    }

    collision_hit colliding(const rect &r, const rect* to_ignore) {
        collision_hit hit = {};
        shared_ptr<LevelObject> walk_owner = nullptr;
        for (auto& coll: level_collision) {
            if (coll.description != to_ignore && (r && *coll.description)) {
                hit.objects.push_back(coll.owner);
                hit.hit = true;
                hit.max_collider_status = max(hit.max_collider_status, coll.typ);
                if (!walk_owner || walk_owner->depth() < coll.owner->depth()) {
                    walk_owner = coll.owner;
                    hit.walkable = coll.owner->walkable;
                }
            }
        }

        return hit;
    }

    template<class T, typename... Args, typename = enable_if_t<is_base_of_v<LevelObject, T>>>
    void destroyObject(shared_ptr<T> obj_ptr) {
        obj_ptr->OnDeath();
        const auto it2 = ranges::find_if(level_collision, [&obj_ptr](collision& c) {
            return c.description == &obj_ptr->collision;
        });
        if (it2 != level_collision.end()) {
            level_collision.erase(it2);
        }
        const auto it = ranges::find_if(objects, obj_ptr);
        if (it != objects.end()) {
            objects.erase(it);
        }

    }

    template<class T, typename... Args, typename = enable_if_t<is_base_of_v<LevelObject, T>>>
    void destroyObject(T* obj_ptr) {
        obj_ptr->OnDeath();
        const auto it2 = ranges::find_if(level_collision, [&obj_ptr](collision& c) {
            return c.description == &obj_ptr->collision;
        });
        if (it2 != level_collision.end()) {
            level_collision.erase(it2);
        }

        const auto it = std::find_if(objects.begin(), objects.end(), [&obj_ptr](const shared_ptr<LevelObject>& p) {
            return p.get() == obj_ptr;
        });

        if (it != objects.end()) {
            //(*it)->OnDeath();
            (*it).reset();
            objects.erase(it);
        }

    }

    void update(seconds_t delta) override {

        //we gon sort the objects by their y position so objects with a higher y value get drawn after those with a lower value, thereby
        //adding a '3D' look
        ranges::sort(objects, [](const shared_ptr<LevelObject>& o1, const shared_ptr<LevelObject> &o2) {
            return o1->depth() < o2->depth();
        });
        for (const auto& obj: objects) {
            if (obj) obj->update(delta);
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

    str getName() {
        return name;
    }

    friend DynamicLevelObject;
};




inline bool DynamicLevelObject::move(const dvec2 amt, bool should_scroll = false) {
    //returns true if we moved at all
    rect future = collision;
    future.x += amt.x;
    collision_hit hit;
    moving = false;
    if (amt.x != 0 && (hit = Level->colliding(future, &collision)).max_collider_status != collisionType::BLOCK_ALL) {
        moving = true;
        on_ground = hit.walkable;
        collision.x += amt.x;
        Level->addScroll({amt.x, 0});
        last_movement.x = amt.x;
    } else {
        moving = false;
        future.x -= amt.x;
        last_movement.x = 0;
    }
    future.y += amt.y;
    if ((amt.y != 0) && (hit = Level->colliding(future, &collision)).max_collider_status != collisionType::BLOCK_ALL) {
        moving = true;
        on_ground = hit.walkable;
        collision.y += amt.y;
        Level->addScroll({0, amt.y});
        last_movement.y = amt.y;
    } else {
        //weird hack
        last_movement.y = 0;
    }
    move_dir = vtod(last_movement);

    return moving;
}


#endif