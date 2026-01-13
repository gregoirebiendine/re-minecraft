#include "TextureRegistry.h"

TextureRegistry::TextureRegistry()
{
    this->registerTexture(MISSING, "missing.png");
    this->registerTexture("dirt", "dirt.png");
    this->registerTexture("grass_block_side", "grass_block_side.png");
    this->registerTexture("grass_block_top", "grass_block_top.png");
    this->registerTexture("cobble", "cobble.png");
    this->registerTexture("stone", "stone.png");
    this->registerTexture("oak_plank", "oak_plank.png");
}

TextureRegistry::~TextureRegistry()
{
    glDeleteTextures(1, &this->ID);
}

void TextureRegistry::createTextures()
{
    const auto texturesFolder = fs::current_path().parent_path().string().append("/resources/textures/");

    // Create OpenGL texture
    glGenTextures(1, &this->ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);

    // Create 2D array of textures with W=32 and H=32
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 32, 32, static_cast<int>(this->textures.size()));

    int w, h, channels;
    for (int i = 0; i < static_cast<int>(this->textures.size()); i++) {
        stbi_uc* tex = stbi_load((texturesFolder + this->textures[i]).c_str(), &w, &h, &channels, 0);

        if (!tex)
            throw std::runtime_error("Error loading texture : " + this->textures[i]);
        if (w != 32 || h != 32)
            throw std::runtime_error("Texture " + this->textures[i] + " is not 32 pixel wide");

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 32, 32, 1, GL_RGBA, GL_UNSIGNED_BYTE, tex);
        stbi_image_free(tex);
    }

    // Set texture loading filter (nearest to keep pixels)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Generate Mipmaps
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // Activate Texture
    glActiveTexture(GL_TEXTURE0);
}

TextureId TextureRegistry::registerTexture(const std::string& name, const std::string& path)
{
    // Prevent duplication
    auto it = this->nameToTextureId.find(name);
    if (it != this->nameToTextureId.end())
        return it->second;

    // Enforce max ID range
    if (this->textures.size() >= std::numeric_limits<TextureId>::max())
        throw std::runtime_error("BlockRegistry overflow");

    auto id = static_cast<TextureId>(this->textures.size());

    this->textures.push_back(path);
    this->nameToTextureId.emplace(name, id);

    return id;
}

const std::string& TextureRegistry::get(const TextureId id) const
{
    if (id >= this->textures.size())
        throw std::out_of_range("Invalid Material");

    return this->textures[id];
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