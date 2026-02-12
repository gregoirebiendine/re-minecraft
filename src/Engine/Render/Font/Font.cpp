#include "Font.h"

Font::Font(const TextureId _textureID) :
    textureID(_textureID)
{
    for (int i = 0; i < static_cast<int>(this->supportedChars.size()); i++) {
        const glm::vec2 p{ (i % CHAR_PER_ROW) * CHAR_SIZE, (i / CHAR_PER_ROW) * CHAR_SIZE };
        const FontUVArray uvs = {
            // Triangle 1: TL, BL, BR
            glm::vec2{ rangeX(p.x), rangeY(p.y) },
            glm::vec2{ rangeX(p.x), rangeY(p.y + CHAR_SIZE) },
            glm::vec2{ rangeX(p.x + CHAR_SIZE), rangeY(p.y + CHAR_SIZE) },

            // Triangle 2: TL, BR, TR
            glm::vec2{ rangeX(p.x), rangeY(p.y) },
            glm::vec2{ rangeX(p.x + CHAR_SIZE), rangeY(p.y + CHAR_SIZE) },
            glm::vec2{ rangeX(p.x + CHAR_SIZE), rangeY(p.y) },
        };

        this->charMap.insert(std::make_pair(this->supportedChars[i], uvs));
    }
}

FontUVArray Font::getUVFromChar(const char c) const
{
    const auto& uvIt = this->charMap.find(c);

    if (uvIt != this->charMap.end())
        return uvIt->second;
    return this->charMap.at('?');
}

std::vector<FontUVArray> Font::getUVFromString(const std::string &str) const
{
    std::vector<FontUVArray> uvsArray;

    for (const auto& c : str)
        uvsArray.emplace_back(this->getUVFromChar(c));

    return uvsArray;
}

// Statics
float Font::rangeX(const float &v)
{
    static constexpr float W = static_cast<float>(CHAR_PER_ROW) * CHAR_SIZE;
    return Maths::mapRange(v, 0.f, W, 0.f, 1.f);
}

float Font::rangeY(const float &v)
{
    static constexpr float H = 5.f * CHAR_SIZE;
    return Maths::mapRange(v, 0.f, H, 0.f, 1.f);
}
