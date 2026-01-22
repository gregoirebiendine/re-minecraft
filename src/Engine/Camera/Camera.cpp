
#include "Camera.h"

Camera::Camera(const glm::vec3 _position) :
    position(_position)
{
    Camera::firstMouse = true;
}

Raycast::Hit Camera::raycast(World& world) const
{
    const glm::vec3 origin = this->getPosition();
    const glm::vec3 dir = this->getForwardVector();

    glm::vec3 pos = origin;
    glm::ivec3 lastBlock(-1);

    float t = 0.f;
    while (t < Raycast::MAX_DISTANCE) {
        pos = origin + dir * t;
        glm::ivec3 blockPos = glm::floor(pos);

        if (blockPos == lastBlock) {
            t += Raycast::STEP;
            continue;
        }

        lastBlock = blockPos;

        if (world.getBlock(blockPos.x, blockPos.y, blockPos.z)) {
            return {
                true,
                blockPos,
                glm::floor(origin + dir * (t - Raycast::STEP))
            };
        }

        t += Raycast::STEP;
    }

    return {
        false,
        lastBlock,
        lastBlock
    };
}

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
    return glm::perspective(FOV, aspect, 0.1f, 256.f);
}

glm::mat4 Camera::getViewMatrix() const
{
    const auto forward = this->getForwardVector();
    return glm::lookAt(this->position, this->position + forward, {0,1,0});
}

void Camera::setViewMatrix(const Shader& shader, const float& aspect) const
{
    shader.use();
    shader.setViewMatrix(this->getViewMatrix());
    shader.setProjectionMatrix(this->getProjectionMatrix(aspect));
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

void Camera::move(const glm::vec3 direction, const float deltaTime)
{
    const glm::vec3 forward = this->getForwardVector();
    const glm::vec3 right = glm::normalize(glm::cross(forward, {0,1,0}));

    if (direction.x != 0)
        this->position += direction.x * right * SPEED * deltaTime;
    if (direction.y != 0)
        this->position += direction.y * glm::vec3{0, 1, 0} * SPEED * deltaTime;
    if (direction.z != 0)
        this->position += direction.z * forward * SPEED * deltaTime;
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
