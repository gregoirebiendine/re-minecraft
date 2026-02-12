#ifndef FARFIELD_RENDERSYSTEM_H
#define FARFIELD_RENDERSYSTEM_H

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
                auto& rotationPool = handler.getPool<Rotation>();

                this->shader.use();
                view.forEach([&]([[maybe_unused]] EntityId id, const Position& pos, const MeshRef& ref)
                {
                    if (!ref.mesh)
                        return;

                    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

                    if (const auto* rot = rotationPool.tryGet(id)) {
                        model = glm::rotate(model, glm::radians(-rot->y - 90.f), glm::vec3(0.f, 1.f, 0.f));

                        // TODO: Remove debug rotation
                        model = glm::translate(model, {0.375, 1.75f, 0.0});
                        model = glm::rotate(model, glm::radians(rot->x + 15.f), glm::vec3(1.f, 0.f, 0.f));
                        model = glm::translate(model, {-0.375, -1.75f, 0.0});
                    }

                    this->shader.setModelMatrix(model);
                    this->shader.setUniformUInt("LayerId", ref.texId);
                    ref.mesh->render();
                });
            }

    };
}

#endif