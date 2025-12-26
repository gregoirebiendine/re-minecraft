#ifndef RE_MINECRAFT_CHUNKMESH_H
#define RE_MINECRAFT_CHUNKMESH_H


#include <glad/glad.h>
#include <vector>

#include "Chunk.h"
#include "CubeVAO.h"

class ChunkMesh
{
    public:
        ChunkMesh() = default;
        ~ChunkMesh() = default;

        void rebuild(Chunk& chunk);
        void render() const;

    private:
        CubeVAO VAO;
        std::vector<GLuint> vertices;
        std::vector<GLfloat> uvs;
};


#endif //RE_MINECRAFT_CHUNKMESH_H