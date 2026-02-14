#ifndef FARFIELD_FONT_H
#define FARFIELD_FONT_H

#include <cstdint>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "TextureRegistry.h"
#include "Utils.h"

using FontUVArray = std::array<glm::vec2, 6>;

class Font
{
    static constexpr std::uint8_t COL = 20;
    static constexpr std::uint8_t ROW = 6;
    const std::string CHARS = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~€£²ÀÂÈÉÊÎÏÔÙÛàâœçèéêîïôùû";

    std::unordered_map<char32_t, FontUVArray> charMap;
    TextureId textureID;

    static float rangeX(const float& v);
    static float rangeY(const float& v);
    static char32_t decodeUtf8(const std::string& s, size_t& i);

    public:
        static constexpr float CHAR_SIZE_X = 16.f;
        static constexpr float CHAR_SIZE_Y = 26.f;

        explicit Font(TextureId _textureID);

        const TextureId& getTextureID() const { return textureID; };

        FontUVArray getUVFromChar(char32_t c) const;
        std::vector<FontUVArray> getUVFromString(const std::string& str) const;
};

#endif