#ifndef FARFIELD_GUI_H
#define FARFIELD_GUI_H

#pragma once

#include <vector>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "VAO.h"
#include "Font.h"
#include "Settings.h"
#include "TextureRegistry.h"
#include "DirectionUtils.h"

struct DigitalColor
{
    float r;
    float g;
    float b;
    float a;

    DigitalColor(const uint8_t r, const uint8_t g, const uint8_t b, const float a)
    {
        this->r = static_cast<float>(r) / 255.f;
        this->g = static_cast<float>(g) / 255.f;
        this->b = static_cast<float>(b) / 255.f;
        this->a = a;
    }

    operator glm::vec4() const
    {
        return glm::vec4{this->r, this->g, this->b, this->a};
    }

    bool operator==(const DigitalColor& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

class GUI
{
    const Font& font;
    const TextureRegistry& textureRegistry;
    Settings& settings;

    Shader guiShader;
    Shader outlineShader;
    VAO guiVao;
    VAO outlineVao;

    const TextureId& fontTexId;
    std::vector<GuiVertex> data;

    static float toScreenSpace(float v, float minIn, float maxIn);
    static float percent(float baseValue, float percentage);

    void createRectangle(float x, float y, float width, float height, DigitalColor color);
    void createText(float x, float y, const std::string& text);
    void createImage(float x, float y, const std::string& image);

    static void createImGuiFrame();
    static void renderImGuiFrame(glm::vec3 pos, glm::vec3 forward, const std::string& selectedBlockName);

    public:
        explicit GUI(const Font& _font, const TextureRegistry& _textureRegistry, Settings& _settings);

        [[nodiscard]] glm::mat4 getGUIProjectionMatrix() const;

        void render(const glm::vec3& pos, const glm::vec3& forward, const std::string& selectedBlockName);
        void renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos);
};

#endif
