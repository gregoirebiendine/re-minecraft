#ifndef RE_MINECRAFT_GUI_H
#define RE_MINECRAFT_GUI_H

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "Camera.h"

class GUI
{
    public:
        static void createImGuiFrame();
        static void renderImGuiFrame(const Camera& camera, const BlockRegistry& blockRegistry);
};

#endif //RE_MINECRAFT_GUI_H