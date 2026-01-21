#include "GUI.h"
#include "Engine.h"

GUI::GUI() :
    shader(
    "../resources/shaders/UIShader/ui.vert",
    "../resources/shaders/UIShader/ui.frag"
    ),
    projectionMatrix(
        glm::ortho(
            0.0f, static_cast<float>(Engine::WindowSize.x),
            static_cast<float>(Engine::WindowSize.y), 0.0f,
            -1.f, 1.0f
        )
    )
{
    this->createCrosshair();

    this->vao.bind();
    this->vao.storeGuiData(this->data);
    this->vao.unbind();
}

void GUI::createCrosshair() {
    constexpr glm::vec2 mid = {Engine::WindowSize.x / 2 , Engine::WindowSize.y / 2};
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

void GUI::render() const
{
    glDisable(GL_DEPTH_TEST);
    glEnable( GL_BLEND);

    this->shader.use();
    this->shader.setUniformMat4("ProjectionMatrix", this->projectionMatrix);

    this->vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->data.size()));
    this->vao.unbind();

    glEnable(GL_DEPTH_TEST);
    glDisable( GL_BLEND);
}

void GUI::createImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::renderImGuiFrame(const Camera& camera)
{
    const auto cameraPos = camera.getPosition();
    const auto facing = forwardToCardinal(camera.getForwardVector());
    const ChunkPos cp{
        static_cast<int>(cameraPos.x) / 16,
        static_cast<int>(cameraPos.y) / 16,
        static_cast<int>(cameraPos.z) / 16
    };

    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("Position : %.2f, %.2f, %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::Text("Chunk : %d, %d, %d", cp.x, cp.y, cp.z);
    ImGui::Text("Facing : %s", facing.c_str());
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

float GUI::percent(const float baseValue, float percentage)
{
    return baseValue * (percentage/100.0f);
}

std::string GUI::forwardToCardinal(const glm::vec3& forwardVector)
{
    const glm::vec3 f = glm::normalize(forwardVector);
    const float ax = abs(f.x);
    const float ay = abs(f.y);
    const float az = abs(f.z);

    if (ax > ay && ax > az)
        return (f.x > 0) ? "EAST" : "WEST";
    if (ay > az)
        return (f.y > 0) ? "UP" : "DOWN";
    return (f.z > 0) ? "SOUTH" : "NORTH";
}
