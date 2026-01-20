#include "Engine.h"

Engine::Engine()
{
    #ifdef _WIN32
        this->frameTimer = CreateWaitableTimerExW(
            nullptr, nullptr,
            CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
            TIMER_ALL_ACCESS
        );
        if (!this->frameTimer)
            this->frameTimer = CreateWaitableTimerW(nullptr, TRUE, nullptr);
    #endif

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

    // Initialize GLAD Manager
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize GLAD");

    // Create viewport
    glViewport(0, 0, WindowSize.x, WindowSize.y);

    // Change transparency function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable 3D depth
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Enable culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // VSYNC
    glfwSwapInterval(this->useVsync);

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

    this->textureRegistry.createTextures();
    this->camera = std::make_unique<Camera>(glm::vec3{8.5f, 17.5f, 8.5f}, this->blockRegistry);
    this->world = std::make_unique<World>(this->blockRegistry, this->textureRegistry);
    this->playerGUI = std::make_unique<GUI>();
}

Engine::~Engine() {
    #ifdef _WIN32
        if (this->frameTimer)
            CloseHandle(this->frameTimer);
    #endif

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->window);
    glfwTerminate();
}

void Engine::loop() {
    auto previousTime = Clock::now();
    double accumulator = 0.0;

    while (!glfwWindowShouldClose(this->window)) {
        auto frameStart = Clock::now();
        double frameTime = std::chrono::duration_cast<Duration>(frameStart - previousTime).count();
        if (frameTime > 0.25)
            frameTime = 0.25;

        previousTime = frameStart;
        accumulator += frameTime;

        // INPUTS
        glfwPollEvents();
        this->handleInputs(frameTime);
        this->clearInputs();

        // UPDATES
        while (accumulator >= dt) {
            this->update();
            accumulator -= dt;
        }

        // RENDERING
        // const double alpha = accumulator / dt;
        this->render();

        // FPS CAP
        auto frameEnd = Clock::now();
        double elapsed = std::chrono::duration_cast<Duration>(frameEnd - frameStart).count();

        if (!this->useVsync && elapsed < targetFrameTime) {
            this->preciseWait(targetFrameTime - elapsed);
        }
    }
}

void Engine::preciseWait(const double seconds) const
{
    if (seconds <= 0.0)
        return;

    #ifdef _WIN32
        if (this->frameTimer) {
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = -static_cast<LONGLONG>(seconds * 10'000'000.0);

            if (SetWaitableTimerEx(this->frameTimer, &dueTime, 0, nullptr, nullptr, nullptr, 0)) {
                WaitForSingleObject(this->frameTimer, INFINITE);
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::duration_cast<Clock::duration>(Duration(seconds)));

    #elif defined(__linux__)
        struct timespec req;
        req.tv_sec = static_cast<time_t>(seconds);
        req.tv_nsec = static_cast<long>((seconds - req.tv_sec) * 1'000'000'000.0);

        while (clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &req) == EINTR) {}

    #else
        std::this_thread::sleep_for(std::chrono::duration_cast<Clock::duration>(Duration(seconds)));
    #endif
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
    // Apply camera position and rotation
    this->camera->setViewMatrix(this->world->getShader(), this->aspectRatio);

    // Update world
    this->world->update(this->camera->getPosition());
}

void Engine::render() const
{
    // Clear window and buffer
    glClearColor(0.509f, 0.784f, 0.898f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render World
    this->textureRegistry.bind();
    this->world->render();

    // Render ImGui Frame
    GUI::createImGuiFrame();
    GUI::renderImGuiFrame(*this->camera, this->blockRegistry);

    // Render GUI
    this->playerGUI->render();

    // Update buffer
    glfwSwapBuffers(this->window);
}

// Statics callback
void keyInputCallback(GLFWwindow* window, const int key, [[maybe_unused]] const int scancode, const int action, [[maybe_unused]] const int mods)
{
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

void mouseButtonInputCallback(GLFWwindow* window, const int button, const int action, [[maybe_unused]] const int mods)
{
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
