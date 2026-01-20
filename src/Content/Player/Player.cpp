#include "Player.h"

Player::Player(const BlockRegistry& blockRegistry) :
    camera({8.5f, 18.5f, 8.5f}),
    selectedMaterial(blockRegistry.getByName("core:oak_plank"))
{
}

Camera& Player::getCamera()
{
    return this->camera;
}

void Player::render() const
{
    GUI::createImGuiFrame();
    GUI::renderImGuiFrame(this->camera);
    this->gui.render();
}

void Player::setSelectedMaterial(const Material newMaterial)
{
    this->selectedMaterial = newMaterial;
}

Material Player::getSelectedMaterial() const
{
    return this->selectedMaterial;
}
