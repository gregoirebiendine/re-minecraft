#ifndef FARFIELD_GUI_H
#define FARFIELD_GUI_H

#pragma once

#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "VAO.h"
#include "Font.h"
#include "ChunkPos.h"
#include "DirectionUtils.h"
#include "Settings.h"
#include "TextureRegistry.h"
#include "RGBA.h"

#include "PanelWidget.h"
#include "ImageWidget.h"

class GUI
{
    const Font& font;
    const TextureRegistry& textureRegistry;
    Settings& settings;

    Shader guiShader;
    Shader outlineShader;
    VAO guiVao;
    VAO outlineVao;
    std::vector<GuiVertex> vertexBuffer;

    std::unique_ptr<PanelWidget> root;

    ImageWidget* hotbarSelection = nullptr;
    PanelWidget* debugPanel = nullptr;

    glm::vec3 currentPos{0.f};
    glm::vec3 currentForward{0.f, 0.f, -1.f};
    std::string currentSelectedBlock;

    void rebuildVertexBuffer();

    public:
        explicit GUI(const Font& _font, const TextureRegistry& _textureRegistry, Settings& _settings);

        [[nodiscard]] glm::mat4 getGUIProjectionMatrix() const;

        void onHotbarSlotChanged(int slot) const;
        void toggleDebugPanel() const;

        void render(const glm::vec3& pos, const glm::vec3& forward, const std::string& selectedBlockName);
        void renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos);
};

#endif
