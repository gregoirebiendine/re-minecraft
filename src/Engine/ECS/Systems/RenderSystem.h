#ifndef RE_MINECRAFT_RENDERSYSTEM_H
#define RE_MINECRAFT_RENDERSYSTEM_H

#include "EntityMeshData.h"
#include "Shader.h"
#include "ECS/System.h"
#include "ECS/Systems/MovementSystem.h"

namespace ECS
{
    struct MeshRefComponent
    {
        std::shared_ptr<EntityMeshData> mesh;
    };

    class RenderSystem : public ISystem
    {
        Shader shader;

        public:
            RenderSystem() :
                shader(
                "../resources/shaders/Entity/entity.vert",
                "../resources/shaders/Entity/entity.frag"
                )
            {}

            void setViewMatrix(const glm::mat4& view) const
            {
                this->shader.use();
                this->shader.setViewMatrix(view);
            }

            void setProjectionMatrix(const glm::mat4& projection) const
            {
                shader.use();
                shader.setProjectionMatrix(projection);
            }

            void update([[maybe_unused]] Handler& handler, [[maybe_unused]] float dt) override
            {
            }

            void render(Handler& handler) const
            {
                this->shader.use();

                auto view = handler.query<Position, MeshRefComponent>();

                view.forEach([&]([[maybe_unused]] EntityId id, const Position& pos, const MeshRefComponent& ref)
                {
                    if (!ref.mesh)
                        return;

                    const glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

                    this->shader.setModelMatrix(model);

                    ref.mesh->render();
                });
            }

    };
}

#endif