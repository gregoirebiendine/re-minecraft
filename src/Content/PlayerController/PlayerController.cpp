#include "PlayerController.h"

#include "Systems/CameraSystem.h"
#include "Components/Movements.h"
#include "Components/Camera.h"
#include "Components/Inventory.h"

PlayerController::PlayerController(World& _world, const MsdfFont& _font, const Viewport& _viewport) :
    world(_world),
    gui(_font, _viewport, this->world.getRegistries().get<TextureRegistry>(), this->world.getRegistries().get<ItemRegistry>()),
    playerEntity(this->world.getPlayerEntity()),
    cameraSystem(this->world.getECSScheduler().getSystem<ECS::CameraSystem>()),
    hotbarComponent(this->world.getECS().getComponent<ECS::Hotbar>(this->playerEntity))
{}

void PlayerController::update()
{
    const auto& pos = this->world.getECS().getComponent<ECS::Position>(this->playerEntity);
    const auto& camera = this->world.getECS().getComponent<ECS::Camera>(this->playerEntity);
    const auto& hotbarInv = this->world.getECS().getComponent<ECS::Hotbar>(this->playerEntity);

    this->gui.update(pos, ECS::CameraSystem::getForwardVector(camera), hotbarInv);
}

void PlayerController::render()
{
    this->gui.render();

    if (this->lastRaycast.hasHitBlock())
        this->gui.renderBlockOutline(this->cameraSystem.getViewMatrix(), this->cameraSystem.getProjectionMatrix(), this->lastRaycast.pos);
}

void PlayerController::handleInputs(const InputState& inputs, Viewport& viewport)
{
    const auto& pos = this->world.getECS().getComponent<ECS::Position>(this->playerEntity);
    const auto& camera = this->world.getECS().getComponent<ECS::Camera>(this->playerEntity);
    const auto& forward = ECS::CameraSystem::getForwardVector(camera);

    this->lastRaycast = Raycast::cast(this->world, pos + camera.eyeOffset, forward);

    // Left Mouse Button (Break block)
    if (inputs.isMouseButtonPressed(Inputs::Mouse::LEFT) && this->lastRaycast.hasHitBlock())
        this->world.setBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z, "core:air");

    // Right Mouse Button (Place block)
    if (inputs.isMouseButtonPressed(Inputs::Mouse::RIGHT) && this->lastRaycast.hasHitBlock() && !this->world.isEntityAt(this->lastRaycast.previousPos))
        this->placeBlock(forward);

    // Middle Mouse Button (Get targeted block in inventory)
    if (inputs.isMouseButtonPressed(Inputs::Mouse::MIDDLE) && this->lastRaycast.hasHitBlock()) {
        // const Material mat = this->world.getBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z);
        // this->hotbarComponent.items[this->selectedSlot].item = mat;
        // this->hotbarComponent.items[this->selectedSlot].stackSize = 1;
    }

    // Scroll
    if (inputs.scroll != Inputs::Scroll::NONE) {
        this->selectedSlot = std::clamp(0, this->selectedSlot + std::to_underlying(inputs.scroll), 8);
        this->gui.onHotbarSlotChanged(this->selectedSlot);
    }

    // Toggle mouse capture
    if (inputs.isKeyPressed(Inputs::Keys::P)) {
        const bool isCaptured = this->cameraSystem.isMouseCaptured();
        viewport.setCursorVisibility(isCaptured);
        this->cameraSystem.setMouseCaptured(!isCaptured);
    }

    // Toggle debug screen (F3)
    if (inputs.isKeyPressed(Inputs::Keys::F3))
        this->getGUI().toggleDebugPanel();

    // Toggle fullscreen (F11)
    if (inputs.isKeyPressed(Inputs::Keys::F11))
        viewport.toggleFullscreen();
}

void PlayerController::placeBlock(const glm::vec3& forward) const
{
    // const auto&[items] = this->world.getECS().getComponent<ECS::Hotbar>(this->world.getPlayerEntity());
    // const auto&[amount, item] = items[selectedSlot];
    //
    // if (amount == 0)
    //     return;

    const auto& blockId = this->world.getRegistries().get<BlockRegistry>().getByName("core:grass");
    const auto& selectedBlockMeta = this->world.getRegistries().get<BlockRegistry>().get(blockId);
    BlockRotation rotation = 0;

    switch (selectedBlockMeta.rotation) {
        case RotationType::NONE:
            rotation = 0;
            break;

        case RotationType::HORIZONTAL:
            {
                const auto playerFacing = DirectionUtils::getHorizontalFacing(forward);
                rotation = DirectionUtils::getOppositeFacing(playerFacing);
            }
            break;

        case RotationType::AXIS:
            rotation = DirectionUtils::getAxisFromHitFace(this->lastRaycast.hitFace);
            break;
    }

    const Material packedMaterial = Material::pack(blockId, rotation);
    this->world.setBlock(this->lastRaycast.previousPos.x, this->lastRaycast.previousPos.y, this->lastRaycast.previousPos.z, packedMaterial);
}
