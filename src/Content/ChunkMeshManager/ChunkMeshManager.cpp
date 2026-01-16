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

    const ChunkNeighbors n = world.getChunkManager()->getNeighbors(job.pos);
    const auto& textureRegistry = world.getTextureRegistry();

    MeshData data;
    data.reserve(36 * Chunk::VOLUME);

    for (int i = 0; i < Chunk::VOLUME; i++) {
        auto [x, y, z] = ChunkCoords::indexToLocalCoords(i);

        // Skip AIR
        if (chunk->isAir(x, y, z))
            continue;

        // Retrieve Block Meta
        const BlockMeta& meta = world.getBlockRegistry().get(chunk->getBlock(x, y, z));

        // NORTH face
        if (isAirAt(*chunk, n, x, y, z - 1)) {
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
        if (isAirAt(*chunk, n, x, y, z + 1)) {
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
        if (isAirAt(*chunk, n, x - 1, y, z)) {
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
        if (isAirAt(*chunk, n, x + 1, y, z)) {
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
        if (isAirAt(*chunk, n, x, y + 1, z)) {
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
        if (isAirAt(*chunk, n, x, y - 1, z)) {
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
bool ChunkMeshManager::isAirAt(const Chunk& c, const ChunkNeighbors& n, const int x, const int y, const int z)
{
    if (x >= 0 && x < Chunk::SIZE &&
        y >= 0 && y < Chunk::SIZE &&
        z >= 0 && z < Chunk::SIZE)
        return c.isAir(x, y, z);

    if (x < 0 && n.west)  return n.west->isAir(x + Chunk::SIZE, y, z);
    if (x >= Chunk::SIZE && n.east) return n.east->isAir(x - Chunk::SIZE, y, z);
    if (z < 0 && n.north) return n.north->isAir(x, y, z + Chunk::SIZE);
    if (z >= Chunk::SIZE && n.south) return n.south->isAir(x, y, z - Chunk::SIZE);
    if (y < 0 && n.down)  return n.down->isAir(x, y + Chunk::SIZE, z);
    if (y >= Chunk::SIZE && n.up)    return n.up->isAir(x, y - Chunk::SIZE, z);

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