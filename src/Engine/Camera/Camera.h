
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
    glm::vec3 _position{};
    float _yaw   = -90.0f;
    float _pitch = 0.0f;

    float speed = 0.2f;
    float sensitivity = 0.1f;

    bool isMouseCaptured = false;

    Material selectedMaterial = Material::OAK_PLANK;

    public:
        static constexpr float FOV = glm::radians(90.f);
    
        explicit Camera(glm::vec3 position);

        void moveCamera(double mouseX, double mouseY);
        void move(glm::vec3 direction);
        [[nodiscard]] Raycast::Hit raycast(const World& world) const;

        [[nodiscard]] glm::vec3 getForwardVector() const;
        [[nodiscard]] bool getMouseCapture() const;
        [[nodiscard]] glm::vec3 getPosition() const;
        [[nodiscard]] glm::vec2 getRotation() const;

        void setPosition(glm::vec3 newPos);
        void toggleMouseCapture();
        void setSelectedMaterial(const Material newMaterial);
        Material getSelectedMaterial() const;
};


#endif //RE_MINECRAFT_CAMERA_H
