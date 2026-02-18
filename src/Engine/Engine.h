#ifndef FARFIELD_ENGINE_H
#define FARFIELD_ENGINE_H

#define GLM_ENABLE_EXPERIMENTAL

#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
    #include <time.h>
    #include <cerrno>
#endif

#include <thread>
#include <memory>
#include <chrono>

#include "Viewport.h"
#include "InputState.h"
#include "BlockRegistry.h"
#include "ItemRegistry.h"
#include "TextureRegistry.h"
#include "PrefabRegistry.h"
#include "World.h"
#include "PlayerController.h"
#include "Font.h"

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double>;

class Engine {
    #ifdef _WIN32
        HANDLE frameTimer = nullptr;
    #endif

    Settings settings;
    mutable Viewport viewport;
    InputState inputs;
    
    TextureRegistry textureRegistry;
    BlockRegistry blockRegistry;
    PrefabRegistry prefabRegistry;
    std::unique_ptr<ItemRegistry> itemRegistry;
    std::unique_ptr<MeshRegistry> meshRegistry;

    std::unique_ptr<Font> font;
    std::unique_ptr<World> world;
    std::unique_ptr<PlayerController> playerController;

    void update() const;
    void render() const;
    void preciseWait(double seconds) const;

    public:
        Engine();
        ~Engine();

        void loop();
};

#endif
