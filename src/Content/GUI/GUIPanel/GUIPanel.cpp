#include "GUIPanel.h"
#include "CrosshairVertices.h"

GUIPanel::GUIPanel(const Font& _font, const TextureRegistry& _textureRegistry, const ItemRegistry& _itemRegistry, const Viewport& _viewport) :
    font(_font),
    textureRegistry(_textureRegistry),
    itemRegistry(_itemRegistry),
    viewport(_viewport),
    shader("/resources/shaders/UI/")
{
    const auto& vpSize = this->viewport.getSize();
    const auto vpX = static_cast<float>(vpSize.x);
    const auto vpY = static_cast<float>(vpSize.y);

    // Set shader uniforms
    this->shader.use();
    this->shader.setUniformInt("Textures", 0);

    // Build widget tree
    this->root = std::make_unique<PanelWidget>();

    // Crosshair
    this->crosshair = dynamic_cast<ShapeWidget*>(
        this->root->addChild(std::make_unique<ShapeWidget>(getCrosshairVertices(vpSize)))
    );

    // Debug panel
    {
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
            ss << std::fixed << std::setprecision(1) << this->viewport.getSettings().getCurrentFps();
            return "FPS: " + ss.str();
        }));

        this->debugPanel->addChild(makeBoundText(30.f, [this] {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
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
    }

    // Hotbar
    {
        const auto texId = this->textureRegistry.getByName("hotbar");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 2.f;
        const float h = static_cast<float>(slot.height) * 2.f;
        const float x = vpX / 2.f - w * 0.5f;
        const float y = vpY - h;
        this->hotbar = dynamic_cast<ImageWidget*>(this->root->addChild(std::make_unique<ImageWidget>(
            this->textureRegistry, "hotbar", glm::vec2{x, y}, glm::vec2{w, h}
        )));

        {
            const float invSlotW = 80.f;
            const auto texSize = glm::vec2{48.f, 48.f};

            for (int i = 0; i < 9; i++) {
                const float slotX = 1.f + (invSlotW * static_cast<float>(i) + (invSlotW * 0.5f) - (texSize.x * 0.5f));
                const float slotY = (h * 0.5f) - (texSize.y * 0.5f);

                auto icon = std::make_unique<ImageWidget>(
                    this->textureRegistry, TextureRegistry::MISSING, glm::vec2{slotX, slotY}, texSize
                );

                icon->bind([this, i]() -> TextureId {
                    const auto& stack = this->hotbarInventory.items[i];
                    const auto& itemMeta = this->itemRegistry.get(stack.itemId);
                    return itemMeta.getIcon();
                });

                icon->bindVisibility([this, i]() -> bool {
                    return this->hotbarInventory.items[i].stackSize > 0;
                });

                icon->setVisible(false);

                // Stack Size text
                {
                    auto stackSizeText = dynamic_cast<TextWidget*>(icon->addChild(
                        std::make_unique<TextWidget>(this->font, glm::vec2{texSize.x - 16.f, texSize.y - 16.f})
                    ));

                    stackSizeText->bind([this, i]() -> std::string {
                        return std::to_string(this->hotbarInventory.items[i].stackSize);
                    });

                    stackSizeText->bindVisibility([this, i]() -> bool {
                        return this->hotbarInventory.items[i].stackSize > 1;
                    });
                }

                this->hotbar->addChild(std::move(icon));
            }
        }
    }

    // Hotbar selection
    {
        const auto texId = this->textureRegistry.getByName("hotbar_selection");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 2.f;
        const float h = static_cast<float>(slot.height) * 2.f;
        this->hotbarSelection = dynamic_cast<ImageWidget*>(
            this->hotbar->addChild(std::make_unique<ImageWidget>(
                this->textureRegistry, "hotbar_selection", glm::vec2{-3.5f, -3.5f}, glm::vec2{w, h}
            ))
        );
    }

    // Track viewport size for resize handling
    this->cachedViewportSize = vpSize;

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
    const auto& vpSize = this->viewport.getSize();

    return glm::ortho(
        0.0f, static_cast<float>(vpSize.x),
        static_cast<float>(vpSize.y), 0.0f,
        -1.f, 1.0f
    );
}

void GUIPanel::onViewportResize(const glm::ivec2 newSize) const
{
    this->crosshair->setVertices(getCrosshairVertices(newSize));

    const auto vpX = static_cast<float>(newSize.x);
    const auto vpY = static_cast<float>(newSize.y);

    // Hotbar: centered horizontally, anchored to bottom
    {
        const auto texId = this->textureRegistry.getByName("hotbar");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 1.5f;
        const float h = static_cast<float>(slot.height) * 1.5f;
        this->hotbar->setPosition({vpX / 2.f - w * 0.5f, vpY - h - 20.f});
    }

    // Hotbar selection: same positioning
    {
        const auto texId = this->textureRegistry.getByName("hotbar_selection");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 1.5f;
        const float h = static_cast<float>(slot.height) * 1.5f;
        this->hotbarSelection->setPosition({vpX / 2.f - w * 0.5f, vpY - h - 20.f});
    }
}

void GUIPanel::onHotbarSlotChanged(const int slot) const
{
    // TODO: Make responsive with viewport size

    if (!this->hotbarSelection)
        return;

    const float x = 80.f * static_cast<float>(slot) - 3.5f;
    const float y = this->hotbarSelection->getPosition().y;

    this->hotbarSelection->setPosition({x, y});
}

void GUIPanel::update(const glm::vec3 &pos, const glm::vec3 &forward ,const ECS::Hotbar& hotbarInv)
{
    this->currentPos = pos;
    this->currentForward = forward;
    this->hotbarInventory = hotbarInv;
}

void GUIPanel::render()
{
    // Check for viewport resize
    const auto currentVp = this->viewport.getSize();
    if (currentVp != this->cachedViewportSize) {
        this->cachedViewportSize = currentVp;
        this->onViewportResize(currentVp);
    }

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

void GUIPanel::toggleDebugPanel() const
{
    this->debugPanel->setVisible(!this->debugPanel->isVisible());
}
