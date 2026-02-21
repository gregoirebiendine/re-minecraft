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

void ChunkMeshManager::scheduleMeshing(const glm::vec3& playerPos)
{
    auto lock = world.getChunkManager().acquireReadLock();
    for (auto&[pos, chunk] : world.getChunkManager().getChunks()) {
        const bool needsFirstMesh = chunk->getState() == ChunkState::DECOR_DONE;
        const bool needsRemesh = chunk->getState() == ChunkState::READY && chunk->isDirty();

        if (!needsFirstMesh && !needsRemesh)
            continue;

        if (needsFirstMesh)
            chunk->setState(ChunkState::MESHING);

        chunk->bumpGenerationID();
        chunk->setDirty(false);

        const auto center = glm::vec3(
            pos.x * Chunk::SIZE + Chunk::SIZE / 2.0f,
            pos.y * Chunk::SIZE + Chunk::SIZE / 2.0f,
            pos.z * Chunk::SIZE + Chunk::SIZE / 2.0f
        );

        workers.enqueue({
            pos,
            glm::distance(playerPos, center),
            chunk->getGenerationID()
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
    neighbors[0] = { north != nullptr, north ? north->getBlockSnapshot() : BlockStorage{} };
    neighbors[1] = { south != nullptr, south ? south->getBlockSnapshot() : BlockStorage{} };
    neighbors[2] = { east  != nullptr, east  ? east->getBlockSnapshot()  : BlockStorage{} };
    neighbors[3] = { west  != nullptr, west  ? west->getBlockSnapshot()  : BlockStorage{} };
    neighbors[4] = { up    != nullptr, up    ? up->getBlockSnapshot()    : BlockStorage{} };
    neighbors[5] = { down  != nullptr, down  ? down->getBlockSnapshot()  : BlockStorage{} };

    const auto& textureRegistry = this->world.getRegistries().get<TextureRegistry>();
    const auto& blockRegistry = this->world.getRegistries().get<BlockRegistry>();

    MeshData data;
    data.reserve(36 * Chunk::VOLUME);

    for (int i = 0; i < Chunk::VOLUME; i++) {
        if (blockRegistry.isAir(blockData[i].getBlockId())) // Skip AIR
            continue;

        const auto [x, y, z] = ChunkPos::indexToLocalCoords(i);
        const BlockId blockId = blockData[i].getBlockId();
        const BlockRotation rotation = blockData[i].getRotation();
        const BlockMeta& meta = blockRegistry.get(blockId);

        // NORTH face
        if (isAirAtSnapshot(blockData, neighbors, x, y, z - 1)) {
            buildFaceMesh(
                data,
                {x, y, z},
                NORTH,
                textureRegistry.getByName(getTextureFromRotation(meta, NORTH, rotation)),
                rotation
            );
        }

        // SOUTH face
        if (isAirAtSnapshot(blockData, neighbors, x, y, z + 1)) {
            buildFaceMesh(
                data,
                {x, y, z},
                SOUTH,
                textureRegistry.getByName(getTextureFromRotation(meta, SOUTH, rotation)),
                rotation
            );
        }

        // WEST face
        if (isAirAtSnapshot(blockData, neighbors, x - 1, y, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 WEST,
                 textureRegistry.getByName(getTextureFromRotation(meta, WEST, rotation)),
                 rotation
            );
        }

        // EAST face
        if (isAirAtSnapshot(blockData, neighbors, x + 1, y, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 EAST,
                 textureRegistry.getByName(getTextureFromRotation(meta, EAST, rotation)),
                 rotation
            );
        }

        // UP face
        if (isAirAtSnapshot(blockData, neighbors, x, y + 1, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 UP,
                 textureRegistry.getByName(getTextureFromRotation(meta, UP, rotation)),
                 rotation
            );
        }

        // DOWN face
        if (isAirAtSnapshot(blockData, neighbors, x, y - 1, z)) {
            buildFaceMesh(
                 data,
                 {x, y, z},
                 DOWN,
                 textureRegistry.getByName(getTextureFromRotation(meta, DOWN, rotation)),
                 rotation
            );
        }
    }

    {
        std::lock_guard lock(uploadMutex);
        uploadQueue.emplace(job.pos, std::move(data));
        if (chunk->getState() == ChunkState::MESHING)
            chunk->setState(ChunkState::MESHED);
    }
}

const ChunkMesh& ChunkMeshManager::getMesh(const ChunkPos &pos) const
{
    return this->meshes.at(pos);
}

bool ChunkMeshManager::isTransparentAtSnapshot(const BlockId blockId) const
{
    return this->world.getRegistries().get<BlockRegistry>().get(blockId).transparent;
}

bool ChunkMeshManager::isAirAtSnapshot(
    const BlockStorage& blockData,
    const NeighborData neighbors[6],
    const int x, const int y, const int z
) const
{
    // Inside current chunk
    if (x >= 0 && x < Chunk::SIZE && y >= 0 && y < Chunk::SIZE && z >= 0 && z < Chunk::SIZE) {
        const BlockId blockId = blockData[ChunkPos::localCoordsToIndex(x, y, z)].getBlockId();
        return blockId == 0 || this->isTransparentAtSnapshot(blockId);
    }

    // Check neighbors
    if (z < 0) {  // NORTH
        if (!neighbors[0].exists)
            return true;
        const BlockId blockId = neighbors[0].blocks[ChunkPos::localCoordsToIndex(x, y, z + Chunk::SIZE)].getBlockId();
        return blockId == 0 || this->isTransparentAtSnapshot(blockId);
    }
    if (z >= Chunk::SIZE) {  // SOUTH
        if (!neighbors[1].exists)
            return true;
        const BlockId blockId = neighbors[1].blocks[ChunkPos::localCoordsToIndex(x, y, z - Chunk::SIZE)].getBlockId();
        return blockId == 0 || this->isTransparentAtSnapshot(blockId);
    }
    if (x >= Chunk::SIZE) {  // EAST
        if (!neighbors[2].exists)
            return true;
        const BlockId blockId = neighbors[2].blocks[ChunkPos::localCoordsToIndex(x - Chunk::SIZE, y, z)].getBlockId();
        return blockId == 0 || this->isTransparentAtSnapshot(blockId);
    }
    if (x < 0) {  // WEST
        if (!neighbors[3].exists)
            return true;
        const BlockId blockId = neighbors[3].blocks[ChunkPos::localCoordsToIndex(x + Chunk::SIZE, y, z)].getBlockId();
        return blockId == 0 || this->isTransparentAtSnapshot(blockId);
    }
    if (y >= Chunk::SIZE) {  // UP
        if (!neighbors[4].exists)
            return true;
        const BlockId blockId = neighbors[4].blocks[ChunkPos::localCoordsToIndex(x, y - Chunk::SIZE, z)].getBlockId();
        return blockId == 0 || this->isTransparentAtSnapshot(blockId);
    }
    if (y < 0) {  // DOWN
        if (!neighbors[5].exists)
            return true;
        const BlockId blockId = neighbors[5].blocks[ChunkPos::localCoordsToIndex(x, y + Chunk::SIZE, z)].getBlockId();
        return blockId == 0 || this->isTransparentAtSnapshot(blockId);
    }
    return true;
}

void ChunkMeshManager::buildFaceMesh(MeshData& mesh, const glm::ivec3& pos, const MaterialFace face, uint16_t texId, const BlockRotation rotation)
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

std::string ChunkMeshManager::getTextureFromRotation(const BlockMeta& meta, const MaterialFace face, const BlockRotation rotation)
{
    if (meta.rotation == RotationType::NONE)
        return meta.getFaceTexture(face);
    if (meta.rotation == RotationType::HORIZONTAL)
        return meta.getFaceTexture(remapFaceForRotation(face, rotation));
    return meta.getFaceTexture(remapFaceForAxisRotation(face, rotation));
}

MaterialFace ChunkMeshManager::remapFaceForRotation(const MaterialFace face, const BlockRotation rotation)
{
    if (face == UP or face == DOWN)
        return face;

    constexpr MaterialFace FACE_REMAP[4][4] = {
        {NORTH, SOUTH, WEST, EAST},
        {SOUTH, NORTH, EAST, WEST},
        {EAST, WEST, NORTH, SOUTH},
        {WEST, EAST, SOUTH, NORTH}
    };

    return FACE_REMAP[rotation][face];
}

MaterialFace ChunkMeshManager::remapFaceForAxisRotation(const MaterialFace face, const BlockRotation rotation)
{
    // Rotation 4 = Y-axis (vertical, no remapping needed)
    if (rotation == 4)
        return face;

    // Rotation 5 = Z-axis (log pointing N/S)
    if (rotation == 5)
        switch (face)
        {
        case UP:    return SOUTH;
        case DOWN:  return NORTH;
        case NORTH: return DOWN;
        case SOUTH: return UP;
        default:    return face;
        }

    // Rotation 6 = X-axis (log pointing E/W)
    if (rotation == 6)
        switch (face)
        {
        case UP:    return EAST;
        case DOWN:  return WEST;
        case EAST:  return DOWN;
        case WEST:  return UP;
        default:    return face;
        }

    return face;
}
