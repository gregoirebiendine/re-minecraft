#ifndef RE_MINECRAFT_RENDERSYSTEM_H
#define RE_MINECRAFT_RENDERSYSTEM_H

#include "Shader.h"
#include "EntityMeshData.h"
#include "ECS/ISystem.h"
#include "Components/MeshRefComponent.h"

namespace ECS
{
    class RenderSystem : public IRenderSystem
    {
        Shader shader;

        public:
            RenderSystem() :
                shader("/resources/shaders/Entity/")
            {}

            void setViewMatrix(const glm::mat4& view)
            {
                this->shader.use();
                this->shader.setViewMatrix(view);
            }

            void setProjectionMatrix(const glm::mat4& projection)
            {
                this->shader.use();
                this->shader.setProjectionMatrix(projection);
            }

            void render(Handler& handler) override
            {
                auto view = handler.query<Position, MeshRefComponent>();

                this->shader.use();
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