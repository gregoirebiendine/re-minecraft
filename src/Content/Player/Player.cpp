#include "Player.h"

Player::Player(const BlockRegistry& blockRegistry) :
    camera({8.5f, 18.5f, 8.5f}),
    selectedMaterial(blockRegistry.getByName("core:oak_leaves"))
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
        world.setBlock(this->lastRaycast.previousPos.x, this->lastRaycast.previousPos.y, this->lastRaycast.previousPos.z, this->selectedMaterial);

    if (inputs.isMouseButtonPressed(Inputs::Mouse::MIDDLE) && this->lastRaycast.hit) {
        const Material block = world.getBlock(this->lastRaycast.pos.x, this->lastRaycast.pos.y, this->lastRaycast.pos.z);
        this->selectedMaterial = block;
    }

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
    GUI::renderImGuiFrame(this->camera);
    this->gui.render();
}

void Player::renderBlockOutline(const float& aspect) const
{
    if (this->lastRaycast.hit)
        this->gui.renderBlockOutline(this->camera, aspect, this->lastRaycast.pos);
}

void Player::setSelectedMaterial(const Material newMaterial)
{
    this->selectedMaterial = newMaterial;
}

Material Player::getSelectedMaterial() const
{
    return this->selectedMaterial;
}
