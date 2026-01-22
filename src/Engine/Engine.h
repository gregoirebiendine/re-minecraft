#ifndef ENGINE_H
#define ENGINE_H

#define GLM_ENABLE_EXPERIMENTAL

#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
    #include <time.h>
    #include <cerrno>
#endif

#include <iostream>
#include <memory>
#include <filesystem>
#include <thread>
#include <chrono>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Viewport.h"
#include "World.h"
#include "Player.h"
#include "InputState.h"
#include "BlockRegistry.h"
#include "TextureRegistry.h"

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double>;

class Engine {
    #ifdef _WIN32
        HANDLE frameTimer = nullptr;
    #endif

    Viewport viewport;

    Raycast::Hit lastRaycastHit{};
    InputState inputs;
    BlockRegistry blockRegistry;
    TextureRegistry textureRegistry;

    std::unique_ptr<Player> player;
    std::unique_ptr<World> world;

    void preciseWait(double seconds) const;
    void handleInputs(double deltaTime);
    void clearInputs();
    void update() const;
    void render() const;

    public:
        Engine();
        ~Engine();

        void loop();
};

void keyInputCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods);
void mouseButtonInputCallback(GLFWwindow* window, int button, int action, [[maybe_unused]] int mods);
void mouseInputCallback(GLFWwindow* window, double x, double y);

#endif //ENGINE_H
