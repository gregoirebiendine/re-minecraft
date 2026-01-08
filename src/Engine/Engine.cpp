#include "Engine.h"

Engine::Engine()
{
    if (!glfwInit())
        throw std::runtime_error("Cannot initialize GLFW3");

    // Pass GLFW version to the lib
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create the window
    this->window = glfwCreateWindow(WindowSize.x, WindowSize.y, "Re Minecraft", nullptr, nullptr);
    if (!this->window) {
        glfwTerminate();
        throw std::runtime_error("Failed to open window");
    }

    // Get main monitor and get monitor screen size
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to get main monitor");
    }

    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    if (!videoMode)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to get video mode");
    }

    this->ScreenSize = glm::ivec2(videoMode->width, videoMode->height);
    this->aspectRatio = static_cast<float>(WindowSize.x) / static_cast<float>(WindowSize.y);

    // Center window
    glfwSetWindowPos(window, (videoMode->width / 2) - (WindowSize.x / 2),  (videoMode->height / 2) - (WindowSize.y / 2));

    // Make window current context for GLFW
    glfwMakeContextCurrent(this->window);
    glfwSwapInterval(1);

    // Initialize GLAD Manager
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize GLAD");

    // Create viewport
    glViewport(0, 0, WindowSize.x, WindowSize.y);

    // Enable 3D depth
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Setup STBI image load
    stbi_set_flip_vertically_on_load(true);

    // Setup ImGui implementation
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Enable RAW mouse input
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // Forward input state array to GLFW
    glfwSetWindowUserPointer(this->window, &this->inputs);

    // Register inputs callback functions to GLFW
    glfwSetKeyCallback(window, keyInputCallback);
    glfwSetCursorPosCallback(window, mouseInputCallback);
    glfwSetMouseButtonCallback(window, mouseButtonInputCallback);

    // Create all members
    this->worldShader = std::make_unique<Shader>(
        "../resources/shaders/WorldShader/WorldVertexShader.vert",
        "../resources/shaders/WorldShader/WorldFragShader.frag"
    );

    this->atlas = std::make_unique<Atlas>();
    this->camera = std::make_unique<Camera>(glm::vec3{16.0f, 26.0f, 35.0f}, this->blockRegistry);
    this->world = std::make_unique<World>(this->blockRegistry);
    this->playerGUI = std::make_unique<GUI>(this->aspectRatio);

    if (!this->worldShader || !this->world || !this->camera || !this->atlas || !this->playerGUI)
        throw std::runtime_error("Failed to initialize pointers");
}

Engine::~Engine() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->window);
    glfwTerminate();
}

void Engine::loop()
{
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(this->window)) {
        // Compute deltaTime
        const double now = glfwGetTime();
        const double deltaTime = now - lastTime;
        lastTime = now;

        // Update and render
        glfwPollEvents();
        this->handleInputs(deltaTime);
        this->update();
        this->render();
        this->clearInputs();
    }
}

void Engine::handleInputs(const double deltaTime) const
{
    this->camera->moveCamera(this->inputs.mouseX, this->inputs.mouseY, deltaTime);

    if (this->inputs.mousePressed[GLFW_MOUSE_BUTTON_LEFT])
    {
        if (const Raycast::Hit raycast = this->camera->raycast(*this->world); raycast.hit)
            this->world->setBlock(raycast.pos.x, raycast.pos.y, raycast.pos.z, this->blockRegistry.getByName("core:air"));
    }

    if (this->inputs.mousePressed[GLFW_MOUSE_BUTTON_RIGHT])
    {
        if (const Raycast::Hit raycast = this->camera->raycast(*this->world); raycast.hit)
            this->world->setBlock(raycast.previousPos.x, raycast.previousPos.y, raycast.previousPos.z, this->camera->getSelectedMaterial());
    }

    if (this->inputs.mousePressed[GLFW_MOUSE_BUTTON_MIDDLE])
    {
        if (const Raycast::Hit raycast = this->camera->raycast(*this->world); raycast.hit)
        {
            const Material block = this->world->getBlock(raycast.pos.x, raycast.pos.y, raycast.pos.z);

            if (!this->blockRegistry.isEqual(block, "core:air"))
                this->camera->setSelectedMaterial(block);
        }
    }

    if (this->inputs.keyPressed[GLFW_KEY_SPACE])
    {
        glfwSetInputMode(window, GLFW_CURSOR, !this->camera->getMouseCapture() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        this->camera->toggleMouseCapture();
    }

    if (this->inputs.keyDown[GLFW_KEY_W])
        this->camera->move({0,0,1}, static_cast<float>(deltaTime));
    if (this->inputs.keyDown[GLFW_KEY_S])
        this->camera->move({0,0,-1}, static_cast<float>(deltaTime));
    if (this->inputs.keyDown[GLFW_KEY_A])
        this->camera->move({-1,0,0}, static_cast<float>(deltaTime));
    if (this->inputs.keyDown[GLFW_KEY_D])
        this->camera->move({1,0,0}, static_cast<float>(deltaTime));
    if (this->inputs.keyDown[GLFW_KEY_Q])
        this->camera->move({0,1,0}, static_cast<float>(deltaTime));
    if (this->inputs.keyDown[GLFW_KEY_E])
        this->camera->move({0,-1,0}, static_cast<float>(deltaTime));
}

void Engine::clearInputs()
{
    std::fill_n(this->inputs.keyPressed, sizeof(this->inputs.keyPressed), false);
    std::fill_n(this->inputs.keyReleased, sizeof(this->inputs.keyReleased), false);
    std::fill_n(this->inputs.mousePressed, sizeof(this->inputs.mousePressed), false);
    std::fill_n(this->inputs.mouseReleased, sizeof(this->inputs.mouseReleased), false);
}

void Engine::update() const
{
    this->worldShader->use();

    // Apply camera position and rotation
    this->setViewMatrix();

    // Update world
    this->world->update();
}

void Engine::render() const
{
    this->worldShader->use();

    // Clear window and buffer (sky : 130,200,229)
    glClearColor(0.509f, 0.784f, 0.898f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render World (chunks)
    this->atlas->bind();
    this->world->render(*this->worldShader);

    // Render ImGui Frame
    GUI::createImGuiFrame();
    GUI::renderImGuiFrame(*this->camera, this->blockRegistry);

    // Render Crosshair
    this->playerGUI->renderCrosshair();

    // Update buffer
    glfwSwapBuffers(this->window);
}

void Engine::setViewMatrix() const
{
    const auto forward = this->camera->getForwardVector();
    const auto cameraPos = this->camera->getPosition();
    const glm::mat4 view = glm::lookAt(cameraPos, cameraPos + forward, {0,1,0});
    const glm::mat4 projection = glm::perspective(Camera::FOV, this->aspectRatio, 0.1f, 100.f);

    this->worldShader->setUniformMat4("ViewMatrix", projection * view);
}

// Statics callback
void keyInputCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    const auto glfwPointer = glfwGetWindowUserPointer(window);

    if (glfwPointer == nullptr || key < 0 || key > GLFW_KEY_LAST)
        return;

    auto* input = static_cast<InputState*>(glfwPointer);

    if (action == GLFW_PRESS)
    {
        input->keyDown[key] = true;
        input->keyPressed[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        input->keyDown[key] = false;
        input->keyReleased[key] = true;
    }
}

void mouseButtonInputCallback(GLFWwindow* window, const int button, const int action, const int mods)
{
    UNUSED(mods);

    const auto glfwPointer = glfwGetWindowUserPointer(window);

    if (glfwPointer == nullptr || button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
        return;

    auto* input = static_cast<InputState*>(glfwPointer);

    if (action == GLFW_PRESS)
    {
        input->mouseDown[button] = true;
        input->mousePressed[button] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        input->mouseDown[button] = false;
        input->mouseReleased[button] = true;
    }
}

void mouseInputCallback(GLFWwindow* window, const double x, const double y)
{
    const auto glfwPointer = glfwGetWindowUserPointer(window);

    if (glfwPointer == nullptr)
        return;

    auto* input = static_cast<InputState*>(glfwPointer);

    input->mouseX = x;
    input->mouseY = y;
}
