#include "Camera.h"

bool Camera::getMouseCapture() const
{
    return this->isMouseCaptured;
}

void Camera::toggleMouseCapture()
{
    this->isMouseCaptured = !this->isMouseCaptured;
}

glm::vec3 Camera::getForwardVector() const
{
    glm::vec3 forward;
    forward.x = glm::cos(glm::radians(this->yaw)) * glm::cos(glm::radians(this->pitch));
    forward.y = glm::sin(glm::radians(this->pitch));
    forward.z = glm::sin(glm::radians(this->yaw)) * glm::cos(glm::radians(this->pitch));
    return glm::normalize(forward);
}

glm::mat4 Camera::getProjectionMatrix(const float& aspect) const
{
    return glm::perspective(glm::radians(static_cast<float>(this->fov)), aspect, 0.1f, 256.f);
}

glm::mat4 Camera::getViewMatrix() const
{
    const auto forward = this->getForwardVector();
    return glm::lookAt(this->position, this->position + forward, {0,1,0});
}

glm::mat4 Camera::setViewMatrix(Shader& shader, const float& aspect)
{
    const auto v = this->getViewMatrix();
    const auto p = this->getProjectionMatrix(aspect);

    shader.use();
    shader.setViewMatrix(v);
    shader.setProjectionMatrix(p);

    return p * v;
}

void Camera::moveCamera(const double mouseX, const double mouseY, const double deltaTime)
{
    if (!this->isMouseCaptured) {
        firstMouse = true;
        return;
    }

    if (firstMouse) {
        lastX = mouseX;
        lastY = mouseY;
        firstMouse = false;
    }

    this->yaw   += static_cast<float>((mouseX - lastX) * SENSITIVITY * deltaTime);
    this->pitch += static_cast<float>((lastY - mouseY) * SENSITIVITY * deltaTime);

    this->yaw = std::fmod(this->yaw, 360.0f);

    if (this->yaw > 180.0f)
        this->yaw -= 360.0f;
    if (this->yaw <= -180.0f)
        this->yaw += 360.0f;

    this->pitch = glm::clamp(this->pitch, -89.9f, 89.9f);

    lastX = mouseX;
    lastY = mouseY;
}

void Camera::setFOV(const uint8_t _fov)
{
    this->fov = _fov;
}

void Camera::setPosition(const glm::vec3 newPos)
{
    this->position = newPos;
}

glm::vec3 Camera::getPosition() const
{
    return this->position;
}

glm::vec2 Camera::getRotation() const
{
    return {this->yaw, this->pitch};
}
