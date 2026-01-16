#include "ChunkManager.h"

ChunkManager::ChunkManager(BlockRegistry _blockRegistry) :
    workers(std::thread::hardware_concurrency()),
    blockRegistry(std::move(_blockRegistry))
{
    workers.setWorker([this](const ChunkJob &job) {
        generateJob(job);
    });
}

void ChunkManager::requestChunk(const ChunkPos& pos)
{
    if (chunks.contains(pos))
        return;

    Chunk& chunk = this->chunks.try_emplace(pos, pos).first->second;

    chunk.bumpGenerationID();
    chunk.setState(ChunkState::GENERATING);

    workers.enqueue({pos, 0.f, chunk.getGenerationID()});
}

void ChunkManager::generateJob(ChunkJob job) {
    Chunk* chunk = getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk)
        return;

    if (chunk->getGenerationID() != job.generationID)
        return;

    TerrainGenerator::generate(*chunk, this->blockRegistry);
    chunk->setState(ChunkState::GENERATED);
    rebuildNeighbors(job.pos);
}

Chunk* ChunkManager::getChunk(const int cx, const int cy, const int cz)
{
    const auto it = chunks.find({cx,cy,cz});
    return it == chunks.end() ? nullptr : &it->second;
}

void ChunkManager::rebuildNeighbors(const ChunkPos& pos)
{
    static const int d[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };

    for (auto& o : d) {
        Chunk* n = this->getChunk(pos.x+o[0], pos.y+o[1], pos.z+o[2]);
        if (n && n->getState() >= ChunkState::GENERATED)
            n->setState(ChunkState::GENERATED);
    }
}

void ChunkManager::updateStreaming(const glm::vec3& cameraPos)
{
    ChunkPos cameraChunk = ChunkPos::fromWorld(cameraPos);
    std::unordered_set<ChunkPos, ChunkPosHash> wanted;

    wanted.reserve((2 * VIEW_DISTANCE + 1) * (2 * VIEW_DISTANCE + 1) * 5);

    for (int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z)
        for (int y = 0; y <= 2; ++y)
            for (int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
                ChunkPos pos {
                    cameraChunk.x + x,
                    y,
                    cameraChunk.z + z
                };

                wanted.insert(pos);

                if (!this->chunks.contains(pos))
                    this->requestChunk(pos);
            }

    for (auto it = chunks.begin(); it != chunks.end(); ) {
        Chunk& chunk = it->second;

        const int dx = chunk.getPosition().x - cameraChunk.x;
        const int dy = chunk.getPosition().y - cameraChunk.y;
        const int dz = chunk.getPosition().z - cameraChunk.z;

        if (std::abs(dx) > VIEW_DISTANCE + 2 ||
            std::abs(dy) > VIEW_DISTANCE + 2 ||
            std::abs(dz) > VIEW_DISTANCE + 2)
        {
            chunk.bumpGenerationID();
            it = chunks.erase(it);
        }
        else
            ++it;
    }

    for (const ChunkPos pos : wanted) {
        Chunk* chunk = this->getChunk(pos.x, pos.y, pos.z);

        if (!chunk)
            continue;

        if (chunk->getState() != ChunkState::UNLOADED)
            continue;

        chunk->setState(ChunkState::GENERATING);
        chunk->bumpGenerationID();

        const auto center = glm::vec3(
            pos.x * Chunk::SIZE + Chunk::SIZE / 2.0f,
            pos.y * Chunk::SIZE + Chunk::SIZE / 2.0f,
            pos.z * Chunk::SIZE + Chunk::SIZE / 2.0f
        );

        workers.enqueue({
            pos,
            glm::distance(cameraPos, center),
            chunk->getGenerationID()
        });
    }
}

std::vector<Chunk *> ChunkManager::getRenderableChunks()
{
    std::vector<Chunk*> out;

    for (auto &c: this->chunks | std::views::values)
        if (c.getState() == ChunkState::READY)
            out.push_back(&c);
    return out;
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