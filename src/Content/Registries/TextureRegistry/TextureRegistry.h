#ifndef FARFIELD_TEXTUREREGISTRY_H
#define FARFIELD_TEXTUREREGISTRY_H

#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <climits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi/stb_image.h>

namespace fs = std::filesystem;

using TextureId = unsigned short;

struct TextureSlot {
    GLuint layer;
    float u0, v0;
    float u1, v1;
    uint16_t width, height;
};

// GPU-side slot for SSBO (std430 layout, 32 bytes)
struct alignas(16) GpuTextureSlot {
    float u0, v0, u1, v1;
    GLuint layer;
    GLuint _pad0, _pad1, _pad2;
};

class AtlasLayer {
    public:
        static constexpr int SIZE = 2048;
        static constexpr int PADDING = 0;

        struct FreeRect {
            int x, y, width, height;
        };

        AtlasLayer();
        std::pair<int, int> tryInsert(int w, int h);

    private:
        std::vector<FreeRect> freeRects;
};

class TextureRegistry
{
    static constexpr int LAYER_SIZE = 2048;
    static constexpr int BORDER = 1;

    GLuint ID{};
    GLuint slotsSSBO{};

    struct PendingTexture {
        std::string name;
        std::string path;
        int width, height;
        stbi_uc* data;
    };

    struct PlacedTexture {
        int x, y;
        size_t layer;
    };

    static stbi_uc* createExtrudedTexture(const stbi_uc* src, int srcW, int srcH, int border);

    std::vector<AtlasLayer> layers;
    std::vector<TextureSlot> textureSlots;
    std::vector<PendingTexture> pending;
    std::unordered_map<std::string, TextureId> nameToTextureId;

    public:
        static constexpr std::string MISSING = "MISSING";

        TextureRegistry();
        ~TextureRegistry();

        TextureId registerTexture(const std::string& name, const std::string& path);
        void registerTextureFromFolder(const std::string& folderPath);

        void createTextures();
        const TextureSlot& getSlot(TextureId id) const;
        TextureId getByName(const std::string& name) const;

        void bind() const;
        void bindSlots() const;
};

#endif