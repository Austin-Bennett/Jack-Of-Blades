#include "src/lib/UI.hpp"
#include "src/lib/lib.hpp"

JoB::rect creatRectangle() {
    f32 x = 0;
    JoB::rect ret = {0, 0, 0, 0};
    cout << "Enter rectangle width: ";
    cin >> x;
    ret.width = x;
    cout << "Enter rectangle height: ";
    cin >> x;
    ret.height = x;
    ret.x = (1280.0/2 - ret.width/2);
    ret.y = (720.0/2 - ret.height/2);

    return ret;
}

template<Arithmetic T, Arithmetic rt>
T round(T val, rt near) {
    if (near == 0.0) return val;
    return round(val/near) * near;
}

int main(void) {
    fvec2 res = JoB::base_resolution;
    res *= 2;

    InitWindow(res.x, res.y, "UI designer");
    HideCursor();
    vector<JoB::rect> rects;
    
    JoB::rect* selection = nullptr;
    JoB::rect* hovered = nullptr;

    JoB::Allocator alloc = JoB::Allocator();

    SetTargetFPS(144);
    JoB::formatter fmt = JoB::formatter();
    bool show_ui = true;
    JoB::UICanvas screen_ui = JoB::UICanvas({0, 0}, alloc);

    screen_ui.add(JoB::UIButton(
        {.85, .85},
        {128, 64},
        MOUSE_LEFT_BUTTON,
        JoB::animation(alloc.createTexture(RAYWHITE, 128, 64)),
        [&]() {
            rects.push_back(creatRectangle());
            cout << "Selected rect: " << rects.back() << "\n";
            selection = &rects.back();
            SetMousePosition(rects.back().x+rects.back().width, rects.back().y+rects.back().height);
        },
        GetFontDefault(),
        BLACK,
        "Create Rect"
    ));

    screen_ui.add(JoB::UIButton(
        {.85, .9444},
        {128, 32},
        MOUSE_LEFT_BUTTON,
        JoB::animation(alloc.createTexture(PURPLE, 128, 32)),
        [&]() {
            if (rects.size() == 0) return;
            rects.push_back({0, 0, rects.back().width, rects.back().height});
            selection = &rects.back();
            SetMousePosition(rects.back().x+rects.back().width, rects.back().y+rects.back().height);
        },
        GetFontDefault(),
        BLACK,
        "Create Last Rect"
    ));

    

    while (!WindowShouldClose()) {
        JoB::rect m_rect = {static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY()), 1, 1};

        if (show_ui) {
            screen_ui.update(0);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            for (auto& rec: rects) {
                if ((rec*2) && m_rect && &rec != selection) {
                    cout << "Selected rect: " << rec << "\n";
                    selection = &rec;
                    SetMousePosition(rec.x+rec.width, rec.y+rec.height);
                    SetWindowFocused();
                    break;
                } if ((rec*2) && m_rect && &rec == selection) {
                    selection = nullptr;
                }
            }
        }

        if (IsKeyPressed(KEY_DELETE) && selection != nullptr) {
            auto it = std::find_if(rects.begin(), rects.end(), [&](JoB::rect& r) {
                return &r == selection;
            });
            selection = nullptr;
            rects.erase(it);
        } 

        if (selection == nullptr) {
            for (auto& rec: rects) {
                if ((rec*2) && m_rect) {
                    hovered = &rec;
                    goto endif1;
                }
            }
            hovered = nullptr;
        }


        endif1:
        if (IsKeyReleased(KEY_F1)) {
            show_ui = !show_ui;
        }

        if (selection != nullptr) {
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                if (IsKeyPressed(KEY_LEFT_SHIFT)) {
                    SetMousePosition(round(selection->x+selection->width, 10),
                    round(selection->y+selection->height, 10)
                    );
                    m_rect = {static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY()), 1, 1};
                    
                }
                selection->x = round(m_rect.x, 10)-selection->width;
                selection->y = round(m_rect.y, 10)-selection->height;
                goto draw;
            }
            if (IsKeyReleased(KEY_LEFT_SHIFT)) {
                SetMousePosition(selection->x+selection->width, selection->y+selection->height);
                goto draw;
            }
            selection->x = (m_rect.x) - selection->width;
            selection->y = (m_rect.y) - selection->height;
        }
        
        draw:
        BeginDrawing();

        ClearBackground(BLACK);

        for (auto& rec: rects) {
            DrawRectangle(rec.x, rec.y, rec.width*2, rec.height*2, Fade(RED, 0.75f));
        }
    
        if (selection != nullptr) DrawRectangleLines(selection->x, selection->y, selection->width*2, selection->height*2, WHITE);

        if (show_ui) {
            //draw the stats
            if (selection != nullptr) fmt << "Selection Scr percent: " << fixed << setprecision(2) << (selection->x/res.x)*100 << "%" << ", " << (selection->y/res.y)*100 << "%";
            if (selection != nullptr) DrawText(fmt.str().c_str(), 10, 10, 20, WHITE);
            if (selection == nullptr) DrawText("Selection Scr %: 0%, 0%", 10, 10, 20, WHITE);

            if (selection != nullptr) fmt << "Selection Center Scr percent: " << fixed << setprecision(2) <<
            ((selection->x+selection->width)/res.x)*100 << "%" << ", " << ((selection->y+selection->height)/res.y)*100 << "%";
            if (selection != nullptr) DrawText(fmt.str().c_str(), 10, 35, 20, WHITE);
            if (selection == nullptr) DrawText("Selection Center Scr percent: 0%, 0%", 10, 35, 20, WHITE);

            if (selection != nullptr) fmt << "Selection: " << fixed << setprecision(2) << ((*selection) - fvec2{selection->x/2, selection->y/2});
            if (selection != nullptr) DrawText(fmt.str().c_str(), 10, 60, 20, WHITE);
            if (selection == nullptr) DrawText("Selection: null", 10, 60, 20, WHITE);
            
            if (selection != nullptr) fmt << "Selection Center: " << fixed << setprecision(2) << fvec2{(selection->x+selection->width)/2, (selection->y+selection->height)/2};
            if (selection != nullptr) DrawText(fmt.str().c_str(), 10, 85, 20, WHITE);
            if (selection == nullptr) DrawText("Selection Center: null", 10, 85, 20, WHITE);

            //for the hover
            if (hovered != nullptr) fmt << "hovered Scr percent: " << fixed << setprecision(2) << (hovered->x/res.x)*100 << "%" << ", " << (hovered->y/res.y)*100 << "%";
            if (hovered != nullptr) DrawText(fmt.str().c_str(), 10, 110, 20, WHITE);
            if (hovered == nullptr) DrawText("hovered Scr %: 0%, 0%", 10, 110, 20, WHITE);

            if (hovered != nullptr) fmt << "hovered Center Scr percent: " << fixed << setprecision(2) <<
            ((hovered->x+hovered->width)/res.x)*100 << "%" << ", " << ((hovered->y+hovered->height)/res.y)*100 << "%";
            if (hovered != nullptr) DrawText(fmt.str().c_str(), 10, 135, 20, WHITE);
            if (hovered == nullptr) DrawText("hovered Center Scr percent: 0%, 0%", 10, 135, 20, WHITE);

            if (hovered != nullptr) fmt << "hovered: " << fixed << setprecision(2) << ((*hovered) - fvec2{hovered->x/2, hovered->y/2});
            if (hovered != nullptr) DrawText(fmt.str().c_str(), 10, 160, 20, WHITE);
            if (hovered == nullptr) DrawText("hovered: null", 10, 160, 20, WHITE);
            
            if (hovered != nullptr) fmt << "hovered Center: " << fixed << setprecision(2) << fvec2{(hovered->x+hovered->width)/2, (hovered->y+hovered->height)/2};
            if (hovered != nullptr) DrawText(fmt.str().c_str(), 10, 185, 20, WHITE);
            if (hovered == nullptr) DrawText("hovered Center: null", 10, 185, 20, WHITE);
        
            screen_ui.draw();
        }

        DrawCircle(m_rect.x, m_rect.y, 3, BLUE);

        
        EndDrawing();

    }
}