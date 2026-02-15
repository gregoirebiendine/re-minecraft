#include "Engine.h"

Engine::Engine() :
    viewport(settings),
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

    // Create textures
    this->textureRegistry.createTextures();

    // Instantiate members
    this->meshRegistry = std::make_unique<MeshRegistry>();
    this->font = std::make_unique<Font>(this->textureRegistry.getByName("font"));
    this->world = std::make_unique<World>(this->blockRegistry, this->textureRegistry, this->prefabRegistry, *this->meshRegistry, this->inputs);
    this->playerController = std::make_unique<PlayerController>(*this->world, *this->font, this->settings);

    // Update view distance from settings
    this->world->getChunkManager().setViewDistance(this->settings.getViewDistance());
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

Engine::~Engine()
{
    #ifdef _WIN32
    if (this->frameTimer)
        CloseHandle(this->frameTimer);
    #endif
}

void Engine::loop()
{
    const auto targetFrameTime = this->settings.getFpsFrameTime();
    auto previousTime = Clock::now();
    double accumulator = 0.0;
    double smoothFps = 0.0;

    while (!this->viewport.shouldClose())
    {
        auto frameStart = Clock::now();
        double frameTime = std::chrono::duration_cast<Duration>(frameStart - previousTime).count();

        if (frameTime > 0.0) {
            const double instantFps = 1.0 / frameTime;
            smoothFps = smoothFps * 0.95 + instantFps * 0.05;
            this->settings.setCurrentFps(smoothFps);
        }

        if (frameTime > 0.25)
            frameTime = 0.25;

        previousTime = frameStart;
        accumulator += frameTime;

        // INPUTS
        Viewport::pollEvents();
        this->playerController->handleInputs(this->inputs, this->viewport);

        // UPDATES
        while (accumulator >= Viewport::dt) {
            this->update();
            accumulator -= Viewport::dt;
        }

        // CLEAR INPUTS
        this->inputs.clear();

        // RENDERING
        this->render();

        // FPS CAP
        auto frameEnd = Clock::now();
        const double elapsed = std::chrono::duration_cast<Duration>(frameEnd - frameStart).count();

        if (!this->viewport.useVSync() && elapsed < targetFrameTime)
            this->preciseWait(targetFrameTime - elapsed);
    }

    this->viewport.closeWindow();
}

void Engine::update() const
{
    this->world->update(this->viewport.getAspectRatio());
    this->playerController->update();
}

void Engine::render() const
{
    // Begin MSAA rendering
    // this->viewport.beginFrame();

    // Clear window and buffer
    glClearColor(0.509f, 0.784f, 0.898f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render World
    this->textureRegistry.bind();
    this->textureRegistry.bindSlots();
    this->world->render();

    // Render Player
    this->playerController->render();

    // Resolve MSAA and swap buffers
    // this->viewport.endFrame();
    this->viewport.swapBuffers();
}