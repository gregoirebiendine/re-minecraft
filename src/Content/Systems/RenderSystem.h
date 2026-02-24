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
        const EntityId& playerId;

        Shader shader;
        Shader itemShader;

        bool renderRightHandItem(const EntityId& id, const ItemStack& rightHandStack, const Position& pos, const Rotation& rot)
        {
            if (rightHandStack.stackSize == 0)
                return false;

            const auto& item = this->itemRegistry.get(rightHandStack.itemId);
            const auto itemMesh = this->itemMeshRegistry.get(item.getIdentifier().getFullIdentifier());

            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            model = glm::rotate(model, glm::radians(-rot.y + 90.0f), {0.0f, 1.0f, 0.0f});    // yaw
            model = glm::translate(model, {0.0f, 1.75f, 0.0f});                                          // up to eye
            model = glm::rotate(model, glm::radians(-rot.x), {0.0f, 0.0f, 1.0f});                   // pitch around eye
            model = glm::translate(model, {-0.35f, -0.25f, -0.4f});                                      // hand offset from eye
            model = glm::scale(model, glm::vec3{0.5f});

            this->itemShader.use();
            this->itemShader.setModelMatrix(model);
            this->itemShader.setUniformUInt("LayerId", item.getTextureId());

            glDisable(GL_CULL_FACE);
            itemMesh->render();
            glEnable(GL_CULL_FACE);

            return id == this->playerId;
        }

        public:
            explicit RenderSystem(const ItemRegistry& _itemRegistry, const ItemMeshRegistry& _itemMeshRegistry, const EntityId& _playerId) :
                itemRegistry(_itemRegistry),
                itemMeshRegistry(_itemMeshRegistry),
                playerId(_playerId),
                shader("Entity/"),
                itemShader("Item/")
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
                const auto& equipmentsPool = handler.getPool<Equipments>();
                const auto& hotbarPool = handler.getPool<Hotbar>();

                view.forEach([&](const EntityId id, const Position& pos, const Rotation& rot, const MeshRef& meshRef)
                {
                    ItemStack rightHandStack;

                    if (hotbarPool.has(id)) {
                        const auto&[items, selectedSlot] = hotbarPool.get(id);
                        rightHandStack = items[selectedSlot];
                    }
                    else if (equipmentsPool.has(id)) {
                        const auto&[armor, rightHand] = equipmentsPool.get(id);
                        rightHandStack = rightHand;
                    }

                    // TODO: set RightArm bone not visible instead of skipping
                    // Render right hand item & skip base mesh render if player is holding an item
                    if (this->renderRightHandItem(id, rightHandStack, pos, rot))
                        return;

                    // Skip if mesh doesn't exist
                    if (!meshRef.mesh)
                        return;

                    // Render entity mesh
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
                    model = glm::rotate(model, glm::radians(-rot.y), {0.0f, 1.0f, 0.0f});

                    this->shader.use();
                    this->shader.setModelMatrix(model);
                    this->shader.setUniformUInt("LayerId", meshRef.texId);
                    meshRef.mesh->render();
                });
            }

    };
}

#endif