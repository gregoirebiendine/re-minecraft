#include "ChunkMesh.h"
#include "World.h"

void ChunkMesh::rebuild(Chunk& chunk, const World& world)
{
    const auto [cx, cy, cz] = chunk.getPosition() * Chunk::SIZE;

    // Set chunk as not dirty, meaning it will not rebuild next frame
    chunk.setDirty(false);

    // Reset old vertices and uvs
    this->vertices.clear();
    this->uvs.clear();

    // Iterate over all Materials to construct blocks
    for (int z = 0; z < Chunk::SIZE; z++) {
        for (int y = 0; y < Chunk::SIZE; y++) {
            for (int x = 0; x < Chunk::SIZE; x++) {
                // Skip AIR
                if (chunk.isAir(x, y, z)) continue;
    
                // Retrieve Material atlas indexes
                const Material block = chunk.getBlock(x, y, z);
                const MaterialAtlasFaces blockTexFaces = MaterialTexFaces[block];
                std::vector<MaterialFace> renderedFaces;
    
                // Front face (0)
                if (world.isAir(cx + x, cy + y, cz + z + 1)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x, y, 1 + z,
                        x, y, 1 + z,
                        x, 1 + y, 1 + z,
                        1 + x, y, 1 + z,
                        x, 1 + y, 1 + z,
                        1 + x, 1 + y, 1 + z,
                    });
                    renderedFaces.push_back(MaterialFace::FRONT);
                }
    
                // Back face (1)
                if (world.isAir(cx + x, cy + y, cz + z - 1)) {
                    this->vertices.insert(this->vertices.end(), {
                        x, y, z,
                        1 + x, y, z,
                        1 + x, 1 + y, z,
                        x, y, z,
                        1 + x, 1 + y, z,
                        x, 1 + y, z,
                    });
                    renderedFaces.push_back(MaterialFace::BACK);
                }
    
                // Left face (2)
                if (world.isAir(cx + x - 1, cy + y, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        x, y, 1 + z,
                        x, y, z,
                        x, 1 + y, z,
                        x, y, 1 + z,
                        x, 1 + y, z,
                        x, 1 + y, 1 + z,
                    });
                    renderedFaces.push_back(MaterialFace::LEFT);
                }
    
                // Right face (3)
                if (world.isAir(cx + x + 1, cy + y, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x, y, z,
                        1 + x, y, 1 + z,
                        1 + x, 1 + y, 1 + z,
                        1 + x, y, z,
                        1 + x, 1 + y, 1 + z,
                        1 + x, 1 + y, z,
                    });
                    renderedFaces.push_back(MaterialFace::RIGHT);
                }
    
                // Top face (4)
                if (world.isAir(cx + x, cy + y + 1, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        x, 1 + y, z,
                        1 + x, 1 + y, z,
                        1 + x, 1 + y, 1 + z,
                        x, 1 + y, z,
                        1 + x, 1 + y, 1 + z,
                        x, 1 + y, 1 + z,
                    });
                    renderedFaces.push_back(MaterialFace::TOP);
                }
    
                // Bottom face (5)
                if (world.isAir(cx + x, cy + y - 1, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        x, y, z,
                        x, y, 1 + z,
                        1 + x, y, 1 + z,
                        x, y, z,
                        1 + x, y, 1 + z,
                        1 + x, y, z,
                    });
                    renderedFaces.push_back(MaterialFace::BOTTOM);
                }
    
                // Add tex offset based on rendered faces
                for (const auto &face : renderedFaces) {
                    const auto t = glm::vec2(0.25f * static_cast<float>(blockTexFaces[face] % 4), floor(static_cast<double>(blockTexFaces[face]) / 4) / 4);
                    this->uvs.insert(this->uvs.end(), {
                        t.x, t.y,     0.25f + t.x, t.y,    0.25f + t.x, 0.25f + t.y,   t.x, t.y,     0.25f + t.x, 0.25f + t.y,   t.x, 0.25f + t.y,
                    });
                }
            }
        }
    }
    
    // Bind VAO
    this->VAO.bind();
    
    // Link datas to VA0 before rendering
    this->VAO.linkVertices(this->vertices);
    this->VAO.linkUvs(this->uvs);
    
    // Unbind VAO
    this->VAO.unbind();
}

void ChunkMesh::render() const
{
    this->VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(this->vertices.size()));
    this->VAO.unbind();
}
