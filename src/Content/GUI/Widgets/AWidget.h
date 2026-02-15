#ifndef FARFIELD_AWIDGET_H
#define FARFIELD_AWIDGET_H

#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "VAOVertices.h"

class AWidget
{
    glm::vec2 position{0.f};
    glm::vec2 size{0.f};
    bool visible{true};
    bool dirty{true};

    std::function<glm::vec2()> positionBinding;

    std::vector<std::unique_ptr<AWidget>> children;

    public:
        virtual ~AWidget() = default;

        void markDirty();
        bool isDirty() const;
        void clearDirty();
        bool isAnyDirty() const;

        void setPosition(glm::vec2 pos);
        void bindPosition(std::function<glm::vec2()> fn);

        void setSize(glm::vec2 s);
        [[nodiscard]] glm::vec2 getSize() const;

        void setVisible(bool v);
        [[nodiscard]] bool isVisible() const;

        AWidget* addChild(std::unique_ptr<AWidget> child);

        void build(std::vector<GuiVertex>& out, glm::vec2 parentOffset);
        virtual void buildSelf(std::vector<GuiVertex>& out, glm::vec2 absolutePos) = 0;

        virtual void tick();
};


#endif