#include "TextureRegistry.h"

stbi_uc* TextureRegistry::createExtrudedTexture(const stbi_uc* src, int srcW, int srcH, int border)
{
    int dstW = srcW + 2 * border;
    int dstH = srcH + 2 * border;
    auto* dst = static_cast<stbi_uc*>(calloc(dstW * dstH * 4, 1));

    // Copy original image into center
    for (int y = 0; y < srcH; y++)
        std::memcpy(dst + ((y + border) * dstW + border) * 4, src + (y * srcW) * 4, srcW * 4);

    // Extrude all rows (including border rows) left and right
    for (int dy = 0; dy < dstH; dy++) {
        int srcY = std::clamp(dy - border, 0, srcH - 1);

        // Fill left border columns
        for (int b = 0; b < border; b++)
            std::memcpy(dst + (dy * dstW + b) * 4, src + (srcY * srcW) * 4, 4);

        // Fill right border columns
        for (int b = 0; b < border; b++)
            std::memcpy(dst + (dy * dstW + border + srcW + b) * 4, src + (srcY * srcW + srcW - 1) * 4, 4);
    }

    // Extrude top and bottom border rows (center portion already correct, fill border rows)
    for (int b = 0; b < border; b++) {
        // Top border row: duplicate first source row into center columns
        std::memcpy(dst + (b * dstW + border) * 4, src, srcW * 4);
        // Bottom border row: duplicate last source row into center columns
        std::memcpy(dst + ((border + srcH + b) * dstW + border) * 4, src + ((srcH - 1) * srcW) * 4, srcW * 4);
    }

    return dst;
}

AtlasLayer::AtlasLayer()
{
    freeRects.push_back({0, 0, SIZE, SIZE});
}

std::pair<int, int> AtlasLayer::tryInsert(int w, int h)
{
    int paddedW = w + PADDING;
    int paddedH = h + PADDING;

    int bestIndex = -1;
    int bestShortSide = INT_MAX;

    for (size_t i = 0; i < freeRects.size(); i++) {
        auto& rect = freeRects[i];
        if (rect.width >= paddedW && rect.height >= paddedH) {
            int shortSide = std::min(rect.width - paddedW, rect.height - paddedH);
            if (shortSide < bestShortSide) {
                bestShortSide = shortSide;
                bestIndex = static_cast<int>(i);
            }
        }
    }

    if (bestIndex < 0)
        return {-1, -1};

    FreeRect chosen = freeRects[bestIndex];
    freeRects.erase(freeRects.begin() + bestIndex);

    // Split remaining space (guillotine split)
    // Right remainder
    if (chosen.width - paddedW > 0) {
        freeRects.push_back({
            chosen.x + paddedW,
            chosen.y,
            chosen.width - paddedW,
            paddedH
        });
    }
    // Bottom remainder
    if (chosen.height - paddedH > 0) {
        freeRects.push_back({
            chosen.x,
            chosen.y + paddedH,
            chosen.width,
            chosen.height - paddedH
        });
    }

    return {chosen.x, chosen.y};
}

// TextureRegistry implementation
TextureRegistry::TextureRegistry()
{
    this->registerTextureFromFolder("/resources/textures/blocks");
    this->registerTextureFromFolder("/resources/textures/items");
    this->registerTextureFromFolder("/resources/textures/entities");
    this->registerTextureFromFolder("/resources/textures/gui");
    this->createTextures();
}

TextureRegistry::~TextureRegistry()
{
    glDeleteTextures(1, &this->ID);
    if (slotsSSBO)
        glDeleteBuffers(1, &this->slotsSSBO);
}

void TextureRegistry::createTextures()
{
    if (pending.empty())
        return;

    // Sort by height descending for better packing (keep indices for mapping)
    std::vector<size_t> sortedIndices(pending.size());
    for (size_t i = 0; i < pending.size(); i++)
        sortedIndices[i] = i;

    std::sort(sortedIndices.begin(), sortedIndices.end(), [this](size_t a, size_t b) {
        return pending[a].height > pending[b].height;
    });

    // Prepare slots vector (indexed by original TextureId)
    textureSlots.resize(pending.size());
    std::vector<PlacedTexture> placements(pending.size());

    // Start with one layer
    layers.emplace_back();

    // Pack textures (using expanded dimensions to account for border extrusion)
    for (const size_t idx : sortedIndices) {
        const auto& tex = pending[idx];
        const int expandedW = tex.width + 2 * BORDER;
        const int expandedH = tex.height + 2 * BORDER;
        bool placed = false;

        for (size_t layerIdx = 0; layerIdx < layers.size(); layerIdx++) {
            auto [x, y] = layers[layerIdx].tryInsert(expandedW, expandedH);
            if (x >= 0) {
                int innerX = x + BORDER;
                int innerY = y + BORDER;
                textureSlots[idx] = {
                    static_cast<GLuint>(layerIdx),
                    static_cast<float>(innerX) / static_cast<float>(LAYER_SIZE),
                    static_cast<float>(innerY) / static_cast<float>(LAYER_SIZE),
                    static_cast<float>(innerX + tex.width) / static_cast<float>(LAYER_SIZE),
                    static_cast<float>(innerY + tex.height) / static_cast<float>(LAYER_SIZE),
                    static_cast<uint16_t>(tex.width),
                    static_cast<uint16_t>(tex.height)
                };
                placements[idx] = {x, y, layerIdx};
                placed = true;
                break;
            }
        }

        if (!placed) {
            // Create new layer
            layers.emplace_back();
            auto [x, y] = layers.back().tryInsert(expandedW, expandedH);
            if (x < 0)
                throw std::runtime_error("Texture too large for atlas: " + tex.path);

            int innerX = x + BORDER;
            int innerY = y + BORDER;
            size_t layerIdx = layers.size() - 1;
            textureSlots[idx] = {
                static_cast<GLuint>(layerIdx),
                static_cast<float>(innerX) / static_cast<float>(LAYER_SIZE),
                static_cast<float>(innerY) / static_cast<float>(LAYER_SIZE),
                static_cast<float>(innerX + tex.width) / static_cast<float>(LAYER_SIZE),
                static_cast<float>(innerY + tex.height) / static_cast<float>(LAYER_SIZE),
                static_cast<uint16_t>(tex.width),
                static_cast<uint16_t>(tex.height)
            };
            placements[idx] = {x, y, layerIdx};
        }
    }

    // Create OpenGL texture array
    glGenTextures(1, &this->ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, LAYER_SIZE, LAYER_SIZE, static_cast<GLsizei>(layers.size()));

    // Upload each texture with extruded borders to its packed position
    for (size_t i = 0; i < pending.size(); i++) {
        auto& place = placements[i];
        auto& tex = pending[i];

        const int expandedW = tex.width + 2 * BORDER;
        const int expandedH = tex.height + 2 * BORDER;

        stbi_uc* extruded = createExtrudedTexture(tex.data, tex.width, tex.height, BORDER);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
            place.x, place.y, static_cast<GLint>(place.layer),
            expandedW, expandedH, 1,
            GL_RGBA, GL_UNSIGNED_BYTE, extruded);

        free(extruded);
        stbi_image_free(tex.data);
    }
    pending.clear();

    // Set texture filtering (no mipmaps)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Activate Texture
    glActiveTexture(GL_TEXTURE0);

    // Create SSBO for texture slots
    std::vector<GpuTextureSlot> gpuSlots;
    gpuSlots.reserve(textureSlots.size());
    for (const auto& slot : textureSlots) {
        gpuSlots.push_back({
            slot.u0, slot.v0, slot.u1, slot.v1,
            slot.layer,
            0, 0, 0
        });
    }

    glGenBuffers(1, &this->slotsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->slotsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 gpuSlots.size() * sizeof(GpuTextureSlot),
                 gpuSlots.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

TextureId TextureRegistry::registerTexture(const std::string& name, const std::string& path)
{
    // Prevent duplication
    auto it = this->nameToTextureId.find(name);
    if (it != this->nameToTextureId.end())
        return it->second;

    // Enforce max ID range
    if (this->pending.size() >= std::numeric_limits<TextureId>::max())
        throw std::runtime_error("[TextureRegistry::registerTexture] TextureRegistry overflow");

    // Load image to get dimensions
    int w, h, channels;
    stbi_uc* data = stbi_load(path.c_str(), &w, &h, &channels, 4);

    if (!data)
        throw std::runtime_error("[TextureRegistry::registerTexture] stbi_load failed on texture: " + path);

    if (w > AtlasLayer::SIZE || h > AtlasLayer::SIZE)
        throw std::runtime_error("[TextureRegistry::registerTexture] Texture too large (max 1024x1024): " + path);

    auto id = static_cast<TextureId>(this->pending.size());

    this->pending.push_back({name, path, w, h, data});
    this->nameToTextureId.emplace(name, id);

    return id;
}

void TextureRegistry::registerTextureFromFolder(const std::string &folderPath)
{
    const auto texturesFolder = fs::current_path().parent_path().string().append(folderPath);

    for (const auto& entry : fs::directory_iterator(texturesFolder)) {
        const auto& path = entry.path();
        const auto texName = path.filename().replace_extension();

        if (texName == "missing")
            this->registerTexture(MISSING, path.string());
        else
            this->registerTexture(texName.string(), path.string());
    }
}

const TextureSlot& TextureRegistry::getSlot(const TextureId id) const
{
    if (id >= this->textureSlots.size())
        throw std::out_of_range("[TextureRegistry::getSlot] Invalid TextureId");

    return this->textureSlots[id];
}

TextureId TextureRegistry::getByName(const std::string& name) const
{
    if (!this->nameToTextureId.contains(name))
        return this->nameToTextureId.at(MISSING);
    return this->nameToTextureId.at(name);
}

void TextureRegistry::bind() const {
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);
}

void TextureRegistry::bindSlots() const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->slotsSSBO);
}
