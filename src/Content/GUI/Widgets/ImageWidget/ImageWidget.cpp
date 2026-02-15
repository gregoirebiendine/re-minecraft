#include "ImageWidget.h"

ImageWidget::ImageWidget(const TextureRegistry& texRegistry, const std::string& textureName, const glm::vec2 position, const glm::vec2 size) :
    texRegistry(texRegistry),
    texId(this->texRegistry.getByName(textureName))
{
    this->setPosition(position);
    this->setSize(size);
}

void ImageWidget::buildSelf(std::vector<GuiVertex>& out, const glm::vec2 abs)
{
    constexpr glm::vec4 color{1.f};
    const glm::vec2 s = this->getSize();

    const float x = abs.x;
    const float y = abs.y;
    const float x1 = x + s.x;
    const float y1 = y + s.y;

    out.insert(out.end(), {
        {{x, y},   {0.f, 0.f}, color, this->texId},
        {{x, y1},  {0.f, 1.f}, color, this->texId},
        {{x1, y1}, {1.f, 1.f}, color, this->texId},

        {{x, y},   {0.f, 0.f}, color, this->texId},
        {{x1, y1}, {1.f, 1.f}, color, this->texId},
        {{x1, y},  {1.f, 0.f}, color, this->texId},
    });
}
