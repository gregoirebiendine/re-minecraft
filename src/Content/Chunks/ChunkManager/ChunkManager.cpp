#include "ChunkManager.h"

ChunkManager::ChunkManager(const BlockRegistry& _blockRegistry, const PrefabRegistry& _prefabRegistry, const Settings& _settings) :
    blockRegistry(_blockRegistry),
    settings(_settings),
    terrainWorkers(std::thread::hardware_concurrency()),
    terrainGenerator(_blockRegistry, _prefabRegistry)
{
    this->terrainWorkers.setWorker([this](const ChunkJob &job) {
        terrainJob(job);
    });
}

std::shared_lock<std::shared_mutex> ChunkManager::acquireReadLock() const
{
    return std::shared_lock(chunksMutex);
}

void ChunkManager::requestChunk(const ChunkPos& pos)
{
    std::unique_lock lock(this->chunksMutex);

    if (this->chunks.contains(pos))
        return;

    auto [it, inserted] = this->chunks.try_emplace(pos, std::make_unique<Chunk>(pos));
    Chunk& chunk = *it->second;

    chunk.bumpGenerationID();
    chunk.setState(ChunkState::TERRAIN_PENDING);

    this->terrainWorkers.enqueue({pos, static_cast<float>(pos.y), chunk.getGenerationID()});
}

Chunk* ChunkManager::getChunk(const int cx, const int cy, const int cz)
{
    std::shared_lock lock(chunksMutex);

    const auto it = chunks.find({cx,cy,cz});
    return it == chunks.end() ? nullptr : it->second.get();
}

void ChunkManager::rebuildNeighbors(const ChunkPos& pos)
{
    static const int d[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };

    for (auto& o : d) {
        Chunk* n = this->getChunk(pos.x+o[0], pos.y+o[1], pos.z+o[2]);
        if (!n)
            continue;

        if (n->getState() == ChunkState::READY)
            n->setDirty(true);
        else if (n->getState() >= ChunkState::MESHING)
            n->setState(ChunkState::MESHING);
    }
}

void ChunkManager::updateStreaming(const glm::vec3& playerPos)
{
    const auto& viewDistance = this->settings.getViewDistance();
    const auto& unloadDistance = this->settings.getViewDistance() + 2;

    const auto [playerX, playerY, playerZ] = ChunkPos::fromWorld(playerPos);
    std::unordered_set<ChunkPos, ChunkPosHash> wanted;

    wanted.reserve((2 * viewDistance + 1) * (2 * viewDistance + 1) * (2 * viewDistance + 1));

    for (int z = -viewDistance; z <= viewDistance; ++z) {
        for (int y = -viewDistance; y <= viewDistance; ++y) {
            const int generatedY = playerY + y;

            for (int x = -viewDistance; x <= viewDistance; ++x) {
                ChunkPos pos {
                    playerX + x,
                    generatedY < 0 ? 0 : generatedY,
                    playerZ + z
                };

                wanted.insert(pos);
            }
        }
    }

    // Request new chunks
    for (const ChunkPos& pos : wanted)
        requestChunk(pos);

    // Unload distant chunks
    {
        std::unique_lock lock(this->chunksMutex);
        for (auto it = this->chunks.begin(); it != this->chunks.end(); ) {
            Chunk& chunk = *it->second;

            const int dx = chunk.getPosition().x - playerX;
            const int dy = chunk.getPosition().y - playerY;
            const int dz = chunk.getPosition().z - playerZ;

            if (std::abs(dx) > unloadDistance || std::abs(dy) > unloadDistance || std::abs(dz) > unloadDistance) {
                chunk.bumpGenerationID();
                it = chunks.erase(it);
            }
            else
                ++it;
        }
    }

}

std::vector<Chunk *> ChunkManager::getRenderableChunks()
{
    std::shared_lock lock(this->chunksMutex);
    std::vector<Chunk*> out;

    for (auto &c: this->chunks | std::views::values)
    {
        if (c->getState() != ChunkState::READY)
            continue;

        const auto& [x, y, z] = c->getPosition();
        const glm::vec3 min(x * 16, y * 16, z * 16);
        const glm::vec3 max = min + glm::vec3(16.0f);

        if (this->frustum.isBoxVisible(min, max))
            out.push_back(c.get());
    }

    return out;
}

void ChunkManager::updateFrustum(const glm::mat4& vpMatrix)
{
    this->frustum.update(vpMatrix);
}

bool ChunkManager::isAreaReady(const ChunkPos center, const int radius)
{
    const int minX = center.x - radius;
    const int minY = center.y - radius;
    const int minZ = center.z - radius;
    const int maxX = center.x + radius;
    const int maxY = center.y + radius;
    const int maxZ = center.z + radius;

    for (int z = minZ; z <= maxZ; ++z)
        for (int y = minY; y <= maxY; ++y)
            for (int x = minX; x <= maxX; ++x) {
                const auto c = this->getChunk(x, y, z);

                if (!c || c->getState() != ChunkState::READY)
                    return false;
            }
    return true;

}

ChunkMap& ChunkManager::getChunks()
{
    return this->chunks;
}

ChunkNeighbors ChunkManager::getNeighbors(const ChunkPos& cp)
{
    return {
        this->getChunk(cp.x, cp.y, cp.z - 1),
        this->getChunk(cp.x, cp.y, cp.z + 1),
        this->getChunk(cp.x + 1, cp.y, cp.z),
        this->getChunk(cp.x - 1, cp.y, cp.z),
        this->getChunk(cp.x, cp.y + 1, cp.z),
        this->getChunk(cp.x, cp.y - 1, cp.z),
    };
}


// Worker jobs
void ChunkManager::terrainJob(const ChunkJob& job)
{
    Chunk* chunk = this->getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk || chunk->getGenerationID() != job.generationID)
        return;

    chunk->setState(ChunkState::TERRAIN_GENERATING);
    this->terrainGenerator.generate(*chunk);
    chunk->finalizeGeneration();
    chunk->setState(ChunkState::TERRAIN_DONE);
}