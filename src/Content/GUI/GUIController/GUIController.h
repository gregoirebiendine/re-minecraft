#ifndef FARFIELD_GUI_H
#define FARFIELD_GUI_H

#pragma once

#include <glm/glm.hpp>

#include "Shader.h"
#include "VAO.h"
#include "MsdfFont.h"
#include "Viewport.h"
#include "TextureRegistry.h"
#include "ItemRegistry.h"
#include "GUIPanel.h"
#include "Components/Inventory.h"

class GUIController
{
    GUIPanel panel;

    Shader shader;
    VAO vao;

    public:
        explicit GUIController(const MsdfFont& _font, const TextureRegistry& _textureRegistry, const ItemRegistry& _itemRegistry, const Viewport& _viewport);

        void onHotbarSlotChanged(int slot) const;
        void toggleDebugPanel() const;

        void update(const glm::vec3& pos, const glm::vec3& forward, const ECS::Hotbar& hotbarInv);
        void render();
        void renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos);
};

#endif
