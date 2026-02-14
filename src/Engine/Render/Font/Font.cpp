#include "Font.h"

Font::Font(const TextureId _textureID) :
    textureID(_textureID)
{
    size_t i = 0;
    size_t glyphIndex = 0;
    while (i < this->CHARS.size()) {
        const glm::vec2 p{ (glyphIndex % COL) * CHAR_SIZE_X, (glyphIndex / COL) * CHAR_SIZE_Y };
        char32_t cp = decodeUtf8(this->CHARS, i);

        const FontUVArray uvs = {
            // Triangle 1: TL, BL, BR
            glm::vec2{ rangeX(p.x), rangeY(p.y) },
            glm::vec2{ rangeX(p.x), rangeY(p.y + CHAR_SIZE_Y) },
            glm::vec2{ rangeX(p.x + CHAR_SIZE_X), rangeY(p.y + CHAR_SIZE_Y) },

            // Triangle 2: TL, BR, TR
            glm::vec2{ rangeX(p.x), rangeY(p.y) },
            glm::vec2{ rangeX(p.x + CHAR_SIZE_X), rangeY(p.y + CHAR_SIZE_Y) },
            glm::vec2{ rangeX(p.x + CHAR_SIZE_X), rangeY(p.y) },
        };

        this->charMap.insert(std::make_pair(cp, uvs));
        ++glyphIndex;
    }
}

FontUVArray Font::getUVFromChar(const char32_t c) const
{
    const auto& uvIt = this->charMap.find(c);

    if (uvIt != this->charMap.end())
        return uvIt->second;
    return this->charMap.at('?');
}

std::vector<FontUVArray> Font::getUVFromString(const std::string &str) const
{
    std::vector<FontUVArray> uvsArray;
    size_t i = 0;

    while (i < str.size()) {
        const char32_t cp = decodeUtf8(str, i);
        uvsArray.emplace_back(this->getUVFromChar(cp));
    }

    return uvsArray;
}

// Statics
float Font::rangeX(const float &v)
{
    static constexpr float W = static_cast<float>(COL) * CHAR_SIZE_X;
    return Maths::mapRange(v, 0.f, W, 0.f, 1.f);
}

float Font::rangeY(const float &v)
{
    static constexpr float H = static_cast<float>(ROW) * CHAR_SIZE_Y;
    return Maths::mapRange(v, 0.f, H, 0.f, 1.f);
}

char32_t Font::decodeUtf8(const std::string& s, size_t& i)
{
    const uint8_t c = s[i];
    char32_t cp;
    int extra;

    if (c < 0x80)        { cp = c;          extra = 0; }
    else if (c < 0xE0)   { cp = c & 0x1F;   extra = 1; }
    else if (c < 0xF0)   { cp = c & 0x0F;   extra = 2; }
    else                 { cp = c & 0x07;   extra = 3; }

    for (int j = 0; j < extra; j++)
        cp = (cp << 6) | (s[++i] & 0x3F);
    ++i;
    return cp;
}