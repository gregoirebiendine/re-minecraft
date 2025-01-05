#ifndef ENGINE_H
#define ENGINE_H

#include <main.h>
#include <Drawer.h>

class Engine {
    GLFWwindow *window = nullptr;
    Drawer *drawer = nullptr;

    private:
        Engine() = default;
        ~Engine() = default;
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

    public:
        static Engine &Instance() {
            static Engine instance;
            return instance;
        }

        static void init();
        static void loop();
        static void draw();
        static void destroy();
};



#endif //ENGINE_H
