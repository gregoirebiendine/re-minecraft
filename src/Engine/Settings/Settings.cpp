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

    this->fps = target;
    this->targetFrameTime =  1.0 / target;
}

double Settings::getFpsTarget() const
{
    return this->fps;
}

double Settings::getFpsFrameTime() const
{
    return this->targetFrameTime;
}

void Settings::setViewportSize(const glm::ivec2 size)
{
    this->viewportSize = size;
}

glm::ivec2 Settings::getViewportSize() const
{
    return this->viewportSize;
}
