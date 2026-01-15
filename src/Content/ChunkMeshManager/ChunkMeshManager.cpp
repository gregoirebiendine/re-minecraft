#include "ChunkMeshManager.h"

ChunkMeshManager::ChunkMeshManager(World& _world) :
    world(_world),
    workers(std::thread::hardware_concurrency())
{
    // Empty
}

ChunkMeshData ChunkMeshManager::buildMesh(
    const Chunk& chunk,
    const ChunkNeighbors& n,
    const BlockRegistry& blockRegistry,
    const TextureRegistry& textureRegistry
)
{
    ChunkMeshData meshData;
    meshData.reserve(sizeof(Vertex) * Chunk::VOLUME);

    for (int i = 0; i < Chunk::VOLUME; i++) {
        auto [x, y, z] = ChunkCoords::indexToLocalCoords(i);

        // Skip AIR
        if (chunk.isAir(x, y, z)) continue;

        // Retrieve Block Meta
        const BlockMeta& meta = blockRegistry.get(chunk.getBlock(x, y, z));

        // NORTH face
        if (isAirAt(chunk, n, x, y, z - 1)) {
            buildFaceMesh(
                meshData,
                {x,       y,       z},
                {x,       1 + y,   z},
                {1 + x,   1 + y,   z},
                {1 + x,   y,       z},
                {0, 0, -1},
                textureRegistry.getByName(meta.getFaceTexture(NORTH))
            );
        }

        // SOUTH face
        if (isAirAt(chunk, n, x, y, z + 1)) {
            buildFaceMesh(
                meshData,
                {1 + x,   y,       1 + z},
                {1 + x,   1 + y,   1 + z},
                {x,       1 + y,   1 + z},
                {x,       y,       1 + z},
                {0, 0, 1},
                textureRegistry.getByName(meta.getFaceTexture(SOUTH))
            );
        }

        // WEST face
        if (isAirAt(chunk, n, x - 1, y, z)) {
            buildFaceMesh(
                meshData,
                {x,       y,       1 + z},
                {x,       1 + y,   1 + z},
                {x,       1 + y,   z},
                {x,       y,       z},
                {-1, 0, 0},
                textureRegistry.getByName(meta.getFaceTexture(WEST))
            );
        }

        // EAST face
        if (isAirAt(chunk, n, x + 1, y, z)) {
             buildFaceMesh(
                meshData,
                {1 + x,   y,       z},
                {1 + x,   1 + y,   z},
                {1 + x,   1 + y,   1 + z},
                {1 + x,   y,       1 + z},
                {1, 0, 0},
                textureRegistry.getByName(meta.getFaceTexture(EAST))
            );
        }

        // UP face
        if (isAirAt(chunk, n, x, y + 1, z)) {
            buildFaceMesh(
                meshData,
                {x,       1 + y,   z},
                {x,       1 + y,   1 + z},
                {1 + x,   1 + y,   1 + z},
                {1 + x,   1 + y,   z},
                {0, 1, 0},
                textureRegistry.getByName(meta.getFaceTexture(UP))
            );
        }

        // DOWN face
        if (isAirAt(chunk, n, x, y - 1, z)) {
            buildFaceMesh(
                meshData,
                {x,       y,       z},
                {1 + x,   y,       z},
                {1 + x,   y,       1 + z},
                {x,       y,       1 + z},
                {0, -1, 0},
                textureRegistry.getByName(meta.getFaceTexture(DOWN))
            );
        }
    }

    return meshData;
}

// void ChunkMeshManager::update(const ChunkManager& chunkManager)
// {
//     for (const auto& chunk : chunkManager.getChunks() | std::views::values) {
//         if (chunk->getState() == ChunkState::GENERATED) {
//             chunk->setState(ChunkState::MESHING);
//             this->meshQueue.push(chunk.get());
//         }
//     }
//
//     // Main thread: upload finished meshes
//     ChunkMesh mesh;
//     while (this->readyMeshes.tryPop(mesh)) {
//         mesh.upload();
//         this->meshes[mesh.position] = std::move(mesh); //Needs modify
//     }
// }

void ChunkMeshManager::workerLoop()
{
    while (true) {
        Chunk* chunk;

        if (!meshQueue.tryPop(chunk))
            continue;

        const auto n = this->chunkManager.getNeighbors(chunk->getPosition());
        ChunkMesh mesh = buildMesh(*chunk, n);

        this->readyMeshes.push(std::move(mesh));
        chunk->setState(ChunkState::READY);
    }
}

void ChunkMeshManager::buildFaceMesh(
    ChunkMeshData& data,
    const glm::ivec3 &v0,
    const glm::ivec3 &v1,
    const glm::ivec3 &v2,
    const glm::ivec3 &v3,
    const glm::ivec3 &normals,
    const uint16_t& texId
)
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

bool ChunkMeshManager::isAirAt(const Chunk& chunk, const ChunkNeighbors& n, const int x, const int y, const int z)
{
    if (x >= 0 && x < Chunk::SIZE &&
        y >= 0 && y < Chunk::SIZE &&
        z >= 0 && z < Chunk::SIZE
    )
        return chunk.isAir(x, y, z);

    if (x < 0 && n.west)
        return n.west->isAir(x + Chunk::SIZE, y, z);

    if (x >= Chunk::SIZE && n.east)
        return n.east->isAir(x - Chunk::SIZE, y, z);

    if (z < 0 && n.north)
        return n.north->isAir(x, y, z + Chunk::SIZE);

    if (z >= Chunk::SIZE && n.south)
        return n.south->isAir(x, y, z - Chunk::SIZE);

    if (y < 0 && n.down)
        return n.down->isAir(x, y + Chunk::SIZE, z);

    if (y >= Chunk::SIZE && n.up)
        return n.up->isAir(x, y - Chunk::SIZE, z);

    return true;
}