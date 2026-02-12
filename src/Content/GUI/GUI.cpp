#include "GUI.h"

GUI::GUI(const Font& _font) :
    font(_font),
    guiShader("/resources/shaders/UI/"),
    outlineShader("/resources/shaders/Outline/")
{
    this->guiShader.use();
    this->guiShader.setUniformInt("Textures", 0);
    this->guiShader.setUniformUInt("LayerId", this->font.getTextureID());
}

void GUI::init(const glm::ivec2 viewportSize)
{
    this->changeViewportSize(viewportSize);

    // General GUI
    this->createCrosshair(viewportSize);
    this->createRectangle(0, static_cast<float>(viewportSize.y) - 80, 400, 80, {20,20,20,0.6f});
    this->createText(20, static_cast<float>(viewportSize.y) - 50, "Hello, World!");

    // Upload GUI data
    this->guiVao.bind();
    this->guiVao.storeGuiData(this->data);
    this->guiVao.unbind();

    // Upload Block Outline data
    this->outlineVao.bind();
    this->outlineVao.storeOutlineData(OUTLINE_VERTICES);
    this->outlineVao.unbind();
}

void GUI::changeViewportSize(const glm::ivec2 size)
{
    this->projectionMatrix = glm::ortho(
        0.0f, static_cast<float>(size.x),
        static_cast<float>(size.y), 0.0f,
        -1.f, 1.0f
    );
}

void GUI::createCrosshair(const glm::ivec2 viewportSize) {
    // TODO: Update viewportSize when resizing
    const glm::vec2 mid = {viewportSize.x / 2 , viewportSize.y / 2};
    const DigitalColor color{200, 200, 200, 0.8f};

    // Hor 1
    this->createRectangle(
        mid.x - (CH_SIZE / 2) - CH_OFFSET,
        mid.y - (CH_THICKNESS / 2),
        (CH_SIZE / 2),
        CH_THICKNESS,
        color
    );
    // Hor 2
    this->createRectangle(
        mid.x + CH_OFFSET,
        mid.y - (CH_THICKNESS / 2),
        (CH_SIZE / 2),
        CH_THICKNESS,
        color
    );

    // Vert 1
    this->createRectangle(
        mid.x - (CH_THICKNESS / 2),
        mid.y - (CH_SIZE / 2) - CH_OFFSET,
        CH_THICKNESS,
        (CH_SIZE / 2),
        color
    );
    // Vert 2
    this->createRectangle(
        mid.x - (CH_THICKNESS / 2),
        mid.y + CH_OFFSET,
        CH_THICKNESS,
        (CH_SIZE / 2),
        color
    );
}

void GUI::createRectangle(const float x, const float y, const float width, const float height, const DigitalColor color)
{
    const float x1 = x + width;
    const float y1 = y + height;

    this->data.insert(this->data.end(), {
        // First triangle
        {{x, y}, {-1.f, -1.f}, color},
        {{x, y1}, {-1.f, -1.f}, color},
        {{x1, y1}, {-1.f, -1.f}, color},

        // Second triangle
        {{x, y}, {-1.f, -1.f}, color},
        {{x1, y1}, {-1.f, -1.f}, color},
        {{x1, y}, {-1.f, -1.f}, color},
    });
}

void GUI::createText(float x, float y, const std::string &text)
{
    const auto& uvs = this->font.getUVFromString(text);
    const DigitalColor color{255, 255, 255, 1.f};
    const float y1 = y + Font::CHAR_SIZE;

    float curX = x;
    for (const auto& uv : uvs) {
        float charX1 = curX + Font::CHAR_SIZE;
        this->data.insert(this->data.end(), {
            {{curX, y},    uv[0], color},
            {{curX, y1},   uv[1], color},
            {{charX1, y1}, uv[2], color},

            {{curX, y},    uv[3], color},
            {{charX1, y1}, uv[4], color},
            {{charX1, y},  uv[5], color},
        });
        curX += Font::CHAR_SIZE;
    }
}

void GUI::render(const glm::vec3& pos, const glm::vec3& forward, const std::string& selectedBlockName)
{
    glDisable(GL_DEPTH_TEST);
    glEnable( GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    createImGuiFrame();
    renderImGuiFrame(pos, forward, selectedBlockName);

    this->guiShader.use();
    this->guiShader.setProjectionMatrix(this->projectionMatrix);

    this->guiVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->data.size()));
    this->guiVao.unbind();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void GUI::renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos)
{
    glDisable(GL_CULL_FACE);
    glPolygonOffset(-1, -1);

    this->outlineShader.use();
    this->outlineShader.setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(pos)));
    this->outlineShader.setViewMatrix(v);
    this->outlineShader.setProjectionMatrix(p);

    this->outlineVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 288);
    this->outlineVao.unbind();

    glPolygonOffset(0, 0);
    glEnable(GL_CULL_FACE);
}

void GUI::createImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::renderImGuiFrame(const glm::vec3 pos, const glm::vec3 forward, const std::string& selectedBlockName)
{
    const auto facing = DirectionUtils::forwardVectorToCardinal(forward);
    const ChunkPos cp{
        static_cast<int>(pos.x) / 16,
        static_cast<int>(pos.y) / 16,
        static_cast<int>(pos.z) / 16
    };

    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("Position : %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("Chunk : %d, %d, %d", cp.x, cp.y, cp.z);
    ImGui::Text("Facing : %s", facing.c_str());
    ImGui::Text("Selected block : %s", selectedBlockName.c_str());
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Statics
float GUI::toScreenSpace(const float v, const float minIn, const float maxIn)
{
    const float res = -1 + (1 - (-1)) * ((v - minIn) / (maxIn - minIn));
    return std::clamp(res, -1.0f, 1.0f);
}

float GUI::percent(const float baseValue, const float percentage)
{
    return baseValue * (percentage/100.0f);
}