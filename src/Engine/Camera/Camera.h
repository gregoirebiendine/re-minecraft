
#ifndef RE_MINECRAFT_CAMERA_H
#define RE_MINECRAFT_CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include "Shaders.h"

class Camera {
    glm::vec3 _position{};
    glm::vec3 _rotation{0.0f, 0.0f, -1.0f};
    glm::vec3 _up{0.0f, 1.0f, 0.0f};

    float speed = 0.1f;
    float sensitivity = 100.0f;

    bool hasClickedWindow = false;

    public:
        Camera(glm::vec3 position);

        void applyMatrix(float FOV, Shaders &shaders, float ratio);
        void handleInputs(GLFWwindow *window);

        void setPosition(glm::vec3 newPos);
        void setRotation(glm::vec3 newRotation);
};


#endif //RE_MINECRAFT_CAMERA_H
