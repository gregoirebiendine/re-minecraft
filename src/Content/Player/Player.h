#ifndef RE_MINECRAFT_PLAYER_H
#define RE_MINECRAFT_PLAYER_H

#include "GUI.h"
#include "Raycast.h"
#include "Camera.h"
#include "InputState.h"
#include "World.h"
#include "Viewport.h"
#include "CollisionBox.h"

class Player
{
    static constexpr float GRAVITY = 0.015f;        // Gravity per tick
    static constexpr float JUMP_FORCE = 0.18f;      // Initial jump velocity
    static constexpr float ACCELERATION = 0.04f;    // Horizontal acceleration
    static constexpr float MAX_SPEED = 0.075f;      // Max horizontal speed
    static constexpr float FRICTION = 0.8f;         // Horizontal drag (80%)

    static constexpr float EYE_LEVEL = 1.75f;       // Blocks above feet
    static constexpr int JUMP_BUFFER = 5;           // Frames to remember jump input

    World& world;

    glm::vec3 position{};
    glm::vec3 velocity{};
    glm::vec3 inputDirection{};

    GUI gui;
    Camera camera;
    CollisionBox collisionBox;

    BlockId selectedBlockId;
    Raycast::Hit lastRaycast{};

    bool isGrounded = false;
    int jumpBufferFrames = 0;

    void changeSelectedMaterial(Inputs::Scroll dir);
    void placeBlock() const;
    void updateCameraPosition();

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