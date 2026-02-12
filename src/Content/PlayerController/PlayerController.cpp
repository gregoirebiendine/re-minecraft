#include "PlayerController.h"

#include "Systems/CameraSystem.h"
#include "Components/Movements.h"
#include "Components/Camera.h"

PlayerController::PlayerController(World& _world, const Font& _font) :
    world(_world),
    gui(_font),
    selectedBlockId(_world.getBlockRegistry().getByName("core:oak_plank"))
{}

void PlayerController::renderGUI()
{
    const auto& playerEntity = this->world.getPlayerEntity();
    const auto& pos = this->world.getECS().getComponent<ECS::Position>(playerEntity);
    const auto& camera = this->world.getECS().getComponent<ECS::Camera>(playerEntity);
    const auto& cameraSystem = this->world.getECSScheduler().getSystem<ECS::CameraSystem>();

    this->gui.render(pos, ECS::CameraSystem::getForwardVector(camera), world.getBlockRegistry().get(selectedBlockId).getName());

    if (this->lastRaycast.hasHitBlock())
        this->gui.renderBlockOutline(cameraSystem.getViewMatrix(), cameraSystem.getProjectionMatrix(), this->lastRaycast.pos);
}

void PlayerController::handleInputs(const InputState& inputs, const Viewport& viewport)
{
    const auto& playerEntity = this->world.getPlayerEntity();
    const auto& pos = this->world.getECS().getComponent<ECS::Position>(playerEntity);
    const auto& camera = this->world.getECS().getComponent<ECS::Camera>(playerEntity);
    auto& cameraSystem = this->world.getECSScheduler().getSystem<ECS::CameraSystem>();
    const auto& forward = ECS::CameraSystem::getForwardVector(camera);

    this->lastRaycast = Raycast::cast(this->world, pos + camera.eyeOffset, forward);

    // Left Mouse Button (Break block)
    if (inputs.isMouseButtonPressed(Inputs::Mouse::LEFT) && this->lastRaycast.hasHitBlock())
        this->world.setBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z, "core:air");

    // Right Mouse Button (Place block)
    if (inputs.isMouseButtonPressed(Inputs::Mouse::RIGHT) && this->lastRaycast.hasHitBlock() && !this->world.isEntityAt(this->lastRaycast.previousPos))
        this->placeBlock(forward);

    // Middle Mouse Button (Get targeted block in inventory)
    if (inputs.isMouseButtonPressed(Inputs::Mouse::MIDDLE) && this->lastRaycast.hasHitBlock())
    {
        const Material mat = world.getBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z);
        this->selectedBlockId = BlockData::getBlockId(mat);
    }

    // Scroll
    if (inputs.scroll != Inputs::Scroll::NONE)
        this->changeSelectedMaterial(inputs.scroll);

    // Toggle mouse capture
    if (inputs.isKeyPressed(Inputs::Keys::P)) {
        const bool isCaptured = cameraSystem.isMouseCaptured();
        viewport.setCursorVisibility(isCaptured);
        cameraSystem.setMouseCaptured(!isCaptured);
    }

    // FUN TEST
    if (inputs.isKeyPressed(Inputs::Keys::E)) {
        auto& vel = this->world.getECS().getComponent<ECS::Velocity>(this->world.getPlayerEntity());
        vel.x = 10.f;
        vel.y = 0.5f;
        vel.z = 10.f;
    }
}

void PlayerController::setSelectedBlockId(const BlockId id)
{
    this->selectedBlockId = id;
}

void PlayerController::changeSelectedMaterial(const Inputs::Scroll dir)
{
    const auto allBlocks = this->world.getBlockRegistry().getAll();

    this->selectedBlockId = (this->selectedBlockId + std::to_underlying(dir) + allBlocks.size()) % allBlocks.size();
    if (this->selectedBlockId == 0)
        this->selectedBlockId = (this->selectedBlockId + std::to_underlying(dir) + allBlocks.size()) % allBlocks.size();
}

void PlayerController::placeBlock(const glm::vec3& forward) const
{
    const auto& selectedBlockMeta = this->world.getBlockRegistry().get(this->selectedBlockId);
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

    const Material packedMaterial = BlockData::packBlockData(selectedBlockId, rotation);
    world.setBlock(this->lastRaycast.previousPos.x, this->lastRaycast.previousPos.y, this->lastRaycast.previousPos.z, packedMaterial);
}
