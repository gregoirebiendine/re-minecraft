#include "Frustum.h"

void Frustum::update(const glm::mat4& vpMatrix)
{
    // Left plane
    planes[0] = glm::vec4(
          vpMatrix[0][3] + vpMatrix[0][0],
          vpMatrix[1][3] + vpMatrix[1][0],
          vpMatrix[2][3] + vpMatrix[2][0],
          vpMatrix[3][3] + vpMatrix[3][0]
      );

    // Right plane
    planes[1] = glm::vec4(
        vpMatrix[0][3] - vpMatrix[0][0],
        vpMatrix[1][3] - vpMatrix[1][0],
        vpMatrix[2][3] - vpMatrix[2][0],
        vpMatrix[3][3] - vpMatrix[3][0]
    );

    // Bottom plane
    planes[2] = glm::vec4(
        vpMatrix[0][3] + vpMatrix[0][1],
        vpMatrix[1][3] + vpMatrix[1][1],
        vpMatrix[2][3] + vpMatrix[2][1],
        vpMatrix[3][3] + vpMatrix[3][1]
    );

    // Top plane
    planes[3] = glm::vec4(
        vpMatrix[0][3] - vpMatrix[0][1],
        vpMatrix[1][3] - vpMatrix[1][1],
        vpMatrix[2][3] - vpMatrix[2][1],
        vpMatrix[3][3] - vpMatrix[3][1]
    );

    // Near plane
    planes[4] = glm::vec4(
        vpMatrix[0][3] + vpMatrix[0][2],
        vpMatrix[1][3] + vpMatrix[1][2],
        vpMatrix[2][3] + vpMatrix[2][2],
        vpMatrix[3][3] + vpMatrix[3][2]
    );

    // Far plane
    planes[5] = glm::vec4(
        vpMatrix[0][3] - vpMatrix[0][2],
        vpMatrix[1][3] - vpMatrix[1][2],
        vpMatrix[2][3] - vpMatrix[2][2],
        vpMatrix[3][3] - vpMatrix[3][2]
    );

    for (auto& plane : planes) {
        const float length = glm::length(glm::vec3(plane));
        plane /= length;
    }
}

bool Frustum::isBoxVisible(const glm::vec3& min, const glm::vec3& max) const
{
    for (const auto& plane : planes) {
        glm::vec3 p;
        p.x = (plane.x >= 0) ? max.x : min.x;
        p.y = (plane.y >= 0) ? max.y : min.y;
        p.z = (plane.z >= 0) ? max.z : min.z;

        if (glm::dot(glm::vec3(plane), p) + plane.w < 0)
            return false;
    }
    return true;
}