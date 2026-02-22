#include "FrameTimer.h"

void FrameTimer::update(const double deltaTime)
{
    this->history[this->head] = deltaTime;
    this->head  = (this->head + 1) % HISTORY_SIZE;
    this->count = std::min(this->count + 1, HISTORY_SIZE);

    this->uiTimer += deltaTime;

    if (this->uiTimer < 0.5)
        return;

    this->uiTimer = 0.0;

    const double sum = std::accumulate(this->history.begin(), this->history.begin() + this->count, 0.0);
    const double avgDt = sum / static_cast<double>(this->count);
    const double minDt = *std::min_element(this->history.begin(), this->history.begin() + this->count);
    const double maxDt = *std::max_element(this->history.begin(), this->history.begin() + this->count);

    this->avgFPS = 1.0 / avgDt;
    this->minFPS = 1.0 / maxDt; // worst frame
    this->maxFPS = 1.0 / minDt; // best frame

    this->displayedFPS = static_cast<int>(avgFPS);
    this->displayedFrameMs = avgDt * 1000.0;
}
