#ifndef RE_MINECRAFT_RENDERSYSTEM_H
#define RE_MINECRAFT_RENDERSYSTEM_H

#include "Shader.h"
#include "EntityMeshData.h"
#include "ECS/ISystem.h"
#include "Components/MeshRef.h"

namespace ECS
{
    class RenderSystem : public IRenderSystem
    {
        Shader shader;

        public:
            RenderSystem() :
                shader("/resources/shaders/Entity/")
            {
                this->shader.use();
                this->shader.setUniformInt("Textures", 0);
            }

            void setViewMatrix(const glm::mat4& view)
            {
                this->shader.setViewMatrix(view);
            }

            void setProjectionMatrix(const glm::mat4& projection)
            {
                this->shader.setProjectionMatrix(projection);
            }

            void render(Handler& handler) override
            {
                auto view = handler.query<Position, MeshRef>();

                this->shader.use();
                view.forEach([&]([[maybe_unused]] EntityId id, const Position& pos, const MeshRef& ref)
                {
                    if (!ref.mesh)
                        return;

                    const glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
                    this->shader.setModelMatrix(model);
                    this->shader.setUniformUInt("LayerId", ref.texId);
                    ref.mesh->render();
                });
            }

    };
}

#endif