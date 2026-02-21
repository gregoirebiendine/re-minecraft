#ifndef FARFIELD_GUIPANEL_H
#define FARFIELD_GUIPANEL_H

#pragma once

#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "VAO.h"
#include "MsdfFont.h"
#include "Viewport.h"
#include "TextureRegistry.h"
#include "ItemRegistry.h"
#include "PanelWidget.h"
#include "ImageWidget.h"
#include "ShapeWidget.h"
#include "TextWidget.h"
#include "ChunkPos.h"
#include "DirectionUtils.h"
#include "RGBA.h"
#include "Components/Inventory.h"

class GUIPanel
{
    // Classes from Engine
    const MsdfFont& font;
    const Viewport& viewport;
    const TextureRegistry& textureRegistry;
    const ItemRegistry& itemRegistry;

    // Render storage
    VAO vao;
    Shader shader;
    std::vector<GuiVertex> vertexBuffer;

    VAO fontVao;
    Shader fontShader;
    std::vector<MSDFVertex> fontBuffer;

    // Widget references
    std::unique_ptr<PanelWidget> root;
    ShapeWidget* crosshair = nullptr;
    PanelWidget* debugPanel = nullptr;
    ImageWidget* hotbar = nullptr;
    ImageWidget* hotbarSelection = nullptr;

    // Viewport tracking
    glm::ivec2 cachedViewportSize{0};

    // Outside variables cache
    glm::vec3 currentPos{0.f};
    glm::vec3 currentForward{0.f, 0.f, -1.f};
    ECS::Hotbar hotbarInventory{};

    // Rebuild UI every frame
    void rebuildVertexBuffer();

    // Calculate GUI Projection Matrix for shader
    [[nodiscard]] glm::mat4 getGUIProjectionMatrix() const;

    public:
        explicit GUIPanel(const MsdfFont& _font, const Viewport& _viewport, const TextureRegistry& _textureRegistry, const ItemRegistry& _itemRegistry);

        // Events
        void toggleDebugPanel() const;
        void onViewportResize(glm::ivec2 newSize) const;
        void onHotbarSlotChanged(int slot) const;

        void update(const glm::vec3& pos, const glm::vec3& forward, const ECS::Hotbar& hotbarInv);
        void render();
};


#endif