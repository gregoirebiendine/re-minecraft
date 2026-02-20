#ifndef FARFIELD_DEBUGAABBSYSTEM_H
#define FARFIELD_DEBUGAABBSYSTEM_H

// =============================================================================
// DEBUG - CollisionBox AABB outline renderer
// Remove this entire file and its references in World.cpp to disable debug AABB
// =============================================================================

#include "Shader.h"
#include "VAO.h"
#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/CollisionBox.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace ECS
{
    class DebugAABBSystem : public IRenderSystem
    {
        Shader shader;
        VAO vao;
        int vertexCount = 0;

        // DEBUG - Build a unit cube wireframe outline (12 edges as thin quads)
        // The cube goes from (0,0,0) to (1,1,1) and is scaled/translated per entity
        static std::vector<GLfloat> buildUnitCubeOutline()
        {
            constexpr float T = 0.005f; // half-thickness of the edge lines

            // 12 edges of a unit cube, each defined by two endpoints
            struct Edge { glm::vec3 a, b; };
            constexpr Edge edges[12] = {
                // Bottom face edges (y=0)
                {{0,0,0}, {1,0,0}},
                {{1,0,0}, {1,0,1}},
                {{1,0,1}, {0,0,1}},
                {{0,0,1}, {0,0,0}},
                // Top face edges (y=1)
                {{0,1,0}, {1,1,0}},
                {{1,1,0}, {1,1,1}},
                {{1,1,1}, {0,1,1}},
                {{0,1,1}, {0,1,0}},
                // Vertical edges
                {{0,0,0}, {0,1,0}},
                {{1,0,0}, {1,1,0}},
                {{1,0,1}, {1,1,1}},
                {{0,0,1}, {0,1,1}},
            };

            std::vector<GLfloat> verts;
            verts.reserve(12 * 6 * 6 * 3); // 12 edges * 6 faces per edge box * 6 verts per face * 3 floats

            for (const auto& [a, b] : edges)
            {
                // Direction along the edge
                const glm::vec3 dir = b - a;

                // Build a thin box around the edge
                // We expand by T in the two axes perpendicular to the edge direction
                glm::vec3 minP = glm::min(a, b) - glm::vec3(T);
                glm::vec3 maxP = glm::max(a, b) + glm::vec3(T);

                // Keep the edge direction axis tight (no extra thickness along it)
                if (dir.x != 0.0f) { minP.x = a.x - T; maxP.x = b.x + T; }
                if (dir.y != 0.0f) { minP.y = a.y - T; maxP.y = b.y + T; }
                if (dir.z != 0.0f) { minP.z = a.z - T; maxP.z = b.z + T; }

                const float x0 = minP.x, y0 = minP.y, z0 = minP.z;
                const float x1 = maxP.x, y1 = maxP.y, z1 = maxP.z;

                // 6 faces of the thin box (2 triangles each = 6 vertices each)
                // Front (z1)
                verts.insert(verts.end(), {x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y0,z1, x1,y1,z1, x0,y1,z1});
                // Back (z0)
                verts.insert(verts.end(), {x0,y0,z0, x0,y1,z0, x1,y1,z0, x0,y0,z0, x1,y1,z0, x1,y0,z0});
                // Top (y1)
                verts.insert(verts.end(), {x0,y1,z0, x0,y1,z1, x1,y1,z1, x0,y1,z0, x1,y1,z1, x1,y1,z0});
                // Bottom (y0)
                verts.insert(verts.end(), {x0,y0,z0, x1,y0,z0, x1,y0,z1, x0,y0,z0, x1,y0,z1, x0,y0,z1});
                // Right (x1)
                verts.insert(verts.end(), {x1,y0,z0, x1,y1,z0, x1,y1,z1, x1,y0,z0, x1,y1,z1, x1,y0,z1});
                // Left (x0)
                verts.insert(verts.end(), {x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y0,z0, x0,y1,z1, x0,y1,z0});
            }

            return verts;
        }

    public:
        // DEBUG - Reuses the Outline shader (simple MVP + solid color)
        DebugAABBSystem() :
            shader("/resources/shaders/Outline/")
        {
            const auto vertices = buildUnitCubeOutline();
            this->vertexCount = static_cast<int>(vertices.size()) / 3;

            this->vao.bind();
            this->vao.storeOutlineData(vertices);
            this->vao.unbind();
        }

        // DEBUG - Call from World::update() to pass camera matrices
        void setViewMatrix(const glm::mat4& view)
        {
            this->shader.setViewMatrix(view);
        }

        void setProjectionMatrix(const glm::mat4& projection)
        {
            this->shader.setProjectionMatrix(projection);
        }

        // DEBUG - Renders AABB wireframe for every entity with Position + CollisionBox
        void render(Handler& handler) override
        {
            auto view = handler.query<Position, CollisionBox>();

            this->shader.use();

            glDisable(GL_CULL_FACE);
            glPolygonOffset(-1, -1);

            this->vao.bind();

            view.forEach([&]([[maybe_unused]] EntityId id, const Position& pos, const CollisionBox& box)
            {
                const glm::vec3& h = box.halfExtents;

                // DEBUG - Compute AABB size: width = 2*halfExt.x, height = 2*halfExt.y, depth = 2*halfExt.z
                // Position is at feet, Y extends upward by halfExt.y * 2
                glm::vec3 aabbMin = { pos.x - h.x, pos.y,            pos.z - h.z };
                glm::vec3 aabbSize = { h.x * 2.0f,  h.y * 2.0f,      h.z * 2.0f };

                glm::mat4 model = glm::translate(glm::mat4(1.0f), aabbMin);
                model = glm::scale(model, aabbSize);

                this->shader.setModelMatrix(model);

                glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
            });

            this->vao.unbind();

            glPolygonOffset(0, 0);
            glEnable(GL_CULL_FACE);
        }
    };
}

#endif
