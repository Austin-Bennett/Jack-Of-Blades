#pragma once


#include <cstdarg>
#include <sstream>
#include "AustinUtils.hpp"
#include "raylib.h"

/*
raylib functions:
IsKeyPressed
IsKeyDown
IsKeyReleased
IsKeyUp
*/

using namespace AustinUtils;



namespace JoB {

    struct rect : public Rectangle {

        static rect of(f32 x, f32 y, rect wh) {
            return {x, y, wh.width, wh.height};
        }

        fvec2 get_center() {
            return {x +width/2, y+height/2};
        }

        bool operator && (rect const& other) {
            if (x+width < other.x) return false;
            if (y+height < other.y) return false;
            if (other.x + other.width < x) return false;
            if (other.y+other.height < y) return false;

            return true;
        }

        rect operator & (rect const& rect2) {
            // Find the maximum of the left edges and the minimum of the right edges
            float x1 = fmax(x, rect2.x);
            float y1 = fmax(y, rect2.y);
            float x2 = fmin(x + width, rect2.x + rect2.width);
            float y2 = fmin(y + height, rect2.y + rect2.height);

            // Check if the rectangles overlap
            if (x1 < x2 && y1 < y2) {
                return { x1, y1, x2 - x1, y2 - y1 }; // Return the overlapping rectangle
            }

            // If there is no collision, return a rectangle with zero width and height
            return { 0, 0, 0, 0 };
        }

        friend std::ostream& operator <<(std::ostream& os, rect self) {
            os << "{" << self.x << ", " << self.y << ", " << self.width << ", " << self.height << "}";
            return os; 
        }

        double area() {
            return width*height;
        }

        template<Arithmetic T>
        rect operator +(v2<T> vec) {
            return {static_cast<float>(x+vec.x), static_cast<float>(y+vec.y), width, height};
        }

        template<Arithmetic T>
        rect operator -(v2<T> vec) {
            return {static_cast<float>(x-vec.x), static_cast<float>(y-vec.y), width, height};
        }

        template<Arithmetic T>
        rect operator *(T scale) {
            return {x, y, static_cast<float>(width*scale), static_cast<float>(height*scale)};
        }
    };

    inline i32 findBestFit(Font f, rect rec, const char* text) {
        i32 cur_size = rec.height;
        Vector2 text_dimensions = MeasureTextEx(f, text, cur_size, 2);
        while ((text_dimensions.x > rec.width || text_dimensions.y > rec.height) && cur_size > 1) {
            cur_size--;
            text_dimensions = MeasureTextEx(f, text, cur_size, 2);
        }

        return abs(cur_size);
    }

    inline bool isButtonPressed(i32 button_code) {
        if (button_code < 7) return IsMouseButtonPressed(button_code);
        if (button_code > 7) return IsKeyPressed(button_code);

        return false;
    }

    inline bool isButtonDown(i32 button_code) {
        if (button_code < 7) return IsMouseButtonDown(button_code);
        if (button_code > 7) return IsKeyDown(button_code);

        return false;
    }

    inline bool isButtonReleased(i32 button_code) {
        if (button_code < 7) return IsMouseButtonReleased(button_code);
        if (button_code > 7) return IsKeyReleased(button_code);

        return false;
    }

    inline bool isButtonUp(i32 button_code) {
        if (button_code < 7) return IsMouseButtonUp(button_code);
        if (button_code > 7) return IsKeyUp(button_code);

        return false;
    }


    template <typename T>
    concept Formattable = requires(std::ostream& os, T value) {
        { os << value } -> std::same_as<std::ostream&>;
    };

    class formatter : public std::stringstream {
        public:
        formatter() = default;

        std::string str() {
            std::string s = std::stringstream::str();
            std::stringstream::str("");
            clear();

            return s;
        }

    };

    class logger {
        public:

        enum LOG_TYPE {
            INFO,
            DEBUG,
            WARN,
            ERROR,
        };

        std::string lTypetoStr(LOG_TYPE typ) {
            switch (typ) {
                case INFO:
                    return "INFO";
                case DEBUG:
                    return "DEBUG";
                case WARN:
                    return "WARN";
                case ERROR:
                    return "ERROR";

            }

            return "LOG";
        }

        private:

        
        std::string name;
        formatter msg;
        public:

        logger(std::string name) {
            this->name = name;
        };

        void clog(LOG_TYPE typ, const char* message) {
            

            // Append log type and logger name
            msg << "[" << lTypetoStr(typ) << "]"
                << "[" << name << "]: " << message;

            // Print the log message to the console
            std::cout << msg.str() << std::endl;
        }


        template<Formattable... Args>
        void log(LOG_TYPE typ, const std::string& fmt, Args&&... args) {
            std::ostringstream msg;

            // Append the log type and logger name
            msg << "[" << lTypetoStr(typ) << "]"
                << "[" << name << "]: ";

            // Store arguments in a tuple
            auto arg_tuple = std::make_tuple(std::forward<Args>(args)...);
            constexpr size_t num_args = sizeof...(args);
            size_t arg_index = 0;

            size_t pos = 0;
            std::string remaining_fmt = fmt;

            // Helper to visit a specific argument from the tuple
            auto visit_arg = [&](auto&& visitor) {
                if (arg_index >= num_args) {
                    throw std::invalid_argument("Too few arguments provided for the format string.");
                }
                std::apply(
                    [&](auto&&... tuple_args) {
                        size_t current = 0;
                        (((current == arg_index ? visitor(tuple_args) : void()), ++current), ...);
                    },
                    arg_tuple);
            };

            // Process the format string
            while ((pos = remaining_fmt.find('{')) != std::string::npos) {
                // Handle escaped braces "{{" and "}}"
                if (remaining_fmt.substr(pos, 2) == "{{") {
                    msg << remaining_fmt.substr(0, pos) << "{";
                    remaining_fmt.erase(0, pos + 2);
                } else if (remaining_fmt.substr(pos, 2) == "}}") {
                    msg << remaining_fmt.substr(0, pos) << "}";
                    remaining_fmt.erase(0, pos + 2);
                } else {
                    // Replace "{}" with the next argument
                    msg << remaining_fmt.substr(0, pos);
                    remaining_fmt.erase(0, pos + 2);

                    // Insert the next argument
                    visit_arg([&](const auto& arg) {
                        msg << arg;
                    });
                    ++arg_index;
                }
            }

            // Append the remaining part of the format string
            msg << remaining_fmt;

            // Check if too many arguments were provided
            if (arg_index < num_args) {
                throw std::invalid_argument("Too many arguments provided for the format string.");
            }

            // Output the final formatted message
            std::cout << msg.str() << "\n";
        }
        

    };

    template<AustinUtils::Arithmetic T, AustinUtils::Arithmetic mT, AustinUtils::Arithmetic MT>
    T clamp(T x, mT min, MT max) {
        return x > max ? max:(x < min ? min:x);
    }

}