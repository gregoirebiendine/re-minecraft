#ifndef RE_MINECRAFT_PLAYER_H
#define RE_MINECRAFT_PLAYER_H

#include "GUI.h"
#include "Camera.h"
#include "InputState.h"
#include "World.h"
#include "Viewport.h"

class Player
{
    World& world;

    glm::vec3 position;
    glm::vec3 velocity;

    GUI gui;
    Camera camera;

    BlockId selectedBlockId;
    Raycast::Hit lastRaycast{};

    void changeSelectedMaterial(Inputs::Scroll dir);
    void placeBlock() const;

    public:
        explicit Player(World& _world);

        Camera &getCamera();
        GUI& getGUI();

        void update(double deltaTime);
        void render() const;
        void handleInputs(const InputState& inputs, const Viewport& viewport, double deltaTime);
        void renderBlockOutline(const float& aspect) const;

        void setSelectedBlockId(Material id);

        void move(glm::vec3 direction, float deltaTime);
        void setPosition(glm::vec3 _pos);
};


#endif //RE_MINECRAFT_PLAYER_H