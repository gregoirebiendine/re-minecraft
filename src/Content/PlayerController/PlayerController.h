#ifndef FARFIELD_PLAYERCONTROLLER_H
#define FARFIELD_PLAYERCONTROLLER_H

#include "GUI.h"
#include "Raycast.h"
#include "InputState.h"
#include "World.h"
#include "Viewport.h"

class PlayerController
{
    World& world;

    GUI gui;
    BlockId selectedBlockId;
    Raycast::Hit lastRaycast{};

    void changeSelectedMaterial(Inputs::Scroll dir);
    void placeBlock(const glm::vec3& forward) const;

    public:
        explicit PlayerController(World& _world, const Font& _font);

        void renderGUI();
        void handleInputs(const InputState& inputs, const Viewport& viewport);
        void setSelectedBlockId(Material id);

        GUI& getGUI() { return this->gui; };
};


#endif