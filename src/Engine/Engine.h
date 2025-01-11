#ifndef ENGINE_H
#define ENGINE_H

#include <main.h>
#include "Drawer.h"
#include "Camera.h"

class Engine {
    GLFWwindow *window = nullptr;
    Drawer *drawer = nullptr;
    Camera *camera = nullptr;

    public:
        Engine() = default;
        ~Engine() = default;
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        static Engine &Instance();
        static void init();
        static void loop();
        static void draw();
        static void destroy();
};



#endif //ENGINE_H
