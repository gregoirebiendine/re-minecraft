#ifndef RE_MINECRAFT_PLAYERINPUTSYSTEM_H
#define RE_MINECRAFT_PLAYERINPUTSYSTEM_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "InputState.h"
#include "ECS/ISystem.h"
#include "Components/CollisionBox.h"
#include "Components/PlayerInput.h"

namespace ECS
{
    class PlayerInputSystem : public ISystem
    {
        const InputState& inputs;

        public:
            explicit PlayerInputSystem(const InputState& _inputs) : inputs(_inputs) {}

            void update(Handler &handler, [[maybe_unused]] float deltaTime) override
            {
                auto view = handler.query<PlayerInput, CollisionBox>();

                view.forEach([&]([[maybe_unused]] EntityId id, PlayerInput& playerInput, [[maybe_unused]] const CollisionBox& box)
                {
                    // Horizontal movement
                    playerInput.direction = {0.f, 0.f, 0.f};
                    if (inputs.isKeyDown(Inputs::Keys::W)) playerInput.direction.z += 1.f;
                    if (inputs.isKeyDown(Inputs::Keys::S)) playerInput.direction.z -= 1.f;
                    if (inputs.isKeyDown(Inputs::Keys::A)) playerInput.direction.x -= 1.f;
                    if (inputs.isKeyDown(Inputs::Keys::D)) playerInput.direction.x += 1.f;

                    // Jump - buffer the input (use keyDown, not keyPressed,
                    // because keyPressed is a one-shot that can be missed
                    // on frames where the fixed timestep doesn't tick)
                    if (inputs.isKeyDown(Inputs::Keys::SPACE))
                        playerInput.jumpBufferFrames = 5;

                    if (playerInput.jumpBufferFrames > 0)
                        playerInput.jumpBufferFrames--;
                });
            }
    };
}

#endif
