#pragma once

#include "util.hpp"
#include <AustinUtils.hpp>
#undef null
#include <raylib.h>
#include "json.hpp"

using namespace std;

/*
this file and anything that uses this are undergoing some refactoring so codes a bit janky
*/


typedef double t_seconds;


using json = nlohmann::json;
using namespace AustinUtils;

namespace JoB {

    inline Vector2 toVector2(fvec2 vec) {
        return {vec.x, vec.y};
    }

    inline fvec2 toFVec2(Vector2 vec) {
        return {vec.x, vec.y};
    }


    inline void drawTextCentered(Font f, const char* text, i32 sx, i32 sy, float f_size, Color c) {
        fvec2 width = toFVec2(MeasureTextEx(f, text, f_size, 2));
        //DrawText(text, sx-width/2, sy-f_size/2, f_size, c);
        DrawTextEx(f, text, {static_cast<float>(sx-(width.x/2)), static_cast<float>(sy-(width.y/2))}, 
        f_size, 2, c);
    }

    


    const fvec2 base_resolution = {640, 360};


    class Allocator {
        private:
        vector<Texture2D> textures;
        vector<void*> allocations;
        vector<void*> dynamic_allocations;
        vector<Font> fonts;
        logger log = logger("Allocator");
        public:

        

        Allocator() {}

        void *allocate_memory(usize amt) {
            void *mem = malloc(amt);
            if (mem == nullptr) {
                log.log(logger::ERROR, "Error allocating {} bytes of memory", amt);
                cout.flush();
                abort();
            }
            allocations.push_back(mem);
            log.log(logger::INFO, "Succesfully allocated {} bytes of memory at {}", amt, mem);
            return mem;
        }

        Texture2D createTexture(const char* image_file) {
            Image im = LoadImage(image_file);

            
            Texture2D texture = LoadTextureFromImage(im);
            UnloadImage(im);
            textures.push_back(texture);
            return texture;
        }

        Texture2D createTexture(Color c, i32 width, i32 height) {
            Image im = GenImageColor(width, height, c);

            Texture2D texture = LoadTextureFromImage(im);
            UnloadImage(im);
            textures.push_back(texture);
            return texture;
        }

        Font createFont(const char* file) {
            Font ret = LoadFontEx(file, 200, NULL, 0);
            fonts.push_back(ret);
            return fonts.back();
        }

        void free() {
            //first free all the textures
            for (auto& texture: textures) {
                UnloadTexture(texture);
            }

            //next the fonts
            for (auto& font: fonts) {
                UnloadFont(font);
            }

            //next free all the allocated memory
            for (auto& memory: allocations) {
                ::free(memory);
                log.log(logger::INFO, "Succesfully freed memory at {}", memory);
            }

            textures.clear();
            allocations.clear();
            fonts.clear();
        }

        void free(void* memory) {
            auto it = std::find(allocations.begin(), allocations.end(), memory);
            
            ::free(*it);
            allocations.erase(it);
            log.log(logger::INFO, "Succesfully freed memory at {}", memory);
        }

        void free(Font font) {
            auto it = std::find_if(fonts.begin(), fonts.end(), [&font](Font& f) {
                return f.texture.id == font.texture.id;
            });
            UnloadFont(*it);
            fonts.erase(it);
        }

        void free(Texture2D texture) {
            auto it = std::find_if(textures.begin(), textures.end(), [&texture](Texture2D& t) {
                return t.id == texture.id;
            });
            UnloadTexture((*it));
            textures.erase(it);
        }
    };


    struct animation {

        enum type {
            LOOP,
            ONCE,
            BOUNCE,
        };

        private:
        
        Texture2D anim_texture;
        usize frames;
        f64 f_ind = 0;
        ivec2 frame_size;
        f64 f_duration = 0.0;
        type anim_type;
        i8 dir = 1;

        public:

        

        animation() {}

        animation(const char* file, Allocator& allocator) {
            anim_texture = allocator.createTexture(file);
            this->f_duration = 0;
            frames = 1;
            this->frame_size = {anim_texture.width, anim_texture.height};
            anim_type = ONCE;
        }

        animation(Allocator& allocator, const char* file, ivec2 frame_size, f64 f_duration, type Typ = LOOP) : anim_texture(allocator.createTexture(file)) {
            
            this->f_duration = f_duration;
            frames = anim_texture.height/frame_size.y;
            this->frame_size = frame_size;
            anim_type = Typ;
        }

        animation(Texture2D text) {
            anim_texture = text;
            this->f_duration = 0;
            frames = 1;
            this->frame_size = {anim_texture.width, anim_texture.height};
            anim_type = ONCE;
        }

        Texture2D getTexture() {
            return anim_texture;
        }

        rect peek() {
            rect ret = {0, 
            static_cast<float>(floor(frame_size.y*floor(f_ind))), 
            static_cast<float>(frame_size.x), static_cast<float>(frame_size.y)};

            

            //returns the current frame rectangle
            return ret;
        }

        rect advance(t_seconds delta) {
            //advances frames and returns the next frame
            f_ind += delta/(f_duration * dir);
            if (f_ind != f_ind) f_ind = 0;
            if (f_ind > frames) {
                if (anim_type == LOOP) {
                    f_ind = 0;
                } else if (anim_type == ONCE) {
                    f_ind = frames+0.1;
                } else if (anim_type == BOUNCE) {
                    dir *= -1;
                }
            }
            return {0, static_cast<float>(floor(frame_size.y*(floor(f_ind)))), static_cast<float>(frame_size.x), static_cast<float>(frame_size.y)};
        }

        bool is_finished() {
            return f_ind >= frames;
        }

        void reset() {
            f_ind = 0.0;
        }

    };




    class component {
        protected:
        void* parent;

        public:
        component(void* parent) {
            this->parent = parent;
        }

        void* getParent() {
            return parent;
        }
    };

    class Level;
    struct levelFloor;

    class levelComponent : public component {
        public:
        u64 levelID;
        Level* level;
        levelFloor* cur_area;

        void (*draw)(levelComponent& self, dvec2 scroll) = nullptr;
        void (*update)(levelComponent& self, double delta_time);

        levelComponent(void* parent) : component(parent) {}

        virtual rect getCollision() {return {NAN, NAN, NAN, NAN};}
        virtual dvec2 getPosition() {return {NAN, NAN};}

        friend Level;

    };

    //a static level object that cannot move
    class levelObject : public levelComponent {
        const dvec2 position;
        rect collision;

        public:

        levelObject(void* parent, dvec2 position, rect collision) : levelComponent(parent), position(position) {
            this->collision = collision;
        }

        rect getCollision() override {
            return collision;
        }

        dvec2 getPosition() override {
            return position;
        }

        virtual ~levelObject() = default;

        friend Level;
    };

    class levelEntity : public levelComponent {
        dvec2 position;
        rect collision;

        public:
        levelEntity(void* parent, dvec2 pos, rect collision) : levelComponent(parent) {
            position = pos;
            this->collision = collision;
        }

        levelEntity() : levelComponent(nullptr) {
            position = {NAN, NAN};
            this->collision = {NAN, NAN, NAN, NAN};
        }

        //move implemented after Level
        //TODO: implement collision checking
        void move(dvec2 delta) {
            position += delta;
        }

        void move(f64 magnitude, radians angle) {
            position += dvec2::of(angle, magnitude);
        }

        void setPosition(dvec2 pos) {
            this->position = pos;
        }

        rect getCollision() override {
            return collision;
        }

        dvec2 getPosition() override {
            return position;
        }

        friend Level;

    };

    
    struct levelFloor {
        private:
        rect area;
        Texture2D texture;
        vector<levelObject> static_objects;
        unordered_map<u64, levelEntity> entities;

        levelFloor(Texture2D texture, rect area) {
            this->texture = texture;
            this->area = area;
        }

        public:
        friend Level;

        levelFloor() = default;

        
    };

    struct entitySpawner {
        private:

        using FactoryFunction = std::function<levelEntity()>;
        using SpawnCheckFunction = std::function<bool(Level& level)>;
        levelObject object;
        FactoryFunction factory;
        SpawnCheckFunction shouldSpawn;
        unordered_map<u64, levelEntity> spawned_entities;

        public:
        entitySpawner(void* parent, dvec2 position, rect collision, FactoryFunction factory, SpawnCheckFunction shouldSpawn) : object(parent, position, collision) {
            this->factory = factory;
            this->shouldSpawn = shouldSpawn;
            object.update = this->update;
        }

        static void update(levelComponent& self, double delta_time);

        //implemented after level is implemented
        //levelEntity& spawnEntity(Level& level);

        //void killSpawnedEntity(Level& level);
    };

    class Level {
        protected:
        std::vector<levelFloor> floors;
        dvec2 vScroll;
        Allocator& allocator;
        u64 next_id = 0;
        public:

        Level(Allocator& allocator) : allocator(allocator) {

        }

        void scroll(dvec2 delta) {
            vScroll += delta;
        }
        void scroll(f64 mag, radians theta) {
            vScroll += dvec2::of(theta, mag);
        }

        void set_scroll(dvec2 scroll) {
            vScroll = scroll;
        }

        void focus_scroll(dvec2 pos) {
            vScroll = {pos.x - base_resolution.x/2, pos.y - base_resolution.y/2};
        }

        template<typename T, typename = std::enable_if_t<std::is_base_of_v<levelObject, T>>>
        T& createStaticComponent(T comp) {
            //first find the floor its on
            T* ret;
            for (auto& floor: floors) {
                if (rect::of(comp.position.x,comp.position.y, rect{0, 0, 1, 1}) && floor.area) {
                    floor.static_objects.push_back(comp);
                    ret = &floor.static_objects.back();
                    ret->cur_area = &floor;
                    ret->level = this;
                    ret->levelID = next_id;
                    next_id++;
                }
            }

            return *ret;
        }        

        bool isOnFloor(levelEntity entity) {
            return entity.collision && entity.cur_area->area;
        }

        template<typename T, typename = std::enable_if_t<std::is_base_of_v<levelEntity, T>>>
        T& spawnEntity(T entity) {
            T* ret;
            for (auto& floor: floors) {
                if (rect::of(entity.position.x,entity.position.y, rect{0, 0, 1, 1}) && floor.area) {
                    floor.entities[next_id] = entity;
                    ret = &floor.entities[next_id];
                    ret->cur_area = &floor;
                    ret->level = this;
                    ret->levelID = next_id;
                    next_id++;
                }
            }

            return *ret;
        }

        void addBackground(rect area, const char* texture_file) {
            floors.push_back(levelFloor(allocator.createTexture(texture_file), area));
        }

        
        void update(double delta) {
            for (auto& floor: floors) {
                for (auto& obj: floor.static_objects) {
                    if (obj.update != nullptr) obj.update(obj, delta);
                }
                for (auto& [id, entity]: floor.entities) {
                    if (entity.update != nullptr) entity.update(entity, delta);
                }
            }
        }

        void draw() {
            for (auto& floor: floors) {
                DrawTexturePro(floor.texture, 
                {0, 0, floor.area.width, floor.area.height}, 
                floor.area - vScroll, {0, 0}, 0, WHITE);
                for (auto& obj: floor.static_objects) {
                    if (obj.draw != nullptr) obj.draw(obj, vScroll);
                }
                for (auto& [id, entity]: floor.entities) {
                    if (entity.draw != nullptr) entity.draw(entity, vScroll);
                }
            }
        }
        

    };


}