#ifndef GAMELIB_HPP
#define GAMELIB_HPP

#include <misc.hpp>
#include <utility>

#include "globals.hpp"
#include "utils.hpp"
#include "enums.hpp"

struct LevelObject;
using namespace AustinUtils;
using namespace std;






//a structure for passing around collision
struct collision {
    //most of the time, description will be valid since we manage the collision explicitly and removal of collision
    //will mean removal of these objects, also the pointer represents a pointer to some stack memory which means we dont have to worry about sudden
    //deallocation since most objects in the game will last the whole runtime, those that dont are explicitly removed, meaning we can explicitly remove these
    //pointer references
    rect* description = nullptr;
    collisionType* typ = nullptr;
    shared_ptr<LevelObject> owner = nullptr;

    friend ostream& operator <<(ostream& os, collision& c) {
        os << "[" << c.description << ", " << cast(*c.typ, u32) << ", " << c.owner << "]";
        return os;
    }
};

struct collision_hit {
    bool hit{};//if we hit anything at all
    bool walkable{};//if we hit anything we can walk on
    vector<shared_ptr<LevelObject>> objects = {};//all the things we hit
    collisionType max_collider_status = collisionType::NO_COLLISION;//the maximum colliding status of an object we hit
};

struct simple_hit {
    bool hit{};
    bool walkable{};
    shared_ptr<LevelObject> obj = nullptr;
    collisionType type = collisionType::NO_COLLISION;
};


class level;


#define GENERATE_LEVEL_OBJECT(name) private:\
                                            static str registered;\
                                            public:\
                                            friend level;\
                                            friend LevelEditor;\
                                            friend struct name##Factory;\
                                            str getRegistryID() override {\
                                                return registered;\
                                            }\

#define GENERATE_DYNAMIC_LEVEL_OBJECT(name) public:\
                                            friend level;\
                                            friend LevelEditor;\



template<typename T>
concept ObjectFactory = requires(json& data)
{
    typename T::factory_type;
    { T::createFromJson(data) } -> same_as<shared_ptr<typename T::factory_type>>;
    { T::createDefault() } -> same_as<shared_ptr<typename T::factory_type>>;

    requires requires(LevelObject& obj)
    {
        { T::objectToJson(obj) } -> same_as<json>;
    };
};



struct DynamicLevelObject;

//static level object, does not move

enum class OPType : u32{
    FLOAT32,//float
    FLOAT64,//double
    INTEGER32,//i32
    INTEGER64,//i64
    UNSIGNED_INTEGER8,
    UNSIGNED_INTEGER32,//u32
    UNSIGNED_INTEGER64,//u64
    BOOLEAN,//bool
    RECT,//rect
    DVEC,//dvec2
    TEXTURE,
    ANIMATION,
    COLOR,
};

struct ObjectParameter {
    str name;
    OPType typ;
    void* data;
    //only applicable if the parameter is a number
    double min = 0;
    double max = 0;
    bool is_enum;
    const char* (*enumToString)(void*) = nullptr;
};

//TODO: separate bounding box from collision box
struct LevelObject : public Object {
private:
    bool dynamic = false;
private:
    static str registered;
public:
    friend level;
    friend LevelEditor;
    friend struct LevelObjectFactory;
    friend struct selection;
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

    //returns a list of parameters that can be edited by the level editor
    virtual pair<str, vector<ObjectParameter>> getParameters() {
        return {
            "Level Object",
            {
                {//collision
                    "Collision",
                    OPType::RECT,
                    &collision,
                    false
                },
                {//collision type
                    "Collision Type",
                    OPType::UNSIGNED_INTEGER32,
                    &eCollision,
                    0,
                    3,
                    true,
                    &collisionTypeToString
                },
                {//walkable
                    "Walkable",
                    OPType::BOOLEAN,
                    &walkable,
                    false
                }
            }
        };
    }

    ~LevelObject() override {
        //just to make sure it's not in the level anymore
        Level = nullptr;
        ID = -1;
    }

    LevelObject(): collision({0, 0, 0, 0}) {
        eCollision = collisionType::NO_COLLISION;
        walkable = true;
    }

    explicit LevelObject(rect  bounding_box) : collision(std::move(bounding_box)) {};

    LevelObject(const rect& r, const collisionType eC, const bool walkable) : collision(r), eCollision(eC), walkable(walkable) {

    }

    virtual shared_ptr<LevelObject> copy(dvec2 pos) {

        return make_shared<LevelObject>(collision.pure().pos(pos), eCollision, walkable);
    }

    LevelObject& CollisionType(const collisionType eCollision) {
        this->eCollision = eCollision;
        return *this;
    }

    NODISCARD collisionType CollisionType() const {
        return eCollision;
    }

    virtual str getRegistryID() {
        return registered;
    }

    [[nodiscard]] level* GetLevel() const {
        return Level;
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

    //returns true if the object should be destroyed
    virtual bool OnDeath() {
        return true;
    }

    [[nodiscard]] bool isDynamic() const {
        return dynamic;
    }
};





class LevelObjectRegistry {
public:
    unordered_set<str> IDS;
    unordered_map<str, function<shared_ptr<LevelObject>(json& dat)>> factories;
    unordered_map<str, function<shared_ptr<LevelObject>()>> defaultFactories;
    unordered_map<str, function<json(LevelObject& l)>> toJsonFactories;

    static LevelObjectRegistry& instance() {
        static LevelObjectRegistry inst;
        return inst;
    }

    template<ObjectFactory T>
    str addEntry(const str& type_id) {
        if (IDS.contains(type_id)) {
            throw Exception("Cannot register id: ", type_id);
        }
        IDS.insert(type_id);
        factories[type_id] = T::createFromJson;
        defaultFactories[type_id] = T::createDefault;
        toJsonFactories[type_id] = T::objectToJson;
        return type_id;
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

    NODISCARD static shared_ptr<factory_type> createDefault() {
        return make_shared<factory_type>();
    }


    NODISCARD static json objectToJson(const factory_type& obj) {
        json ret;
        ret["collision"] = obj.getCollision().components();
        ret["collision_type"] = cast(obj.CollisionType(), u32);
        ret["walkable"] = obj.Walkable();
        return ret;
    }


};

#define REGISTER(TYPE, ID) inline str TYPE::registered = LevelObjectRegistry::instance().addEntry<TYPE##Factory>(ID);

REGISTER(LevelObject, "level_object")


struct LevelLightSource : public LevelObject {
    GENERATE_LEVEL_OBJECT(LevelLightSource)
private:
    float radius;
    Color c;
    u8 light_level;

public:
    LevelLightSource() : LevelObject({0, 0, 32, 32}, collisionType::NO_COLLISION, true), radius(0), c(), light_level() {}

    LevelLightSource(dvec2 center, const float radius, const Color lc, u8 level) :
    LevelObject({center, 32 ,32}, collisionType::NO_COLLISION, true), radius(radius), c(lc),
    light_level(level) {}

    void drawLighting(dvec2 offset) override {
        DrawCircleGradient(cast(collision.x-offset.x, int), cast(collision.y-offset.y, int), radius,
            Fade(c, cast(light_level, float)/255.0f),
            Fade(c, 0.003));
        //DrawCircle(cast(collision.x-offset.x, int), cast(collision.y-offset.y, int), radius, RED);
    }



    shared_ptr<LevelObject> copy(dvec2 pos) override {
        return make_shared<LevelLightSource>(collision.pos(), radius, c, light_level);
    }

    pair<str, vector<ObjectParameter>> getParameters() override {
        return {
            "Level Light Source",
        {
                ObjectParameter{"Radius", OPType::FLOAT32, &radius, -1.0/0.0, 1.0/0.0},
                ObjectParameter{"Color", OPType::COLOR, &c},
                ObjectParameter{"Light Level", OPType::UNSIGNED_INTEGER8, &light_level, 0, 255}
            }
        };
    }
};

struct LevelLightSourceFactory {
    using factory_type = LevelLightSource;

    NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {
        dvec2 pos;
        Color c;
        float radius;
        u8 light_level;

        if (validateJsonData(data, "pos", json::value_t::array)) {
            auto v = data["pos"].get<vector<double>>();
            if (v.size() != 2) throw Exception("Cannot make LevelLightSource with json data");

            pos.x = v[0];
            pos.y = v[1];
        }
        if (validateJsonData(data, "color", json::value_t::array)) {
            auto v = data["color"].get<vector<u8>>();
            if (v.size() != 3) throw Exception("Cannot make LevelLightSource with json data");

            c.r = v[0];
            c.g = v[1];
            c.b = v[2];
        }
        if (validateJsonData(data, "light_level", json::value_t::number_unsigned)) {
            light_level = data["light_level"].get<u8>();
        }
        if (validateJsonData(data, "radius", JSON_NUMBERS)) {
            radius = data["radius"].get<float>();
        }

        return make_shared<factory_type>(pos, radius, c, light_level);
    }

    NODISCARD static shared_ptr<factory_type> createDefault() {
        return make_shared<factory_type>();
    }

    NODISCARD static json objectToJson(LevelObject& x) {
        LevelLightSource& obj = *dynamic_cast<LevelLightSource *>(&x);
        json res;

        res["pos"] = {obj.collision.x, obj.collision.y};
        res["color"] = {obj.c.r, obj.c.g, obj.c.b};
        res["radius"] = obj.radius;
        res["light_level"] = obj.light_level;

        return res;
    }

};

REGISTER(LevelLightSource, "level_light_source")

struct LevelFloor : public LevelObject {
GENERATE_LEVEL_OBJECT(LevelFloor)
protected:
    shared_ptr<animation> floor_texture{};
    u8 light;
public:

    pair<str, vector<ObjectParameter>> getParameters() override {
        return {
                "Level Floor",
                {
                    ObjectParameter{"Area", OPType::RECT, &collision},
                    ObjectParameter{"Texture", OPType::ANIMATION, &floor_texture},
                    ObjectParameter{"Light Level", OPType::UNSIGNED_INTEGER8, &light, 0, 255},
                }
        };
    }

    LevelFloor() : LevelObject({0, 0, 32, 32}, collisionType::NO_COLLISION, true), light(255) {
        floor_texture = AnimationRegistry::Instance().get("default");
    }

    LevelFloor(const rect &area, const shared_ptr<animation>& anim, const u8 light_level) :
    LevelObject(area, collisionType::NO_COLLISION, true),
    floor_texture(anim), light(light_level)
    {}


    void draw(const dvec2 offset) override {
        DrawAnimation(*floor_texture, collision.pure(), collision - offset);
    }

    void drawLighting(const dvec2 offset) override {
        DrawRectangle(EXPAND_R(collision-offset), Fade(BLACK, cast(255-light, float)/255));
    }

    shared_ptr<LevelObject> copy(const dvec2 pos) override {
        return make_shared<LevelFloor>(collision.pure().pos(pos), floor_texture, light);
    }

    double depth() override {
        return -(1.0/0.0);
    }
};



struct LevelFloorFactory {

    using factory_type = LevelFloor;

    NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {
        rect c = rect();
        str t = "default";
        u8 light_level = 255;
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
            t = data["texture"].get<string>();
        }
        if (validateJsonData(data, "light_level", json::value_t::number_unsigned)) {
            light_level = data["light_level"].get<u8>();
        }

        return make_shared<factory_type>(c, AnimationRegistry::Instance().get(t), light_level);
    }

    NODISCARD static shared_ptr<factory_type> createDefault() {
        return make_shared<factory_type>();
    }

    NODISCARD static json objectToJson(LevelObject& x) {
        const auto* obj = dynamic_cast<LevelFloor*>(&x);
        json ret;
        ret["area"] = obj->getCollision().components();
        ret["texture"] = obj->floor_texture->getId();
        ret["light_level"] = obj->light;
        return ret;
    }

};

REGISTER(LevelFloor, "level_floor")



struct LevelProp : public LevelObject {
GENERATE_LEVEL_OBJECT(LevelProp)
protected:
    shared_ptr<animation> texture;
    Color tint;

public:
    explicit LevelProp(shared_ptr<animation> anim,
        const rect& r = {0, 0, 0, 0},
        collisionType eCollision = collisionType::BLOCK_ALL,
        Color tint = WHITE, bool w = false) :
    LevelObject(r, eCollision, w)
    {
        texture = anim;
        this->tint = tint;
    }

    LevelProp() : LevelObject({0, 0, 32, 32}) {
        texture = AnimationRegistry::Instance().get("default");
        tint = WHITE;
    }

    void update(seconds_t delta) override {
        texture->update(delta);
    }

    shared_ptr<animation> getAnimation() const {
        return texture;
    }

    shared_ptr<LevelObject> copy(dvec2 pos) override {
        return make_shared<LevelProp>(texture, collision.pure().pos(pos), eCollision, tint, walkable);
    }

    void draw(dvec2 offset) override {
        DrawAnimation(*texture, collision.pure(), collision-offset, tint);
    }

    pair<str, vector<ObjectParameter>> getParameters() override {
        auto params = LevelObject::getParameters();
        params.second.push_back(ObjectParameter{"Animation", OPType::ANIMATION, &texture});
        params.second.emplace_back("Tint", OPType::COLOR, &tint);
        params.first = "Level Prop";
        return params;
    }

};

struct LevelPropFactory {
    using factory_type = LevelProp;

    NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {
        rect c = rect();
        auto eC = collisionType::NO_COLLISION;
        bool w = true;
        Color tint = WHITE;
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
        if (validateJsonData(data, "tint", json::value_t::array)) {

            try {
                auto vals = data["tint"].get<vector<u8>>();
                if (vals.size() != 3) throw Exception("Cannot create level prop from json data");
                tint.r = vals[0];
                tint.g = vals[1];
                tint.b = vals[2];
            } catch (exception& e) {
                try {
                    auto vals = data["tint"].get<vector<float>>();
                    tint.r = cast(clamp(vals[0], 0, 1)*255, u8);
                    tint.g = cast(clamp(vals[1], 0, 1)*255, u8);
                    tint.b = cast(clamp(vals[2], 0, 1)*255, u8);
                } catch (exception& e2) {
                    throw Exception("Cannot create level prop from json data");
                }
            }
        }
        if (validateJsonData(data, "texture", json::value_t::string)) {
            anim = AnimationRegistry::Instance().get(data["texture"].get<string>());
        } else {
            throw Exception("Cannot create level prop from json data");
        }

        return make_shared<factory_type>(anim, c, eC, tint, w);
    }

    NODISCARD static shared_ptr<factory_type> createDefault() {
        return make_shared<factory_type>();
    }

    NODISCARD static json objectToJson(LevelObject& x) {
        auto* obj = dynamic_cast<LevelProp*>(&x);
        json ret;
        ret["collision"] = obj->getCollision().components();
        ret["collision_type"] = static_cast<u32>(obj->CollisionType());
        ret["walkable"] = obj->Walkable();
        ret["texture"] = obj->getAnimation()->getId();
        ret["tint"] = vector{obj->tint.r, obj->tint.g, obj->tint.b};

        return ret;
    }

};


REGISTER(LevelProp, "level_prop")


/*
 *a level object that can move, must be spawned by a static level object though or manually through the level, CANNOT be created through a factory
 *or through the level json
*/
struct DynamicLevelObject : public LevelObject {
GENERATE_DYNAMIC_LEVEL_OBJECT(DynamicLevelObject)
protected:

    bool on_ground = true;
    bool moving = false;
    dvec2 last_movement{};
    dvec2 last_valid_pos;
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


    virtual bool move(dvec2 amt, bool should_scroll = false);

    virtual bool setPosition(dvec2 pos, bool check_collision, bool should_scroll = false);

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
    bool started = false;

public:

    friend Game;
    friend LevelEditor;

    level() = default;

    ~level() override = default;

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
            &objects.back()->collision, &objects.back()->eCollision, objects.back());


            LLevel.info("Created object of type [registry name]: ", obj["type"].get<string>());
        }
    }

    void start() {
        started = true;
        for (usize i = 0; i < objects.size(); i++) {
            objects[i]->OnSpawn();
        }
    }

    void addScroll(dvec2 amt) {
        scroll += amt;
    }

    void addScroll(double x, double y) {
        scroll.x += x;
        scroll.y += y;
    }

    void focusScroll(dvec2 pos) {
        pos.x -= base_resolution.x/2.0;
        pos.y -= base_resolution.y/2.0;

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

        level_collision.emplace_back(
            &objects.back()->collision, &objects.back()->eCollision, objects.back());

        if (started) objects.back()->OnSpawn();



        auto ret = dynamic_pointer_cast<T>(objects.back());
        if (!ret) throw Exception("Error creating object of type: ", getTypename<T>());
        LLevel.info("Created object of type: ", getTypename<T>(), ", Level: ", this);
        return ret;
    }


    shared_ptr<LevelObject> createObject(const str &registryID) {
        objects.push_back(LevelObjectRegistry::instance().defaultFactories[registryID]());
        level_collision.push_back({&objects.back()->collision, &objects.back()->eCollision, objects.back()});
        return objects.back();
    }


    shared_ptr<LevelObject> addObject(const shared_ptr<LevelObject> &obj) {
        objects.push_back(obj);
        level_collision.push_back({&objects.back()->collision, &objects.back()->eCollision, objects.back()});
        return objects.back();
    }

    collision_hit getAllObjects(const rect &area) {
        collision_hit ret;
        shared_ptr<LevelObject> walk_owner = nullptr;
        for (auto& obj: objects) {
            if (obj->getCollision() && area) {
                ret.hit = true;
                ret.max_collider_status = max(ret.max_collider_status, obj->eCollision);
                ret.objects.push_back(obj);
                if (!walk_owner || walk_owner->depth() < obj->depth()) {
                    walk_owner = obj;
                    ret.walkable = obj->walkable;
                }
            }
        }

        return ret;
    }

    simple_hit getTopObject(const dvec2 pos) {
        simple_hit ret;

        for (auto& obj: objects) {
            if (obj->collision && pos) {
                ret.hit = true;
                if (ret.obj && obj->depth() > ret.obj->depth()) {
                    ret.obj = obj;
                } else if (!ret.obj) {
                    ret.obj = obj;
                }
            }
        }
        if (ret.hit) {
            ret.type = ret.obj->eCollision;
            ret.walkable = ret.obj->walkable;
        }
        return ret;
    }

    collision_hit colliding(const rect &r, const rect* to_ignore) {
        collision_hit hit = {};
        shared_ptr<LevelObject> walk_owner = nullptr;
        for (auto& coll: level_collision) {
            if (coll.description != to_ignore && (r && *coll.description)) {
                hit.objects.push_back(coll.owner);
                hit.hit = true;
                hit.max_collider_status = max(hit.max_collider_status, *coll.typ);
                if (!walk_owner || walk_owner->depth() < coll.owner->depth()) {
                    walk_owner = coll.owner;
                    hit.walkable = coll.owner->walkable;
                }
            }
        }

        return hit;
    }

    /*
     * casts a ray in the level from p1 to p2 and returns a hit result depending on the parameters
     * return_closest_only: if true, only returns the closest object hit by the ray to p1, otherwise returns objects sorted by distance to p1
     * filter: only returns objects that have a collisionType greater than or equal to the filter
     */
    collision_hit RayCast(const dvec2 p1, const dvec2 p2, const rect* ignore,
        bool return_closest_only = false, collisionType filter = collisionType::NO_COLLISION) {

        collision_hit hit{};
        shared_ptr<LevelObject> walk_owner = nullptr;

        if (return_closest_only) {
            //find the closest hit
            for (auto& coll: level_collision) {
                if (coll.description == ignore) continue;
                if (!coll.description->intersects(p1, p2)) continue;
                if (!hit.hit && coll.owner->eCollision >= filter) {
                    hit.hit = true;
                    hit.objects.push_back(coll.owner);
                    hit.walkable = coll.owner->walkable;
                    hit.max_collider_status = *coll.typ;
                }
                if ((p1 - coll.description->center()).length2() < (p1 - hit.objects[0]->collision.center()).length2()) {
                    hit.objects[0] = coll.owner;
                    hit.walkable = coll.owner->walkable;
                    hit.max_collider_status = *coll.typ;
                }
            }
        } else {

            for (auto& coll: level_collision) {
                if (coll.description == ignore) continue;
                if (!coll.description->intersects(p1, p2)) continue;
                hit.hit = true;
                hit.objects.push_back(coll.owner);
                hit.max_collider_status = max(hit.max_collider_status, *coll.typ);
                hit.walkable = hit.walkable || coll.owner->walkable;
            }
            sort(hit.objects.begin(), hit.objects.end(), [&p1](const std::shared_ptr<LevelObject>& o1, const std::shared_ptr<LevelObject>& o2) {
                return (p1-o1->collision.center()).length2() < (p1-o2->collision.center()).length2();
            });
            return hit;
        }
    }

    template<class T, typename = enable_if_t<is_base_of_v<LevelObject, T>>>
    void destroyObject(shared_ptr<T> obj_ptr) {
        if (!obj_ptr->OnDeath()) {
            return;
        }
        const auto it2 = ranges::find_if(level_collision, [&obj_ptr](collision& c) {
            return c.description == &obj_ptr->collision;
        });
        if (it2 != level_collision.end()) {
            level_collision.erase(it2);
        }
        const auto it = ranges::find(objects, obj_ptr);
        if (it != objects.end()) {
            objects.erase(it);
        }
        LLevel.info("Destroyed object of type ", getTypename<T>());
    }

    template<class T, typename = enable_if_t<is_base_of_v<LevelObject, T>>>
    void destroyObject(T* obj_ptr) {
        if (!obj_ptr->OnDeath()) {
            return;
        }
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

    template<class T, typename = enable_if_t<is_base_of_v<LevelObject, T>>>
    void forceDestroyObject(shared_ptr<T> obj_ptr) {
        obj_ptr->OnDeath();

        const auto it2 = ranges::find_if(level_collision, [&obj_ptr](collision& c) {
            return c.description == &obj_ptr->collision;
        });
        if (it2 != level_collision.end()) {
            level_collision.erase(it2);
        }
        const auto it = ranges::find(objects, obj_ptr);
        if (it != objects.end()) {
            objects.erase(it);
        }
        LLevel.info("Destroyed object of type ", getTypename<T>());
    }

    template<class T, typename = enable_if_t<is_base_of_v<LevelObject, T>>>
    void forceDestroyObject(T* obj_ptr) {
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

    void reset() {
        for (const auto& obj: objects) {

            if (obj && obj->isDynamic()) {
                forceDestroyObject<LevelObject>(obj);
            } else if (obj) {
                obj->OnDeath();
            }

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

    void drawLighting(dvec2 offset) override {
        for (const auto& obj: objects) {
            obj->drawLighting(scroll);
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




inline bool DynamicLevelObject::move(const dvec2 amt, bool should_scroll) {
    //returns true if we moved at all
    rect future = collision;
    future.x += amt.x;
    collision_hit hit;
    moving = false;
    if (amt.x != 0 && (hit = Level->colliding(future, &collision)).max_collider_status != collisionType::BLOCK_ALL) {
        moving = true;
        on_ground = hit.walkable;
        collision.x += amt.x;
        if (should_scroll) Level->addScroll({amt.x, 0});
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
        if (should_scroll) Level->addScroll({0, amt.y});
        last_movement.y = amt.y;
    } else {
        last_movement.y = 0;
    }
    move_dir = vtod(last_movement);

    if (on_ground) {
        last_valid_pos = collision.pos();
    }

    return moving;
}

inline bool DynamicLevelObject::setPosition(dvec2 pos, bool check_collision, bool should_scroll) {
    if (check_collision) {
        collision_hit hit;
        rect future = collision;
        future.x = pos.x;
        future.y = pos.y;
        if ((hit = Level->colliding(future, &collision)).max_collider_status != collisionType::BLOCK_ALL) {
            on_ground = hit.walkable;
            moving = true;
            collision.x = pos.x;
            collision.y = pos.y;
            if (should_scroll) {
                Level->focusScroll(collision.center());
            }
            return true;
        }
        return false;
    }
    collision.x = pos.x;
    collision.y = pos.y;
    if (should_scroll) {
        Level->focusScroll(collision.center());
    }
    return true;
}




#endif