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

    // Init GLFW window and viewport
    this->viewport.initWindow(&this->inputs);
    this->viewport.initViewport();

    // Instantiate members
    this->textureRegistry.createTextures();
    this->player = std::make_unique<Player>(this->blockRegistry);
    this->world = std::make_unique<World>(this->blockRegistry, this->textureRegistry);

    // Init GUI
    this->player->getGUI().init(this->viewport.getSettings().getViewportSize());
}

Engine::~Engine()
{
    #ifdef _WIN32
    if (this->frameTimer)
        CloseHandle(this->frameTimer);
    #endif
}

void Engine::loop()
{
    const auto targetFrameTime = this->viewport.getSettings().getFpsFrameTime();
    auto previousTime = Clock::now();
    double accumulator = 0.0;

    while (!this->viewport.shouldClose())
    {
        auto frameStart = Clock::now();
        double frameTime = std::chrono::duration_cast<Duration>(frameStart - previousTime).count();
        if (frameTime > 0.25)
            frameTime = 0.25;

        previousTime = frameStart;
        accumulator += frameTime;

        // INPUTS
        Viewport::pollEvents();
        this->handleInputs(frameTime);
        this->inputs.clear();

        // UPDATES
        while (accumulator >= Viewport::dt) {
            this->update();
            accumulator -= Viewport::dt;
        }

        // RENDERING
        // const double alpha = accumulator / dt;
        this->render();

        // FPS CAP
        auto frameEnd = Clock::now();
        double elapsed = std::chrono::duration_cast<Duration>(frameEnd - frameStart).count();

        if (!this->viewport.useVSync() && elapsed < targetFrameTime) {
            this->preciseWait(targetFrameTime - elapsed);
        }
    }

    this->viewport.closeWindow();
}

void Engine::preciseWait(const double seconds) const
{
    if (seconds <= 0.0) return;

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

void Engine::handleInputs(const double deltaTime)
{
    this->player->getCamera().moveCamera(this->inputs.mouseX, this->inputs.mouseY, deltaTime);
    this->lastRaycastHit = this->player->getCamera().raycast(*this->world);

    if (this->inputs.isMouseButtonPressed(Inputs::Mouse::LEFT) && this->lastRaycastHit.hit)
        this->world->setBlock(this->lastRaycastHit.pos.x, this->lastRaycastHit.pos.y, this->lastRaycastHit.pos.z, this->blockRegistry.getByName("core:air"));

    if (this->inputs.isMouseButtonPressed(Inputs::Mouse::RIGHT) && this->lastRaycastHit.hit)
        this->world->setBlock(this->lastRaycastHit.previousPos.x, this->lastRaycastHit.previousPos.y, this->lastRaycastHit.previousPos.z, this->player->getSelectedMaterial());

    if (this->inputs.isMouseButtonPressed(Inputs::Mouse::MIDDLE) && this->lastRaycastHit.hit) {
        const Material block = this->world->getBlock(this->lastRaycastHit.pos.x, this->lastRaycastHit.pos.y, this->lastRaycastHit.pos.z);

        if (!this->blockRegistry.isEqual(block, "core:air"))
            this->player->setSelectedMaterial(block);
    }

    if (this->inputs.isKeyPressed(Inputs::Keys::SPACE)) {
        glfwSetInputMode(this->viewport.getWindow(), GLFW_CURSOR, !this->player->getCamera().getMouseCapture() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        this->player->getCamera().toggleMouseCapture();
    }

    if (this->inputs.isKeyDown(Inputs::Keys::W))
        this->player->getCamera().move({0,0,1}, static_cast<float>(deltaTime));
    if (this->inputs.isKeyDown(Inputs::Keys::S))
        this->player->getCamera().move({0,0,-1}, static_cast<float>(deltaTime));
    if (this->inputs.isKeyDown(Inputs::Keys::A))
        this->player->getCamera().move({-1,0,0}, static_cast<float>(deltaTime));
    if (this->inputs.isKeyDown(Inputs::Keys::D))
        this->player->getCamera().move({1,0,0}, static_cast<float>(deltaTime));
    if (this->inputs.isKeyDown(Inputs::Keys::Q))
        this->player->getCamera().move({0,1,0}, static_cast<float>(deltaTime));
    if (this->inputs.isKeyDown(Inputs::Keys::E))
        this->player->getCamera().move({0,-1,0}, static_cast<float>(deltaTime));
}

void Engine::update() const
{
    this->player->getCamera().setViewMatrix(this->world->getShader(), this->viewport.getAspectRatio());
    this->world->update(this->player->getCamera().getPosition());
}

void Engine::render() const
{
    // Clear window and buffer
    glClearColor(0.509f, 0.784f, 0.898f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render World
    this->textureRegistry.bind();
    this->world->render();

    // Render faced block outline
    if (this->lastRaycastHit.hit)
        this->player->renderBlockOutline(this->viewport.getAspectRatio(), this->lastRaycastHit.pos);

    // Render Player
    this->player->render();

    // Update buffer
    this->viewport.swapBuffers();
}