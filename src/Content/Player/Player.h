#ifndef RE_MINECRAFT_PLAYER_H
#define RE_MINECRAFT_PLAYER_H

#include "GUI.h"
#include "Camera.h"
#include "InputState.h"
#include "World.h"
#include "Viewport.h"

class Player
{
    static constexpr float GRAVITY = 1.125f * Viewport::dt;         // Gravity per tick
    static constexpr float JUMP_FORCE = 0.1875f;                    // Initial jump velocity
    static constexpr float ACCELERATION = 2.f * Viewport::dt;       // Horizontal acceleration
    static constexpr float MAX_SPEED = 4.f * Viewport::dt;          // Max horizontal speed
    static constexpr float FRICTION = 0.80f;                        // Horizontal drag (80%)

    World& world;

    glm::vec3 position;
    glm::vec3 velocity{};
    glm::vec3 inputDirection{};

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

        void update();
        void render() const;
        void handleInputs(const InputState& inputs, const Viewport& viewport, double deltaTime);
        void renderBlockOutline(const float& aspect) const;

        void setSelectedBlockId(Material id);

        void applyHorizontalMovement();
        void setPosition(glm::vec3 _pos);
};


#endif //RE_MINECRAFT_PLAYER_H