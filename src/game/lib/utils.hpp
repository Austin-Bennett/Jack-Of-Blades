#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <raylib.h>

#include <utility>
#include "AustinUtils.hpp"
#undef null
#include <stack>
#include <thread>
#include <unordered_set>

#include "enums.hpp"
#include "json.hpp"
using namespace nlohmann;

#define EXPAND_V(VEC) (VEC).x, (VEC).y

using namespace AustinUtils;
using namespace std;

typedef double seconds_t;

#define JSON_NUMBERS json::value_t::number_integer, json::value_t::number_float, json::value_t::number_unsigned
#define JSON_INTEGERS json::value_t::number_integer, json::value_t::number_unsigned
#define SENUM(E) case E: return #E;
#define SIGN(n) (n < 0 ? -1:1)
#define EXPAND_R(r) (r).x,(r).y, (r).w, (r).h

#include <string>
#include "raylib.h"

inline fvec2 tofvec(const Vector2 v) {
    return {v.x, v.y};
}

template<Arithmetic T, Arithmetic U>
T max(T a, U b) {
    return cast(a > b ? a:b, T);
}

template<Arithmetic T, Arithmetic U>
T min(T a, U b) {
    return cast(a < b ? a:b, T);
}

class LevelEditor;

template<typename T>
ostream& operator <<(ostream& os, vector<T> v) {
    os << "[";
    for (usize i = 0; i < v.size(); i++) {
        os << v[i];
        if (i != v.size()-1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

inline string getRelativePath(const char* path) {
    return filesystem::relative(path).generic_string();
}

inline string getFile(const char* path) {
    return filesystem::path(path).filename().generic_string();
}

enum direction {
    NONE,
    EAST,
    NORTHEAST,
    NORTH,
    NORTHWEST,
    WEST,
    SOUTHWEST,
    SOUTH,
    SOUTHEAST,
};

inline str dtos(const direction dir) {
    switch (dir) {
        SENUM(NONE)
        SENUM(EAST)
        SENUM(NORTHEAST);
        SENUM(NORTH)
        SENUM(NORTHWEST)
        SENUM(WEST)
        SENUM(SOUTHWEST)
        SENUM(SOUTH)
        SENUM(SOUTHEAST)
        default: return "NULL";
    }
}

inline radians dtorad(const direction d) {
    if (d == NONE) {
        return 0.0;
    }
    return (cast(d, u32)-1)*(M_PI/4);
}

template<Arithmetic T>
direction vtod(v2<T> vec) {
    //if the both components are 0, there is no direction
    if (vec.x == 0 && vec.y == 0) {
        return NONE;
    }
    //if x == 0, we're either moving up or down
    if (vec.x == 0) {
        return vec.y > 0 ? SOUTH : NORTH;
    }
    //if y == 0, we're either moving left or right
    if (vec.y == 0) {
        return vec.x < 0 ? WEST:EAST;
    }
    //if x is less than zero we must be moving either southwest or northwest
    if (vec.x < 0) {
        return vec.y > 0 ? SOUTHWEST : NORTHWEST;
    }

    //otherwise we must be moving either southeast or northeast
    return vec.y > 0 ? SOUTHEAST : NORTHEAST;
}

inline str KeyToString(const KeyboardKey key) {
    switch (key) {
        case KEY_NULL: return "KEY_NULL";
        case KEY_APOSTROPHE: return "KEY_APOSTROPHE";
        case KEY_COMMA: return "KEY_COMMA";
        case KEY_MINUS: return "KEY_MINUS";
        case KEY_PERIOD: return "KEY_PERIOD";
        case KEY_SLASH: return "KEY_SLASH";
        case KEY_ZERO: return "KEY_ZERO";
        case KEY_ONE: return "KEY_ONE";
        case KEY_TWO: return "KEY_TWO";
        case KEY_THREE: return "KEY_THREE";
        case KEY_FOUR: return "KEY_FOUR";
        case KEY_FIVE: return "KEY_FIVE";
        case KEY_SIX: return "KEY_SIX";
        case KEY_SEVEN: return "KEY_SEVEN";
        case KEY_EIGHT: return "KEY_EIGHT";
        case KEY_NINE: return "KEY_NINE";
        case KEY_SEMICOLON: return "KEY_SEMICOLON";
        case KEY_EQUAL: return "KEY_EQUAL";
        case KEY_A: return "KEY_A";
        case KEY_B: return "KEY_B";
        case KEY_C: return "KEY_C";
        case KEY_D: return "KEY_D";
        case KEY_E: return "KEY_E";
        case KEY_F: return "KEY_F";
        case KEY_G: return "KEY_G";
        case KEY_H: return "KEY_H";
        case KEY_I: return "KEY_I";
        case KEY_J: return "KEY_J";
        case KEY_K: return "KEY_K";
        case KEY_L: return "KEY_L";
        case KEY_M: return "KEY_M";
        case KEY_N: return "KEY_N";
        case KEY_O: return "KEY_O";
        case KEY_P: return "KEY_P";
        case KEY_Q: return "KEY_Q";
        case KEY_R: return "KEY_R";
        case KEY_S: return "KEY_S";
        case KEY_T: return "KEY_T";
        case KEY_U: return "KEY_U";
        case KEY_V: return "KEY_V";
        case KEY_W: return "KEY_W";
        case KEY_X: return "KEY_X";
        case KEY_Y: return "KEY_Y";
        case KEY_Z: return "KEY_Z";
        case KEY_LEFT_BRACKET: return "KEY_LEFT_BRACKET";
        case KEY_BACKSLASH: return "KEY_BACKSLASH";
        case KEY_RIGHT_BRACKET: return "KEY_RIGHT_BRACKET";
        case KEY_GRAVE: return "KEY_GRAVE";
        case KEY_SPACE: return "KEY_SPACE";
        case KEY_ESCAPE: return "KEY_ESCAPE";
        case KEY_ENTER: return "KEY_ENTER";
        case KEY_TAB: return "KEY_TAB";
        case KEY_BACKSPACE: return "KEY_BACKSPACE";
        case KEY_INSERT: return "KEY_INSERT";
        case KEY_DELETE: return "KEY_DELETE";
        case KEY_RIGHT: return "KEY_RIGHT";
        case KEY_LEFT: return "KEY_LEFT";
        case KEY_DOWN: return "KEY_DOWN";
        case KEY_UP: return "KEY_UP";
        case KEY_PAGE_UP: return "KEY_PAGE_UP";
        case KEY_PAGE_DOWN: return "KEY_PAGE_DOWN";
        case KEY_HOME: return "KEY_HOME";
        case KEY_END: return "KEY_END";
        case KEY_CAPS_LOCK: return "KEY_CAPS_LOCK";
        case KEY_SCROLL_LOCK: return "KEY_SCROLL_LOCK";
        case KEY_NUM_LOCK: return "KEY_NUM_LOCK";
        case KEY_PRINT_SCREEN: return "KEY_PRINT_SCREEN";
        case KEY_PAUSE: return "KEY_PAUSE";
        case KEY_F1: return "KEY_F1";
        case KEY_F2: return "KEY_F2";
        case KEY_F3: return "KEY_F3";
        case KEY_F4: return "KEY_F4";
        case KEY_F5: return "KEY_F5";
        case KEY_F6: return "KEY_F6";
        case KEY_F7: return "KEY_F7";
        case KEY_F8: return "KEY_F8";
        case KEY_F9: return "KEY_F9";
        case KEY_F10: return "KEY_F10";
        case KEY_F11: return "KEY_F11";
        case KEY_F12: return "KEY_F12";
        case KEY_LEFT_SHIFT: return "KEY_LEFT_SHIFT";
        case KEY_LEFT_CONTROL: return "KEY_LEFT_CONTROL";
        case KEY_LEFT_ALT: return "KEY_LEFT_ALT";
        case KEY_LEFT_SUPER: return "KEY_LEFT_SUPER";
        case KEY_RIGHT_SHIFT: return "KEY_RIGHT_SHIFT";
        case KEY_RIGHT_CONTROL: return "KEY_RIGHT_CONTROL";
        case KEY_RIGHT_ALT: return "KEY_RIGHT_ALT";
        case KEY_RIGHT_SUPER: return "KEY_RIGHT_SUPER";
        case KEY_KB_MENU: return "KEY_KB_MENU";
        case KEY_KP_0: return "KEY_KP_0";
        case KEY_KP_1: return "KEY_KP_1";
        case KEY_KP_2: return "KEY_KP_2";
        case KEY_KP_3: return "KEY_KP_3";
        case KEY_KP_4: return "KEY_KP_4";
        case KEY_KP_5: return "KEY_KP_5";
        case KEY_KP_6: return "KEY_KP_6";
        case KEY_KP_7: return "KEY_KP_7";
        case KEY_KP_8: return "KEY_KP_8";
        case KEY_KP_9: return "KEY_KP_9";
        case KEY_KP_DECIMAL: return "KEY_KP_DECIMAL";
        case KEY_KP_DIVIDE: return "KEY_KP_DIVIDE";
        case KEY_KP_MULTIPLY: return "KEY_KP_MULTIPLY";
        case KEY_KP_SUBTRACT: return "KEY_KP_SUBTRACT";
        case KEY_KP_ADD: return "KEY_KP_ADD";
        case KEY_KP_ENTER: return "KEY_KP_ENTER";
        case KEY_KP_EQUAL: return "KEY_KP_EQUAL";
        case KEY_BACK: return "KEY_BACK";
        case KEY_MENU: return "KEY_MENU";
        case KEY_VOLUME_UP: return "KEY_VOLUME_UP";
        case KEY_VOLUME_DOWN: return "KEY_VOLUME_DOWN";
        default: return "UNKNOWN_KEY";
    }
}


inline Color InvertColor(Color color) {
    // Inverts the RGB values while leaving the alpha unchanged
    return (Color){ cast(255 - color.r, u8), cast(255 - color.g, u8), cast(255 - color.b, u8), color.a};
}

template<Arithmetic T, Arithmetic T2>
T round(T x, T2 nearest) {
    return cast(round(x/nearest)*nearest, T);
}

template<Arithmetic T>
v2<T> round(v2<T> x, T closest = 1) {
    x.x = round(x.x, closest);
    x.y = round(x.y, closest);
    return x;
}

template<Arithmetic T, Arithmetic T2>
T floor(T x, T2 nearest) {
    return cast(floor(x/abs(nearest))*abs(nearest), T);
}

template<Arithmetic T, Arithmetic T2>
T ceil(T x, T2 nearest) {
    return cast(ceil(abs(x)/nearest)*nearest * SIGN(x), T);
}

template<Arithmetic T>
v2<T> ceil(v2<T> x, T closest = 1) {
    x.x = ceil(x.x, closest);
    x.y = ceil(x.y, closest);
    return x;
}

template<Arithmetic T1, Arithmetic T2, Arithmetic T3>
T1 clamp(T1 x, T2 min, T3 max) {
    return cast(x < min ? min : (x > max ? max:x), T1);
}

template<Arithmetic T1, Arithmetic T2, Arithmetic T3>
bool inRange(T1 x, T2 one, T3 two) {
    return x > min(one, two) && x < max(one, two);
}

//the capital I just means inclusive
template<Arithmetic T1, Arithmetic T2, Arithmetic T3>
bool IinRange(T1 x, T2 one, T3 two) {
    return x >= min(one, two) && x < max(one, two);
}

constexpr Color debug_colors[] = {WHITE, GRAY, PINK, GOLD, VIOLET, RED};

template<FloatingPoint T, FloatingPoint T2>
bool fequal(T x1, T2 x2, float epsilion = 0.000001) {
    return abs(x1-x2) < epsilion;
}





//checks if the line from p1 to p2 intersects the line from p3 to p4
template<Arithmetic T1, Arithmetic T2, Arithmetic T3, Arithmetic T4>
bool intersects(v2<T1> p1, v2<T2> p2, v2<T3> p3, v2<T4> p4) {

    //if both lines are vertical return true if their x values are equal
    if (fequal(p1.x, p2.x) && fequal(p3.x, p4.x)) {
        return p1.x == p3.x;
    }

    //if line 1 is vertical return true if the y value of line 2 at the x value of line 1
    //is within the range of line 1
    if (fequal(p1.x, p2.x)) {
        const float m2 = (p4.y-p3.y)/(p4.x-p3.x);
        const float b2 = (p3.y - m2*p3.x);

        return IinRange(m2*p1.x + b2, p1.y, p2.y);
    }
    //same thing but in reverse for line 2
    if (fequal(p3.x, p4.x)) {
        const float m1 = (p2.y-p1.y)/(p2.x-p1.x);
        const float b1 = (p1.y - m1*p1.x);

        return IinRange(m1*p3.x + b1, p3.y, p4.y);
    }


    const float m1 = (p2.y-p1.y)/(p2.x-p1.x);
    const float b1 = (p1.y - m1*p1.x);

    const float m2 = (p4.y-p3.y)/(p4.x-p3.x);
    const float b2 = (p3.y - m2*p3.x);

    v2<T1> intersection;

    if (fequal(m2, m1) && b2 == b1) {
        // return true if they overlap at all
        return IinRange(p1.x, p3.x, p4.x) || IinRange(p2.x, p3.x, p4.x) ||
       IinRange(p3.x, p1.x, p2.x) || IinRange(p4.x, p1.x, p2.x);
    }
    if (fequal(m1, m2)) return false;//they dont ever intersect

    intersection.x = (b1-b2)/(m2-m1);
    intersection.y = m1*intersection.x + b1;//using line 1 to get the intersection y component

    return IinRange(intersection.x, p1.x, p2.x) && IinRange(intersection.x, p3.x, p4.x) &&
        IinRange(intersection.y, p1.y, p2.y) && IinRange(intersection.y, p3.y, p4.y);
}


struct rect {
    double x{};
    double y{};
    double w{};
    double h{};




    rect() = default;

    template<Arithmetic tx, Arithmetic ty, Arithmetic tw, Arithmetic th>
    rect(tx x = 0, ty y = 0, tw w = 0, th h = 0) : x(x), y(y), w(w), h(h) {}

    template<Arithmetic VT1, Arithmetic VT2>
    rect(v2<VT1> v1, v2<VT2> v2) {
        x = min(v1.x, v2.x);
        y = min(v1.y, v2.y);
        w = max(v1.x, v2.x) - x;
        h = max(v1.y, v2.y) - y;
    }

    template<Arithmetic T, Arithmetic tw, Arithmetic th>
    rect(v2<T> v, tw w, th h) : x(v.x), y(v.y), w(w), h(h) {}

    rect(const rect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {

    }

    rect(const rect&& r) noexcept : x(r.x), y(r.y), w(r.w), h(r.h) {

    }

    void draw(dvec2 offset, const Color c, float thickness = 0) const {
        if (thickness == 0) DrawRectangle(cast(x-offset.x, i32), cast(y-offset.y, i32), cast(w, i32), cast(h, i32), c);
        DrawRectangleLinesEx(cast(*this-offset, rect), thickness, c);
    }

    friend ostream& operator <<(ostream& os, const rect& self) {
        os << "[" << self.x << ", " << self.y << ", " << self.w << ", " << self.h << "]";
        return os;
    }

    rect& operator =(const rect& r) {
        x = r.x;
        y = r.y;
        w = r.w;
        h = r.h;
        return *this;
    }



    rect& operator =(rect&& r) noexcept {
        x = r.x;
        y = r.y;
        w = r.w;
        h = r.h;


        return *this;
    }

    vector<double> components() {
        return {x, y, w, h};
    }

    NODISCARD dvec2 pos() const {
        return {x, y};
    }

    rect& pos(const dvec2 v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    operator Rectangle() const {
        return {cast(x, float), cast(y, float), cast(w, float), cast(h, float)};
    }

    [[nodiscard]] double area() const {
        return w*h;
    }

    [[nodiscard]] rect pure() const {
        return {0, 0, w, h};
    }

    [[nodiscard]] dvec2 center() const {
        return {x+w/2, y+h/2};
    }

    //slightly more data-efficient than (r1 & r2).area()
    //especially since all the variables in here are consts, the compiler can just inline them to use only 8 bytes for the return value
    [[nodiscard]] double collideArea(const rect& r2) const {
        if (!(*this && r2)) {
            return 0;
        }
        const double x1 = std::max(x, r2.x);
        const double y1 = std::max(y, r2.y);
        const double x2 = std::min(x + w, r2.x + r2.w);
        const double y2 = std::min(y + h, r2.y + r2.h);

        const double intersectWidth = x2 - x1;
        const double intersectHeight = y2 - y1;


        return intersectHeight*intersectWidth;
    }

    //check if a line from p1 to p2 intersects the rectangle
    template<Arithmetic T1, Arithmetic T2>
    bool intersects(v2<T1> p1, v2<T2> p2) {
        if (IinRange(p1.x, x, x+w) && IinRange(p1.y, y, y+h)) {
            return true;
        }
        if (IinRange(p2.x, x, x+w) && IinRange(p2.y, y, y+h)) {
            return true;
        }

        //check if the lines intersect with any rectangle sides
        if (::intersects(p1, p2, dvec2{x, y}, dvec2{x+w, y})) return true;
        if (::intersects(p1, p2, dvec2{x+w, y}, dvec2{x+w, y+h})) return true;
        if (::intersects(p1, p2, dvec2{x+w, y+h}, dvec2{x, y+h})) return true;
        if (::intersects(p1, p2, dvec2{x, y+h}, dvec2{x, y})) return true;

        return false;
    }

    bool operator &&(const rect other) const {
        if (x+w < other.x) return false;
        if (x > other.x+other.w) return false;
        if (y+h < other.y) return false;
        if (y > other.y+other.h) return false;
        return true;
    }

    rect operator &(const rect& r2) const {
        const double x1 = std::max(x, r2.x);
        const double y1 = std::max(y, r2.y);
        const double x2 = std::min(x + w, r2.x + r2.w);
        const double y2 = std::min(y + h, r2.y + r2.h);

        double intersectWidth = x2 - x1;
        double intersectHeight = y2 - y1;

        if (intersectWidth <= 0 || intersectHeight <= 0) {
            return {0, 0, 0, 0}; // No intersection
        }

        return {x1, y1, intersectWidth, intersectHeight};
    }

    template<Arithmetic T>
    bool operator&&(const v2<T> v) {
        if (v.x < x) return false;
        if (v.x > x+w) return false;
        if (v.y < y) return false;
        if (v.y > y+h) return false;

        return true;
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
        ret.y+=vec.y;

        return ret;
    }


    rect operator +(const rect& r) const {
        rect ret = *this;
        ret.w += r.w;
        ret.h += r.h;
        ret.x+=r.x;
        ret.y+=r.y;

        return ret;
    }

    rect operator -(const rect& r) const {
        rect ret = *this;
        ret.w -= r.w;
        ret.h -= r.h;
        ret.x-=r.x;
        ret.y-=r.y;

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
        y+=vec.y;

        return *this;
    }


    rect& operator +=(const rect& r) {

        w += r.w;
        h += r.h;
        x +=r.x;
        y +=r.y;

        return *this;
    }

    rect& operator -=(const rect& r) {

        w -= r.w;
        h -= r.h;
        x -= r.x;
        y -= r.y;

        return *this;
    }

    template<Arithmetic T>
    rect& operator -=(v2<T> vec) {

        x -= vec.x;
        y -= vec.y;

        return *this;
    }
};

const rect screen_rect = {0, 0, 1280, 720};

class Allocator {
    vector<void*> memory;
    unordered_map<str, Texture2D> textures;
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
        auto it = std::find_if(vec.begin(), vec.end(), std::move(predicate));

        if (it == vec.end()) throw Exception("Cannot free object at ", &x, " as it is not allocated by this allocator");
        destroy(x);
        vec.erase(it);
    }

    explicit Allocator() {
        //load EVERY texture in resources so we dont waste time loading them on-the-fly
        for (const auto& entry: filesystem::recursive_directory_iterator("resources")) {
            if (entry.is_regular_file() && entry.path().extension() == ".png") {
                IallocateTexture(entry.path().string().data());
            }
        }
    }

public:

    Allocator(Allocator&) = delete;
    Allocator(Allocator&& a) = delete;
    Allocator& operator =(Allocator&) = delete;
    Allocator& operator =(Allocator&& a) = delete;

    template<typename T, typename... ConstructorArgs>
    static T* allocateEmplaced(ConstructorArgs... args) {
        return instance().IallocateEmplaced<T>(std::forward<ConstructorArgs...>(args...));
    }

    static void* allocate(const usize amt) {

        return instance().Iallocate(amt);
    }

    static void reloadTextures(bool delete_previous, bool& finished);

    static Texture2D allocateTexture(const char* filename) {
        return instance().IallocateTexture(filename);
    }

    static RenderTexture2D allocateRenderTexture(const i32 w, const i32 h) {
        return instance().IallocateRenderTexture(w, h);
    }

    static Shader allocateShader(const char* vShader, const char* fShader) {
        return instance().IallocateShader(vShader, fShader);
    }

    static unordered_map<str, Texture2D> getAllTextures() {
        return instance().textures;
    }

    template<typename T>
    static void free(T* ptr) {
        instance().Ifree(ptr);
    }

    static void free(void* ptr) {
        instance().Ifree(ptr);
    }

    static void free(const Texture2D &texture) {
        instance().Ifree(texture);
    }

    static void free(const RenderTexture2D &rtexture) {
        instance().Ifree(rtexture);
    }

    static void free(const Shader shader) {
        instance().Ifree(shader);
    }

    static void free() {
        instance().Ifree();
    }

    static pair<str, Texture2D> getTexture(u32 id) {
        auto it = std::find_if(instance().textures.begin(), instance().textures.end(), [id](pair<str, Texture2D> t) {
            return t.second.id == id;
        });
        if (it == instance().textures.end()) {
            throw Exception("No texture allocated with id ", id);
        }
        return *it;
    }

    static Texture2D getTexture(const str& path) {
        return instance().IallocateTexture(path.data());
    }

private:
    static Allocator& instance() {
        static Allocator A;
        return A;
    }


    template<typename T, typename... ConstructorArgs>
    T* IallocateEmplaced(ConstructorArgs... args) {
        T* ret = new T(args...);

        memory.push_back(ret);

        return ret;
    }

    void* Iallocate(usize amt) {
        void* ret = ::operator new(amt);
        memory.push_back(ret);

        return ret;
    }

    Texture2D IallocateTexture(const char* filename) {
        string f = filesystem::relative(filename).generic_string();
        for (auto& c: f) {
            if (c == '\\') c = '/';
        }
        str s = f;
        if (textures.contains(s.data())) cout << "Texture: " << s.data() << " already exists!";
        if (!textures.contains(s.data())) {
            textures[s.data()] = LoadTexture(s.data());
            cout << "Allocating texture: " << s.data() << "\n";
        }
        return textures[s.data()];
    }

    RenderTexture2D IallocateRenderTexture(i32 w, i32 h) {
        render_textures.push_back(LoadRenderTexture(w, h));
        return render_textures.back();
    }

    Shader IallocateShader(const char* vShader, const char* fShader) {
        shaders.push_back(LoadShader(vShader, fShader));
        return shaders.back();
    }



    template<typename T>
    void Ifree(T* ptr) {
        free(ptr, memory, [](T*& p) {
            delete p;
        });
    }

    void Ifree(void* ptr) {
        free<void*, void*>(ptr, memory, [](void*& p) {
            ::operator delete(p);
        });
    }

    void Ifree(Texture2D texture) {
        auto it = std::find_if(textures.begin(), textures.end(), [texture](const pair<str, Texture2D> p) {
            return p.second.id == texture.id;
        });

        UnloadTexture(it->second);
        textures.erase(it);
    }

    void Ifree(RenderTexture2D rtexture) {
        free<RenderTexture2D, RenderTexture2D>(rtexture, render_textures,
        [](const RenderTexture2D& t) {
            UnloadRenderTexture(t);
        },
        [&rtexture](const RenderTexture2D& t) {
            return t.id == rtexture.id;
        });
    }

    void Ifree(Shader shader) {
        free<Shader, Shader>(shader, shaders,
        [](const Shader& t) {
            UnloadShader(t);
        },
        [&shader](const Shader& t) {
            return t.id == shader.id;
        });
    }

    void Ifree() {
        for (auto& mem: memory) {
            ::operator delete(mem);
        }
        memory.clear();
        for (const auto &texture: textures | views::values) {
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

template<typename T>
str getTypename() {
    int status;
    char* demangle = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);

    // Create the result string (class name + pointer address)
    str result = (status == 0 ? demangle : typeid(T).name());

    // Free the demangled string if it's not null
    std::free(demangle);

    return result;
}

template<typename T>
str getTypename(const T& obj) {
    int status;
    char* demangle = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);

    // Create the result string (class name + pointer address)
    str result = (status == 0 ? demangle : typeid(T).name());

    // Free the demangled string if it's not null
    std::free(demangle);

    return result;
}


struct Object {
    virtual ~Object() = default;
    Object() = default;
    virtual void update(seconds_t delta) {};
    virtual void draw(dvec2 offset) {};
    virtual void drawLighting(dvec2 offset) {};
    virtual str toStr() {

        str result = getTypename(*this);
        result = result + "@" + str(reinterpret_cast<void*>(this));

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
    animation_type typ;
    str id;
    str path;
    friend struct AnimationRegistry;
public:



    animation() : frame_duration(0.0), frame_height(0), typ() {

    }

    animation(const Texture2D &texture, const i32 frame_height, const seconds_t frame_duration, animation_type t) : frame_duration(frame_duration), frame_height(frame_height),
    typ(t) {
        this->texture = texture;
        max_keyframe = round(cast(texture.height, double)/cast(frame_height, double));
    }

    animation(const char* file, const i32 frame_height, const seconds_t frame_duration, animation_type t) : frame_duration(frame_duration), frame_height(frame_height),
    typ(t) {
        this->texture = Allocator::allocateTexture(file);
        max_keyframe = round(cast(texture.height, double)/cast(frame_height, double));
    }

    animation_type getType() {
        return typ;
    }

    void setMaxFrame(i32 x) {
        max_keyframe = clamp(x, 0, texture.height/frame_height);
    }

    NODISCARD str& getId() {
        return id;
    }

    NODISCARD rect getFrameRect() const {
        return {0, floor(keyframe)*frame_height, texture.width, frame_height};
    }

    NODISCARD Texture2D& getTexture() {
        return texture;
    }

    NODISCARD i32 width() const {
        return texture.width;
    }

    NODISCARD i32& height() {
        return frame_height;
    }

    NODISCARD double& duration() {
        return frame_duration;
    }

    NODISCARD animation_type& type() {
        return typ;
    }

    NODISCARD str& Path() {
        return path;
    }

    NODISCARD i32 getCurrentFramePos() const {
        return floor(keyframe);
    }

    NODISCARD i32 getMaxFrame() const {
        return floor(max_keyframe);
    }

    void setFramePos(const i32 x) {
        keyframe = clamp(x, 0, max_keyframe);
    }



    void update(const seconds_t delta) override {
        if (keyframe >= max_keyframe) {
            switch (typ) {
                case animation_type::NONE:
                    return;
                case animation_type::LOOP:
                    keyframe = 0.0;
                case animation_type::ONCE:
                    return;
            }
        }
        keyframe += delta/frame_duration;
    }

    [[nodiscard]] bool isFinished() const {
        return keyframe > max_keyframe;
    }

    void reset() {
        keyframe = 0.0;

    }

};

inline void DrawAnimation(animation& anim, const i32 x, const i32 y, const Color tint = WHITE) {
    if (anim.type() == animation_type::NONE) return;
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(), rect{x, y, anim.width(), anim.height()}, {0, 0}, 0.0, tint);
}

inline void DrawAnimation(animation& anim, rect source, const rect &dest, const Color tint = WHITE) {
    if (anim.type() == animation_type::NONE) return;
    //draw the texture tiled

    source.y += anim.getFrameRect().y;
    for (double x = dest.x; x <= dest.x+dest.w; x += anim.width()) {
        for (double y = dest.y; y <= dest.y+dest.h; y += anim.height()) {
            const double w = min<double>(anim.width(), dest.x+dest.w-x);
            const double h = min<double>(anim.height(), dest.y+dest.h-y);
            rect r = anim.getFrameRect();
            r.w = w;
            r.h = h;
            DrawTexturePro(
                anim.getTexture(),
                r,
                rect{x, y, w, h},
                {0, 0},
                0.0,
                tint
                );
        }
    }
}

inline void DrawAnimation(animation& anim, const dvec2 pos, const arcdegrees rotation, const double scale, const Color tint = WHITE) {
    if (anim.type() == animation_type::NONE) return;
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(),
        rect{pos.x, pos.y, anim.width()*scale, anim.height()*scale}, {0, 0}, cast(rotation, float), tint);
}

inline void DrawAnimation(animation& anim, const rect& dest, Color tint = WHITE) {
    if (anim.type() == animation_type::NONE) return;
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(), dest
        , {0, 0}, 0.0, tint);
}

inline void DrawAnimation(animation& anim, dvec2 pos, Color tint = WHITE) {
    if (anim.type() == animation_type::NONE) return;
    DrawTexturePro(anim.getTexture(), anim.getFrameRect(), rect{pos.x, pos.y, anim.width(), anim.height()}, {0, 0}, 0.0, tint);
}

inline void DrawAnimation(animation& anim, const rect& source, dvec2 pos, Color tint = WHITE) {
    if (anim.type() == animation_type::NONE) return;
    DrawTexturePro(anim.getTexture(), source + anim.getFrameRect().pos(),
        {cast(pos.x, float), cast(pos.y, float), cast(anim.width(), float), cast(anim.height(), float)},
        {0, 0}, 0.0, tint);
}



/*
 * Animation ID rules:
 * the ID will be used as a name for the json file it will look for
 *
 */

template<typename... Args, typename = std::enable_if_t< ( ( (is_same_v<Args, json::value_t>) && ...) ) >>
bool validateJsonData(json data, const str& key, Args... type) {
    if (!data.contains(key.data()) || ((data[key.data()].type() != type) && ...)) {

        return false;
    }
    return true;
}

template<typename... Args, typename = std::enable_if_t< ( ( (is_same_v<Args, json::value_t>) && ...) ) >>
bool assertJsonData(json data, const str& key, Args... type) {
    if (!data.contains(key.data()) || ((data[key.data()].type() != type) && ...)) {
        throw Exception("Error parsing json, could not find key: ", key, " or key is not any of types: [ ",
                        ((str(json::type_name(type)) + " "), ...));
    }
    return true;
}

template<typename... Args, typename = std::enable_if_t< ( ( (is_same_v<Args, json::value_t>) && ...) ) >>
bool assertJsonData(json data, const str& key, str error_message, Args... type) {
    if (!data.contains(key.data()) || ((data[key.data()].type() != type) && ...)) {
        throw Exception("Error parsing json, could not find key: ", key, " or key is not any of types: [ ",
                        ((str(json::type_name(type)) + " "), ...));
    }
    return true;
}



struct AnimationRegistry {
    private:


    logger LAnimationRegistry = logger("animation-registry");

    static bool validateData(json& data) {
        if (!validateJsonData(data, "path", json::value_t::string)) {
            return false;
        }
        if (!validateJsonData(data, "frame_height", json::value_t::number_unsigned)) {
            return false;
        }
        if (!validateJsonData(data, "frame_duration", JSON_NUMBERS)) {
            return false;
        }
        if (!validateJsonData(data, "type", JSON_INTEGERS)) {
            return false;
        }

        return true;
    }

    //helper function to load animations
    void add(const str& id) {
        std::ifstream jsonFile(("data/animation"_str + "/" + id + ".json").data());
        if (!jsonFile.is_open()) throw Exception("Could not find json file for identifier ", id);

        json AnimationData;
        jsonFile >> AnimationData;
        jsonFile.close();
        if (!validateData(AnimationData)) {
            throw Exception("Could not create animation from json file pointed by identifier ", id);
        }
        animations[id] = make_shared<animation>(("resources/"_str + str(AnimationData["path"].get<string>())).data(),
                                                AnimationData["frame_height"].get<u32>(),
                                                AnimationData["frame_duration"].get<double>(),
                                                cast(AnimationData["type"].get<u32>(), animation_type));
        animations[id]->id = id;
        animations[id]->path = AnimationData["path"].get<string>();
        LAnimationRegistry.info("Created animation: ", id);

    }

    public:

    unordered_map<str, shared_ptr<animation>> animations;

    AnimationRegistry() {
        try {
            for (const auto& entry: filesystem::directory_iterator("data/animation")) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    cout << "id: " << entry.path().stem().generic_string() << "\n";
                    add(str(entry.path().stem().generic_string()));
                }
            }
        } catch (exception& e) {
            cerr << e.what() << "\n";
        }
        if (!animations.contains("default")) {
            animations["default"] = make_shared<animation>();
        }
    }

    vector<pair<str, animation>> getAnimations() {
        vector<pair<str, animation>> ret;
        for (auto&[id, a]: animations) {
            ret.emplace_back(id, *a);
        }

        return ret;
    }

    void saveAnimations() {
        for (auto&[id, anim]: animations) {
            ofstream outfile(("data/animation/"_str + id + ".json").data());
            json dat;
            dat["path"] = anim->path;
            dat["frame_height"] = anim->height();
            dat["frame_duration"] = anim->duration();
            dat["type"] = cast(anim->type(), u32);
            outfile << dat.dump(4);
            outfile.close();
        }
    }

    bool Register(const str& path, const str& id, const double duration, const i32 height, const animation_type typ) {
        for (char& c: id) {
            if (!std::isalnum(c) && c != '_') {
                return false;
            }
        }
        if (id.empty()) return false;
        if (animations.contains(id)) return false;
        animations[id] = make_shared<animation>(animation(("resources/" + path).data(), height, duration, typ));
        animations[id]->id = id;
        animations[id]->path = path;
        return true;
    }

    void reload() {
        for (auto &anim: animations | views::values) {
            anim->texture = Allocator::getTexture("resources/"_str + anim->path);
        }
    }

    static AnimationRegistry& Instance() {
        static AnimationRegistry instance;
        return instance;
    }

    shared_ptr<animation> get(const str& id) {
        if (!animations.contains(id)) return animations["default"];
        return animations[id];
    }

    shared_ptr<animation> operator[](const str& id) {
        return get(id);
    }
};


inline void Allocator::reloadTextures(bool delete_previous, bool &finished)  {
    if (delete_previous) {
        for (auto &text: instance().textures | views::values) {
            UnloadTexture(text);
        }
        instance().textures.clear();
    }
    for (const auto& entry: filesystem::recursive_directory_iterator("resources")) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            instance().IallocateTexture(entry.path().string().data());
        }
    }
    AnimationRegistry::Instance().reload();
    finished = true;
}



#endif
