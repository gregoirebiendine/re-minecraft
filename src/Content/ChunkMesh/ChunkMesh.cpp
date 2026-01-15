#include "ChunkMesh.h"
#include "World.h"

ChunkMesh::ChunkMesh(const ChunkPos& cp) :
    position(cp)
{
    this->meshData.reserve(sizeof(Vertex) * Chunk::VOLUME);
}

void ChunkMesh::rebuild(Chunk& chunk, const World& world)
{
    const auto [cx, cy, cz] = chunk.getPosition() * Chunk::SIZE;

    // Reset meshes data
    this->meshData.clear();
    this->meshData.reserve(36 * Chunk::VOLUME);

    // Iterate over all Materials to construct blocks
    for (int i = 0; i < Chunk::VOLUME; i++) {
        auto [x, y, z] = coords(i);

        // Skip AIR
        if (chunk.isAir(x, y, z)) continue;

        // Retrieve Block Meta
        const BlockMeta& meta = world.getBlockRegistry().get(chunk.getBlock(x, y, z));

        // NORTH face
        if (world.isAir(cx + x, cy + y, cz + z - 1)) {
            const TextureId texId = world.getTextureRegistry().getByName(meta.getFaceTexture(NORTH));

            createFace(
                this->meshData,
                {x,       y,       z},
                {x,       1 + y,   z},
                {1 + x,   1 + y,   z},
                {1 + x,   y,       z},
                {0, 0, -1},
                texId
            );
        }

        // SOUTH face
        if (world.isAir(cx + x, cy + y, cz + z + 1)) {
            const TextureId texId = world.getTextureRegistry().getByName(meta.getFaceTexture(SOUTH));

            createFace(
                this->meshData,
                {1 + x,   y,       1 + z},
                {1 + x,   1 + y,   1 + z},
                {x,       1 + y,   1 + z},
                {x,       y,       1 + z},
                {0, 0, 1},
                texId
            );
        }

        // WEST face
        if (world.isAir(cx + x - 1, cy + y, cz + z)) {
            const TextureId texId = world.getTextureRegistry().getByName(meta.getFaceTexture(WEST));

            createFace(
                this->meshData,
                {x,       y,       1 + z},
                {x,       1 + y,   1 + z},
                {x,       1 + y,   z},
                {x,       y,       z},
                {-1, 0, 0},
                texId
            );
        }

        // EAST face
        if (world.isAir(cx + x + 1, cy + y, cz + z)) {
            const TextureId texId = world.getTextureRegistry().getByName(meta.getFaceTexture(EAST));

            createFace(
                this->meshData,
                {1 + x,   y,       z},
                {1 + x,   1 + y,   z},
                {1 + x,   1 + y,   1 + z},
                {1 + x,   y,       1 + z},
                {1, 0, 0},
                texId
            );
        }

        // UP face
        if (world.isAir(cx + x, cy + y + 1, cz + z)) {
            const TextureId texId = world.getTextureRegistry().getByName(meta.getFaceTexture(UP));

            createFace(
                this->meshData,
                {x,       1 + y,   z},
                {x,       1 + y,   1 + z},
                {1 + x,   1 + y,   1 + z},
                {1 + x,   1 + y,   z},
                {0, 1, 0},
                texId
            );
        }

        // DOWN face
        if (world.isAir(cx + x, cy + y - 1, cz + z)) {
            const TextureId texId = world.getTextureRegistry().getByName(meta.getFaceTexture(DOWN));

            createFace(
                this->meshData,
                {x,       y,       z},
                {1 + x,   y,       z},
                {1 + x,   y,       1 + z},
                {x,       y,       1 + z},
                {0, -1, 0},
                texId
            );
        }
    }
    
    // Link data to VA0 before rendering
    // this->VAO.bind();
    // this->VAO.storeBlockData(this->meshData);
    // this->VAO.unbind();

    // Set chunk as not dirty, meaning it will not rebuild next frame
    // chunk.setDirty(false);
}

void ChunkMesh::upload() const
{
    // Link data to VA0 before rendering
    this->VAO.bind();
    this->VAO.storeBlockData(this->meshData);
    this->VAO.unbind();
}

void ChunkMesh::render() const
{
    this->VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->meshData.size()));
    this->VAO.unbind();
}

// Statics
std::tuple<GLint, GLint, GLint> ChunkMesh::coords(const int index) {
    return {
        index / (Chunk::SIZE * Chunk::SIZE),
        (index / Chunk::SIZE) % Chunk::SIZE,
        index % Chunk::SIZE
    };
}

void ChunkMesh::createFace(std::vector<Vertex> &data, const glm::ivec3 &v0, const glm::ivec3 &v1, const glm::ivec3 &v2, const glm::ivec3 &v3, const glm::ivec3 &normals, const uint16_t& texId)
{
    data.insert(data.end(), {
        {v0, normals, {1, 0}, texId},
        {v1, normals, {1, 1}, texId},
        {v2, normals, {0, 1}, texId},
        {v0, normals, {1, 0}, texId},
        {v2, normals, {0, 1}, texId},
        {v3, normals, {0, 0}, texId},
    });
}