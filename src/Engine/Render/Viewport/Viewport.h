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

    // MSAA FBO
    static constexpr int MSAA_SAMPLES = 4;
    GLuint msaaFBO{0};
    GLuint msaaColorBuffer{0};
    GLuint msaaDepthBuffer{0};
    int fbWidth{0}, fbHeight{0};

    static void initGLFW();
    static GLFWmonitor *getMonitor();
    static const GLFWvidmode *getVideoMode();

    void createMSAABuffers();
    void deleteMSAABuffers();

    public:
        static constexpr double dt = 1.f / 60.f; // 60Hz

        void initWindow(InputState* inputs);
        void initViewport();
        void closeWindow();

        [[nodiscard]] GLFWwindow *getWindow() const;
        [[nodiscard]] bool shouldClose() const;
        static void pollEvents();
        void swapBuffers() const;

        // MSAA methods
        void beginFrame();
        void endFrame();

        Settings& getSettings();

        void setVSyncUsage(bool useVSync);
        [[nodiscard]] bool useVSync() const;

        [[nodiscard]] float getAspectRatio() const;

        void toggleCursor(bool isMouseCaptured) const;


};

#endif //RE_MINECRAFT_VIEWPORT_H