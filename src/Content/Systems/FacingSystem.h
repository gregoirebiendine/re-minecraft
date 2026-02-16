#ifndef FARFIELD_FACINGSYSTEM_H
#define FARFIELD_FACINGSYSTEM_H

#include <cmath>
#include <glm/glm.hpp>

#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/Camera.h"

namespace ECS
{
    class FacingSystem : public ISystem
    {
        public:
            void update(Handler& handler, [[maybe_unused]] float dt) override
            {
                auto view = handler.query<Velocity, Rotation>();
                const auto& cameraPool = handler.getPool<Camera>();

                view.forEach([&](const EntityId id, const Velocity& vel, Rotation& rot)
                {
                    if (cameraPool.has(id))
                        return;

                    const float hSpeed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
                    if (hSpeed < 0.0001f)
                        return;

                    const glm::vec3 dir = normalize(vel);
                    rot.y = glm::degrees(std::atan2(dir.x, dir.z) + glm::pi<float>());
                });
            }
    };
}

#endif
