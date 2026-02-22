#pragma once

#include <array>
#include <algorithm>
#include <numeric>

class FrameTimer
{
public:
    static constexpr size_t HISTORY_SIZE = 120;

    void update(double deltaTime);

    [[nodiscard]] int getFPS() const { return displayedFPS; }
    [[nodiscard]] double getFrameTimeMs() const { return displayedFrameMs; }
    [[nodiscard]] double getAvgFPS() const { return avgFPS; }
    [[nodiscard]] double getMinFPS() const { return minFPS; }
    [[nodiscard]] double getMaxFPS() const { return maxFPS; }

    [[nodiscard]] const std::array<double, HISTORY_SIZE>& getHistory() const { return history; }

private:
    std::array<double, HISTORY_SIZE> history{};
    size_t head = 0;
    size_t count = 0;

    double uiTimer = 0.0;

    int displayedFPS = 0;
    double displayedFrameMs = 0.0;

    double avgFPS = 0.0;
    double minFPS = 0.0;
    double maxFPS = 0.0;
};
