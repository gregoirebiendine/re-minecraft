#ifndef FARFIELD_IMAGEWIDGET_H
#define FARFIELD_IMAGEWIDGET_H

#include <string>
#include <functional>

#include "AWidget.h"
#include "TextureRegistry.h"

class ImageWidget : public AWidget
{
    const TextureRegistry& texRegistry;
    TextureId texId;
    std::function<TextureId()> textureBinding;

    public:
        ImageWidget(const TextureRegistry& texRegistry, const std::string& textureName, glm::vec2 position, glm::vec2 size);

        void bind(std::function<TextureId()> fn);

        void tick() override;
        void buildSelf(std::vector<GuiVertex>& out, glm::vec2 abs) override;
};

#endif
