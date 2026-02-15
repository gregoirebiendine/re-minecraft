#ifndef FARFIELD_IMAGEWIDGET_H
#define FARFIELD_IMAGEWIDGET_H

#include <string>

#include "AWidget.h"
#include "TextureRegistry.h"

class ImageWidget : public AWidget
{
    const TextureRegistry& texRegistry;
    std::string textureName;

    public:
        ImageWidget(const TextureRegistry& texRegistry, const std::string& textureName,
                    glm::vec2 position, glm::vec2 size);

        void buildSelf(std::vector<GuiVertex>& out, glm::vec2 abs) override;
};

#endif
