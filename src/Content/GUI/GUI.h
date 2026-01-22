#ifndef RE_MINECRAFT_GUI_H
#define RE_MINECRAFT_GUI_H

#pragma once

#include <vector>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Shader.h"
#include "VAO.h"
#include "OutlineVertices.h"

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
        return {this->r, this->g, this->b, this->a};
    }

    bool operator==(const DigitalColor& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

class GUI
{
    static constexpr float SIZE = 20.f;
    static constexpr float THICKNESS = 4.f;
    static constexpr float OFFSET = 3.f;

    Shader guiShader;
    Shader outlineShader;
    VAO guiVao;
    VAO outlineVao;

    glm::mat4 projectionMatrix{};
    std::vector<GuiVertex> data;

    static float toScreenSpace(float v, float minIn, float maxIn);
    static float percent(float baseValue, float percentage);
    static std::string forwardToCardinal(const glm::vec3& forwardVector);

    void createCrosshair(glm::ivec2 viewportSize);
    void createRectangle(float x, float y, float width, float height, DigitalColor color);

    public:
        explicit GUI();

        void init(glm::ivec2 viewportSize);
        void changeViewportSize(glm::ivec2 size);

        void render() const;
        void renderBlockOutline(const Camera& camera, const float& aspect, const glm::vec3& cubePos) const;

        static void createImGuiFrame();
        static void renderImGuiFrame(const Camera& camera);
};

#endif //RE_MINECRAFT_GUI_H
