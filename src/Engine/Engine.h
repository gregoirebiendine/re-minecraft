#ifndef ENGINE_H
#define ENGINE_H

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
#include "TextureRegistry.h"
#include "PrefabRegistry.h"
#include "World.h"
#include "Player.h"

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double>;

class Engine {
    #ifdef _WIN32
        HANDLE frameTimer = nullptr;
    #endif

    mutable Viewport viewport;
    InputState inputs;
    
    BlockRegistry blockRegistry;
    TextureRegistry textureRegistry;
    PrefabRegistry prefabRegistry;
    std::unique_ptr<MeshRegistry> meshRegistry;

    std::unique_ptr<Player> player;
    std::unique_ptr<World> world;

    void update() const;
    void render() const;
    void preciseWait(double seconds) const;

    public:
        Engine();
        ~Engine();

        void loop();
};

#endif //ENGINE_H
