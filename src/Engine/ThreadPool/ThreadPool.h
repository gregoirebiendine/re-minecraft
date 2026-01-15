#ifndef RE_MINECRAFT_THREADPOOL_H
#define RE_MINECRAFT_THREADPOOL_H

#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(size_t workers);
    ~ThreadPool();

    void enqueue(std::function<void()> job);

private:
    void workerLoop();

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;

    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
};

#endif //RE_MINECRAFT_THREADPOOL_H