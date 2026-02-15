#include "Settings.h"

void Settings::setVSync(const bool useVSync)
{
    this->vsync = useVSync;
}

bool Settings::isVSync() const
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

void Settings::setViewportSize(const glm::ivec2 size)
{
    this->viewportSize = size;
}

glm::ivec2 Settings::getViewportSize() const
{
    return this->viewportSize;
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
