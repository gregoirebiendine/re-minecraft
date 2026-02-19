#include "TextWidget.h"

TextWidget::TextWidget(const Font& font, const glm::vec2 position, const float scale) :
    font(font),
    fontTexId(this->font.getTextureID()),
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

void TextWidget::buildSelf(std::vector<GuiVertex>& out, const glm::vec2 abs)
{
    if (this->cachedText.empty())
        return;

    const auto& uvs = this->font.getUVFromString(this->cachedText);
    const float charW = Font::CHAR_SIZE_X * this->scale;
    const float charH = Font::CHAR_SIZE_Y * this->scale;
    const float y = abs.y;
    const float y1 = y + charH;
    const auto textWidth = static_cast<float>(uvs.size()) * charW;

    float curX = abs.x;
    if (this->align == TextAlign::Center)
        curX -= textWidth / 2.f;
    else if (this->align == TextAlign::Right)
        curX -= textWidth;

    for (const auto& uv : uvs) {
        const float charX1 = curX + charW;

        out.insert(out.end(), {
            {{curX, y},     uv[0], this->color, this->fontTexId},
            {{curX, y1},    uv[1], this->color, this->fontTexId},
            {{charX1, y1},  uv[2], this->color, this->fontTexId},

            {{curX, y},     uv[0], this->color, this->fontTexId},
            {{charX1, y1},  uv[2], this->color, this->fontTexId},
            {{charX1, y},   uv[3], this->color, this->fontTexId},
        });
        curX += charW;
    }
    this->setSize({textWidth, charH});
}
