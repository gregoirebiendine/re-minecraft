#ifndef FARFIELD_SETTINGS_H
#define FARFIELD_SETTINGS_H

#include <glm/glm.hpp>

class Settings
{
    // FPS
    double targetFrameRate{1.0};
    double targetFrameTime{1.0};
    double currentFps{0.0};
    bool vsync{true};

    // Camera settings
    uint8_t viewDistance{8};
    uint8_t fov{100};

    // Window settings
    bool fullscreen{false};

    public:
        void useVSync(bool use);
        [[nodiscard]] bool isUsingVSync() const;

        void setFpsTarget(double target);
        [[nodiscard]] double getFpsTarget() const;
        [[nodiscard]] double getFpsFrameTime() const;

        void setCurrentFps(double _fps);
        [[nodiscard]] double getCurrentFps() const;

        void setViewDistance(uint8_t distance);
        [[nodiscard]] uint8_t getViewDistance() const;

        void setFOV(uint8_t _fov);
        [[nodiscard]] uint8_t getFOV() const;

        void setFullscreen(bool full);
        [[nodiscard]] bool isFullscreen() const;
};

#endif