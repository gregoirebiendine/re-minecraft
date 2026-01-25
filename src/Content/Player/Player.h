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
    const BlockRegistry& blockRegistry;

    GUI gui;
    Camera camera;

    BlockId selectedBlockId;
    Raycast::Hit lastRaycast{};

    void changeSelectedMaterial(Inputs::Scroll dir);

    public:
        explicit Player(const BlockRegistry& _blockRegistry);

        Camera &getCamera();
        GUI& getGUI();

        void handleInputs(const InputState& inputs, const Viewport& viewport, World& world, double deltaTime);
        void render() const;
        void renderBlockOutline(const float& aspect) const;

        void setSelectedBlockId(Material id);

        void placeBlock(World& world) const;
};


#endif //RE_MINECRAFT_PLAYER_H