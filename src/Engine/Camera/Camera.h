
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
#include "BlockRegistry.h"
#include "Raycast.h"

class Camera {
    static constexpr float SPEED = 8.f;
    static constexpr float SENSITIVITY = 5.f;

    glm::vec3 _position{};
    float _yaw   = -90.0f;
    float _pitch = 0.0f;

    bool isMouseCaptured = false;
    Material selectedMaterial = 0; //defaults to "core:air" or 0

    public:
        static constexpr float FOV = glm::radians(90.f);
    
        explicit Camera(glm::vec3 position, const BlockRegistry& blockRegistry);

        void moveCamera(double mouseX, double mouseY, double deltaTime);
        void move(glm::vec3 direction, float deltaTime);

        [[nodiscard]] Raycast::Hit raycast(const World& world) const;
        [[nodiscard]] glm::vec3 getForwardVector() const;

        [[nodiscard]] bool getMouseCapture() const;
        void toggleMouseCapture();

        void setPosition(glm::vec3 newPos);
        [[nodiscard]] glm::vec3 getPosition() const;
        [[nodiscard]] glm::vec2 getRotation() const;

        void setSelectedMaterial(Material newMaterial);
        [[nodiscard]] Material getSelectedMaterial() const;
};


#endif //RE_MINECRAFT_CAMERA_H
