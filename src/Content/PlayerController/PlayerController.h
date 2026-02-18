#ifndef FARFIELD_PLAYERCONTROLLER_H
#define FARFIELD_PLAYERCONTROLLER_H

#include "Raycast.h"
#include "InputState.h"
#include "World.h"
#include "Viewport.h"
#include "GUIController.h"
#include "Systems/CameraSystem.h"

class PlayerController
{
    World& world;

    GUIController gui;
    int selectedSlot = 0;

    // Cache
    Raycast::Hit lastRaycast{};
    ECS::IEntity& playerEntity;
    ECS::CameraSystem& cameraSystem;
    ECS::Hotbar& hotbarComponent;

    void placeBlock(const glm::vec3& forward) const;

    public:
        explicit PlayerController(World& _world, const Font& _font, const Viewport& _viewport);

        void update();
        void render();
        void handleInputs(const InputState& inputs, Viewport& viewport);

        GUIController& getGUI() { return this->gui; };
};


#endif