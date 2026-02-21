#include "GUIController.h"
#include "OutlineVertices.h"

GUIController::GUIController(const MsdfFont& _font, const Viewport& _viewport, const TextureRegistry& _textureRegistry, const ItemRegistry& _itemRegistry) :
    panel(_font, _viewport, _textureRegistry, _itemRegistry),
    shader("/resources/shaders/Outline/")
{
    this->vao.bind();
    this->vao.storeOutlineData(OUTLINE_VERTICES);
    this->vao.unbind();
}

void GUIController::onHotbarSlotChanged(const int slot) const
{
    this->panel.onHotbarSlotChanged(slot);
}

void GUIController::toggleDebugPanel() const
{
    this->panel.toggleDebugPanel();
}

void GUIController::update(const glm::vec3 &pos, const glm::vec3 &forward, const ECS::Hotbar& hotbarInv)
{
    this->panel.update(pos, forward, hotbarInv);
}

void GUIController::render()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->panel.render();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void GUIController::renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos)
{
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_CULL_FACE);
    glPolygonOffset(-1, -1);

    this->shader.use();
    this->shader.setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(pos)));
    this->shader.setViewMatrix(v);
    this->shader.setProjectionMatrix(p);

    this->vao.draw();

    glPolygonOffset(0, 0);
    glEnable(GL_CULL_FACE);
    glDisable(GL_MULTISAMPLE);
}