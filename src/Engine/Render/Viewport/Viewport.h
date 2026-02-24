#ifndef FARFIELD_VIEWPORT_H
#define FARFIELD_VIEWPORT_H

#include <iostream>
#include <unordered_map>
#include <ranges>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "FrameTimer.h"
#include "Settings.h"
#include "InputState.h"

const std::unordered_map<int, int> SCREEN_SIZES = {
    {4320, 7680},
    {2160, 3840},
    {1440, 2560},
    {1080, 1920},
    {720, 1280},
    {480, 854}
};

class Viewport
{
    GLFWwindow *window{nullptr};
    Settings& settings;
    FrameTimer frameTimer;

    // Windows state
    glm::ivec2 size{1280, 720};
    glm::ivec2 lastSize{size};
    float aspectRatio{16.f / 9.f};
    double currentFps{60.0};

    static void initGLFW();
    static GLFWmonitor *getMonitor();
    static const GLFWvidmode *getVideoMode();

    void initWindow(InputState* inputs);
    void initViewport() const;

    static glm::ivec2 getClosestResolution(const GLFWvidmode* videoMode);
    void centerWindow(const GLFWvidmode* videoMode) const;

    public:
        static constexpr double dt = 1.f / 60.f; // 60Hz

        explicit Viewport(Settings& _settings, InputState* inputs);

        void closeWindow() const;

        [[nodiscard]] Settings& getSettings() const { return this->settings; }
        [[nodiscard]] GLFWwindow* getWindow() const { return this->window; }

        static void pollEvents();
        void swapBuffers() const;
        [[nodiscard]] bool shouldClose() const;

        // Resize update
        void toggleFullscreen();
        void setSize(glm::ivec2 _size, bool modifyViewport = true);
        [[nodiscard]] glm::ivec2 getSize() const;

        // VSync
        void useVSync(bool use) const;
        [[nodiscard]] bool isUsingVSync() const;

        // Aspect ratio
        void setAspectRatio(const float aspect) { this->aspectRatio = aspect; }
        [[nodiscard]] float getAspectRatio() const { return this->aspectRatio; }

        // Current viewport FPS
        void updateFrameTimer(const double _dt) { this->frameTimer.update(_dt); }
        [[nodiscard]] const FrameTimer& getFrameTimer() const { return this->frameTimer; }

        void setCursorVisibility(bool showCursor) const;
};

#endif