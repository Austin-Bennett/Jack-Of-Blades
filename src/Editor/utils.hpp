#ifndef LUTILS_HPP
#define LUTILS_HPP
#include <AustinUtils.hpp>
#include "../game/lib/globals.hpp"
#include "../game/lib/utils.hpp"

#include "../imgui-1.91.9b/imgui.h"
#include "../imgui-1.91.9b/imgui_internal.h"

using namespace AustinUtils;


inline str OpenFileDialog(const vector<const char*> &patterns, const char* title, const char* description, const char* default_file_path = "") {//returns a file path
    char file_name[512];
    const char *filePath = tinyfd_openFileDialog(title, default_file_path, 1, patterns.data(),
        description, false);

    if (filePath) {
        strncpy(file_name, filePath, sizeof(file_name) - 1);
    } else {
        file_name[0] = '\0';
    }

    return file_name;
}

inline void OpenFileDialog
(char* buffer, usize buffer_size, const vector<const char*> &patterns,
    const char* title, const char* description, const char* default_file_path = "") {//returns a file path
    const char *filePath = tinyfd_openFileDialog(title, default_file_path, 1, patterns.data(),
                                                 description, false);

    if (filePath) {
        strncpy(buffer, filePath, buffer_size);
    } else {
        buffer[0] = '\0';
    }
}

template<Arithmetic T>
v2<T> screenToWorld(v2<T> v) {
    return (v-win_pos.convert_data<T>())/win_scale;
}

template<Arithmetic T>
v2<T> worldToScreen(v2<T> v) {
    return win_pos.convert_data<T>() + v * win_scale;
}

enum class severity {
    INFORMATION,
    WARNING,
    ERROR
};

inline int popupMessageGUI(const severity s, const char* message) {
    ImGui::SetNextWindowSize({214, 104}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({cast(GetScreenWidth(), float)/2.0f - 107.0f, cast(GetScreenHeight(), float)/2.0f - 52.0f}, ImGuiCond_Always);
    int ret = 2;//2 for nothing happened
    switch (s) {
        case severity::INFORMATION:
            ImGui::Begin("Information");
            break;
        case severity::WARNING:
            ImGui::Begin("Warning");
            break;
        case severity::ERROR:
            ImGui::Begin("Error");
            break;
        default:
            //just to make sure imgui begins
            ImGui::Begin("Message");
            break;
    }

    ImGui::PushTextWrapPos(ImGui::GetContentRegionMax().x);
    switch (s) {
        case severity::ERROR:
            ImGui::TextColored({1, 0, 0, 1}, "%s", message);
            break;
        case severity::WARNING:
            ImGui::TextColored({1, 1, 0, 1}, "%s", message);
            break;
        default:
            ImGui::TextColored({1, 1, 1, 1}, "%s", message);
            break;
    }
    ImGui::PopTextWrapPos();
    ImGui::NewLine();
    if (ImGui::Button("Okay")) {
        ret = 1;//1 for the okay button
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
        ret = 0;//otherwise 0
    }

    ImGui::End();

    return ret;
}



template<typename EType>
bool enumDropDown(EType* e, EType min, EType max, const char*(*toStr)(void*) = nullptr) {
    const char* items[max-min+1];
    size_t itemsp = 0;
    if (toStr) {
        for (EType i = min; i <= max; ++i) {
            items[itemsp] = toStr(&i);
            itemsp++;
        }
    } else {
        for (EType i = min; i <= max; ++i) {
            items[itemsp] = to_string(i).data();
            itemsp++;
        }
    }
    int index = static_cast<int>(*e);
    if (ImGui::Combo("##xx", &index, items, IM_ARRAYSIZE(items))) {
        *e = static_cast<EType>(index);
        return true;
    }
    return false;
}


optional<pair<str, Texture2D>> textureManager(bool* open, bool& reload_state) {
    ImGui::Begin("Texture Manager", open);

    static auto textures = Allocator::getAllTextures();

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    float half = contentSize.x / 2.0f;
    float space = ImGui::GetStyle().ItemSpacing.x;
    float windowWidth = ImGui::GetWindowWidth();

    if (reload_state) {
        textures = Allocator::getAllTextures();
        reload_state = false;
    }

    if (ImGui::Button("Reload Textures")) Allocator::reloadTextures(false, reload_state);
    if (ImGui::Button("Force Reload Textures")) Allocator::reloadTextures(true, reload_state);

    ImGui::BeginChild("Textures", ImVec2(half - space * 0.5f, 0), true);
    static char buf[512];
    ImGui::InputText("Search", buf, 512);
    str b = buf;
    b.toLowercase();
    static optional<pair<str, Texture2D>> selected;
    optional<pair<str, Texture2D>> ret{};


    for (auto&[pth, text]: textures) {
        if (buf[0] != '\0' && (pth.lowercase().find(b) == str::npos)) continue;
        rlImGuiImageSize(&text, text.width * ((windowWidth / 2.4) / (text.width+text.height)),
                         text.height * ((windowWidth / 2.4) / (text.width+text.height)));
        if (ImGui::IsItemHovered() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            selected = {pth, text};
        }


        ImGui::Text("Path: %s", pth.data());
        ImGui::Text("ID: %d", text.id);
        ImGui::Text("Size: [%d, %d]", text.width, text.height);

        ImGui::Separator();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Selected Texture", ImVec2(half - space * 0.5f, 0), true);
    if (selected.has_value()) {
        static float image_zoom = 2.0;
        ImGui::SliderFloat("Zoom", &image_zoom, 1, 15, "%.2f");
        if (ImGui::Button("Reset Zoom")) {
            image_zoom = 2.0;
        }
        rlImGuiImageSize(&selected->second, cast(selected->second.width, float) * image_zoom, cast(selected->second.height, float) * image_zoom);

        if (ImGui::Button("Select")) {
            ret = selected.value();
        }
    }

    ImGui::EndChild();

    ImGui::End();
    if (open && !(*open)) return {};
    return ret;
}


shared_ptr<animation> animationManager(double delta, bool* open = nullptr) {
    ImGui::Begin("Animation Manager", open);

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    float windowWidth = ImGui::GetWindowWidth();
    float half = contentSize.x / 2.0f;
    float space = ImGui::GetStyle().ItemSpacing.x;
    shared_ptr<animation> ret;
    static auto anims = AnimationRegistry::Instance().getAnimations();


    static optional<pair<str, animation>> selected;




    ImGui::BeginChild("Animations", ImVec2(half - space * 0.5f, 0), true);

    if (ImGui::Button("Reload")) anims = AnimationRegistry::Instance().getAnimations();
    static char buf[512];
    ImGui::InputText("Search", buf, 512);
    str b = buf;
    b.toLowercase();

    for (auto&[id, anim]: anims) {
        if (!b.empty() && id.lowercase().find(b) == str::npos) continue;

        ImGui::PushID(id.c_str()); // Unique ID for this block



        ImGui::TextColored({1, 0, 1, 1}, id.data());
        ImGui::Separator();

        // Capture the starting Y position
        ImVec2 block_start = ImGui::GetCursorScreenPos();

        auto t = anim.getTexture();
        float w = cast(anim.width(), float) * (windowWidth / 128);
        float h = cast(anim.height(), float) * (windowWidth / 128);
        rlImGuiImageRect(&t, w, h, anim.getFrameRect());

        anim.update(delta);
        if (anim.isFinished()) anim.reset();

        ImGui::Text("Frame: %d", anim.getCurrentFramePos());
        ImGui::Text("Size: [%d, %d]", anim.width(), anim.height());

        // Capture the ending Y position
        ImVec2 block_end = ImGui::GetCursorScreenPos();

        // Handle selection using the rect we just captured
        ImVec2 min = block_start;
        ImVec2 max = {block_start.x + ImGui::GetContentRegionAvail().x, block_end.y};

        ImDrawList* draw = ImGui::GetWindowDrawList();

        // Detect click inside region
        ImVec2 mouse = ImGui::GetMousePos();
        bool hovered = mouse.x >= min.x && mouse.x <= max.x && mouse.y >= min.y && mouse.y <= max.y;

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            selected = {id, anim};
        }

        // Optional: Draw outline if hovered or selected
        ImU32 col = 0;
        if (hovered) col = IM_COL32(255, 255, 0, 128);
        if (selected && selected->first == id) col = IM_COL32(255, 0, 255, 255);
        if (col) draw->AddRect(min, max, col, 0, 0, 2.0f);

        ImGui::Dummy({0, 8}); // Spacing between entries

        ImGui::PopID();
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Selected Animation", ImVec2(half - space * 0.5f, 0), true);
    if (selected.has_value()) {
        auto&[id, anim] = selected.value();
        const Texture t = anim.getTexture();
        static bool autoUpdate = true;

        static char ID_IP[512];
        memcpy(ID_IP, anim.getId().data(), 512);

        if (ImGui::InputText("ID", ID_IP, 512)) {
            anim.getId() = ID_IP;
        }

        ImGui::Checkbox("Auto Update", &autoUpdate);
        if (!autoUpdate) {
            static i32 x;
            x = anim.getCurrentFramePos();
            ImGui::SliderInt("Frame", &x, 0, anim.getMaxFrame());
            anim.setFramePos(x);
        }
        ImGui::Separator();

        static float anim_zoom = 2.0;
        ImGui::SliderFloat("Zoom", &anim_zoom, 1, 15, "%.2f");
        if (ImGui::Button("Reset Zoom")) {
            anim_zoom = 2.0;
        }
        ImGui::Separator();
        rlImGuiImageRect(&t, cast(anim.width(), float) * anim_zoom, cast(anim.height(), float) * anim_zoom,
                         anim.getFrameRect());
        if (autoUpdate) anim.update(delta);

        ImGui::Separator();
        bool was_texture_changed = false;
        if (ImGui::Button("Change Texture")) {
            was_texture_changed = true;
            str rawpath = OpenFileDialog({"*.png"}, "Images", "png files", "resources/");
            if (!rawpath.empty()) {
                string out = filesystem::relative(rawpath.data(), "resources").string();
                for (auto& c: out) {
                    if (c == '\\') c = '/';
                }

                anim = animation(("resources/"_str + out).data(), anim.height(), anim.duration(), anim.type());
                anim.getId() = id;
                anim.Path() = out;
            } else was_texture_changed = false;
        }

        ImGui::InputInt("Frame Height", &anim.height(), 1, 5);
        ImGui::InputDouble("Frame Duration", &anim.duration(), 0.01, 0.1);
        enumDropDown<u32>(reinterpret_cast<u32 *>(&anim.type()), 0, 2, animation_typeToString);
        if (ImGui::Button("Reset")) {
            anim.reset();
        }

        if (!was_texture_changed) {
            AnimationRegistry::Instance().animations[id]->height() = anim.height();
            AnimationRegistry::Instance().animations[id]->duration() = anim.duration();
            AnimationRegistry::Instance().animations[id]->type() = anim.type();
        } else {
            AnimationRegistry::Instance().animations[id] = make_shared<animation>(anim);
        }
        ImGui::Separator();
        if (ImGui::Button("Select")) {
            ret = AnimationRegistry::Instance().get(id);
        }
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));         // Normal color
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(8.0f, 0, 0, 1.0f));  // Hover color
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));   // Active (clicked) color

        if (ImGui::Button("Delete")) {
            AnimationRegistry::Instance().animations.erase(id);
            filesystem::remove(("data/animation/"_str + id + ".json").data());
            selected.reset();
        }

        ImGui::PopStyleColor(3);
    }

    ImGui::EndChild();

    ImGui::End();

    if (open && !(*open)) return nullptr;
    return ret;
}

#endif