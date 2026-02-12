#include "Font.h"

Font::Font()
{
    for (int i = 0; i < static_cast<int>(this->supportedChars.size()); i++) {
        const glm::vec2 p{ i % CHAR_PER_ROW, i / CHAR_PER_ROW };
        const FontUVArray uvs = {
            glm::vec2{ rangeX(p.x), rangeY(p.y + CHAR_SIZE) },                  // uv 1 : x | y + 30
            glm::vec2{ rangeX(p.x + CHAR_SIZE), rangeY(p.y + CHAR_SIZE) },    // uv 2 : x + 30 | y + 30
            glm::vec2{ rangeX(p.x), rangeY(p.y) },                                // uv 3 : x | y

            glm::vec2{ rangeX(p.x + CHAR_SIZE), rangeY(p.y + CHAR_SIZE) },    // uv 4 : x + 30 | y + 30
            glm::vec2{ rangeX(p.x + CHAR_SIZE), rangeY(p.y) },                  // uv 5 : x + 30 | y
            glm::vec2{ rangeX(p.x), rangeY(p.y) }                                 // uv 6 : x | y
        };

        this->charMap.insert(std::make_pair(this->supportedChars[i], uvs));
    }
}

FontUVArray Font::getUVFromChar(const char c) const
{
    const auto& uvIt = this->charMap.find(c);

    if (uvIt != this->charMap.end())
        return uvIt->second;
    return this->charMap.at(0);
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
    return Maths::mapRange(v, 0.f, FONT_W, 0.f, 1.f);
}

float Font::rangeY(const float &v)
{
    return Maths::mapRange(v, 0.f, FONT_H, 0.f, 1.f);
}
