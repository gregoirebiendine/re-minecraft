#include "Settings.h"

void Settings::useVSync(const bool use)
{
    this->vsync = use;
}

bool Settings::isUsingVSync() const
{
    return this->vsync;
}

void Settings::setFpsTarget(const double target)
{
    if (target == 0)
        return;

    this->targetFrameRate = target;
    this->targetFrameTime =  1.0 / target;
}

double Settings::getFpsTarget() const
{
    return this->targetFrameRate;
}

double Settings::getFpsFrameTime() const
{
    return this->targetFrameTime;
}

void Settings::setCurrentFps(const double _fps)
{
    this->currentFps = _fps;
}

double Settings::getCurrentFps() const
{
    return this->currentFps;
}

void Settings::setViewDistance(const uint8_t distance)
{
    this->viewDistance = distance;
}

uint8_t Settings::getViewDistance() const
{
    return this->viewDistance;
}

void Settings::setFOV(const uint8_t _fov)
{
    this->fov = _fov;
}

uint8_t Settings::getFOV() const
{
    return this->fov;
}

void Settings::setFullscreen(const bool full)
{
    this->fullscreen = full;
}

bool Settings::isFullscreen() const
{
    return this->fullscreen;
}
