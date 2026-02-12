#ifndef FARFIELD_FONT_H
#define FARFIELD_FONT_H

#include <cstdint>
#include <iostream>
#include <array>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "TextureRegistry.h"
#include "Utils.h"

using FontUVArray = std::array<glm::vec2, 6>;

class Font
{
    static constexpr std::uint8_t CHAR_PER_ROW = 20;

    const std::string supportedChars = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    std::unordered_map<unsigned char, FontUVArray> charMap;
    TextureId textureID;

    static float rangeX(const float& v);
    static float rangeY(const float& v);

    public:
        static constexpr float CHAR_SIZE = 20.f;

        explicit Font(TextureId _textureID);

        const TextureId& getTextureID() const { return textureID; };

        FontUVArray getUVFromChar(char c) const;
        std::vector<FontUVArray> getUVFromString(const std::string& str) const;
};

#endif