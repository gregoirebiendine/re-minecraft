#ifndef RE_MINECRAFT_PLAYER_H
#define RE_MINECRAFT_PLAYER_H

#include "GUI.h"
#include "Camera.h"
#include "BlockRegistry.h"
#include "InputState.h"
#include "World.h"
#include "Viewport.h"

class Player
{
    GUI gui;
    Camera camera;
    Material selectedMaterial;
    Raycast::Hit lastRaycast{};

    public:
        explicit Player(const BlockRegistry& blockRegistry);

        Camera &getCamera();
        GUI& getGUI();

        void handleInputs(const InputState& inputs, Viewport& viewport, World& world, double deltaTime);
        void render() const;
        void renderBlockOutline(const float& aspect) const;

        void setSelectedMaterial(Material newMaterial);
        [[nodiscard]] Material getSelectedMaterial() const;
};


#endif //RE_MINECRAFT_PLAYER_H