#pragma once

#include "lib.hpp"
#include "util.hpp"
using namespace AustinUtils;

namespace JoB {

    class UICanvas;
    
    class UIObject {
        protected:
        fvec2 scr_pos;//the position as a percentage of the screen as to where it should appear
        bool is_active = true;
        double scale = 1.0;

        public:
        UIObject(fvec2 scr_pos) {
            this->scr_pos.x = JoB::clamp(scr_pos.x, 0, 1);
            this->scr_pos.y = JoB::clamp(scr_pos.y, 0, 1);
        }

        virtual void update(double delta_time) {}
        virtual void draw() {}

        void setActive(bool new_val) {
            is_active = new_val;
        }

        virtual void setScale(double n_s) {
            scale = n_s;
        }

        double getScale() {
            return scale;
        }

        bool isActive() {
            return is_active;
        }

        fvec2 getScrPos() {
            return scr_pos;
        }

        ivec2 getCurrentScreenPosition() {
            return (scr_pos * ivec2{GetScreenWidth(), GetScreenHeight()}).convert_data<i32>();
        }

        friend UICanvas;
    };

    class UICanvas : public UIObject {
        protected:
        vector<UIObject*> objects;
        Allocator& allocator;

        public:
        UICanvas(fvec2 scr_pos, Allocator& allocator) : UIObject(scr_pos), allocator(allocator) {}

        template<class T, typename = std::enable_if_t<std::is_base_of_v<UIObject, T>>>
        T& add(T object) {
            object.scr_pos += scr_pos;
            T* allocation = static_cast<T*>(allocator.allocate_memory(sizeof(T)));
            objects.push_back(allocation);
            memcpy(allocation, &object, sizeof(T));

            return *allocation;
        }

        void setActive(bool active) {
            for (auto& obj: objects) {
                obj->setActive(active);
            }
            is_active = active;
        }

        template<class T, typename = std::enable_if_t<std::is_base_of_v<UIObject, T>>>
        void remove(T& obj) {
            auto it = std::find(objects.begin(), objects.end(), &obj);
            allocator.free(&obj);
            objects.erase(it);
        }

        void setScale(double n) override {
            scale = n;
            for (auto& obj: objects) {
                obj->setScale(n);
            }
        }

        void update(double delta) override {
            for (auto& obj: objects) {
                obj->update(delta);
            }
        }

        void draw() override {
            for (auto& obj: objects) {
                obj->draw();
            }
        }

    };
    
    //allows you to run a function whenever you press the button
    class UIButton : public UIObject {

        std::function<void()> pressed;
        rect size;
        i32 button_press_code;
        animation texture;
        const char* text;
        Color text_color;
        bool is_pressed = false;
        bool is_hovered = false;
        i32 text_size;
        Font f;
        public:

        UIButton(fvec2 scr_pos, fvec2 size, i32 button_code, animation anim,
         std::function<void()> press_function, Font fnt = GetFontDefault(),
         Color textcolor = BLANK, const char* text = "") : 
        
        UIObject(scr_pos), 
        pressed(press_function),
        button_press_code(button_code),
        texture(anim)
        
        {
            this->size = {0, 0, size.x, size.y};
            this->text = text;
            text_color = textcolor;
            f = fnt,
            text_size = findBestFit(f, this->size, text)-1;
        }

        void setFont(Font f) {
            this->f = f;
            text_size = findBestFit(f, size, text)-1;
        }

        void setFontSize(float n_size) {
            text_size = n_size;
        }

        //optional, only for automatic button checking
        void update(double delta_time) override {
            if (!is_active) return;

            texture.advance(delta_time);

            
            ivec2 spos = {static_cast<int>(GetScreenWidth() * scr_pos.x), static_cast<int>(GetScreenHeight() * scr_pos.y)};
            
            
            if (isButtonDown(button_press_code)) {
                if (is_pressed) return;
                if ((size + spos)*scale && rect{static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY()), 1, 1}) {
                    
                    pressed();
                    is_pressed = 1;
                }
            } else if ((size+spos)*scale && rect{static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY()), 1, 1}) {
                is_hovered = 1;
                is_pressed = 0;
            } else {
                is_hovered = 0;
                is_pressed = 0;
            }
        }

        void press() {
            if (is_active) pressed();
            is_pressed = 1;
        }

        void draw() override {
            if (!is_active) return;
            ivec2 spos = {static_cast<int>(GetScreenWidth() * scr_pos.x), static_cast<int>(GetScreenHeight() * scr_pos.y)};
            fvec2 center = ((size+spos)*scale).get_center();

            
            
            if (is_pressed) {
                //draw normally
                DrawTexturePro(texture.getTexture(), texture.peek(), 
                (size + spos)*scale, 
                {0, 0}, 
                0.0, 
                WHITE);
                
                drawTextCentered(f, text, center.x, center.y, text_size*scale, text_color);
            } else if (is_hovered) {
                //draw offset a bit and with a darker version behind
                DrawTexturePro(texture.getTexture(), texture.peek(), 
                (size + spos)*scale, 
                {0, 0}, 
                0.0, 
                {100, 100, 100, 200});
                DrawTexturePro(texture.getTexture(), texture.peek(), 
                (size + (spos-ivec2{2, 2}))*scale, 
                {0, 0}, 
                0.0, 
                WHITE);
                
                //when hovered also have a backdrop on the text
                
                drawTextCentered(f, text, center.x, center.y, text_size*scale, BLACK);
                drawTextCentered(f, text, (center.x-2), (center.y-2), text_size*scale, text_color);
            } else {
                //draw offset a bit and with a darker version behind
                DrawTexturePro(texture.getTexture(), texture.peek(), 
                (size + spos)*scale, 
                {0, 0}, 
                0.0, 
                {100, 100, 100, 200});
                DrawTexturePro(texture.getTexture(), texture.peek(), 
                (size + (spos-ivec2{2, 2}))*scale, 
                {0, 0}, 
                0.0, 
                WHITE);
                drawTextCentered(f, text, center.x, center.y, text_size*scale, text_color);
            }

            

        }

    };
}