#ifndef FARFIELD_MSDFFONT_H
#define FARFIELD_MSDFFONT_H

#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

// charset -> !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~€£²ÀÂÈÉÊÎÏÔÙÛàâçèéêîïôùû

struct GlyphInfo {
    int x, y;
    int width, height;
    int xOffset;
    int yOffset;
    int xAdvance;
};

class MsdfFont {
    std::unordered_map<char32_t, GlyphInfo> glyphs;
    GLuint ID{0};

    int atlasW{0}, atlasH{0};
    int lineHeight{0};
    int base{0};
    float distanceRange{4.f};

    public:
        explicit MsdfFont(const std::string& jsonPath, const std::string& pngPath);
        ~MsdfFont();

        void bindTexture() const;

        const GlyphInfo* getGlyph(char32_t c) const;
        std::array<glm::vec2, 4> getUVs(const GlyphInfo& g) const;

        GLuint getTextureId() const { return ID; }
        int getLineHeight() const { return lineHeight; }
        int getBase() const { return base; }
        float getDistanceRange() const { return distanceRange; }
        int getAtlasW() const { return atlasW; }
        int getAtlasH() const { return atlasH; }

        static char32_t decodeUtf8(const std::string& s, size_t& i);
};


#endif