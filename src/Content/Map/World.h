#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>
#include <memory>
#include <ranges>

#include <ChunkMeshManager.h>
#include <Chunk.h>
#include <Shader.h>

class World {
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunks;
    ChunkMeshManager meshManager;

    static int mod(int a, int b);
    static ChunkPos worldToChunk(int wx, int wy, int wz);
    static int floorDiv(int a, int b);

    public:
        World();

        // Getters
        Chunk& getOrCreateChunk(int cx, int cy, int cz);
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz);
        [[nodiscard]] Material getBlock(int wx, int wy, int wz) const;

        // Setters
        void setBlock(int wx, int wy, int wz, Material id);

        void render(const Shader& shaders);
};

#endif //WORLD_H
