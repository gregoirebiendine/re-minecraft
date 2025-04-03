#ifndef ENGINE_H
#define ENGINE_H

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Atlas.h"
#include "Shader.h"
#include "Map.h"
#include "Camera.h"

class Engine {
    int W  = 1280;
    int H = 1280;

    GLFWwindow *window = nullptr;

    std::unique_ptr<Atlas> atlas;
    std::unique_ptr<Shader> shaders;
    std::unique_ptr<Map> map;
    std::unique_ptr<Camera> camera;

    public:
        Engine();
        ~Engine();

        void init();
        void loop();
        void draw();
};



#endif //ENGINE_H
