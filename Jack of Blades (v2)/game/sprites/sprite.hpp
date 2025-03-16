#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "../lib/globals.hpp"
#include "../lib/JOB.hpp"

struct sprite;

//a damage type struct, allows for complex damage like poison for example
struct damageType : public Object {
private:

public:
    ~damageType() override = default;
    str name;
    sprite* s = nullptr;

    damageType() {
        name = "damage";
    }

    void draw(dvec2 offset) final {
        //you cant draw damage soory mate
    }

    //returns true if the damage is no long effective
    virtual bool isFinished() {
        return true;
    }
};

struct spriteSpawnPoint;

//a sprite is a living moving object
//a sprite should be inherited from, however it provides a default collision rect of size 32*32
struct sprite : public DynamicLevelObject {
    protected:
    float health;
    float max_health;
    unique_ptr<damageType> damage_source;//only one kind of damage at a time, for now
    spriteSpawnPoint* spawn = nullptr;

public:

    struct Settings {
        float max_health = 1;
        float starting_health = 1;
        rect collision = rect();
        collisionType cType = collisionType::NO_COLLISION;

        Settings() {
            max_health = 1;
            starting_health = 1;
            collision = rect();
            cType = collisionType::NO_COLLISION;
        }

        Settings& maxHealth(const float x) {
            max_health = x;
            return *this;
        }

        Settings& startingHealth(const float x) {
            starting_health = x;
            return *this;
        }

        Settings& Collision(const rect& x) {
            collision = x;
            return *this;
        }

        Settings& collisionType(const collisionType x) {
            cType = x;
            return *this;
        }
    };

    explicit sprite() : DynamicLevelObject(rect(0, 0, 32, 32), collisionType::BLOCK_ALL, true),
    health(1), max_health(1) {}

    explicit sprite(dvec2 pos) : DynamicLevelObject({pos, 32, 32}, collisionType::BLOCK_ALL, true),
    health(1), max_health(1) {}

    explicit sprite(const rect &r) : DynamicLevelObject(r, collisionType::BLOCK_ALL, true),
    health(1), max_health(1) {

    }

    explicit sprite(spriteSpawnPoint* r);

    explicit sprite(const Settings sprSettings = Settings()) : DynamicLevelObject(sprSettings.collision, sprSettings.cType, true),
    health(sprSettings.starting_health), max_health(abs(sprSettings.max_health)) {}

    void addHealth(const float amt) {
        health += amt;
        health = clamp(health, 0, max_health);
    }

    void damage(unique_ptr<damageType> damage) {
        this->damage_source = std::move(damage);
    }

    void setMaxHealth(const float _new) {
        max_health = abs(_new);
    }

    [[nodiscard]] bool isDead() const {
        return health <= 0;
    }


    //should call this in derived types
    void update(seconds_t delta) override {
        if (damage_source) {
            damage_source->update(delta);
            if (damage_source->isFinished()) damage_source = nullptr;
        }

        if (isDead()) {
            Level->destroyObject(this);
        }
    }
};


struct spriteSpawnPoint : LevelObject {
    GENERATE_LEVEL_OBJECT();


    spriteSpawnPoint() : LevelObject(rect(), collisionType::NO_COLLISION, true) {

    }

    explicit spriteSpawnPoint(dvec2 pos) : LevelObject(rect(pos, 0, 0), collisionType::NO_COLLISION, true) {}

    virtual shared_ptr<sprite> spawn() {
        shared_ptr<sprite> ret = Level->spawnObject<sprite>(this);
        return ret;
    }

    void OnSpawn() override {
        spawn();
    }
};

inline sprite::sprite(spriteSpawnPoint* r) : DynamicLevelObject(r->getCollision(), collisionType::BLOCK_ALL, true),
    health(1), max_health(1) {
    this->spawn = r;
}


struct spriteSpawnPointFactory {
    using factory_type = spriteSpawnPoint;

    NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {
        if (validateJsonData(data, "position", json::value_t::array)) {
            auto pos = data["position"].get<vector<double>>();
            if (pos.size() != 2) {
                throw Exception("Cannot create type from json data!");
            }
            return make_shared<factory_type>(dvec2{pos[0], pos[1]});
        }
        return make_shared<factory_type>();
    }
};

REGISTER(spriteSpawnPoint, "sprite_spawn_point")



//makes a spawnpoint named <SPRITE_TYPE>SpawnPoint (where <SPRITE_TYPE> is the type of sprite)
//its data id is the specified ID
#define GENERATE_SPAWN_POINT(SPRITE_TYPE, ID) struct SPRITE_TYPE##SpawnPoint : spriteSpawnPoint {\
                                                GENERATE_LEVEL_OBJECT()\
                                                SPRITE_TYPE##SpawnPoint() = default;\
                                                explicit SPRITE_TYPE##SpawnPoint(const dvec2 pos) : spriteSpawnPoint(pos) {}\
                                                shared_ptr<sprite> spawn() override {\
                                                    shared_ptr<SPRITE_TYPE> ret = Level->spawnObject<SPRITE_TYPE>(this);\
                                                    return ret;\
                                                }\
                                          };\
                                          struct SPRITE_TYPE##SpawnPointFactory {\
                                              using factory_type = SPRITE_TYPE##SpawnPoint;\
                                              NODISCARD static shared_ptr<factory_type> createFromJson(json& data) {\
                                                  if (validateJsonData(data, "position", json::value_t::array)) {\
                                                      auto pos = data["position"].get<vector<double>>();\
                                                      if (pos.size() != 2) {\
                                                          throw Exception("Cannot create type from json data!");\
                                                      }\
                                                      return make_shared<factory_type>(dvec2{pos[0], pos[1]});\
                                                  }\
                                                  return make_shared<factory_type>();\
                                              }\
                                          };\
                                          REGISTER(SPRITE_TYPE##SpawnPoint, ID)

//the most basic damage type, deals damage to the sprites health one time
struct physicalDamage : public damageType {
    private:
    float damage_amount;
    public:
    explicit physicalDamage(float amt) : damageType(), damage_amount(abs(amt)) {
        name = "physical_damage";
    }

    void update(seconds_t delta) override {
        if (s) {
            s->addHealth(-damage_amount);
        }
        //ensures we only deal damage once
        s = nullptr;
    }
};

#endif
