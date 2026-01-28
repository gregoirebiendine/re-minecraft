#include "Engine.h"

Engine::Engine() :
    prefabRegistry(blockRegistry)
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
    this->world = std::make_unique<World>(this->blockRegistry, this->textureRegistry, this->prefabRegistry);

    // Apply settings to classes
    const auto settings = this->viewport.getSettings();

    this->player->getGUI().init(settings.getViewportSize());
    this->player->getCamera().setFOV(settings.getFOV());
    this->world->getChunkManager().setViewDistance(settings.getViewDistance());
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
        this->player->handleInputs(this->inputs, this->viewport, *this->world, frameTime);
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

        if (!this->viewport.useVSync() && elapsed < targetFrameTime)
            this->preciseWait(targetFrameTime - elapsed);
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

void Engine::update() const
{
    const auto vpMatrix = this->player->getCamera().setViewMatrix(this->world->getShader(), this->viewport.getAspectRatio());

    this->world->update(this->player->getCamera().getPosition(), vpMatrix);
}

void Engine::render() const
{
    // Clear window and buffer
    glClearColor(0.509f, 0.784f, 0.898f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render World
    this->textureRegistry.bind();
    this->world->render();

    // Render Player
    this->player->renderBlockOutline(this->viewport.getAspectRatio());
    this->player->render();

    // Update buffer
    this->viewport.swapBuffers();
}