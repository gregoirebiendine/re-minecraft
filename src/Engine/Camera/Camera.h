
#ifndef RE_MINECRAFT_CAMERA_H
#define RE_MINECRAFT_CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shaders.h"

class Camera {
    glm::vec3 _position{};
    glm::vec3 _rotation{0.0f, 0.0f, -1.0f};
    glm::vec3 _up{0.0f, 1.0f, 0.0f};

    float speed = 0.1f;
    float sensitivity = 100.0f;

    public:
        Camera(glm::vec3 position);

        void applyMatrix(float FOV, Shaders &shaders, float ratio);
        void setPosition(glm::vec3 newPos);
        void setRotation(glm::vec3 newRotation);
};


#endif //RE_MINECRAFT_CAMERA_H
