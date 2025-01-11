
#include "Camera.h"

Camera::Camera(glm::vec3 position)
{
    this->_position = position;
}

void Camera::applyMatrix(float FOV, Shaders &shaders, float ratio)
{
    glm::mat4 view(1.0f);
    glm::mat4 projection(1.0f);

    view = glm::lookAt(this->_position, this->_position + this->_rotation, this->_up);
    projection = glm::perspective(glm::radians(FOV), ratio, 0.1f, 100.f);

    shaders.setUniformMat4("ViewMatrix", projection * view);
}

void Camera::setPosition(glm::vec3 newPos)
{
    this->_position = newPos;
}

void Camera::setRotation(glm::vec3 newRotation)
{
    this->_rotation = newRotation;
}
