#include "ChunkManager.h"
#include "World.h"

Chunk* ChunkManager::getChunk(const int cx, const int cy, const int cz) const
{
    const auto it = this->chunks.find({cx, cy, cz});
    return it != this->chunks.end() ? it->second.get() : nullptr;
}

Chunk& ChunkManager::getOrCreateChunk(const int cx, const int cy, const int cz)
{
    const ChunkPos pos{cx, cy, cz};

    std::unique_ptr<Chunk>& chunk = this->chunks[pos];

    if (!chunk) {
        chunk = std::make_unique<Chunk>(pos);
        this->markNeighborsDirty(pos);
    }
    return *chunk;
}

const ChunkMap& ChunkManager::getChunks() const
{
    return this->chunks;
}

bool ChunkManager::chunkExist(const int cx, const int cy, const int cz) const
{
    return this->chunks.contains({cx, cy, cz});
}

void ChunkManager::markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp) const
{
    std::vector<glm::ivec3> neighbors;

    if (bp.has_value())
    {
        const auto v = bp.value();
        if (v.x == 0)                  neighbors.emplace_back(-1, 0, 0);
        if (v.x == Chunk::SIZE - 1)    neighbors.emplace_back(1, 0, 0);
        if (v.y == 0)                  neighbors.emplace_back(0, -1, 0);
        if (v.y == Chunk::SIZE - 1)    neighbors.emplace_back(0, 1, 0);
        if (v.z == 0)                  neighbors.emplace_back(0, 0, -1);
        if (v.z == Chunk::SIZE - 1)    neighbors.emplace_back(0, 0, 1);
    } else
    {
        neighbors = {
            { 1, 0, 0}, {-1, 0, 0},
            { 0, 1, 0}, { 0,-1, 0},
            { 0, 0, 1}, { 0, 0,-1},
        };
    }

    for (auto& n : neighbors) {
        const ChunkPos pos{cp.x + n[0], cp.y + n[1], cp.z + n[2]};

        if (const auto chunk = this->getChunk(pos.x, pos.y, pos.z))
            chunk->setDirty(true);
    }
}

void ChunkManager::update(const World& world, const glm::vec3& cameraWorldPos)
{
    const ChunkPos center = ChunkPos::fromWorld(cameraWorldPos);
    std::unordered_set<ChunkPos, ChunkPosHash> wanted;

    // Compute desired chunks
    for (int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
        for (int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
            wanted.insert({
                center.x + x,
                0,
                center.z + z
            });
        }
    }

    // Load missing chunks
    for (const ChunkPos& pos : wanted) {
        if (!chunks.contains(pos)) {
            Chunk& chunk = this->getOrCreateChunk(pos.x, pos.y, pos.z);
            world.generateChunkTerrain(chunk);
        }
    }

    // Unload far chunks
    for (auto it = chunks.begin(); it != chunks.end(); ) {
        if (!wanted.contains(it->first))
            it = chunks.erase(it);
        else
            ++it;
    }
}