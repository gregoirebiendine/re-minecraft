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
#include "World.h"
#include "Raycast.h"

class Camera {
    static constexpr float FOV = glm::radians(90.f);
    static constexpr float SPEED = 8.f;
    static constexpr float SENSITIVITY = 5.f;

    glm::vec3 position{};
    float yaw = -90.0f;
    float pitch = 0.0f;
    bool isMouseCaptured = false;

    double lastX{};
    double lastY{};
    bool firstMouse = true;

    public:

        explicit Camera(glm::vec3 _position);

        void moveCamera(double mouseX, double mouseY, double deltaTime);
        void move(glm::vec3 direction, float deltaTime);

        [[nodiscard]] Raycast::Hit raycast(World& world) const;
        glm::mat4 setViewMatrix(const Shader& shader, const float& aspect) const;

        [[nodiscard]] glm::vec3 getForwardVector() const;
        [[nodiscard]] glm::mat4 getViewMatrix() const;
        [[nodiscard]] glm::mat4 getProjectionMatrix(const float& aspect) const;

        [[nodiscard]] bool getMouseCapture() const;
        void toggleMouseCapture();

        void setPosition(glm::vec3 newPos);
        [[nodiscard]] glm::vec3 getPosition() const;
        [[nodiscard]] glm::vec2 getRotation() const;
};

#endif //RE_MINECRAFT_CAMERA_H
