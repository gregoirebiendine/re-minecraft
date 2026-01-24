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
    for (auto&[pos, chunk] : world.getChunkManager().getChunks()) {
        const bool needsFirstMesh = chunk.getState() == ChunkState::GENERATED;
        const bool needsRemesh = chunk.getState() == ChunkState::READY && chunk.isDirty();

        if (!needsFirstMesh && !needsRemesh)
            continue;

        if (needsFirstMesh)
            chunk.setState(ChunkState::MESHING);

        chunk.bumpGenerationID();
        chunk.setDirty(false);

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
        mesh.swapBuffers();

        if (Chunk* c = world.getChunkManager().getChunk(pos.x, pos.y, pos.z)) {
            if (c->getState() == ChunkState::MESHED)
                c->setState(ChunkState::READY);
        }
    }
}

void ChunkMeshManager::buildMeshJob(const ChunkJob& job)
{
    Chunk* chunk = world.getChunkManager().getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk)
        return;

    if (chunk->getGenerationID() != job.generationID)
        return;

    const auto blockData = chunk->getBlockSnapshot();
    const auto [north, south, east, west, up, down] = world.getChunkManager().getNeighbors(job.pos);

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
                {x, y, z},
                NORTH,
                textureRegistry.getByName(meta.getFaceTexture(NORTH))
            );
        }

        // SOUTH face
        if (isAirAtSnapshot(blockData, neighbors, x, y, z + 1)) {
            buildFaceMesh(
                data,
                {x, y, z},
                SOUTH,
                textureRegistry.getByName(meta.getFaceTexture(SOUTH))
            );
        }

        // WEST face
        if (isAirAtSnapshot(blockData, neighbors, x - 1, y, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 WEST,
                 textureRegistry.getByName(meta.getFaceTexture(WEST))
            );
        }

        // EAST face
        if (isAirAtSnapshot(blockData, neighbors, x + 1, y, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 EAST,
                 textureRegistry.getByName(meta.getFaceTexture(EAST))
            );
        }

        // UP face
        if (isAirAtSnapshot(blockData, neighbors, x, y + 1, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 UP,
                 textureRegistry.getByName(meta.getFaceTexture(UP))
            );
        }

        // DOWN face
        if (isAirAtSnapshot(blockData, neighbors, x, y - 1, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 DOWN,
                 textureRegistry.getByName(meta.getFaceTexture(DOWN))
            );
        }
    }

    {
        std::lock_guard lock(uploadMutex);
        uploadQueue.emplace(job.pos, std::move(data));
    }

    // Only change state for first-time meshing, not for remeshing
    if (chunk->getState() == ChunkState::MESHING)
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

void ChunkMeshManager::buildFaceMesh(MeshData& mesh, const glm::ivec3& pos, const MaterialFace face, uint16_t texId, const uint8_t rotation)
{
    static constexpr uint8_t FACE_VERTEX_DATA[6][6][5] = {
        // NORTH face (-Z)
        {{0,0,0, 0,0}, {1,1,0, 1,1}, {1,0,0, 1,0}, {0,0,0, 0,0}, {0,1,0, 0,1}, {1,1,0, 1,1}},
        // SOUTH face (+Z)
        {{0,0,1, 1,0}, {1,0,1, 0,0}, {1,1,1, 0,1}, {0,0,1, 1,0}, {1,1,1, 0,1}, {0,1,1, 1,1}},
        // WEST face (-X)
        {{0,0,0, 1,0}, {0,0,1, 0,0}, {0,1,1, 0,1}, {0,0,0, 1,0}, {0,1,1, 0,1}, {0,1,0, 1,1}},
        // EAST face (+X)
        {{1,0,0, 0,0}, {1,1,1, 1,1}, {1,0,1, 1,0}, {1,0,0, 0,0}, {1,1,0, 0,1}, {1,1,1, 1,1}},
        // UP face (+Y)
        {{0,1,0, 1,0}, {0,1,1, 1,1}, {1,1,1, 0,1}, {0,1,0, 1,0}, {1,1,1, 0,1}, {1,1,0, 0,0}},
        // DOWN face (-Y)
        {{0,0,0, 1,1}, {1,0,1, 0,0}, {0,0,1, 1,0}, {0,0,0, 1,1}, {1,0,0, 0,1}, {1,0,1, 0,0}},
    };

    const auto x = static_cast<uint8_t>(pos.x);
    const auto y = static_cast<uint8_t>(pos.y);
    const auto z = static_cast<uint8_t>(pos.z);
    const auto normalIndex = static_cast<uint8_t>(face);

    for (int i = 0; i < 6; ++i) {
        const auto& vd = FACE_VERTEX_DATA[face][i];

        mesh.emplace_back(
            x + vd[0], y + vd[1], z + vd[2],    // position
            normalIndex,                        // normal
            rotation,                           // rotation
            vd[3], vd[4],                     // uv
            texId                               // texture
        );
    }
}