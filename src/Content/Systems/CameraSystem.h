#ifndef FARFIELD_CAMERASYSTEM_H
#define FARFIELD_CAMERASYSTEM_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "InputState.h"
#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/Camera.h"

namespace ECS
{
    class CameraSystem : public ISystem
    {
        const InputState& inputs;

        glm::mat4 viewMatrix{1.f};
        glm::mat4 projectionMatrix{1.f};
        glm::vec3 cameraPosition{};
        float aspect = 1.f;

        double lastX = 0.0;
        double lastY = 0.0;
        bool firstCapture = true;
        bool mouseCaptured = false;

        public:
            explicit CameraSystem(const InputState& _inputs) : inputs(_inputs) {}

            void setAspect(const float a) { aspect = a; }
            void setMouseCaptured(const bool captured)
            {
                if (!captured)
                    firstCapture = true;
                mouseCaptured = captured;
            }

            [[nodiscard]] bool isMouseCaptured() const { return this->mouseCaptured; }
            [[nodiscard]] const glm::mat4& getViewMatrix() const { return viewMatrix; }
            [[nodiscard]] const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
            [[nodiscard]] const glm::vec3& getCameraPosition() const { return cameraPosition; }

            void update(Handler &handler, [[maybe_unused]] float dt) override
            {
                if (this->firstCapture) {
                    this->lastX = inputs.mouseX;
                    this->lastY = inputs.mouseY;
                    this->firstCapture = false;
                    return;
                }

                double dx = 0;
                double dy = 0;

                if (this->mouseCaptured) {
                    dx = inputs.mouseX - lastX;
                    dy = lastY - inputs.mouseY;
                    lastX = inputs.mouseX;
                    lastY = inputs.mouseY;
                }

                auto view = handler.query<Position, Camera>();

                view.forEach([&]([[maybe_unused]] EntityId id, [[maybe_unused]] const Position& pos, Camera& camera)
                {
                    // Compute yaw/pitch from mouse delta
                    camera.yaw += static_cast<float>(dx * camera.sensitivity);
                    camera.pitch += static_cast<float>(dy * camera.sensitivity);

                    camera.yaw = std::fmod(camera.yaw, 360.0f);
                    if (camera.yaw >  180.0f) camera.yaw -= 360.0f;
                    if (camera.yaw <= -180.0f) camera.yaw += 360.0f;

                    camera.pitch = glm::clamp(camera.pitch, -89.99f, 89.99f);

                    // Store matrices data
                    this->cameraPosition = pos + camera.eyeOffset;
                    this->viewMatrix = glm::lookAt(this->cameraPosition, this->cameraPosition + getForwardVector(camera), {0,1,0});
                    this->projectionMatrix = glm::perspective(glm::radians(camera.fov), this->aspect, camera.nearPlane, camera.farPlane);
                });
            }

            static glm::vec3 getForwardVector(const Camera& camera)
            {
                glm::vec3 forward;
                forward.x = glm::cos(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
                forward.y = glm::sin(glm::radians(camera.pitch));
                forward.z = glm::sin(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
                return glm::normalize(forward);
            }
    };
}

#endif