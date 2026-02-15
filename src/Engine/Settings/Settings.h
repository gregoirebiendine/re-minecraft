#ifndef FARFIELD_SETTINGS_H
#define FARFIELD_SETTINGS_H

#include <glm/glm.hpp>

class Settings
{
    double targetFrameRate{1.0};
    double targetFrameTime{1.0};
    double currentFps{0.0};
    bool vsync{true};

    uint8_t viewDistance{8};
    uint8_t fov{100};
    glm::ivec2 viewportSize{1600, 900};

    public:
        void setVSync(bool useVSync);
        [[nodiscard]] bool isVSync() const;

        void setFpsTarget(double target);
        [[nodiscard]] double getFpsTarget() const;
        [[nodiscard]] double getFpsFrameTime() const;
        void setCurrentFps(double _fps);
        [[nodiscard]] double getCurrentFps() const;

        void setViewportSize(glm::ivec2 size);
        [[nodiscard]] glm::ivec2 getViewportSize() const;

        void setViewDistance(uint8_t distance);
        [[nodiscard]] uint8_t getViewDistance() const;

        void setFOV(uint8_t _fov);
        [[nodiscard]] uint8_t getFOV() const;
};

#endif