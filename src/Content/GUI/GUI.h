#ifndef RE_MINECRAFT_GUI_H
#define RE_MINECRAFT_GUI_H

#pragma once

class Engine; // forward declaration

#include <memory>
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

struct DigitalColor
{
    float r;
    float g;
    float b;
    float a;

    bool operator==(const DigitalColor& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

class GUI
{
    static constexpr float SIZE = 26.f;
    static constexpr float THICKNESS = 4.f;

    std::unique_ptr<Shader> shader;

    std::vector<GLfloat> vertices{};
    VAO VAO;

    static float toScreenSpace(float v, float minIn, float maxIn);
    static float percent(float baseValue, float percentage);
    static DigitalColor color(uint8_t r, uint8_t g, uint8_t b, float a);

    void createCrosshair();
    void createRectangle(float x, float y, float width, float height, DigitalColor color);

    public:
        explicit GUI(float windowRatio);

        void render() const;

        static void createImGuiFrame();
        static void renderImGuiFrame(const Camera& camera, const BlockRegistry& blockRegistry);
};

std::vector<GLfloat> test(float x, float y);

#endif //RE_MINECRAFT_GUI_H