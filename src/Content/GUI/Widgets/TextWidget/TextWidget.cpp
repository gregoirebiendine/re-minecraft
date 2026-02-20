#include "TextWidget.h"

TextWidget::TextWidget(const MsdfFont& font, const glm::vec2 position, const float scale) :
    font(font),
    scale(scale)
{
    this->setPosition(position);
}

void TextWidget::setText(const std::string& text)
{
    if (text != this->cachedText) {
        this->cachedText = text;
        this->markDirty();
    }
}

void TextWidget::bind(std::function<std::string()> fn)
{
    this->textBinding = std::move(fn);
}

void TextWidget::setAlignment(const TextAlign a)
{
    this->align = a;
    this->markDirty();
}

void TextWidget::setShadow(bool _shadow)
{
    this->shadow = _shadow;
    this->markDirty();
}

void TextWidget::tick()
{
    if (this->textBinding) {
        auto newText = this->textBinding();
        if (newText != this->cachedText) {
            this->cachedText = std::move(newText);
            this->markDirty();
        }
    }
    AWidget::tick();
}

void TextWidget::buildMsdf(std::vector<MSDFVertex>& out, const glm::vec2 abs)
{
    if (this->cachedText.empty())
        return;

    const auto lineH = static_cast<float>(this->font.getLineHeight());
    float curX = abs.x;

    if (this->align != TextAlign::Left) {
        float totalW = 0.f;
        size_t i = 0;

        while (i < cachedText.size()) {
            char32_t cp = MsdfFont::decodeUtf8(cachedText, i);
            if (const GlyphInfo* g = this->font.getGlyph(cp))
                totalW += static_cast<float>(g->xAdvance) * scale;
        }

        if (align == TextAlign::Center)
            curX -= totalW / 2.f;
        else if (align == TextAlign::Right)
            curX -= totalW;
    }

    const float shadowOffset = this->scale * 3.f;
    const RGBA shadowColor = this->color * 0.25f;

    // Shadow text pass
    if (this->shadow)
        this->buildGlyph(out, curX, abs.y, shadowOffset, shadowColor);

    // Main text pass
    this->buildGlyph(out, curX, abs.y, 0.f, this->color);
    this->setSize({curX - abs.x, lineH * scale});
}

void TextWidget::buildGlyph(std::vector<MSDFVertex>& out, float curX, const float y, const float offset, const RGBA c) const
{
    size_t i = 0;
    while (i < cachedText.size()) {
        const char32_t cp = MsdfFont::decodeUtf8(this->cachedText, i);
        const GlyphInfo* g = this->font.getGlyph(cp);

        if (!g)
            continue;

        if (g->width > 0 && g->height > 0) {
            const float x0 = curX + static_cast<float>(g->xOffset) * scale + offset;
            const float y0 = y + static_cast<float>(g->yOffset) * scale + offset;
            const float x1 = x0 + static_cast<float>(g->width)  * scale;
            const float y1 = y0 + static_cast<float>(g->height) * scale;

            const auto uvs = this->font.getUVs(*g);

            out.insert(out.end(), {
                {{x0, y0}, uvs[0], c},
                {{x0, y1}, uvs[1], c},
                {{x1, y1}, uvs[2], c},

                {{x0, y0}, uvs[0], c},
                {{x1, y1}, uvs[2], c},
                {{x1, y0}, uvs[3], c},
            });
        }
        curX += static_cast<float>(g->xAdvance) * scale;
    }
}
