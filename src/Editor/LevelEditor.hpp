#ifndef LEVELEDITOR_HPP
#define LEVELEDITOR_HPP

#include "../game.hpp"
#include "tinyfiledialogs.h"
#include "utils.hpp"
#include "../imgui-1.91.9b/imgui_internal.h"

struct GUI_Flags {
    bool color_mode = false;//default dark

    bool main_window = true;
    bool show_fps_extra = false;

    bool settings_window = false;
    bool properties_window = false;
    bool textureWindow = false;
    bool animationWindow = false;
    bool createAnimation = false;
    bool createObject = false;
};

struct editor_settings {
    float drag_sensitivity = 1.0;
};

enum class mouseInputMode : u32 {
    NONE,
    DRAG,
    MOVE,
    SELECT,
    SCALE,
};

struct selection {
    rect selection_rect = {};
    bool visible = false;
    vector<shared_ptr<LevelObject>> selected_objects = {};

    void expand(const shared_ptr<LevelObject>& obj) {
        const rect r = obj->getCollision();
        selected_objects.push_back(obj);
        if (std::isinf(selection_rect.x) || std::isinf(selection_rect.y)) {
            selection_rect = r;
            return;
        }

        const double x = min(r.x, selection_rect.x);
        const double y = min(r.y, selection_rect.y);
        const double w = max(r.x + r.w - x, selection_rect.x + selection_rect.w - x);
        const double h = max(r.y + r.h - y, selection_rect.y + selection_rect.h - y);

        selection_rect.x = x;
        selection_rect.y = y;
        selection_rect.w = w;
        selection_rect.h = h;
    }


    void move(const dvec2 amt) {
        selection_rect += amt;
        for (auto& obj: selected_objects) {
            obj->collision += amt;
        }
    }

    void set_pos(const dvec2 p) {
        for (const auto& obj: selected_objects) {
            const auto [x, y] = obj->collision.pos() - selection_rect.pos();
            obj->collision.x = p.x+x;
            obj->collision.y = p.y+y;
        }

        selection_rect.x = p.x;
        selection_rect.y = p.y;
    }
};

class LevelEditor {
public:
    Game game{};
    char file_name[512]{};
    bool running = false;
    GUI_Flags Gsettings{};
    editor_settings ESettings{};
    fvec2 last_mouse_pos{};
    struct {
        bool show{};
        dvec2 pos{};
    } level_visual_point{};

    struct {
        str msg = "";
        double duration = 0.0;

        void reset(const str &msg) {
            this->msg = msg;
            duration = 5.0;
        }

        void reset(const str &msg, const double custom_duration) {
            this->msg = msg;
            duration = custom_duration;
        }

        void update(double delta) {
            if (duration > 0) duration -= delta;

        }

        void draw() const {
            if (duration > 0.0) {
                if (duration <= 1) {
                    DrawText(msg.data(), 20, GetScreenHeight()-30, 20, Fade(WHITE, cast(duration, float)));
                    return;
                }
                if (duration > 4.75) {
                    DrawText(msg.data(), 20,
                             cast(GetScreenHeight(), double) - 30 * sqrt(1 - clamp((duration - 4.75) * 4, 0.0, 1.0)), 20,
                             Fade(WHITE, cast(duration, float)));
                    return;
                }
                DrawText(msg.data(), 20, GetScreenHeight()-30, 20, WHITE);
            }
        }
    } message_display{};
    bool* main_running{};

    selection obj_selection{};
    shared_ptr<LevelObject> edited_object;
    optional<pair<str, Texture2D>> texture_manager_ret{};//reset after consuming
    shared_ptr<animation> animation_manager_ret{};//reset after consuming

    fvec2 initial_mouse_pos{};
    mouseInputMode mouse_mode = mouseInputMode::NONE;
    bool trying_to_quit = false;

    bool texture_reload_state = false;

    double updateTime{};
    double drawTime{};
    double uiDrawTime{};
    high_resolution_clock::time_point start;

    shared_ptr<LevelObject> copied_object;



    explicit LevelEditor(bool* r) : game(true), file_name{}, main_running(r) {
        memcpy(file_name, "None", 5);
        //read in the editor settings
        ifstream infile("editor.json");
        if (!infile.is_open()) {
            ofstream out("editor.json");
            out.close();
            return;
        }
        json data;
        infile >> data;
        infile.close();

        try {
            assertJsonData(data, "settings", json::value_t::object);
            data = data["settings"];

            assertJsonData(data, "mouse_sensitivity", JSON_NUMBERS);
            ESettings.drag_sensitivity = data["mouse_sensitivity"].get<float>();

            assertJsonData(data, "color_theme", json::value_t::boolean);
            Gsettings.color_mode = data["color_theme"].get<bool>();

            assertJsonData(data, "last_file", json::value_t::string);
            auto s = data["last_file"].get<string>();
            memcpy(file_name, s.data(), std::min(s.size()+1/*for null char*/, cast(512, usize)));
            game.change_level(file_name);

            assertJsonData(data, "last_level_scroll", json::value_t::array);
            auto p = data["last_level_scroll"].get<vector<double>>();
            if (p.size() != 2) throw Exception();
            if (game.current_level) game.current_level->Scroll(p[0], p[1]);
        } catch (AustinUtils::Exception& e) {
            ESettings = editor_settings();
        }
        if (Gsettings.color_mode) {
            ImGui::StyleColorsLight();
        } else {
            ImGui::StyleColorsDark();
        }
    }



    void levelEditorQuit() {
        trying_to_quit = true;
    }

    ~LevelEditor() {
        json data;
        data["settings"] = json();

        data["settings"]["mouse_sensitivity"] = ESettings.drag_sensitivity;
        data["settings"]["color_theme"] = static_cast<bool>(Gsettings.color_mode);
        if (game.current_level) data["settings"]["last_file"] = file_name;
        else data["settings"]["last_file"] = "";

        if (game.current_level) data["settings"]["last_level_scroll"] = vector{game.current_level->Scroll().x, game.current_level->Scroll().y};
        else data["settings"]["last_level_scroll"] = vector{0.0, 0.0};

        ofstream out("editor.json");
        out << data.dump(4);
        out.close();
    }

    void UIOptionsMenu() {
        ImGui::Begin("Options", &Gsettings.settings_window);
        if (ImGui::CollapsingHeader("Mouse input")) {
            ImGui::SliderFloat("Sensitivity", &ESettings.drag_sensitivity, 0, 2);
        }
        if (ImGui::CollapsingHeader("Editor")) {
            if (ImGui::Button(Gsettings.color_mode ? "Light Mode":"Dark Mode")) {
                Gsettings.color_mode = !Gsettings.color_mode;
                if (Gsettings.color_mode) {
                    ImGui::StyleColorsLight();
                } else {
                    ImGui::StyleColorsDark();
                }
            }
        }
        ImGui::End();
    }

    void UICreateObjectMenu() {
        ImGui::Begin("Create Object", &Gsettings.createObject);

        static auto registryIDs = vector<str>(LevelObjectRegistry::instance().IDS.begin(), LevelObjectRegistry::instance().IDS.end());

        static usize registry_name_index = 0;

        if (ImGui::BeginCombo("Type", registryIDs[registry_name_index].data())) {
            for (usize i = 0; i < registryIDs.size(); ++i) {
                bool is_selected = (registry_name_index == i);
                if (ImGui::Selectable(registryIDs[i].data(), is_selected)) {
                    registry_name_index = i;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Create") && game.current_level) {
            edited_object = game.current_level->createObject(registryIDs[registry_name_index]);
            Gsettings.properties_window = true;
            Gsettings.createObject = false;
        }

        ImGui::End();
    }

    void UICreateAnimationWindow() {
        ImGui::Begin("Create Animation", &Gsettings.createAnimation);

        static optional<pair<str, Texture2D>> texture{};
        static bool wantsTexture = false;
        static char id[256];

        static animation temp = animation();

        ImGui::InputText("ID", id, 256);

        if (ImGui::Button("Select Texture")) {
            wantsTexture = true;
            Gsettings.textureWindow = true;
        }

        if (!Gsettings.textureWindow) {
            wantsTexture = false;
            texture_manager_ret.reset();
        } else if (texture_manager_ret.has_value() && wantsTexture) {
            wantsTexture = false;
            texture = move(texture_manager_ret);
            texture_manager_ret.reset();
            Gsettings.textureWindow = false;
            temp.getTexture() = texture->second;
            temp.duration() = 1.0;
            temp.height() = 32;
            temp.type() = animation_type::LOOP;
            temp.setMaxFrame(texture->second.height/temp.height());
        }



        if (texture.has_value()) {
            float scale = (ImGui::GetWindowWidth()/cast(texture->second.width, float))/4;
            static bool autoUpdate = true;
            ImGui::Checkbox("Auto Update", &autoUpdate);
            if (!autoUpdate) {
                int frame = temp.getCurrentFramePos();
                ImGui::SliderInt("frame", &frame, 0, temp.getMaxFrame());
                temp.setFramePos(frame);
            }
            ImGui::Separator();
            rlImGuiImageRect(&temp.getTexture(), temp.width()*scale, temp.height()*scale, temp.getFrameRect());
            temp.update(game.delta);
            ImGui::Separator();
            if (ImGui::Button("Reset")) {
                temp.reset();
            }

            ImGui::InputInt("Frame Height", &temp.height(), 1, 5);
            ImGui::InputDouble("Frame Duration", &temp.duration());
            enumDropDown<u32>(reinterpret_cast<u32*>(&temp.type()), 0, 2, animation_typeToString);
            ImGui::Separator();

            if (ImGui::Button("Create")) {
                texture->first = filesystem::relative(texture->first.data(), "resources").generic_string();
                for (auto& c: texture->first) {
                    if (c == '\\') c = '/';
                }
                if (AnimationRegistry::Instance().Register(texture->first, id, temp.duration(), temp.height(), temp.type())) {
                    message_display.reset("Created texture successfully");
                    temp = animation();
                    autoUpdate = true;
                    wantsTexture = false;
                    id[0] = '\0';
                    texture.reset();
                    Gsettings.createAnimation = false;
                } else {
                    message_display.reset("Invalid animation ID");
                }
            }
        }

        ImGui::End();
    }

    void UILevelMain()  {

        static double x;
        static double y;

        if (!Gsettings.main_window) return;
        ImGui::Begin("Editor menu", &Gsettings.main_window, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open")) {
                    OpenFileDialog(file_name, 512, {"*.json"}, "Open level json", "json files", "data/");
                    if (file_name[0] != '\0') {
                        game.change_level(file_name);
                    }
                }
                if (ImGui::MenuItem("Save")) {
                    saveLevel();
                }
                if (ImGui::MenuItem("Close")) {
                    edited_object = nullptr;
                    obj_selection = {};

                    game.change_level("");
                }
                if (ImGui::MenuItem("Options")) {
                    Gsettings.settings_window = !Gsettings.settings_window;
                }
                if (ImGui::MenuItem("Exit")) {
                    trying_to_quit = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Create Animation")) {
                    Gsettings.createAnimation = true;
                }
                if (ImGui::MenuItem("Create Object")) {
                    Gsettings.createObject = true;
                }
                if (ImGui::MenuItem("Edit Selected Object")) {
                    if (obj_selection.selected_objects.empty()) {
                        message_display.reset("No Selected Object!");
                    } else {
                        edited_object = obj_selection.selected_objects[0];
                        obj_selection.visible = false;
                        obj_selection.selected_objects.clear();
                        Gsettings.properties_window = true;
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {

                if (ImGui::MenuItem("Textures")) {
                    Gsettings.textureWindow = !Gsettings.textureWindow;
                }
                if (ImGui::MenuItem("Animations")) {
                    Gsettings.animationWindow = !Gsettings.animationWindow;
                }
                if (ImGui::MenuItem("Properties")) {
                    Gsettings.properties_window = !Gsettings.properties_window;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }


        if (ImGui::CollapsingHeader("Info")) {
            ImGui::PushTextWrapPos(ImGui::GetContentRegionMax().x);

            ImGui::TextColored({1, 0.7, 1, 1}, "Framerate");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1, 0, 1, 1), "Frame Time (ms): %.2f", game.delta * 1000);
            ImGui::TextColored(ImVec4(1, 0, 1, 1), "FPS: %.0f", game.smoothed_fps);


            ImGui::Checkbox("Show FPS Statistics", &Gsettings.show_fps_extra);
            if (Gsettings.show_fps_extra) {
                ImGui::TextColored(ImVec4(1, 0, 1, 1), "Update method time (ms): %.2f", updateTime*1000);
                ImGui::TextColored(ImVec4(1, 0, 1, 1), "Draw method time (ms): %.2f", drawTime*1000);
                ImGui::TextColored(ImVec4(1, 0, 1, 1), "DrawUI method time (ms): %.2f: ", uiDrawTime*1000);
                ImGui::TextColored(ImVec4(1, 0, 1, 1), "Average FPS: %.2f", game.average_fps);
                ImGui::TextColored(ImVec4(1, 0, 1, 1), "1%% High: %.0f", game.high_1fps);
                ImGui::TextColored(ImVec4(1, 0, 1, 1), "1%% Low: %.0f", game.low_1fps);
            }

            ImGui::NewLine();
            ImGui::TextColored({1, 0.7, 1, 1}, "Level");
            ImGui::Separator();
            if (game.current_level) {
                ImGui::TextColored(ImVec4(0, 1, 0.1, 1), "Current level: %s",
                                                       game.current_level->getName().c_str());

                ImGui::TextColored(ImVec4(0, 0.7, 0.5, 1), "Current Level Scroll: <%.0f, %.0f>",
                                                       EXPAND_V(game.current_level->Scroll()));



                ImGui::PushItemWidth(ImGui::GetContentRegionMax().x/2.4f);
                ImGui::InputDouble("x", &x, 1, 5, "%.2f");
                ImGui::SameLine();
                ImGui::InputDouble("y", &y, 1, 5, "%.2f");
                if (ImGui::Button("Set Scroll")) {
                    game.current_level->Scroll(x, y);
                }
                ImGui::PopItemWidth();
            } else ImGui::TextColored(ImVec4(0, 1, 0.1, 1), "Current level: None");
            ImGui::NewLine();
            ImGui::TextColored({1, 0.7, 1, 1}, "Editor");
            ImGui::Separator();

            ImGui::SliderFloat("Zoom", &zoom, 1, 10, "%.2f");
            if (ImGui::Button("Reset Zoom")) {
                zoom = 4;
                message_display.reset("Reset Zoom");
            }

            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Mouse Screen Pos: <%.0f, %.0f>", EXPAND_V(GetMousePosition()));
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Mouse Local Pos: <%.0f, %.0f>", EXPAND_V(getMousePos()));



            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Selection box: {%.1f, %.1f, %.1f, %.1f}",
                               EXPAND_R(obj_selection.selection_rect));
            ImGui::PopTextWrapPos();
        }


        if (ImGui::CollapsingHeader("Visualization")) {
            ImGui::PushItemWidth(ImGui::GetContentRegionMax().x/2.4f);
            ImGui::InputDouble("x##xx", &level_visual_point.pos.x, 1, 5, "%.2f");
            ImGui::SameLine();
            ImGui::InputDouble("y##xx", &level_visual_point.pos.y, 1, 5, "%.2f");

            ImGui::Checkbox("Show point", &level_visual_point.show);
            ImGui::PopItemWidth();

            static bool b{};
            ImGui::Checkbox("Show Mouse Positions", &b);
            if (b) {
                DrawCircle(EXPAND_V(GetMousePosition()), 5, RED);
                DrawCircle(EXPAND_V(getMousePos()), 5, BLUE);
            }
        }
        ImGui::End();
    }

    void UIProperties() {
        ImGui::Begin("Properties", &Gsettings.properties_window);
        static float color[3];
        if (!edited_object) {
            ImGui::End();
            return;
        }
        auto[name, parameters] = edited_object->getParameters();
        ImGui::TextColored({0, 1, 1, 1}, "%s", name.data());
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 5);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));         // Normal color
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(8.0f, 0, 0, 1.0f));  // Hover color
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));   // Active (clicked) color

        if (ImGui::Button("Delete")) {
            game.current_level->forceDestroyObject(edited_object);
            edited_object = nullptr;
            Gsettings.properties_window = false;
            ImGui::PopStyleColor(3);
            ImGui::End();

            return;
        }

        ImGui::PopStyleColor(3);

        for (auto& i: parameters) {
            ImGui::PushID(i.name.data());
            ImGui::TextColored({1, 0, 0, 1}, i.name.data());
            ImGui::Separator();
            if (i.typ == OPType::RECT) {
                const auto r = cast(i.data, rect*);
                ImGui::PushItemWidth(ImGui::GetContentRegionMax().x/4.75f);

                ImGui::InputDouble("X", &r->x, 1, 5, "%.2f");
                ImGui::SameLine();

                ImGui::InputDouble("Y", &r->y, 1, 5, "%.2f");
                ImGui::SameLine();

                ImGui::InputDouble("W", &r->w, 1, 5, "%.2f");
                ImGui::SameLine();

                ImGui::InputDouble("H", &r->h, 1, 5, "%.2f");
                ImGui::SameLine();

                ImGui::PopItemWidth();
                ImGui::NewLine();
                static double rounding = 1;
                if (ImGui::Button("Round")) {
                    r->x = round(r->x, rounding);
                    r->y = round(r->y, rounding);
                    r->w = round(r->w, rounding);
                    r->h = round(r->h, rounding);
                }
                ImGui::SameLine();
                ImGui::InputDouble("Rounding", &rounding, 1, 5, "%.2f");

            }
            else if (i.typ == OPType::TEXTURE) {
                auto t = cast(i.data, Texture2D*);
                rlImGuiImage(t);
                static bool want_texture = false;
                if (ImGui::Button("Select Texture")) {
                    want_texture = true;
                    Gsettings.textureWindow = true;
                }
                if (!Gsettings.textureWindow) {
                    want_texture = false;
                    texture_manager_ret.reset();
                }
                if (want_texture && texture_manager_ret.has_value()) {
                    if (texture_manager_ret->second.format == 0) {
                        want_texture = false;
                        texture_manager_ret.reset();
                    } else {
                        *t = texture_manager_ret->second;
                        want_texture = false;
                        Gsettings.textureWindow = false;
                        texture_manager_ret.reset();
                    }
                }
            }
            else if (i.typ == OPType::ANIMATION) {
                auto a = cast(i.data, shared_ptr<animation>*);
                auto t = (*a)->getTexture();
                rlImGuiImageRect(&t, (*a)->width(), (*a)->height(), (*a)->getFrameRect());
                static bool wantAnimtion = false;
                if (ImGui::Button("Select Animation")) {
                    wantAnimtion = true;
                    Gsettings.animationWindow = true;
                }
                if (!Gsettings.animationWindow) {
                    wantAnimtion = false;
                    animation_manager_ret.reset();
                }
                if (wantAnimtion && animation_manager_ret != nullptr) {
                    *a = animation_manager_ret;
                    wantAnimtion = false;
                    Gsettings.animationWindow = false;
                    animation_manager_ret.reset();
                }
            }
            else if (i.typ == OPType::COLOR) {
                Color* c = cast(i.data, Color *);
                color[0] = cast(c->r, float)/255.0f;
                color[1] = cast(c->g, float)/255.0f;
                color[2] = cast(c->b, float)/255.0f;
                ImGui::ColorPicker3("##xx", color);

                c->r = cast(color[0]*255, u8);
                c->g = cast(color[1]*255, u8);
                c->b = cast(color[2]*255, u8);
            }
            else if (i.typ == OPType::BOOLEAN) {
                ImGui::Checkbox("##xx", static_cast<bool*>(i.data));
            }
            else if (i.typ == OPType::UNSIGNED_INTEGER8) {
                u8 min = cast(i.min, u8);
                u8 max = cast(i.max, u8);
                if (i.is_enum) {
                    enumDropDown<u32>(static_cast<u32*>(i.data), min, max, i.enumToString);
                } else if (!std::isinf(i.max) && !std::isinf(i.min)) {
                    ImGui::SliderScalar("##xx", ImGuiDataType_U8, i.data, &min, &max);
                } else {
                    u32 small = 1, fast = 5;
                    u32 v = *static_cast<u32*>(i.data);
                    ImGui::InputScalar("##xx", ImGuiDataType_U32, &v, &small, &fast);
                    if (!std::isinf(i.min)) {
                        if (v < min) v = min;
                    } else if (!std::isinf(i.max)) {
                        if (v > max) v = max;
                    }
                    *static_cast<u32*>(i.data) = v;
                }
            }
            else if (i.typ == OPType::UNSIGNED_INTEGER32) {
                u32 min = cast(i.min, u32);
                u32 max = cast(i.max, u32);
                if (i.is_enum) {
                    enumDropDown<u32>(static_cast<u32*>(i.data), min, max, i.enumToString);
                } else if (!std::isinf(i.max) && !std::isinf(i.min)) {
                    ImGui::SliderScalar("##xx", ImGuiDataType_U32, i.data, &min, &max);
                } else {
                    u32 small = 1, fast = 5;
                    u32 v = *static_cast<u32*>(i.data);
                    ImGui::InputScalar("##xx", ImGuiDataType_U32, &v, &small, &fast);
                    if (!std::isinf(i.min)) {
                        if (v < min) v = min;
                    } else if (!std::isinf(i.max)) {
                        if (v > max) v = max;
                    }
                    *static_cast<u32*>(i.data) = v;
                }
            }
            else if (i.typ == OPType::FLOAT32) {
                float min = cast(i.min, float);
                float max = cast(i.max, float);

                if (!std::isinf(i.max) && !std::isinf(i.min)) {
                    ImGui::SliderScalar("##xx", ImGuiDataType_Float, i.data, &min, &max);
                } else {
                    float small = 1, fast = 5;
                    float v = *static_cast<float*>(i.data);
                    ImGui::InputScalar("##xx", ImGuiDataType_Float, &v, &small, &fast);
                    if (!std::isinf(i.min)) {
                        if (v < min) v = min;
                    } else if (!std::isinf(i.max)) {
                        if (v > max) v = max;
                    }
                    *static_cast<float*>(i.data) = v;
                }
            }
            ImGui::PopID();
        }
        ImGui::End();
    }

    rect getScaleRect() const {
        const float side_length = cast(min(min(edited_object->collision.w, edited_object->collision.h)/2, 15), float);
        const float x = cast(edited_object->collision.x + edited_object->collision.w, float)-game.current_level->scroll.x;
        const float y = cast(edited_object->collision.y + edited_object->collision.h, float)-game.current_level->scroll.y;
        return rect{x-side_length, y-side_length, side_length, side_length};
    }

    void draw() {
        if (Gsettings.show_fps_extra) start = high_resolution_clock::now();
        game.draw();

        if (obj_selection.visible && !running) {
            for (const auto& o: obj_selection.selected_objects) {
                o->getCollision().draw(game.current_level->Scroll(),
                                       Fade(SKYBLUE, 0.7), 2);
            }
            obj_selection.selection_rect.draw(game.current_level->Scroll(), UV_COLOR, 1);

        }
        if (level_visual_point.show && game.current_level) DrawCircle(cast(level_visual_point.pos.x - game.current_level->scroll.x, int)
            , cast(level_visual_point.pos.y - game.current_level->scroll.y, int)
            , clamp(5/zoom, 1, 5), RED);

        if (edited_object && !running) {
            edited_object->collision.draw(game.current_level->Scroll(), RED, 1);
            const float side_length = cast(min(min(edited_object->collision.w, edited_object->collision.h)/2, 15), float);
            const float x = cast(edited_object->collision.x + edited_object->collision.w, float)-game.current_level->scroll.x;
            const float y = cast(edited_object->collision.y + edited_object->collision.h, float)-game.current_level->scroll.y;
            if (rect{x-side_length, y-side_length, side_length, side_length} && getMousePos()) {
                DrawTriangle({x, y-side_length}, {x-side_length, y}, {x, y}, {255, 0,0, 255});
            } else
            DrawTriangle({x, y-side_length}, {x-side_length, y}, {x, y}, RED);
        }

        if (Gsettings.show_fps_extra) drawTime = cast(
                                          duration_cast<nanoseconds>((high_resolution_clock::now() - start)).count(),
                                          double) / 1e9;
    }

    void drawUI() {
        if (Gsettings.show_fps_extra) start = high_resolution_clock::now();
        if (!running) {
            UILevelMain();
            if (Gsettings.settings_window) UIOptionsMenu();
            if (Gsettings.properties_window) UIProperties();
            if (Gsettings.textureWindow) {

                texture_manager_ret = textureManager(&Gsettings.textureWindow, texture_reload_state);
            }
            if (Gsettings.animationWindow) animation_manager_ret = animationManager(
                                               game.delta, &Gsettings.animationWindow);
            if (Gsettings.createAnimation) UICreateAnimationWindow();
            if (Gsettings.createObject) UICreateObjectMenu();
        }
        message_display.draw();
        if (mouse_mode == mouseInputMode::SELECT) {
            const rect r = {ivec2{GetMouseX(), GetMouseY()}, initial_mouse_pos};
            r.draw({}, WHITE, 2);
        }
        if (trying_to_quit) {
            if (const int ret = popupMessageGUI(severity::WARNING, "Changes may not be saved!"); ret == 1) *main_running = false;
            else if (ret == 0) trying_to_quit = false;
        }
        if (Gsettings.show_fps_extra) uiDrawTime = cast(
                                          duration_cast<nanoseconds>((high_resolution_clock::now() - start)).count(),
                                          double) / 1e9;
    }

    void drawLighting() {
        if (game.current_level) game.current_level->drawLighting({});
    }

    void update(double delta) {
        if (Gsettings.show_fps_extra) start = high_resolution_clock::now();
        if (trying_to_quit) return;
        game.delta = delta;
        game.runtime += delta;
        game.update_fps(delta);
        if (running) {
            game.update(delta);
        } else {
            if (!ImGui::IsAnyItemActive()) {

                if (IsKeyReleased(KEY_N)) {
                    Gsettings.properties_window = !Gsettings.properties_window;
                }

                if (mouse_mode == mouseInputMode::MOVE && obj_selection.visible  && !ImGui::GetIO().WantCaptureMouse) {//moving
                    dvec2 mdelta = (getMousePos()-last_mouse_pos).convert_data<double>();



                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        obj_selection.set_pos(round(obj_selection.selection_rect.pos(), 5.0));
                        if (mdelta.x < 0) mdelta.x = -5;
                        if (mdelta.x > 0) mdelta.x = 5;
                        if (mdelta.y < 0) mdelta.y = -5;
                        if (mdelta.y > 0) mdelta.y = 5;
                        obj_selection.move(mdelta);
                    } else {
                        mdelta *= ESettings.drag_sensitivity;
                        obj_selection.move(mdelta);
                    }

                } else if (mouse_mode == mouseInputMode::SCALE && edited_object && !ImGui::GetIO().WantCaptureMouse) {//scaling
                    auto mpos = getMousePos().convert_data<double>();

                    edited_object->collision = {edited_object->collision.pos(), mpos+game.current_level->scroll};
                }

                game.editor_update(delta);
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_ALT)  && !ImGui::GetIO().WantCaptureMouse) {                                  //dragging
                    mouse_mode = mouseInputMode::DRAG;
                    if (game.current_level) {
                        fvec2 mdelta = (getMousePos()-last_mouse_pos) * zoom;
                        game.current_level->addScroll(-mdelta.x * ESettings.drag_sensitivity / zoom,
                            -mdelta.y * ESettings.drag_sensitivity / zoom);
                    }
                } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && game.current_level && !ImGui::GetIO().WantCaptureMouse) {//start moving/selecting/scaling
                    const dvec2 mpos = getMousePos().convert_data<double>();

                    if (edited_object && (getScaleRect() && getMousePos())) {
                        mouse_mode = mouseInputMode::SCALE;

                    } else if (obj_selection.visible && (obj_selection.selection_rect && mpos + game.current_level->Scroll())) {//moving
                        mouse_mode = mouseInputMode::MOVE;

                    } else if (IsKeyDown(KEY_LEFT_SHIFT)) {//selecting more
                        mouse_mode = mouseInputMode::SELECT;
                        obj_selection.visible = false;
                        initial_mouse_pos = tofvec(GetMousePosition());

                    } else {//selecting
                        mouse_mode = mouseInputMode::SELECT;
                        initial_mouse_pos = tofvec(GetMousePosition());
                        obj_selection.visible = false;
                        obj_selection.selected_objects.clear();
                        obj_selection.selection_rect = {};
                        obj_selection.selection_rect.x = 1.0/0.0;//positive infinity
                        obj_selection.selection_rect.y = 1.0/0.0;

                    }
                } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && game.current_level  && !ImGui::GetIO().WantCaptureMouse) {//stop moving/selecting/scaling


                    if (mouse_mode == mouseInputMode::SELECT) {
                        const dvec2 mpos = tofvec(GetMousePosition()).convert_data<double>();

                        if ((mpos - initial_mouse_pos).length2() < 0.1) {//didnt move
                            simple_hit hit = game.current_level->getTopObject(screenToWorld(mpos) + game.current_level->Scroll());

                            if (hit.hit) {
                                edited_object = nullptr;
                                obj_selection.visible = true;
                                obj_selection.expand(hit.obj);
                                message_display.reset("Selected 1 object");
                            } else {
                                obj_selection.visible = false;
                                edited_object = nullptr;
                            }
                        } else {//multi selection
                            rect r = {screenToWorld(initial_mouse_pos), screenToWorld(mpos)};
                            r += game.current_level->Scroll();
                            collision_hit hit = game.current_level->getAllObjects(r);

                            if (hit.hit) {
                                edited_object = nullptr;
                                obj_selection.visible = true;
                                for (auto& o: hit.objects) {
                                    obj_selection.expand(o);
                                }
                                message_display.reset("Selected "_str + obj_selection.selected_objects.size() + " object" +
                                    (obj_selection.selected_objects.size() > 1 ? "s":""));
                            } else {
                                obj_selection.visible = false;
                                edited_object = nullptr;
                            }
                        }
                    }
                    mouse_mode = mouseInputMode::NONE;
                } else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)  && !ImGui::GetIO().WantCaptureMouse) {
                    dvec2 mpos = getMousePos().convert_data<double>();
                    simple_hit hit = game.current_level->getTopObject(mpos + game.current_level->Scroll());

                    if (hit.hit) {
                        obj_selection.visible = false;
                        mouse_mode = mouseInputMode::NONE;

                        edited_object = hit.obj;
                        Gsettings.properties_window = true;
                    } else {
                        edited_object = nullptr;
                    }
                } else if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) || (IsKeyPressed(KEY_LEFT_CONTROL) && IsKeyDown(KEY_C))) {
                    if (!obj_selection.selected_objects.empty()) {
                        copied_object = obj_selection.selected_objects[0];
                        message_display.reset("Copied object");
                    }
                } else if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) || (IsKeyPressed(KEY_LEFT_CONTROL) && IsKeyDown(KEY_V))) {
                    if (copied_object && game.current_level) {
                        obj_selection.selected_objects.clear();
                        obj_selection.visible = true;
                        obj_selection.selection_rect = {};
                        obj_selection.selection_rect.x = 1.0/0.0;//positive infinity
                        obj_selection.selection_rect.y = 1.0/0.0;
                        game.current_level->addObject(
                            copied_object->copy(getMousePos().convert_data<double>() + game.current_level->Scroll()));
                        obj_selection.expand(game.current_level->objects.back());
                    }
                } else if (IsKeyDown(KEY_DELETE)) {
                    if (obj_selection.visible && !obj_selection.selected_objects.empty()) {
                        auto o = obj_selection.selected_objects.back();
                        obj_selection.selected_objects.clear();
                        obj_selection.visible = false;
                        game.current_level->forceDestroyObject(o);
                    }
                }

                if (!ImGui::GetIO().WantCaptureMouse) {
                    const float scroll = GetMouseWheelMove();
                    zoom += scroll/4;

                    if (zoom < 1) zoom = 1;
                }


                if (IsKeyDown(KEY_ESCAPE)) {
                    obj_selection.visible = false;
                    obj_selection.selected_objects.clear();
                    obj_selection.selection_rect = {};
                    edited_object = nullptr;
                }






                if (IsKeyReleased(KEY_F2)) {
                    Gsettings.main_window = !Gsettings.main_window;
                }
            } else if (obj_selection.visible || (mouse_mode == mouseInputMode::SELECT)) {
                obj_selection.visible = false;
                obj_selection.selected_objects.clear();
                obj_selection.selection_rect = {};
                mouse_mode = mouseInputMode::NONE;
            }
        }
        if (IsKeyReleased(KEY_F1)) {
            if (running) {
                running = false;
                game.current_level->reset();
            } else {
                message_display.reset("Starting Game...");
                zoom = 4;
                running = true;
                if (game.current_level) game.current_level->start();
            }
        }
        message_display.update(delta);
        last_mouse_pos = getMousePos();


        if (Gsettings.show_fps_extra) updateTime = cast(
                                          duration_cast<nanoseconds>((high_resolution_clock::now() - start)).count(),
                                          double) / 1e9;
    }


    void saveLevel() {
        if (!game.current_level) return;

        //output ALL objects to the level file
        /*
         * file structure:
         * file_name: str
         * objects: list[obj]
         */
        try {
            json out;
            out["name"] = game.current_level->getName();
            vector<json> objs;
            for (auto& obj: game.current_level->objects) {
                if (obj->isDynamic()) return;
                objs.push_back(LevelObjectRegistry::instance().toJsonFactories[obj->getRegistryID()](*obj));
                objs.back()["type"] = obj->getRegistryID();
            }
            out["objects"] = objs;

            ofstream outdat(file_name);
            outdat << out.dump(4);
            AnimationRegistry::Instance().saveAnimations();
            message_display.reset("Level saved successfully");
        } catch (const exception& e) {
            message_display.reset("Could not save level: "_str + e.what());
        }
    }



    [[nodiscard]] static fvec2 getMousePos() {
        const auto v = screenToWorld(fvec2{EXPAND_V(GetMousePosition())});

        return v;
    }
};



#endif