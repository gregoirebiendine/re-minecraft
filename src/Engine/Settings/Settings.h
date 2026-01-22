#ifndef RE_MINECRAFT_SETTINGS_H
#define RE_MINECRAFT_SETTINGS_H

#include <glm/glm.hpp>

class Settings
{
    double fps{120.0};
    double targetFrameTime{1.0 / fps};

    bool vsync{true};
    glm::ivec2 viewportSize{1600, 900};

    public:
        void setVSync(bool useVSync);
        [[nodiscard]] bool isVSync() const;

        void setFpsTarget(double target);
        [[nodiscard]] double getFpsTarget() const;
        [[nodiscard]] double getFpsFrameTime() const;

        void setViewportSize(glm::ivec2 size);
        [[nodiscard]] glm::ivec2 getViewportSize() const;
};

#endif //RE_MINECRAFT_SETTINGS_H