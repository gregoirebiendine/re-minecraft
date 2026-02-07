#ifndef RE_MINECRAFT_MOVEMENTSYSTEM_H
#define RE_MINECRAFT_MOVEMENTSYSTEM_H

#include <ostream>
#include <glm/glm.hpp>
#include "ECS/System.h"

namespace ECS
{
    struct Position : glm::vec3
    {
        using glm::vec3::vec3;

        friend std::ostream& operator<<(std::ostream& os, const Position& pos)
        {
            return os << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
        }
    };

    struct Velocity : glm::vec3
    {
        using glm::vec3::vec3;

        friend std::ostream& operator<<(std::ostream& os, const Velocity& vel)
        {
            return os << "(" << vel.x << ", " << vel.y << ", " << vel.z << ")";
        }
    };

    class MovementSystem : public ISystem
    {
        public:
            void update(Handler& handler, float dt) override
            {
                auto view = handler.query<Position, Velocity>();

                view.forEach([dt]([[maybe_unused]] EntityId id, Position& pos, const Velocity& vel)
                {
                    pos += vel * dt;
                });
            }
    };
}

#endif