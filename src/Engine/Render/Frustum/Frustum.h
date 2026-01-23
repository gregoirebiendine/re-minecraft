#ifndef RE_MINECRAFT_FRUSTUM_H
#define RE_MINECRAFT_FRUSTUM_H

#include <array>

#include <glm/glm.hpp>

class Frustum {
    std::array<glm::vec4, 6> planes;

    public:
        void update(const glm::mat4& vpMatrix);
        [[nodiscard]] bool isBoxVisible(const glm::vec3& min, const glm::vec3& max) const;
};

#endif //RE_MINECRAFT_FRUSTUM_H