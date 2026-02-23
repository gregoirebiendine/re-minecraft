#ifndef FARFIELD_RENDERSYSTEM_H
#define FARFIELD_RENDERSYSTEM_H

#include "Shader.h"
#include "EntityMeshData.h"
#include "ItemRegistry.h"
#include "ItemMeshRegistry.h"
#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/MeshRef.h"
#include "Components/Inventory.h"

namespace ECS
{
    class RenderSystem : public IRenderSystem
    {
        const ItemRegistry& itemRegistry;
        const ItemMeshRegistry& itemMeshRegistry;

        Shader shader;
        Shader itemShader;

        public:
            explicit RenderSystem(const ItemRegistry& _itemRegistry, const ItemMeshRegistry& _itemMeshRegistry) :
                shader("/resources/shaders/Entity/"),
                itemShader("/resources/shaders/Item/"),
                itemRegistry(_itemRegistry),
                itemMeshRegistry(_itemMeshRegistry)
            {
                this->shader.setTextureSamplerId(0);
                this->itemShader.setTextureSamplerId(0);
            }

            void setViewMatrix(const glm::mat4& view)
            {
                this->shader.setViewMatrix(view);
                this->itemShader.setViewMatrix(view);
            }

            void setProjectionMatrix(const glm::mat4& projection)
            {
                this->shader.setProjectionMatrix(projection);
                this->itemShader.setProjectionMatrix(projection);
            }

            void render(Handler& handler) override
            {
                auto view = handler.query<Position, Rotation, MeshRef>();
                auto& equipmentsPool = handler.getPool<Equipments>();

                view.forEach([&](const EntityId id, const Position& pos, const Rotation& rot, const MeshRef& meshRef)
                {
                    // Render right hand item
                    if (equipmentsPool.has(id)) {
                        const auto& equipment = equipmentsPool.get(id);

                        if (equipment.rightHand.stackSize > 0) {
                            const auto& item = itemRegistry.get(equipment.rightHand.itemId);
                            const auto meshData = this->itemMeshRegistry.get(item.getIdentifier().getFullIdentifier());

                            this->itemShader.use();

                            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

                            // TODO: replace with hand socket
                            model = glm::scale(model, glm::vec3{0.5f}),
                            model = glm::rotate(model, glm::radians(90.f), {0.0f, 1.0f, 0.0f});
                            model = glm::translate(model, glm::vec3(0.2f, 2.f, 0.75f));

                            this->shader.setModelMatrix(model);
                            this->shader.setUniformUInt("LayerId", item.getTextureId());

                            meshData->render();
                        }
                    }

                    if (!meshRef.mesh)
                        return;

                    // Render entity mesh
                    this->shader.use();

                    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
                    model = glm::rotate(model, glm::radians(-rot.y), {0.0f, 1.0f, 0.0f});

                    this->shader.setModelMatrix(model);
                    this->shader.setUniformUInt("LayerId", meshRef.texId);
                    meshRef.mesh->render();
                });
            }

    };
}

#endif