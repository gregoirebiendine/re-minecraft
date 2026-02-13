#include "ChunkManager.h"

ChunkManager::ChunkManager(const BlockRegistry& _blockRegistry, const PrefabRegistry& _prefabRegistry) :
    blockRegistry(_blockRegistry),
    terrainWorkers(std::thread::hardware_concurrency()),
    decorationWorkers(std::thread::hardware_concurrency()),
    terrainGenerator(_blockRegistry, _prefabRegistry)
{
    this->terrainWorkers.setWorker([this](const ChunkJob &job) {
        terrainJob(job);
    });

    this->decorationWorkers.setWorker([this](const ChunkJob &job) {
        decorationJob(job);
    });
}

std::shared_lock<std::shared_mutex> ChunkManager::acquireReadLock() const
{
    return std::shared_lock(chunksMutex);
}

void ChunkManager::requestChunk(const ChunkPos& pos)
{
    std::unique_lock lock(chunksMutex);

    if (chunks.contains(pos))
        return;

    auto [it, inserted] = this->chunks.try_emplace(pos, std::make_unique<Chunk>(pos));
    Chunk& chunk = *it->second;

    chunk.bumpGenerationID();
    chunk.setState(ChunkState::TERRAIN_PENDING);

    this->terrainWorkers.enqueue({pos, static_cast<float>(pos.y), chunk.getGenerationID()});
}

void ChunkManager::terrainJob(const ChunkJob& job)
{
    Chunk* chunk = getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk || chunk->getGenerationID() != job.generationID)
        return;

    chunk->setState(ChunkState::TERRAIN_GENERATING);
    this->terrainGenerator.generate(*chunk);
    chunk->setState(ChunkState::TERRAIN_DONE);

    // Queue neighbors for decoration
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                this->tryQueueDecoration({job.pos.x + dx, job.pos.y + dy, job.pos.z + dz});
            }
        }
    }
}

void ChunkManager::decorationJob(const ChunkJob& job)
{
    Chunk* chunk = this->getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk || chunk->getGenerationID() != job.generationID)
        return;

    if (!tryAcquireDecorationLock(job.pos)) {
        chunk->setState(ChunkState::TERRAIN_DONE);
        tryQueueDecoration(job.pos);
        return;
    }

    chunk->setState(ChunkState::DECOR_GENERATING);

    NeighborAccess neighbors(job.pos, [this](const ChunkPos& p) {
        return this->getChunk(p.x, p.y, p.z);
    });

    if (!neighbors.allNeighborsReady()) {
        chunk->setState(ChunkState::TERRAIN_DONE);
        releaseDecorationLock(job.pos);
        return;
    }

    this->terrainGenerator.decorate(*chunk, neighbors);
    chunk->setState(ChunkState::DECOR_DONE);

    releaseDecorationLock(job.pos);

    chunk->finalizeGeneration();
}

void ChunkManager::tryQueueDecoration(const ChunkPos& pos)
{
    Chunk* chunk = this->getChunk(pos.x, pos.y, pos.z);

    if (!chunk)
        return;
    if (chunk->getState() != ChunkState::TERRAIN_DONE)
        return;

    if (this->canDecorate(pos)) {
        chunk->setState(ChunkState::DECOR_PENDING);
        this->decorationWorkers.enqueue({pos, static_cast<float>(pos.y), chunk->getGenerationID()});
    }
}

bool ChunkManager::canDecorate(const ChunkPos& pos)
{
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                const int neighborY = pos.y + dy;
                const Chunk* neighbor = this->getChunk(pos.x + dx, neighborY, pos.z + dz);

                if (!neighbor) {
                    if (neighborY >= 0)
                        return false;
                    continue;
                }

                if (!hasTerrainComplete(neighbor->getState()))
                    return false;
            }
        }
    }
    return true;
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
        else if (n->getState() >= ChunkState::DECOR_DONE)
            n->setState(ChunkState::DECOR_DONE);
    }
}

void ChunkManager::setViewDistance(const uint8_t dist)
{
    this->viewDistance = dist;
    this->unloadDistance = dist + 2;
}

void ChunkManager::updateStreaming(const glm::vec3& playerPos)
{
    auto [playerX, playerY, playerZ] = ChunkPos::fromWorld(playerPos);
    std::unordered_set<ChunkPos, ChunkPosHash> wanted;

    wanted.reserve((2 * this->viewDistance + 1) * (2 * this->viewDistance + 1) * (2 * this->viewDistance + 1));

    for (int z = -this->viewDistance; z <= this->viewDistance; ++z) {
        for (int y = -this->viewDistance; y <= this->viewDistance; ++y) {
            const int generatedY = playerY + y;

            for (int x = -this->viewDistance; x <= this->viewDistance; ++x) {
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
    for (const ChunkPos& pos : wanted) {
        requestChunk(pos);
    }

    // Unload distant chunks
    {
        std::unique_lock lock(chunksMutex);
        for (auto it = chunks.begin(); it != chunks.end(); ) {
            Chunk& chunk = *it->second;

            const int dx = chunk.getPosition().x - playerX;
            const int dy = chunk.getPosition().y - playerY;
            const int dz = chunk.getPosition().z - playerZ;

            if (std::abs(dx) > this->unloadDistance ||
                std::abs(dy) > this->unloadDistance ||
                std::abs(dz) > this->unloadDistance)
            {
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
    std::shared_lock lock(chunksMutex);
    std::vector<Chunk*> out;

    for (auto &c: this->chunks | std::views::values)
    {
        if (c->getState() != ChunkState::READY)
            continue;

        const auto& [x, y, z] = c->getPosition();
        const glm::vec3 min(x * 16, y * 16, z * 16);
        const glm::vec3 max = min + glm::vec3(16.0f);

        if (frustum.isBoxVisible(min, max))
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

bool ChunkManager::tryAcquireDecorationLock(const ChunkPos& pos)
{
    std::lock_guard lock(decorationLockMutex);

    // Check if any chunk in 3x3x3 region is already locked
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                ChunkPos check = {pos.x + dx, pos.y + dy, pos.z + dz};
                if (decorationLocks.contains(check)) {
                    return false;  // Conflict with another decoration job
                }
            }
        }
    }

    // Lock all chunks in region
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                decorationLocks.insert({pos.x + dx, pos.y + dy, pos.z + dz});
            }
        }
    }
    return true;
}

void ChunkManager::releaseDecorationLock(const ChunkPos& pos)
{
    std::lock_guard lock(decorationLockMutex);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                decorationLocks.erase({pos.x + dx, pos.y + dy, pos.z + dz});
            }
        }
    }
}