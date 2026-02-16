#ifndef FARFIELD_PLAYERMOVEMENTSYSTEM_H
#define FARFIELD_PLAYERMOVEMENTSYSTEM_H

#include "ECS/ISystem.h"
#include "CameraSystem.h"
#include "Components/Movements.h"
#include "Components/PlayerInput.h"
#include "Components/Camera.h"
#include "Components/CollisionBox.h"
#include "Components/Friction.h"

namespace ECS
{
    class PlayerMovementSystem : public ISystem
    {
        static constexpr float ACCELERATION = 0.04f;
        static constexpr float MAX_SPEED    = 0.075f;
        static constexpr float JUMP_FORCE   = 0.136f;

        public:
            void update(Handler &handler, [[maybe_unused]] float deltaTime) override
            {
                auto view = handler.query<PlayerInput, Camera, Velocity, CollisionBox>();
                auto& frictionPool = handler.getPool<Friction>();

                view.forEach([&]([[maybe_unused]] const EntityId id, PlayerInput& input, const Camera& camera, Velocity& vel, const CollisionBox& box) {
                    float friction = 0.8f;

                    if (frictionPool.has(id)) {
                        const auto&[ground, air] = frictionPool.get(id);
                        friction = box.isGrounded ? ground : air;
                    }

                    // Jump
                    if (box.isGrounded && input.jumpBufferFrames > 0) {
                        vel.y = JUMP_FORCE;
                        input.jumpBufferFrames = 0;
                    }

                    // Horizontal movement
                    const glm::vec3 forward = CameraSystem::getForwardVector(camera);
                    const glm::vec3 flatForward = glm::normalize(glm::vec3{forward.x, 0, forward.z});
                    const glm::vec3 right = glm::cross(flatForward, {0, 1, 0});

                    glm::vec3 wishDir = right * input.direction.x + flatForward * input.direction.z;
                    if (glm::length(wishDir) > 0.f)
                        wishDir = glm::normalize(wishDir);

                    vel.x += wishDir.x * ACCELERATION;
                    vel.z += wishDir.z * ACCELERATION;

                    vel.x *= friction;
                    vel.z *= friction;

                    glm::vec2 hVel{vel.x, vel.z};
                    if (glm::length(hVel) > MAX_SPEED) {
                        hVel = glm::normalize(hVel) * MAX_SPEED;
                        vel.x = hVel.x;
                        vel.z = hVel.y;
                    }
                });
            }
    };
}

#endif