#ifndef RE_MINECRAFT_CAMERA_H
#define RE_MINECRAFT_CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Shader.h"

class Camera {
    static constexpr float SPEED = 8.f;
    static constexpr float SENSITIVITY = 5.f;

    glm::vec3 position{};
    uint8_t fov = 90;
    float yaw = -90.0f;
    float pitch = 0.0f;
    bool isMouseCaptured = false;

    double lastX{};
    double lastY{};
    bool firstMouse = true;

    public:
        void moveCamera(double mouseX, double mouseY, double deltaTime);

        [[nodiscard]] glm::mat4 setViewMatrix(Shader& shader, const float& aspect);

        [[nodiscard]] glm::vec3 getForwardVector() const;
        [[nodiscard]] glm::mat4 getViewMatrix() const;
        [[nodiscard]] glm::mat4 getProjectionMatrix(const float& aspect) const;

        [[nodiscard]] bool getMouseCapture() const;
        void toggleMouseCapture();

        void setFOV(uint8_t _fov);
        void setPosition(glm::vec3 newPos);
        [[nodiscard]] glm::vec3 getPosition() const;
        [[nodiscard]] glm::vec2 getRotation() const;
};

#endif //RE_MINECRAFT_CAMERA_H
