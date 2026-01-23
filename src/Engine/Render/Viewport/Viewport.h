#ifndef RE_MINECRAFT_VIEWPORT_H
#define RE_MINECRAFT_VIEWPORT_H

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi/stb_image.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "Settings.h"
#include "InputState.h"

class Viewport
{
    Settings settings;

    GLFWwindow *window{nullptr};
    float aspectRatio{};

    static void initGLFW();
    static GLFWmonitor *getMonitor();
    static const GLFWvidmode *getVideoMode();

    public:
        static constexpr double dt = 0.0166;

        void initWindow(InputState* inputs);
        void initViewport() const;
        void closeWindow() const;

        [[nodiscard]] GLFWwindow *getWindow() const;
        [[nodiscard]] bool shouldClose() const;
        static void pollEvents();
        void swapBuffers() const;

        Settings& getSettings();

        void setVSyncUsage(bool useVSync);
        [[nodiscard]] bool useVSync() const;

        [[nodiscard]] float getAspectRatio() const;

        void toggleCursor(bool isMouseCaptured) const;


};

#endif //RE_MINECRAFT_VIEWPORT_H