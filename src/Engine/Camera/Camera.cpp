
#include "Camera.h"

Camera::Camera(glm::vec3 position)
{
    this->_position = position;
}

void Camera::applyMatrix(const float FOV, const std::unique_ptr<Shader> &shaders, const float ratio) const
{
    const glm::mat4 view = glm::lookAt(this->_position, this->_position + this->_rotation, this->_up);
    const glm::mat4 projection = glm::perspective(glm::radians(FOV), ratio, 0.1f, 100.f);

    shaders->setUniformMat4("ViewMatrix", projection * view);
}

void Camera::setPosition(glm::vec3 newPos)
{
    this->_position = newPos;
}

void Camera::setRotation(glm::vec3 newRotation)
{
    this->_rotation = newRotation;
}

void Camera::handleInputs(GLFWwindow *window)
{
    if (glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS)
        this->_position += this->speed * this->_rotation;
    if (glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS)
        this->_position += speed * -glm::normalize(glm::cross(this->_rotation, this->_up));
    if (glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS)
        this->_position += speed * -this->_rotation;
    if (glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS)
        this->_position += speed * glm::normalize(glm::cross(this->_rotation, this->_up));

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        this->_position += speed * this->_up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        this->_position += speed * -this->_up;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (this->hasClickedWindow) {
            glfwSetCursorPos(window, (1280 / 2), (1280 / 2)); // Should be width/2 & height/2
            this->hasClickedWindow = false;
        }

        double mouseX;
        double mouseY;

        glfwGetCursorPos(window, &mouseX, &mouseY);

        float rotX = sensitivity * (float)(mouseY - (1280 / 2)) / 1280; // height
        float rotY = sensitivity * (float)(mouseX - (1280 / 2)) / 1280; // width

        glm::vec3 newRotation = glm::rotate(this->_rotation, glm::radians(-rotX), glm::normalize(glm::cross(this->_rotation, this->_up)));

        if (abs(glm::angle(newRotation, this->_up) - glm::radians(90.0f)) <= glm::radians(85.0f))
            this->_rotation = newRotation;

        // Rotates the Orientation left and right
        this->_rotation = glm::rotate(this->_rotation, glm::radians(-rotY), this->_up);

        glfwSetCursorPos(window, (1280 / 2), (1280 / 2)); // Should be width/2 & height/2
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        this->hasClickedWindow = true;
    }
}
