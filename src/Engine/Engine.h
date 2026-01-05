#ifndef ENGINE_H
#define ENGINE_H

#define GLM_ENABLE_EXPERIMENTAL
#define UNUSED(expr) (void)(expr)

#include <iostream>
#include <memory>

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

#include "Atlas.h"
#include "Shader.h"
#include "World.h"
#include "Camera.h"
#include "InputState.h"
#include "BlockRegistry.h"

class Engine {
    const glm::ivec2 WindowSize{1280, 720};
    glm::ivec2 ScreenSize{};

    GLFWwindow *window = nullptr;

    BlockRegistry blockRegistry;
    InputState inputs;

    std::unique_ptr<Shader> shaders;
    std::unique_ptr<Atlas> atlas;
    std::unique_ptr<World> world;
    std::unique_ptr<Camera> camera;

    public:
        Engine();
        ~Engine();

        void loop();
        void handleInputs(float deltaTime) const;
        void clearInputs();
        void update() const;
        void render(float deltaTime) const;
        void setViewMatrix() const;
};

void keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonInputCallback(GLFWwindow* window, int button, int action, int mods);
void mouseInputCallback(GLFWwindow* window, double x, double y);

#endif //ENGINE_H
