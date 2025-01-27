#include "lib/UI.hpp"
#include "raylib.h"
#include "AustinUtils.hpp"
#include "lib/lib.hpp"
#include "lib/util.hpp"
#include "player.hpp"

using namespace std::chrono;

JoB::logger L_raylib = JoB::logger("raylib");

void raylibLogCallback(int logLevel, const char* text, va_list args) {
    try {
        //set buffer to prevent errors
        char buffer[1024];
        
        //safe print using vsnprintf
        vsnprintf(buffer, sizeof(buffer), text, args);

        //convert the raylib log into my own logging enum
        switch (logLevel) {
            case LOG_WARNING:
                L_raylib.clog(JoB::logger::WARN, buffer);
            case LOG_ERROR:
                L_raylib.clog(JoB::logger::ERROR, buffer);
            case LOG_DEBUG:
                L_raylib.clog(JoB::logger::DEBUG, buffer);
            default:
                L_raylib.clog(JoB::logger::INFO, buffer);
        }
    } catch (const std::exception& e) {
        L_raylib.log(JoB::logger::ERROR, "Error logging");
    }
}

void player::draw(JoB::levelComponent &self_comp, dvec2 scroll) {
    //get the player which is (hopefully) the parent of the level_component
    player* p = static_cast<player*>(self_comp.getParent());

    //draw the players current animation frame
    DrawTexturePro(p->current_animation->getTexture(), 
    p->current_animation->peek(), //get the animation frame rectangle

    {static_cast<float>(self_comp.getPosition().x-scroll.x), 
    static_cast<float>(self_comp.getPosition().y-scroll.y),
     64, 64}, 

     {0, 0}, 0, WHITE);
    
}

void player::update(JoB::levelComponent& component, t_seconds delta) {
    JoB::levelEntity& selfSpr = *reinterpret_cast<JoB::levelEntity*>(&component);
    player& self = *reinterpret_cast<player*>(selfSpr.getParent());
    JoB::Level& l = *selfSpr.level;
    if (self.is_falling) {
        //when the falling animation finishes we respawn and reset the animation state
        if (self.current_animation->is_finished()) {
            self.respawn();
            //reset the falling animation for the next time the player falls
            self.current_animation->reset();
            //for now we just reset to the front idle animation
            self.current_animation = &self.animations["front_idle"];
            self.is_falling = false;
            //reset the scroll
            l.focus_scroll(selfSpr.getPosition() + dvec2{32, 32});
        } else {
            self.current_animation->advance(delta);
            //if we are falling we just want the animation to run and then return
            return;
        }
    }

    
    //advance the current animation
    self.current_animation->advance(delta);
    
    dvec2 movement = {0, 0};

    if (!l.isOnFloor(selfSpr)) {
        self.start_falling();
        return;
    }

    if (JoB::isButtonDown(self.player_settings.keybinds["move_up"].keycode)) {
        movement.y = -1;
    }
    if (JoB::isButtonDown(self.player_settings.keybinds["move_left"].keycode)) {
        movement.x = -1;
    }
    if (JoB::isButtonDown(self.player_settings.keybinds["move_right"].keycode)) {
        movement.x = 1;
    }
    if (JoB::isButtonDown(self.player_settings.keybinds["move_down"].keycode)) {
        movement.y = 1;
    }

    if (!self.is_moving && movement.length2() > 0) {//when we start moving
        self.current_animation = &self.animations["front_walking"];
    }

    //update if we are moving
    self.is_moving = movement.length2() > 0;

    if (!self.is_moving) {
        self.current_animation = &self.animations["front_idle"];
    }
    

    if (movement.length2() == 0) return;

    //normalize movement
    movement = movement.normalized() * (self.max_velocity*delta);
    //move
    selfSpr.move(movement);
    //scroll the same amount
    l.scroll(movement);

    
}

//for formmating
JoB::formatter fmt = JoB::formatter();

enum WindowMode {
    Windowed,
    WindowedFullscreen
};

void updateWindow(WindowMode mode) {
    if (mode == WindowedFullscreen) {
        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        SetWindowPosition(0, 0);
    } else if (mode == Windowed) {
        SetWindowSize(1280, 720);
        SetWindowPosition(GetMonitorWidth(0)/2-1280/2, GetMonitorHeight(0)/2-720/2);
    }
}

int main() {


    //fvec2 aspect_ratio = {16, 9};

    //boilerplate starting stuff
    // the logger for main
    JoB::logger main_log = JoB::logger("main");

    main_log.log(JoB::logger::INFO, "Using font Alagard by Hewett Tsoi");

    bool paused = false;
    bool running = true;
    WindowMode windowMode = Windowed;

    

    SetTraceLogLevel(LOG_ALL);
    SetTraceLogCallback(raylibLogCallback);
    

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED);
    InitWindow(JoB::base_resolution.x, JoB::base_resolution.y, "Jack of Blades");
    SetWindowSize(1280, 720);
    SetWindowPosition(GetMonitorWidth(0)/2-1280/2, GetMonitorHeight(0)/2-720/2);
    SetExitKey(0);
    
    

    //create the allocator
    JoB::Allocator allocator = JoB::Allocator();
    RenderTexture2D rbuf = LoadRenderTexture(JoB::base_resolution.x, JoB::base_resolution.y);
    
    //create the font
    Font alagard = allocator.createFont("resources\\alagard.ttf");

    //create the cursor texture
    Texture2D cursor_texture = allocator.createTexture("resources\\UI\\Cursor.png");
    HideCursor();

    //test level
    JoB::Level level = JoB::Level(allocator);

    //create the player character
    player Player = player(allocator);
    Player.debugPrintKeybinds();

    //for delta time calculations
    auto f_start = high_resolution_clock::now();
    auto f_end = high_resolution_clock::now();
    t_seconds delta = 0.0;

    //add stuff to the level
    level.addBackground({0, 0, 1024, 1024}, "resources\\Enviroment\\FloorTiles.png");
    level.spawnEntity(Player.getSpriteComponent());
    

    //setup our screen canvas
    JoB::UICanvas screenUI = JoB::UICanvas({0, 0}, allocator);
    
    JoB::UIButton& max_button = screenUI.add(
        JoB::UIButton(
            {0.975, 0},
            {16, 16},
            Player.getKeybind("ui_interact").keycode,
            JoB::animation("resources\\UI\\Maximize_WindowButton.png", allocator),
            [&]() {
                windowMode = WindowedFullscreen;
                updateWindow(windowMode);
                
            }
        )
    );

    max_button.setActive(false);

    JoB::UIButton& min_button = screenUI.add(
        JoB::UIButton(
            {0.975, 0},
            {16, 16},
            Player.getKeybind("ui_interact").keycode,
            JoB::animation("resources\\UI\\Minimize_WindowButton.png", allocator),
            [&]() {
                windowMode = Windowed;
                updateWindow(windowMode);
            }
        )
    );
    min_button.setActive(false);

    JoB::UICanvas& pause_menu = screenUI.add(JoB::UICanvas({0, 0}, allocator));
    pause_menu.add(
        JoB::UIButton(
            {0.2469, 0.7833},
            {93.33, 66.66},
            Player.getKeybind("ui_interact").keycode,
            JoB::animation("resources\\UI\\Button-Clubs.png", allocator),
            [&]() {
                main_log.log(JoB::logger::INFO, "Going back to main menu");
            },
            alagard,
            RED,
            "Main Menu"
        )
    );
    pause_menu.add(
        JoB::UIButton(
            {0.4266, 0.7833},
            {93.33, 66.66},
            Player.getKeybind("ui_interact").keycode,
            JoB::animation("resources\\UI\\Button-Heart.png", allocator),
            [&]() {
                main_log.log(JoB::logger::INFO, "Opening options menu");
            },
            alagard,
            RED,
            "Settings"
        )
    );
    pause_menu.add(
        JoB::UIButton(
            {0.6062, 0.7833},
            {93.33, 66.66},
            Player.getKeybind("ui_interact").keycode,
            JoB::animation("resources\\UI\\Button-Diamonds.png", allocator),
            [&]() {
                main_log.log(JoB::logger::INFO, "Quitting game");
                running = false;
            },
            alagard,
            RED,
            "Quit Game"
        )
    );
    pause_menu.setActive(false);
    
    
    //focus the scroll of the level on the players center
    level.focus_scroll(Player.getSpriteComponent().getPosition() + dvec2{32, 32});

    while (running) {
        
        if (WindowShouldClose()) {
            running = false;
        }
        
        //update the delta time
        delta = duration_cast<nanoseconds>(f_end-f_start).count()/1000000000.0;
        if (paused) delta = 0;
        f_start = high_resolution_clock::now();

        //update the window
        if (windowMode == WindowedFullscreen) {
            min_button.setActive(paused);
            max_button.setActive(false);
        } else {
            max_button.setActive(paused);
            min_button.setActive(false);
        }
        
        if (JoB::isButtonReleased(Player.getKeybind("pause_game").keycode)) {
            paused = !paused;
            pause_menu.setActive(paused);
            if (windowMode == WindowedFullscreen) {
                min_button.setActive(paused);
            } else {
                max_button.setActive(paused);
            }
        }

        if (JoB::isButtonReleased(Player.getKeybind("fullscreen").keycode)) {
            i32 n_mode = static_cast<i32>(windowMode) + 1;
            if (n_mode > 2) n_mode = 0;
            windowMode = static_cast<WindowMode>(n_mode);

            updateWindow(windowMode);
        }
        
        //updates
        level.update(delta);
        screenUI.update(delta);

        
        //draw to the buffer
        BeginTextureMode(rbuf);

        //clear the background
        ClearBackground(BLACK);

        //draw the current level
        level.draw();

        EndTextureMode();

        //calculate the scaling so it fits on the current screen resolution nicely
        float scale = fmax(GetScreenWidth() / JoB::base_resolution.x, GetScreenHeight()/JoB::base_resolution.y);
        fvec2 res = {scale*JoB::base_resolution.x, scale*JoB::base_resolution.y};
        fvec2 pos = {-((res.x-GetScreenWidth())/2), -((res.y-GetScreenHeight())/2)};
        
        screenUI.setScale(scale);

        //start drawing to the screen
        BeginDrawing();
        
        if (!paused) {
            //draw the framebuffer
            DrawTexturePro(rbuf.texture, 
            {0, 0, JoB::base_resolution.x, -JoB::base_resolution.y}, 
            {pos.x, pos.y, res.x, res.y}, {0, 0}, 0, WHITE);
        } else {
            DrawTexturePro(rbuf.texture, 
            {0, 0, JoB::base_resolution.x, -JoB::base_resolution.y}, 
            {pos.x, pos.y, res.x, res.y}, {0, 0}, 0, {150, 150, 150, 255});
            DrawTexturePro(rbuf.texture, 
            {0, 0, JoB::base_resolution.x, -JoB::base_resolution.y}, 
            {pos.x-2, pos.y-2, res.x, res.y}, {0, 0}, 0, {200, 200, 200, 255});
        }

        
        screenUI.draw();

        //draw the cursor
        if (IsCursorOnScreen()) DrawTexturePro(cursor_texture, 
        {0, 0, 16, 16}, {static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY()), 32, 32},
        {0, 0},
        0.0, 
        WHITE);

        EndDrawing();

        f_end = high_resolution_clock::now();
    }

    
    //cleanup
    CloseWindow();
    allocator.free();
    UnloadRenderTexture(rbuf);
}