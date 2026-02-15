#ifndef FARFIELD_PLAYERCONTROLLER_H
#define FARFIELD_PLAYERCONTROLLER_H

#include "../GUI/GUIController/GUIController.h"
#include "Raycast.h"
#include "InputState.h"
#include "World.h"
#include "Viewport.h"
#include "Settings.h"
#include "Systems/CameraSystem.h"

class PlayerController
{
    World& world;

    GUIController gui;
    BlockId selectedBlockId;

    // Cache
    Raycast::Hit lastRaycast{};
    ECS::IEntity& playerEntity;
    ECS::CameraSystem& cameraSystem;

    void changeSelectedMaterial(Inputs::Scroll dir);
    void placeBlock(const glm::vec3& forward) const;

    public:
        explicit PlayerController(World& _world, const Font& _font, const Settings& _settings);

        void update();
        void render();
        void handleInputs(const InputState& inputs, const Viewport& viewport);

        GUIController& getGUI() { return this->gui; };
};


#endif