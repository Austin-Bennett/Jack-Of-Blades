#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <raylib.h>

#include <utility>
#include "AustinUtils.hpp"
#undef null
#include "json.hpp"
using namespace nlohmann;


using namespace AustinUtils;
using namespace std;

typedef double seconds;

template<Arithmetic T, Arithmetic T2>
T round(T x, T2 nearest) {
    return cast(round(x/nearest)*nearest, T);
}

template<Arithmetic T, Arithmetic T2>
T floor(T x, T2 nearest) {
    return cast(floor(x/nearest)*nearest, T);
}

template<Arithmetic T, Arithmetic T2>
T ceil(T x, T2 nearest) {
    return cast(ceil(x/nearest)*nearest, T);
}

template<Arithmetic T1, Arithmetic T2, Arithmetic T3>
T1 clamp(T1 x, T2 min, T3 max) {
    return cast(x < min ? min : (x > max ? max:x), T1);
}

constexpr Color debug_colors[] = {WHITE, GRAY, PINK, GOLD, VIOLET, RED};

struct rect {
    double x;
    double y;
    double w;
    double h;


    rect() {
        x = y = w = h = 0;
    }

    template<Arithmetic tx, Arithmetic ty, Arithmetic tw, Arithmetic th>
    rect(tx x = 0, ty y = 0, tw w = 0, th h = 0) : x(x), y(y), w(w), h(h) {}

    template<Arithmetic T, Arithmetic tw, Arithmetic th>
    rect(v2<T> v, tw w, th h) : x(v.x), y(v.y), w(w), h(h) {}

    NODISCARD dvec2 pos() const {
        return {x, y};
    }

    operator Rectangle() const {
        return {cast(x, float), cast(y, float), cast(w, float), cast(h, float)};
    }

    rect pure() const {
        return {0, 0, w, h};
    }

    bool operator &&(const rect other) const {
        if (x+w < other.x) return false;
        if (x > other.x+other.w) return false;
        if (y+h < other.y) return false;
        if (y > other.y+other.h) return false;
        return true;
    }

    rect operator &(const rect& r2) const {
        double x1 = std::max(x, r2.x);
        double y1 = std::max(y, r2.y);
        double x2 = std::min(x + w, r2.x + r2.w);
        double y2 = std::min(y + h, r2.y + r2.h);

        double intersectWidth = x2 - x1;
        double intersectHeight = y2 - y1;

        if (intersectWidth <= 0 || intersectHeight <= 0) {
            return {0, 0, 0, 0}; // No intersection
        }

        return {x1, y1, intersectWidth, intersectHeight};
    }

    rect operator *(const double scalar) const {
        rect ret = *this;
        ret.w*=scalar;
        ret.h*=scalar;

        return ret;
    }

    template<Arithmetic T>
    rect operator +(v2<T> vec) const {
        rect ret = *this;
        ret.x+=vec.x;
        ret.h*=vec.y;

        return ret;
    }


    rect operator +(const rect r) const {
        rect ret = *this;
        ret.w += r.w;
        ret.h += r.h;
        ret.x+=r.x;
        ret.h*=r.y;

        return ret;
    }

    rect operator -(const rect r) const {
        rect ret = *this;
        ret.w -= r.w;
        ret.h -= r.h;
        ret.x-=r.x;
        ret.h-=r.y;

        return ret;
    }

    template<Arithmetic T>
    rect operator -(v2<T> vec) const {
        rect ret = *this;
        ret.x-=vec.x;
        ret.y-=vec.y;

        return ret;
    }

    rect& operator *=(const double scalar) {

        w *= scalar;
        h *= scalar;

        return *this;
    }

    template<Arithmetic T>
    rect& operator +=(v2<T> vec) {

        x+=vec.x;
        h*=vec.y;

        return *this;
    }


    rect& operator +=(const rect r) {

        w += r.w;
        h += r.h;
        x+=r.x;
        h*=r.y;

        return *this;
    }

    rect& operator -=(const rect r) {

        w -= r.w;
        h -= r.h;
        x -= r.x;
        h -= r.y;

        return *this;
    }

    template<Arithmetic T>
    rect& operator -=(v2<T> vec) {

        x -= vec.x;
        h -= vec.y;

        return *this;
    }
};


class Allocator {
    vector<void*> memory;
    vector<Texture2D> textures;
    vector<RenderTexture2D> render_textures;
    vector<Shader> shaders;

    template<typename T, typename vT>
    static void free(T& x, vector<vT> vec, function<void(T&)> destroy) {
        auto it = std::find(vec.begin(), vec.end(), x);

        if (it == vec.end()) throw Exception("Cannot free object at ", &x, " as it is not allocated by this allocator");
        destroy(x);
        vec.erase(it);
    }

    template<typename T, typename vT>
    static void free(T& x, vector<vT> vec, function<void(T&)> destroy, function<bool(vT&)> predicate) {
        auto it = std::find_if(vec.begin(), vec.end(), predicate);

        if (it == vec.end()) throw Exception("Cannot free object at ", &x, " as it is not allocated by this allocator");
        destroy(x);
        vec.erase(it);
    }

public:
    Allocator() = default;
    Allocator(Allocator&) = delete;
    Allocator(Allocator&& a)  noexcept {
        memory = std::move(a.memory);
        a.memory.clear();
    }
    Allocator& operator =(Allocator&) = delete;
    Allocator& operator =(Allocator&& a)  noexcept {
        memory = std::move(a.memory);
        a.memory.clear();
        return *this;
    }

    static Allocator& instance() {
        static Allocator A;
        return A;
    }

    template<typename T, typename... ConstructorArgs>
    T* allocateEmplaced(ConstructorArgs... args) {
        T* ret = new T(args...);

        memory.push_back(ret);

        return ret;
    }

    void* allocate(usize amt) {
        void* ret = ::operator new(amt);
        memory.push_back(ret);

        return ret;
    }

    Texture2D allocateTexture(const char* filename) {
        textures.push_back(LoadTexture(filename));
        return textures.back();
    }

    RenderTexture2D allocateRenderTexture(i32 w, i32 h) {
        render_textures.push_back(LoadRenderTexture(w, h));
        return render_textures.back();
    }

    Shader allocateShader(const char* vShader, const char* fShader) {
        shaders.push_back(LoadShader(vShader, fShader));
        return shaders.back();
    }



    template<typename T>
    void free(T* ptr) {
        free(ptr, memory, [](T*& p) {
            delete p;
        });
    }

    void free(void* ptr) {
        free<void*, void*>(ptr, memory, [](void*& p) {
            ::operator delete(p);
        });
    }

    void free(Texture2D texture) {
        free<Texture2D, Texture2D>(texture, textures,
        [](const Texture2D& t) {
            UnloadTexture(t);
        },
        [&texture](const Texture2D& t) {
            return t.id == texture.id;
        });
    }

    void free(RenderTexture2D rtexture) {
        free<RenderTexture2D, RenderTexture2D>(rtexture, render_textures,
        [](const RenderTexture2D& t) {
            UnloadRenderTexture(t);
        },
        [&rtexture](const RenderTexture2D& t) {
            return t.id == rtexture.id;
        });
    }

    void free(Shader shader) {
        free<Shader, Shader>(shader, shaders,
        [](const Shader& t) {
            UnloadShader(t);
        },
        [&shader](const Shader& t) {
            return t.id == shader.id;
        });
    }

    void free() {
        for (auto& mem: memory) {
            ::operator delete(mem);
        }
        memory.clear();
        for (const auto& texture: textures) {
            UnloadTexture(texture);
        }
        textures.clear();
        for (const auto& rtexture: render_textures) {
            UnloadRenderTexture(rtexture);
        }
        render_textures.clear();
        for (const auto& shader: shaders) {
            UnloadShader(shader);
        }
        shaders.clear();
    }
};

struct Object {
    virtual ~Object() = default;
    Object() = default;
    virtual void update(seconds delta) {};
    virtual void draw(dvec2 offset) {};
    virtual str toStr() {
        int status;
        char* demangle = abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, &status);

        // Create the result string (class name + pointer address)
        str result = (status == 0 ? demangle : typeid(*this).name());
        result = result + "@" + str(reinterpret_cast<void*>(this));

        // Free the demangled string if it's not null
        std::free(demangle);

        return result;
    }
};

struct animation : public Object {
private:
    double keyframe = 0.0;
    double max_keyframe = 0.0;
    double frame_duration;
    i32 frame_height;
    Texture2D texture{};
public:


    animation() : frame_duration(0.0), frame_height(0) {

    }

    animation(const Texture2D &texture, const i32 frame_height, const seconds frame_duration) : frame_duration(frame_duration), frame_height(frame_height) {
        this->texture = texture;
        max_keyframe = round(cast(texture.height, double)/cast(frame_height, double));
    }

    animation(const char* file, const i32 frame_height, const seconds frame_duration) : frame_duration(frame_duration), frame_height(frame_height) {
        this->texture = Allocator::instance().allocateTexture(file);
        max_keyframe = round(cast(texture.height, double)/cast(frame_height, double));
    }

    NODISCARD rect getFrameRect() const {
        return {0, floor(keyframe)*frame_height, texture.width, frame_height};
    }

    NODISCARD Texture2D getTexture() const {
        return texture;
    }

    NODISCARD i32 width() const {
        return texture.width;
    }

    NODISCARD i32 height() const {
        return texture.height;
    }

    void update(seconds delta) override {
        keyframe += delta;
        if (keyframe > max_keyframe) {
            keyframe = 0.0;
        }

    }

};

inline void DrawAnimation(const animation& anim, const i32 x, const i32 y, const Color tint = WHITE) {
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(), rect{x, y, anim.width(), anim.height()}, {0, 0}, 0.0, tint);
}

inline void DrawAnimation(const animation& anim, const rect &source, const rect &dest, const dvec2 origin, const arcdegrees rotation, const Color tint = WHITE) {
    DrawTexturePro(anim.getTexture(), source + anim.getFrameRect().pos(),
        dest, {cast(origin.x, float), cast(origin.y, float)}, cast(rotation, float), tint);
}

inline void DrawAnimation(const animation& anim, const dvec2 pos, const arcdegrees rotation, const double scale, const Color tint = WHITE) {
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(),
        rect{pos.x, pos.y, anim.width()*scale, anim.height()*scale}, {0, 0}, cast(rotation, float), tint);
}

inline void DrawAnimation(const animation& anim, const rect& dest, Color tint = WHITE) {
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(), dest
        , {0, 0}, 0.0, tint);
}

inline void DrawAnimation(const animation& anim, dvec2 pos, Color tint = WHITE) {
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(), rect{pos.x, pos.y, anim.width(), anim.height()}, {0, 0}, 0.0, tint);
}

inline void DrawAnimation(const animation& anim, const rect& source, dvec2 pos, Color tint = WHITE) {
    DrawTexturePro(anim.getTexture(), source + anim.getFrameRect().pos(),
        {cast(pos.x, float), cast(pos.y, float), cast(anim.width(), float), cast(anim.height(), float)},
        {0, 0}, 0.0, tint);
}

class Identifier {
    str globalID;//essentially, the folder where the data is
    str specificID;//the name of the specific thing

    public:
    Identifier(str global, str specific) : globalID(std::move(global)), specificID(std::move(specific)) {}

    str getGlobalID() const {
        return globalID;
    }

    str getSpecificID() const {
        return specificID;
    }

    str toStr() {
        return globalID + "::" + specificID;
    }

    bool operator==(const Identifier &o) const {
        return globalID == o.globalID && specificID == o.specificID;
    }

    friend ostream& operator <<(std::ostream& os, Identifier& self) {
        os << self.toStr();
        return os;
    }
};

template<>
struct std::hash<Identifier> {
    usize operator()(const Identifier& id) const noexcept {
        return std::hash<str>{}(id.getGlobalID()) + std::hash<str>{}(id.getSpecificID());
    }
};

/*
 * Animation ID rules:
 * the ID will be used as a name for the json file it will look for
 *
 */

inline bool validateJsonData(json& data, const str& key, json::value_t type) {
    if (!data.contains(key.data()) || data[key.data()].type() != type) {
        return false;
    }
    return true;
}

inline bool assertJsonData(json& data, const str& key, json::value_t type) {
    if (!data.contains(key.data()) || data[key.data()].type() != type) {
        throw Exception("Error parsing json, could not find key: ", key, " or key is not type ", json::type_name(type));
    }
    return true;
}



struct AnimationRegistry {
    private:
    unordered_map<Identifier, shared_ptr<animation>> animations;

    static bool validateData(json& data) {
        if (!validateJsonData(data, "path", json::value_t::string)) {
            return false;
        }
        if (!validateJsonData(data, "frame_height", json::value_t::number_unsigned)) {
            return false;
        }
        if (!validateJsonData(data, "frame_duration", json::value_t::number_float)) {
            return false;
        }

        return true;
    }

    //helper function to load animations
    void add(const Identifier& id) {
        std::ifstream jsonFile(("data/"_str + id.getGlobalID() + "/" + id.getSpecificID() + ".json").data());
        if (!jsonFile.is_open()) throw Exception("Could not find json file for identifier ", id);

        json AnimationData;
        jsonFile >> AnimationData;
        jsonFile.close();
        if (!validateData(AnimationData)) {
            throw Exception("Could not create animation from json file pointed by identifier ", id);
        }
        animations[id] = make_shared<animation>(("resources/"_str + str(AnimationData["path"].get<string>())).data(),
                                                AnimationData["frame_height"].get<u32>(),
                                                AnimationData["frame_duration"].get<double>());


    }

    public:
    AnimationRegistry() {
        try {
            for (const auto& entry: filesystem::directory_iterator("data/animation")) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    add(Identifier("animation", entry.path().stem().generic_string()));
                }
            }
        } catch (exception& e) {
            cerr << e.what() << "\n";
        }
    }

    static AnimationRegistry& Instance() {
        static AnimationRegistry instance;
        return instance;
    }

    shared_ptr<animation> get(const Identifier& id) {
        return animations[id];
    }
};



#endif
