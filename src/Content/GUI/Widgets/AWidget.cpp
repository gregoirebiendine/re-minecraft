#include "AWidget.h"

void AWidget::markDirty()
{
    this->dirty = true;
}

bool AWidget::isDirty() const
{
    return this->dirty;
}

void AWidget::clearDirty()
{
    this->dirty = false;
    for (const auto& child : this->children)
        child->clearDirty();
}

bool AWidget::isAnyDirty() const
{
    if (!this->dirty && !this->visible)
        return false;

    if (this->dirty)
        return true;

    for (const auto& child : this->children)
        if (child->isAnyDirty())
            return true;
    return false;
}

void AWidget::setPosition(const glm::vec2 pos)
{
    this->position = pos;
    this->markDirty();
}

void AWidget::setSize(const glm::vec2 s)
{
    this->size = s;
    this->markDirty();
}

glm::vec2 AWidget::getSize() const
{
    return this->size;
}

void AWidget::setVisible(const bool v)
{
    this->visible = v;
    this->markDirty();
}

bool AWidget::isVisible() const
{
    return this->visible;
}

AWidget* AWidget::addChild(std::unique_ptr<AWidget> child)
{
    AWidget* ptr = child.get();
    this->children.push_back(std::move(child));
    return ptr;
}

void AWidget::build(std::vector<GuiVertex>& out, const glm::vec2 parentOffset)
{
    if (!visible)
        return;

    const glm::vec2 abs = parentOffset + this->position;

    buildSelf(out, abs);
    for (const auto& child : this->children)
        child->build(out, abs);
}

void AWidget::tick()
{
    for (const auto& child : this->children) {
        if (child->isVisible())
            child->tick();
    }
}
