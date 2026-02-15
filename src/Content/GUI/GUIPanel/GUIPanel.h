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
#include "Font.h"
// #include "Settings.h"
#include "Viewport.h"
#include "TextureRegistry.h"
#include "PanelWidget.h"
#include "ImageWidget.h"
#include "ShapeWidget.h"
#include "TextWidget.h"
#include "ChunkPos.h"
#include "DirectionUtils.h"
#include "RGBA.h"

class GUIPanel
{
    // Classes from Engine
    const Font& font;
    const TextureRegistry& textureRegistry;
    const Viewport& viewport;

    // Render storage
    Shader shader;
    VAO vao;
    std::vector<GuiVertex> vertexBuffer;

    // Widget references
    std::unique_ptr<PanelWidget> root;
    ShapeWidget* crosshair = nullptr;
    ImageWidget* hotbar = nullptr;
    ImageWidget* hotbarSelection = nullptr;
    PanelWidget* debugPanel = nullptr;

    // Viewport tracking
    glm::ivec2 cachedViewportSize{0};

    // Outside variables cache
    glm::vec3 currentPos{0.f};
    glm::vec3 currentForward{0.f, 0.f, -1.f};
    std::string currentSelectedBlock;

    // Rebuild UI every frame
    void rebuildVertexBuffer();

    // Events
    void onViewportResize(glm::ivec2 newSize) const;

    // Calculate GUI Projection Matrix for shader
    [[nodiscard]] glm::mat4 getGUIProjectionMatrix() const;

    public:
        explicit GUIPanel(const Font& _font, const TextureRegistry& _textureRegistry, const Viewport& _viewport);

        void toggleDebugPanel() const;

        void update(const glm::vec3& pos, const glm::vec3& forward, const std::string& selectedBlockName);
        void render();
};


#endif