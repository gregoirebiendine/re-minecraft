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

void Player::update()
{
    const auto downRaycast = Raycast::cast(this->world, this->position, {0, -1, 0}, 2.1f);
    const bool isGrounded = downRaycast.hit;

    if (!isGrounded)
    {
        this->velocity.y -= GRAVITY;                                        // Gravity
        this->velocity.y = glm::max(this->velocity.y, -(GRAVITY * 8));  // Terminal velocity
    } else {
        if (this->velocity.y < 0.f) {
            this->velocity.y = 0.f;
            this->position.y = static_cast<float>(downRaycast.previousPos.y + 2);  // Snap to top of block
        }

        if (this->inputDirection.y > 0.f)
            this->velocity.y = JUMP_FORCE;
    }

    this->inputDirection.y = 0.f;
    this->applyHorizontalMovement();

    this->position += this->velocity;
    this->camera.setPosition(this->position);
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

    // Left Mouse Button
    if (inputs.isMouseButtonPressed(Inputs::Mouse::LEFT) && this->lastRaycast.hit)
        this->world.setBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z, "core:air");

    // Right Mouse Button
    if (inputs.isMouseButtonPressed(Inputs::Mouse::RIGHT) && this->lastRaycast.hit)
        this->placeBlock();

    // Middle Mouse Button
    if (inputs.isMouseButtonPressed(Inputs::Mouse::MIDDLE) && this->lastRaycast.hit)
    {
        const Material mat = world.getBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z);
        this->selectedBlockId = BlockData::getBlockId(mat);
    }

    // Scroll
    if (inputs.scroll != Inputs::Scroll::NONE)
        this->changeSelectedMaterial(inputs.scroll);

    // Toggle mouse capture
    if (inputs.isKeyPressed(Inputs::Keys::P)) {
        viewport.toggleCursor(!this->camera.getMouseCapture());
        this->camera.toggleMouseCapture();
    }

    // Horizontal movement
    this->inputDirection.x = 0.f;
    this->inputDirection.z = 0.f;
    if (inputs.isKeyDown(Inputs::Keys::W)) this->inputDirection.z += 1.f;
    if (inputs.isKeyDown(Inputs::Keys::S)) this->inputDirection.z -= 1.f;
    if (inputs.isKeyDown(Inputs::Keys::A)) this->inputDirection.x -= 1.f;
    if (inputs.isKeyDown(Inputs::Keys::D)) this->inputDirection.x += 1.f;

    // Jump
    if (inputs.isKeyPressed(Inputs::Keys::SPACE)) this->inputDirection.y = 1.f;
}

void Player::applyHorizontalMovement()
{
    const glm::vec3 forward = this->camera.getForwardVector();
    const glm::vec3 linearForward = glm::normalize(glm::vec3{forward.x, 0, forward.z});
    const glm::vec3 right = glm::cross(linearForward, {0,1,0});

    glm::vec3 wishDir = right * this->inputDirection.x + linearForward * this->inputDirection.z;
    if (glm::length(wishDir) > 0.f)
        wishDir = glm::normalize(wishDir);

    // Apply acceleration
    this->velocity.x += wishDir.x * ACCELERATION;
    this->velocity.z += wishDir.z * ACCELERATION;

    // Apply friction
    this->velocity.x *= FRICTION;
    this->velocity.z *= FRICTION;

    // Clamp to max speed
    glm::vec2 horizontalVel{this->velocity.x, this->velocity.z};
    if (glm::length(horizontalVel) > MAX_SPEED) {
        horizontalVel = glm::normalize(horizontalVel) * MAX_SPEED;
        this->velocity.x = horizontalVel.x;
        this->velocity.z = horizontalVel.y;
    }
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

void Player::setPosition(const glm::vec3 _pos)
{
    this->position = _pos;
    this->camera.setPosition(_pos);
}
