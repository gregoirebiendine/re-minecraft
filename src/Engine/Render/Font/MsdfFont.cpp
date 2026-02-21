#define STB_IMAGE_IMPLEMENTATION_ALREADY_DEFINED

#include "MsdfFont.h"

#include <fstream>
#include <stdexcept>

#include <json.hpp>
#include <stbi/stb_image.h>

MsdfFont::MsdfFont(const std::string &jsonPath, const std::string &pngPath)
{
    std::ifstream f(jsonPath);

    if (!f)
        throw std::runtime_error("[MsdfFont::load] Cannot open " + jsonPath);

    nlohmann::json j = nlohmann::json::parse(f);
    const auto& common = j["common"];
    const auto& df = j["distanceField"];

    this->atlasW      = common["scaleW"];
    this->atlasH      = common["scaleH"];
    this->lineHeight  = common["lineHeight"];
    this->base        = common["base"];
    this->distanceRange = df["distanceRange"].get<float>();

    for (auto& ch : j["chars"]) {
        GlyphInfo g{};

        g.x        = ch["x"];
        g.y        = ch["y"];
        g.width    = ch["width"];
        g.height   = ch["height"];
        g.xOffset  = ch["xoffset"];
        g.yOffset  = ch["yoffset"];
        g.xAdvance = ch["xadvance"];

        this->glyphs[static_cast<char32_t>(ch["id"].get<int>())] = g;
    }

    int w, h, channels;
    unsigned char* data = stbi_load(pngPath.c_str(), &w, &h, &channels, 4);

    if (!data)
        throw std::runtime_error("[MsdfFont::load] Cannot load " + pngPath);

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
}

MsdfFont::~MsdfFont()
{
    glDeleteTextures(1, &ID);
}

const GlyphInfo* MsdfFont::getGlyph(const char32_t c) const
{
    auto it = glyphs.find(c);

    if (it != glyphs.end())
        return &it->second;

    it = glyphs.find('?');
    return it != glyphs.end() ? &it->second : nullptr;
}

std::array<glm::vec2, 4> MsdfFont::getUVs(const GlyphInfo& g) const
{
    const float u0 = static_cast<float>(g.x) / atlasW;
    const float v0 = static_cast<float>(g.y) / atlasH;
    const float u1 = static_cast<float>(g.x + g.width) / atlasW;
    const float v1 = static_cast<float>(g.y + g.height) / atlasH;

    return {{ {u0,v0}, {u0,v1}, {u1,v1}, {u1,v0} }};
}

void MsdfFont::bindTexture() const
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->ID);
}

char32_t MsdfFont::decodeUtf8(const std::string& s, size_t& i)
{
    const uint8_t c = s[i];
    char32_t cp; int extra;
    if      (c < 0x80) { cp = c;        extra = 0; }
    else if (c < 0xE0) { cp = c & 0x1F; extra = 1; }
    else if (c < 0xF0) { cp = c & 0x0F; extra = 2; }
    else               { cp = c & 0x07; extra = 3; }
    for (int j = 0; j < extra; j++) cp = (cp << 6) | (s[++i] & 0x3F);
    ++i;
    return cp;
}