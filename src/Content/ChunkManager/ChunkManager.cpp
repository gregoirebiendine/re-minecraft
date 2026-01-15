#include "ChunkManager.h"

ChunkManager::ChunkManager() :
    workers(std::thread::hardware_concurrency()
)
{
    workers.setWorker([this](const ChunkJob &job) {
        generateJob(job);
    });
}

void ChunkManager::requestChunk(const ChunkPos& pos)
{
    if (chunks.contains(pos))
        return;

    const auto& chunk = this->chunks[pos];

    chunk->bumpGenerationID();
    chunk->setState(ChunkState::GENERATING);

    workers.enqueue({pos, 0.f, chunk->getGenerationID()});
}

void ChunkManager::generateJob(ChunkJob job) {
    Chunk* chunk = getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk)
        return;

    if (chunk->getGenerationID() != job.generationID)
        return;

    TerrainGenerator::generate(*chunk); // Needs blockRegistry
    chunk->setState(ChunkState::READY);
    rebuildNeighbors(job.pos);
}

Chunk* ChunkManager::getChunk(const int cx, const int cy, const int cz) const
{
    const auto it = this->chunks.find({cx, cy, cz});
    return it != this->chunks.end() ? it->second.get() : nullptr;
}

void ChunkManager::rebuildNeighbors(const ChunkPos& pos) const
{
    static const int d[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };

    for (auto& o : d) {
        Chunk* n = getChunk(pos.x+o[0], pos.y+o[1], pos.z+o[2]);
        if (n && n->getState() == ChunkState::MESHED)
            n->setState(ChunkState::READY);
    }
}

void ChunkManager::updateStreaming(const glm::vec3&) {}

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

ChunkNeighbors ChunkManager::getNeighbors(const ChunkPos& cp) const
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