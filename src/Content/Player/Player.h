#ifndef RE_MINECRAFT_PLAYER_H
#define RE_MINECRAFT_PLAYER_H

#include "Camera.h"
#include "GUI.h"
#include "BlockRegistry.h"

class Player
{
    Camera camera;
    GUI gui;
    Material selectedMaterial;

    public:
        explicit Player(const BlockRegistry& blockRegistry);

        Camera &getCamera();
        void render() const;
        void renderBlockOutline(const float& aspect, const glm::vec3& cubePos) const;

        void setSelectedMaterial(Material newMaterial);
        [[nodiscard]] Material getSelectedMaterial() const;
};


#endif //RE_MINECRAFT_PLAYER_H