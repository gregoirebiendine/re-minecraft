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
    this->normals.clear();
    this->textureIndexes.clear();

    // Iterate over all Materials to construct blocks
    for (int z = 0; z < Chunk::SIZE; z++) {
        for (int y = 0; y < Chunk::SIZE; y++) {
            for (int x = 0; x < Chunk::SIZE; x++) {
                // Skip AIR
                if (chunk.isAir(x, y, z)) continue;
    
                // Retrieve Block Meta
                const Material block = chunk.getBlock(x, y, z);
                const BlockMeta& meta = world.getBlockRegistry().get(block);
                std::vector<MaterialFace> renderedFaces;
    
                // NORTH face
                if (world.isAir(cx + x, cy + y, cz + z - 1)) {
                    this->vertices.insert(this->vertices.end(), {
                        x,       y,       z,
                        x,       1 + y,   z,
                        1 + x,   1 + y,   z,

                        x,       y,       z,
                        1 + x,   1 + y,   z,
                        1 + x,   y,       z,
                    });
                    this->normals.insert(this->normals.end(), {
                        0.0f, 0.0f, -1.0f,
                        0.0f, 0.0f, -1.0f,
                        0.0f, 0.0f, -1.0f,
                        0.0f, 0.0f, -1.0f,
                        0.0f, 0.0f, -1.0f,
                        0.0f, 0.0f, -1.0f,
                    });
                    renderedFaces.push_back(MaterialFace::NORTH);
                }
    
                // SOUTH face
                if (world.isAir(cx + x, cy + y, cz + z + 1)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x,   y,       1 + z,
                        1 + x,   1 + y,   1 + z,
                        x,       1 + y,   1 + z,

                        1 + x,   y,       1 + z,
                        x,       1 + y,   1 + z,
                        x,       y,       1 + z,
                    });
                    this->normals.insert(this->normals.end(), {
                        0.0f, 0.0f, 1.0f,
                        0.0f, 0.0f, 1.0f,
                        0.0f, 0.0f, 1.0f,
                        0.0f, 0.0f, 1.0f,
                        0.0f, 0.0f, 1.0f,
                        0.0f, 0.0f, 1.0f,
                    });
                    renderedFaces.push_back(MaterialFace::SOUTH);
                }
    
                // WEST face
                if (world.isAir(cx + x - 1, cy + y, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        x,       y,       1 + z,
                        x,       1 + y,   1 + z,
                        x,       1 + y,   z,

                        x,       y,       1 + z,
                        x,       1 + y,   z,
                        x,       y,       z,
                    });
                    this->normals.insert(this->normals.end(), {
                        -1.0f, 0.0f, 0.0f,
                        -1.0f, 0.0f, 0.0f,
                        -1.0f, 0.0f, 0.0f,
                        -1.0f, 0.0f, 0.0f,
                        -1.0f, 0.0f, 0.0f,
                        -1.0f, 0.0f, 0.0f,
                    });
                    renderedFaces.push_back(MaterialFace::WEST);
                }
    
                // EAST face
                if (world.isAir(cx + x + 1, cy + y, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x,   y,       z,
                        1 + x,   1 + y,   z,
                        1 + x,   1 + y,   1 + z,

                        1 + x,   y,       z,
                        1 + x,   1 + y,   1 + z,
                        1 + x,   y,       1 + z,
                    });
                    this->normals.insert(this->normals.end(), {
                        1.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f,
                    });
                    renderedFaces.push_back(MaterialFace::EAST);
                }
    
                // UP face
                if (world.isAir(cx + x, cy + y + 1, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        x,       1 + y,   z,
                        x,       1 + y,   1 + z,
                        1 + x,   1 + y,   1 + z,

                        x,       1 + y,   z,
                        1 + x,   1 + y,   1 + z,
                        1 + x,   1 + y,   z,
                    });
                    this->normals.insert(this->normals.end(), {
                        0.0f, 1.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                    });
                    renderedFaces.push_back(MaterialFace::UP);
                }
    
                // DOWN face
                if (world.isAir(cx + x, cy + y - 1, cz + z)) {
                    this->vertices.insert(this->vertices.end(), {
                        x,       y,       z,
                        1 + x,   y,       z,
                        1 + x,   y,       1 + z,

                        x,       y,       z,
                        1 + x,   y,       1 + z,
                        x,       y,       1 + z,
                    });
                    this->normals.insert(this->normals.end(), {
                        0.0f, -1.0f, 0.0f,
                        0.0f, -1.0f, 0.0f,
                        0.0f, -1.0f, 0.0f,
                        0.0f, -1.0f, 0.0f,
                        0.0f, -1.0f, 0.0f,
                        0.0f, -1.0f, 0.0f,
                    });
                    renderedFaces.push_back(MaterialFace::DOWN);
                }

                for (const auto& face : renderedFaces)
                {
                    const auto texIndex = meta.getFaceTexture(face);

                    this->textureIndexes.insert(this->textureIndexes.end(), {
                        texIndex,
                        texIndex,
                        texIndex,
                        texIndex,
                        texIndex,
                        texIndex,
                    });
                    this->uvs.insert(this->uvs.end(), {
                        // Triangle 1
                        1.0f, 0.0f,
                        1.0f, 1.0f,
                        0.0f, 1.0f,
                        // Triangle 2
                        1.0f, 0.0f,
                        0.0f, 1.0f,
                        0.0f, 0.0f
                    });
                }
            }
        }
    }
    
    // Bind VAO
    this->VAO.bind();
    
    // Link datas to VA0 before rendering
    this->VAO.addData<GLint, GL_INT>(this->vertices,0, 3);
    this->VAO.addData<GLfloat, GL_FLOAT>(this->uvs,1, 2);
    this->VAO.addData<GLfloat, GL_FLOAT>(this->normals,2, 3);
    this->VAO.addData<GLint, GL_INT>(this->textureIndexes,3, 1);

    // Unbind VAO
    this->VAO.unbind();
}

void ChunkMesh::render() const
{
    this->VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(this->vertices.size()));
    this->VAO.unbind();
}