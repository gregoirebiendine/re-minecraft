#include "Player.h"

Player::Player(const BlockRegistry& _blockRegistry) :
    blockRegistry(_blockRegistry),
    camera({8.5f, 80.5f, 8.5f}),
    selectedBlockId(_blockRegistry.getByName("core:oak_leaves"))
{
}

Camera& Player::getCamera()
{
    return this->camera;
}

GUI& Player::getGUI()
{
    return this->gui;
}

void Player::handleInputs(const InputState& inputs, const Viewport& viewport, World& world, const double deltaTime)
{
    this->camera.moveCamera(inputs.mouseX, inputs.mouseY, deltaTime);
    this->lastRaycast = this->camera.raycast(world);

    if (inputs.isMouseButtonPressed(Inputs::Mouse::LEFT) && this->lastRaycast.hit)
        world.setBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z, "core:air");

    if (inputs.isMouseButtonPressed(Inputs::Mouse::RIGHT) && this->lastRaycast.hit)
        this->placeBlock(world);

    if (inputs.isMouseButtonPressed(Inputs::Mouse::MIDDLE) && this->lastRaycast.hit)
    {
        const Material mat = world.getBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z);
        this->selectedBlockId = BlockData::getBlockId(mat);
    }


    if (inputs.scroll != Inputs::Scroll::NONE)
        this->changeSelectedMaterial(inputs.scroll);

    if (inputs.isKeyPressed(Inputs::Keys::SPACE)) {
        viewport.toggleCursor(!this->camera.getMouseCapture());
        this->camera.toggleMouseCapture();
    }

    if (inputs.isKeyDown(Inputs::Keys::W))
        this->camera.move({0,0,1}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::S))
        this->camera.move({0,0,-1}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::A))
        this->camera.move({-1,0,0}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::D))
        this->camera.move({1,0,0}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::Q))
        this->camera.move({0,1,0}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::E))
        this->camera.move({0,-1,0}, static_cast<float>(deltaTime));
}

void Player::render() const
{
    GUI::createImGuiFrame();
    GUI::renderImGuiFrame(this->camera, this->blockRegistry.get(this->selectedBlockId).getName());
    this->gui.render();
}

void Player::renderBlockOutline(const float& aspect) const
{
    if (this->lastRaycast.hit)
        this->gui.renderBlockOutline(this->camera, aspect, this->lastRaycast.pos);
}

void Player::setSelectedBlockId(const BlockId id)
{
    this->selectedBlockId = id;
}

void Player::changeSelectedMaterial(const Inputs::Scroll dir)
{
    const auto allBlocks = this->blockRegistry.getAll();

    this->selectedBlockId = (this->selectedBlockId + dir + allBlocks.size()) % allBlocks.size();
    if (this->selectedBlockId == 0)
        this->selectedBlockId = (this->selectedBlockId + dir + allBlocks.size()) % allBlocks.size();
}

void Player::placeBlock(World& world) const
{
    const auto& selectedBlockMeta = this->blockRegistry.get(this->selectedBlockId);
    BlockRotation rotation = 0;

    switch (selectedBlockMeta.rotation) {
        case RotationType::NONE:
            rotation = 0;
            break;

        case RotationType::HORIZONTAL:
        {
            const auto playerFacing = DirectionUtils::getHorizontalFacing(
                this->camera.getForwardVector()
            );
            rotation = DirectionUtils::getOppositeFacing(playerFacing);
        }
            break;

        case RotationType::AXIS:
            rotation = DirectionUtils::getAxisFromHitFace(this->lastRaycast.hitFace);
            break;
    }

    const auto packedMaterial = BlockData::packBlockData(selectedBlockId, rotation);
    world.setBlock(this->lastRaycast.previousPos.x, this->lastRaycast.previousPos.y, this->lastRaycast.previousPos.z, packedMaterial);
}