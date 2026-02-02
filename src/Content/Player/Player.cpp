#include "Player.h"

Player::Player(World& _world) :
    world(_world),
    position(8.5f, 80.5f, 8.5f),
    camera(position),
    selectedBlockId(_world.getBlockRegistry().getByName("core:oak_leaves"))
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

void Player::update(const double deltaTime)
{
    // Raycast below player to detect blocks
    const auto downRaycast = Raycast::cast(this->world, this->position, {0, -1, 0}, 2.f);

    if (downRaycast.hit)
        return;

    glm::vec3 pos{this->position};

    pos.y = pos.y - 0.1f * static_cast<float>(deltaTime);
    this->position = pos;
    this->camera.setPosition(pos);

}

void Player::render() const
{
    GUI::createImGuiFrame();
    GUI::renderImGuiFrame(this->camera, this->world.getBlockRegistry().get(this->selectedBlockId).getName());
    this->gui.render();
}

void Player::handleInputs(const InputState& inputs, const Viewport& viewport, const double deltaTime)
{
    this->camera.moveCamera(inputs.mouseX, inputs.mouseY, deltaTime);
    this->lastRaycast = Raycast::cast(this->world, this->camera.getPosition(), this->camera.getForwardVector());

    if (inputs.isMouseButtonPressed(Inputs::Mouse::LEFT) && this->lastRaycast.hit)
        this->world.setBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z, "core:air");

    if (inputs.isMouseButtonPressed(Inputs::Mouse::RIGHT) && this->lastRaycast.hit)
        this->placeBlock();

    if (inputs.isMouseButtonPressed(Inputs::Mouse::MIDDLE) && this->lastRaycast.hit)
    {
        const Material mat = world.getBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z);
        this->selectedBlockId = BlockData::getBlockId(mat);
    }

    if (inputs.scroll != Inputs::Scroll::NONE)
        this->changeSelectedMaterial(inputs.scroll);

    if (inputs.isKeyPressed(Inputs::Keys::P)) {
        viewport.toggleCursor(!this->camera.getMouseCapture());
        this->camera.toggleMouseCapture();
    }

    if (inputs.isKeyDown(Inputs::Keys::W))
        this->move({0,0,1}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::S))
        this->move({0,0,-1}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::A))
        this->move({-1,0,0}, static_cast<float>(deltaTime));
    if (inputs.isKeyDown(Inputs::Keys::D))
        this->move({1,0,0}, static_cast<float>(deltaTime));

    // if (inputs.isKeyDown(Inputs::Keys::Q))
    //     this->camera.move({0,1,0}, static_cast<float>(deltaTime));
    // if (inputs.isKeyDown(Inputs::Keys::E))
    //     this->camera.move({0,-1,0}, static_cast<float>(deltaTime));
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
    const auto allBlocks = this->world.getBlockRegistry().getAll();

    this->selectedBlockId = (this->selectedBlockId + std::to_underlying(dir) + allBlocks.size()) % allBlocks.size();
    if (this->selectedBlockId == 0)
        this->selectedBlockId = (this->selectedBlockId + std::to_underlying(dir) + allBlocks.size()) % allBlocks.size();
}

void Player::placeBlock() const
{
    const auto& selectedBlockMeta = this->world.getBlockRegistry().get(this->selectedBlockId);
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

void Player::move(const glm::vec3 direction, const float deltaTime)
{
    constexpr float MAX_SPEED = 8.f;
    const glm::vec3 forward = this->camera.getForwardVector();
    const glm::vec3 linearForward = glm::normalize(glm::vec3{forward.x, 0, forward.z});
    const glm::vec3 right = glm::normalize(glm::cross(linearForward, {0,1,0}));

    glm::vec3 tmpPos{this->position};

    if (direction.x != 0)
        tmpPos += direction.x * right * MAX_SPEED * deltaTime;
    if (direction.y != 0)
        tmpPos += direction.y * glm::vec3{0, 1, 0} * MAX_SPEED * deltaTime;
    if (direction.z != 0)
        tmpPos += direction.z * linearForward * MAX_SPEED * deltaTime;

    this->setPosition(tmpPos);
}

void Player::setPosition(const glm::vec3 _pos)
{
    this->position = _pos;
    this->camera.setPosition(_pos);
}
