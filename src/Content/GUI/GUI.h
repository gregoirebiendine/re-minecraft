#ifndef RE_MINECRAFT_GUI_H
#define RE_MINECRAFT_GUI_H

#include <memory>
#include <vector>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <glad/glad.h>

#include "Camera.h"
#include "Shader.h"
#include "VAO.h"

class GUI
{
    static constexpr float SIZE = 0.012f;
    static constexpr float THICKNESS = 0.0035f;

    std::unique_ptr<Shader> shader;
    VAO CrosshairVAO;

    public:
        explicit GUI(float windowRatio);

        void renderCrosshair() const;

        static void createImGuiFrame();
        static void renderImGuiFrame(const Camera& camera, const BlockRegistry& blockRegistry);
};

#endif //RE_MINECRAFT_GUI_H