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

    // Windows state
    const glm::ivec2 baseSize{1600, 900};
    glm::ivec2 size{baseSize};
    float aspectRatio{static_cast<float>(size.x) / static_cast<float>(size.y)};

    // MSAA FBO
    GLuint msaaFBO{0};
    GLuint msaaColorBuffer{0};
    GLuint msaaDepthBuffer{0};
    int fbWidth{0}, fbHeight{0};

    static void initGLFW();
    static GLFWmonitor *getMonitor();
    static const GLFWvidmode *getVideoMode();

    void centerWindow(const GLFWvidmode* videoMode) const;

    void createMSAABuffers();
    void deleteMSAABuffers();

    public:
        static constexpr double dt = 1.f / 60.f; // 60Hz

        explicit Viewport(Settings& _settings);

        void initWindow(InputState* inputs);
        void initViewport() const;
        void closeWindow() const;

        [[nodiscard]] Settings& getSettings() const { return this->settings; };
        [[nodiscard]] GLFWwindow* getWindow() const { return this->window; };

        [[nodiscard]] bool shouldClose() const;
        static void pollEvents();
        void swapBuffers() const;

        // Resize update
        void setSize(glm::ivec2 _size);
        [[nodiscard]] glm::ivec2 getSize() const;

        // MSAA methods
        void beginFrame() const;
        void endFrame() const;

        void useVSync(bool use) const;
        [[nodiscard]] bool isUsingVSync() const;

        void setAspectRatio(float aspect);
        [[nodiscard]] float getAspectRatio() const;

        void setCursorVisibility(bool showCursor) const;

        void toggleFullscreen();
};

#endif