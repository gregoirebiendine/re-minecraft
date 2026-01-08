
#include "Camera.h"

static double lastX{};
static double lastY{};
static bool firstMouse = true;

Camera::Camera(const glm::vec3 position, const BlockRegistry& blockRegistry)
{
    this->_position = position;
    this->selectedMaterial = blockRegistry.getByName("core:oak_plank");
}

Raycast::Hit Camera::raycast(const World& world) const
{
    const glm::vec3 origin = this->getPosition();
    const glm::vec3 dir = this->getForwardVector();

    glm::vec3 pos = origin;
    glm::ivec3 lastBlock(-1);

    float t = 0.f;
    while (t < Raycast::MAX_DISTANCE) {
        pos = origin + dir * t;
        glm::ivec3 blockPos = glm::floor(pos);

        if (blockPos == lastBlock)
        {
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
    forward.x = glm::cos(glm::radians(this->_yaw)) * glm::cos(glm::radians(this->_pitch));
    forward.y = glm::sin(glm::radians(this->_pitch));
    forward.z = glm::sin(glm::radians(this->_yaw)) * glm::cos(glm::radians(this->_pitch));
    return glm::normalize(forward);
}

void Camera::moveCamera(const double mouseX, const double mouseY, const double deltaTime)
{
    if (!this->isMouseCaptured)
    {
        firstMouse = true;
        return;
    }

    if (firstMouse) {
        lastX = mouseX;
        lastY = mouseY;
        firstMouse = false;
    }

    this->_yaw   += static_cast<float>((mouseX - lastX) * SENSITIVITY * deltaTime);
    this->_pitch += static_cast<float>((lastY - mouseY) * SENSITIVITY * deltaTime);

    this->_yaw = std::fmod(this->_yaw, 360.0f);

    if (this->_yaw > 180.0f)
        this->_yaw -= 360.0f;
    if (this->_yaw <= -180.0f)
        this->_yaw += 360.0f;

    this->_pitch = glm::clamp(this->_pitch, -89.9f, 89.9f);

    lastX = mouseX;
    lastY = mouseY;
}

void Camera::move(const glm::vec3 direction, float deltaTime)
{
    const glm::vec3 forward = this->getForwardVector();
    const glm::vec3 right = glm::normalize(glm::cross(forward, {0,1,0}));

    if (direction.x != 0)
        this->_position += direction.x * right * SPEED * deltaTime;
    if (direction.y != 0)
        this->_position += direction.y * glm::vec3{0, 1, 0} * SPEED * deltaTime;
    if (direction.z != 0)
        this->_position += direction.z * forward * SPEED * deltaTime;
}

void Camera::setPosition(const glm::vec3 newPos)
{
    this->_position = newPos;
}

void Camera::setSelectedMaterial(const Material newMaterial)
{
    this->selectedMaterial = newMaterial;
}

Material Camera::getSelectedMaterial() const
{
    return this->selectedMaterial;
}

glm::vec3 Camera::getPosition() const
{
    return this->_position;
}

glm::vec2 Camera::getRotation() const
{
    return {this->_yaw, this->_pitch};
}
