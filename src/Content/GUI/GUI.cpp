#include "GUI.h"

GUI::GUI() :
    guiShader("/resources/shaders/UI/"),
    outlineShader("/resources/shaders/Outline/")
{
    // Empty
}

void GUI::init(const glm::ivec2 viewportSize)
{
    this->changeViewportSize(viewportSize);

    // General GUI
    this->createCrosshair(viewportSize);
    this->guiVao.bind();
    this->guiVao.storeGuiData(this->data);
    this->guiVao.unbind();

    // Block Outline GUI
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
        mid.x - (SIZE / 2) - OFFSET,
        mid.y - (THICKNESS / 2),
        (SIZE / 2),
        THICKNESS,
        color
    );
    // Hor 2
    this->createRectangle(
        mid.x + OFFSET,
        mid.y - (THICKNESS / 2),
        (SIZE / 2),
        THICKNESS,
        color
    );

    // Vert 1
    this->createRectangle(
        mid.x - (THICKNESS / 2),
        mid.y - (SIZE / 2) - OFFSET,
        THICKNESS,
        (SIZE / 2),
        color
    );
    // Vert 2
    this->createRectangle(
        mid.x - (THICKNESS / 2),
        mid.y + OFFSET,
        THICKNESS,
        (SIZE / 2),
        color
    );
}

void GUI::createRectangle(const float x, const float y, const float width, const float height, const DigitalColor color)
{
    const float x1 = x + width;
    const float y1 = y + height;

    this->data.insert(this->data.end(), {
        // First triangle
        {{x, y}, {color.r, color.g, color.b, color.a}},
        {{x, y1}, {color.r, color.g, color.b, color.a}},
        {{x1, y1}, {color.r, color.g, color.b, color.a}},

        // Second triangle
        {{x, y}, {color.r, color.g, color.b, color.a}},
        {{x1, y1}, {color.r, color.g, color.b, color.a}},
        {{x1, y}, {color.r, color.g, color.b, color.a}},
    });
}

void GUI::render()
{
    glDisable(GL_DEPTH_TEST);
    glEnable( GL_BLEND);

    this->guiShader.use();
    this->guiShader.setProjectionMatrix(this->projectionMatrix);

    this->guiVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->data.size()));
    this->guiVao.unbind();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void GUI::renderBlockOutline(const Camera& camera, const float& aspect, const glm::vec3& cubePos)
{
    glDisable(GL_CULL_FACE);
    glPolygonOffset(-1, -1);

    this->outlineShader.use();
    this->outlineShader.setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(cubePos)));
    this->outlineShader.setViewMatrix(camera.getViewMatrix());
    this->outlineShader.setProjectionMatrix(camera.getProjectionMatrix(aspect));

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