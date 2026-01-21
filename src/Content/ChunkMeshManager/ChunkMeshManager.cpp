#include "ChunkMeshManager.h"
#include "World.h"

ChunkMeshManager::ChunkMeshManager(World& _world) :
    world(_world),
    workers(std::thread::hardware_concurrency()
)
{
    this->workers.setWorker([this](const ChunkJob &job) {
        this->buildMeshJob(job);
    });
}

void ChunkMeshManager::requestRebuild(Chunk& chunk, const float distance)
{
    chunk.bumpGenerationID();
    chunk.setState(ChunkState::MESHING);

    const ChunkJob job{
        chunk.getPosition(),
        distance,
        chunk.getGenerationID()
    };

    this->workers.enqueue(job);
}

void ChunkMeshManager::scheduleMeshing(const glm::vec3& cameraPos)
{
    for (auto&[pos, chunk] : world.getChunkManager()->getChunks()) {
        if (chunk.getState() != ChunkState::GENERATED)
            continue;

        chunk.setState(ChunkState::MESHING);
        chunk.bumpGenerationID();

        const auto center = glm::vec3(
            pos.x * Chunk::SIZE + Chunk::SIZE / 2.0f,
            pos.y * Chunk::SIZE + Chunk::SIZE / 2.0f,
            pos.z * Chunk::SIZE + Chunk::SIZE / 2.0f
        );

        workers.enqueue({
            pos,
            glm::distance(cameraPos, center),
            chunk.getGenerationID()
        });
    }
}

void ChunkMeshManager::update()
{
    std::lock_guard lock(uploadMutex);

    while (!uploadQueue.empty()) {
        auto [pos, data] = std::move(uploadQueue.front());

        uploadQueue.pop();

        auto& mesh = meshes.try_emplace(pos, pos).first->second;
        mesh.upload(std::move(data));

        if (Chunk* c = world.getChunkManager()->getChunk(pos.x, pos.y, pos.z))
            c->setState(ChunkState::READY);
    }
}

void ChunkMeshManager::buildMeshJob(const ChunkJob& job)
{
    Chunk* chunk = world.getChunkManager()->getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk)
        return;

    if (chunk->getGenerationID() != job.generationID)
        return;

    const auto blockData = chunk->getBlockSnapshot();
    const auto [north, south, east, west, up, down] = world.getChunkManager()->getNeighbors(job.pos);

    NeighborData neighbors[6];
    neighbors[0] = { north != nullptr, north ? north->getBlockSnapshot() : std::array<Material, Chunk::VOLUME>{} };
    neighbors[1] = { south != nullptr, south ? south->getBlockSnapshot() : std::array<Material, Chunk::VOLUME>{} };
    neighbors[2] = { east  != nullptr, east  ? east->getBlockSnapshot()  : std::array<Material, Chunk::VOLUME>{} };
    neighbors[3] = { west  != nullptr, west  ? west->getBlockSnapshot()  : std::array<Material, Chunk::VOLUME>{} };
    neighbors[4] = { up    != nullptr, up    ? up->getBlockSnapshot()    : std::array<Material, Chunk::VOLUME>{} };
    neighbors[5] = { down  != nullptr, down  ? down->getBlockSnapshot()  : std::array<Material, Chunk::VOLUME>{} };


    const auto& textureRegistry = world.getTextureRegistry();

    MeshData data;
    data.reserve(36 * Chunk::VOLUME);

    for (int i = 0; i < Chunk::VOLUME; i++) {
        auto [x, y, z] = ChunkCoords::indexToLocalCoords(i);
        const Material mat = blockData[i];

        if (mat == 0) // Skip AIR
            continue;

        const BlockMeta& meta = world.getBlockRegistry().get(mat);

        // NORTH face
        if (isAirAtSnapshot(blockData, neighbors, x, y, z - 1)) {
            buildFaceMesh(
                data,
                {x,       y,       z},
                {x,       1 + y,   z},
                {1 + x,   1 + y,   z},
                {1 + x,   y,       z},
                {0, 0, -1},
                textureRegistry.getByName(meta.getFaceTexture(NORTH))
            );
        }

        // SOUTH face
        if (isAirAtSnapshot(blockData, neighbors, x, y, z + 1)) {
            buildFaceMesh(
                data,
                {1 + x,   y,       1 + z},
                {1 + x,   1 + y,   1 + z},
                {x,       1 + y,   1 + z},
                {x,       y,       1 + z},
                {0, 0, 1},
                textureRegistry.getByName(meta.getFaceTexture(SOUTH))
            );
        }

        // WEST face
        if (isAirAtSnapshot(blockData, neighbors, x - 1, y, z)) {
            buildFaceMesh(
                data,
                {x,       y,       1 + z},
                {x,       1 + y,   1 + z},
                {x,       1 + y,   z},
                {x,       y,       z},
                {-1, 0, 0},
                textureRegistry.getByName(meta.getFaceTexture(WEST))
            );
        }

        // EAST face
        if (isAirAtSnapshot(blockData, neighbors, x + 1, y, z)) {
             buildFaceMesh(
                data,
                {1 + x,   y,       z},
                {1 + x,   1 + y,   z},
                {1 + x,   1 + y,   1 + z},
                {1 + x,   y,       1 + z},
                {1, 0, 0},
                textureRegistry.getByName(meta.getFaceTexture(EAST))
            );
        }

        // UP face
        if (isAirAtSnapshot(blockData, neighbors, x, y + 1, z)) {
            buildFaceMesh(
                data,
                {x,       1 + y,   z},
                {x,       1 + y,   1 + z},
                {1 + x,   1 + y,   1 + z},
                {1 + x,   1 + y,   z},
                {0, 1, 0},
                textureRegistry.getByName(meta.getFaceTexture(UP))
            );
        }

        // DOWN face
        if (isAirAtSnapshot(blockData, neighbors, x, y - 1, z)) {
            buildFaceMesh(
                data,
                {x,       y,       z},
                {1 + x,   y,       z},
                {1 + x,   y,       1 + z},
                {x,       y,       1 + z},
                {0, -1, 0},
                textureRegistry.getByName(meta.getFaceTexture(DOWN))
            );
        }
    }

    {
        std::lock_guard lock(uploadMutex);
        uploadQueue.emplace(job.pos, std::move(data));
    }

    chunk->setState(ChunkState::MESHED);
}

const ChunkMesh& ChunkMeshManager::getMesh(const ChunkPos &pos) const
{
    return this->meshes.at(pos);
}

// Statics
bool ChunkMeshManager::isAirAtSnapshot(
    const std::array<Material, Chunk::VOLUME>& blockData,
    const NeighborData neighbors[6],
    const int x, const int y, const int z
)
{
    // Inside current chunk
    if (x >= 0 && x < Chunk::SIZE &&
        y >= 0 && y < Chunk::SIZE &&
        z >= 0 && z < Chunk::SIZE
    )
        return blockData[ChunkCoords::localCoordsToIndex(x, y, z)] == 0;

    // Check neighbors
    if (z < 0) {  // NORTH
        if (!neighbors[0].exists) return true;
        return neighbors[0].blocks[ChunkCoords::localCoordsToIndex(x, y, z + Chunk::SIZE)] == 0;
    }
    if (z >= Chunk::SIZE) {  // SOUTH
        if (!neighbors[1].exists) return true;
        return neighbors[1].blocks[ChunkCoords::localCoordsToIndex(x, y, z - Chunk::SIZE)] == 0;
    }
    if (x >= Chunk::SIZE) {  // EAST
        if (!neighbors[2].exists) return true;
        return neighbors[2].blocks[ChunkCoords::localCoordsToIndex(x - Chunk::SIZE, y, z)] == 0;
    }
    if (x < 0) {  // WEST
        if (!neighbors[3].exists) return true;
        return neighbors[3].blocks[ChunkCoords::localCoordsToIndex(x + Chunk::SIZE, y, z)] == 0;
    }
    if (y >= Chunk::SIZE) {  // UP
        if (!neighbors[4].exists) return true;
        return neighbors[4].blocks[ChunkCoords::localCoordsToIndex(x, y - Chunk::SIZE, z)] == 0;
    }
    if (y < 0) {  // DOWN
        if (!neighbors[5].exists) return true;
        return neighbors[5].blocks[ChunkCoords::localCoordsToIndex(x, y + Chunk::SIZE, z)] == 0;
    }

    return true;
}

void ChunkMeshManager::buildFaceMesh(MeshData& data, const glm::ivec3 &v0, const glm::ivec3 &v1, const glm::ivec3 &v2, const glm::ivec3 &v3, const glm::ivec3 &normals, const uint16_t& texId)
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