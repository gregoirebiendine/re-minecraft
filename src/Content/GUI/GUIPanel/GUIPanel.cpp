#include "GUIPanel.h"

GUIPanel::GUIPanel(const Font& _font, const TextureRegistry& _textureRegistry, const Settings& _settings) :
    font(_font),
    textureRegistry(_textureRegistry),
    settings(_settings),
    shader("/resources/shaders/UI/")
{
    const auto& viewportSize = this->settings.getViewportSize();
    const auto vpX = static_cast<float>(viewportSize.x);
    const auto vpY = static_cast<float>(viewportSize.y);

    // Set shader uniforms
    this->shader.use();
    this->shader.setUniformInt("Textures", 0);

    // Build widget tree
    this->root = std::make_unique<PanelWidget>();

    // Crosshair
    this->root->addChild(std::make_unique<ShapeWidget>(getCrosshairVertices(viewportSize)));

    // Debug panel
    this->debugPanel = dynamic_cast<PanelWidget*>(this->root->addChild(
        std::make_unique<PanelWidget>(
            glm::vec2{0.f, 0.f},
            glm::vec2{450.f, 140.f},
            RGBA{50, 50, 50, 0.25f}
        )
    ));
    this->debugPanel->setVisible(false);

    auto makeBoundText = [&](const float y, std::function<std::string()> fn) {
        auto t = std::make_unique<TextWidget>(this->font, glm::vec2{10.f, y});
        t->bind(std::move(fn));
        return t;
    };

    this->debugPanel->addChild(makeBoundText(5.f, [this] {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << this->settings.getCurrentFps();
        return "FPS: " + ss.str();
    }));

    this->debugPanel->addChild(makeBoundText(30.f, [this] {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2)
           << this->currentPos.x << ", "
           << this->currentPos.y << ", "
           << this->currentPos.z;
        return "Pos: " + ss.str();
    }));

    this->debugPanel->addChild(makeBoundText(55.f, [this] {
        const ChunkPos cp{
            static_cast<int>(this->currentPos.x) / 16,
            static_cast<int>(this->currentPos.y) / 16,
            static_cast<int>(this->currentPos.z) / 16
        };
        return "Chunk: " + cp;
    }));

    this->debugPanel->addChild(makeBoundText(80.f, [this] {
        return "Facing: " + DirectionUtils::forwardVectorToCardinal(this->currentForward);
    }));

    this->debugPanel->addChild(makeBoundText(105.f, [this] {
        return "Block: " + this->currentSelectedBlock;
    }));

    // Hotbar image
    {
        const auto texId = this->textureRegistry.getByName("hotbar");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 1.5f;
        const float h = static_cast<float>(slot.height) * 1.5f;
        const float x = vpX / 2.f - w * 0.5f;
        const float y = vpY - h - 20.f;
        this->root->addChild(std::make_unique<ImageWidget>(
            this->textureRegistry, "hotbar", glm::vec2{x, y}, glm::vec2{w, h}
        ));
    }

    // Hotbar selection
    {
        const auto texId = this->textureRegistry.getByName("hotbar_selection");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 1.5f;
        const float h = static_cast<float>(slot.height) * 1.5f;
        const float x = vpX / 2.f - w * 0.5f;
        const float y = vpY - h - 20.f;
        this->hotbarSelection = dynamic_cast<ImageWidget*>(
            this->root->addChild(std::make_unique<ImageWidget>(
                this->textureRegistry, "hotbar_selection", glm::vec2{x, y}, glm::vec2{w, h}
            ))
        );
    }

    // Initial build
    this->rebuildVertexBuffer();
}

void GUIPanel::rebuildVertexBuffer()
{
    this->vertexBuffer.clear();
    this->root->build(this->vertexBuffer, {0.f, 0.f});
    this->root->clearDirty();

    this->vao.bind();
    this->vao.storeGuiData(this->vertexBuffer);
    this->vao.unbind();
}

glm::mat4 GUIPanel::getGUIProjectionMatrix() const
{
    const auto& viewportSize = this->settings.getViewportSize();

    return glm::ortho(
        0.0f, static_cast<float>(viewportSize.x),
        static_cast<float>(viewportSize.y), 0.0f,
        -1.f, 1.0f
    );
}

void GUIPanel::render()
{
    // Evaluates bindings
    this->root->tick();

    // Rebuild VBO only if something changed
    if (this->root->isAnyDirty())
        this->rebuildVertexBuffer();

    // Draw
    this->shader.use();
    this->shader.setProjectionMatrix(this->getGUIProjectionMatrix());

    this->vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->vertexBuffer.size()));
    this->vao.unbind();
}

void GUIPanel::update(const glm::vec3 &pos, const glm::vec3 &forward, const std::string &selectedBlockName)
{
    this->currentPos = pos;
    this->currentForward = forward;
    this->currentSelectedBlock = selectedBlockName;
}

void GUIPanel::toggleDebugPanel() const
{
    this->debugPanel->setVisible(!this->debugPanel->isVisible());
}
