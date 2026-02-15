#ifndef FARFIELD_VIEWPORT_H
#define FARFIELD_VIEWPORT_H

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi/stb_image.h>
#include "Settings.h"
#include "InputState.h"

class Viewport
{
    static constexpr int MSAA_SAMPLES = 4;

    GLFWwindow *window{nullptr};
    Settings& settings;

    float aspectRatio{};

    // MSAA FBO
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

        explicit Viewport(Settings& _settings);

        void initWindow(InputState* inputs);
        void initViewport();
        void closeWindow();

        [[nodiscard]] GLFWwindow *getWindow() const;
        [[nodiscard]] bool shouldClose() const;
        static void pollEvents();
        void swapBuffers() const;

        // MSAA methods
        void beginFrame() const;
        void endFrame() const;

        void setVSyncUsage(bool useVSync) const;
        [[nodiscard]] bool useVSync() const;

        [[nodiscard]] float getAspectRatio() const;

        void setCursorVisibility(bool showCursor) const;


};

#endif