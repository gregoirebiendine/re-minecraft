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
#include "RGBA.h"
#include "Settings.h"
#include "TextureRegistry.h"
#include "PanelWidget.h"
#include "ImageWidget.h"
#include "GUIPanel.h"

class GUIController
{
    GUIPanel panel;

    Shader shader;
    VAO vao;

    public:
        explicit GUIController(const Font& _font, const TextureRegistry& _textureRegistry, const Settings& _settings);

        void onHotbarSlotChanged(int slot) const;
        void toggleDebugPanel() const;

        void update(const glm::vec3& pos, const glm::vec3& forward, const std::string& selectedBlockName);
        void render();
        void renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos);
};

#endif
