#include "Chunk.h"

std::vector<GLfloat> getUVFromIndex(const char index)
{
    const char tiles = 4;
    glm::vec2 UV(float(index%tiles)/tiles, 1.0f - ((std::round(index/tiles) + 1.0f) * (1.0f/tiles)));

    return {
            UV.x, UV.y,                                         // Left lower
            UV.x + (1.0f / tiles), UV.y,                        // Right lower
            UV.x + (1.0f / tiles), UV.y + (1.0f / tiles),       // Right upper
            UV.x, UV.y,                                         // Left lower
            UV.x + (1.0f / tiles), UV.y + (1.0f / tiles),       // Right upper
            UV.x, UV.y + (1.0f / tiles),                        // Left upper
    };
}

Chunk::Chunk()
{
    this->vertices = {
            // Front face
            -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,

            // Left face
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

            // Right face
            1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,

            // Back face
            1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,

            // Top face
            -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

            // Bottom face
            -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f
    };

    const char BlockFaces[6] = {1, 1, 1, 1, 2, 0};

    for (char faceIndex : BlockFaces) {
        auto faceUVs = getUVFromIndex(faceIndex);
        this->uv.insert(this->uv.end(), faceUVs.begin(), faceUVs.end());
    }

    // Bind VAO
    this->VAO.bind();

    // Create VAA and VBO with vertices and UV data
    this->VAO.linkVertices(this->vertices);
    this->VAO.linkUV(this->uv);

    this->VAO.unbind();
}

void Chunk::bind() const {
    this->VAO.bind();
}

void Chunk::draw() const
{
    this->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
