#ifndef FARFIELD_THREADPOOL_H
#define FARFIELD_THREADPOOL_H

#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>

template<typename Job>
class ThreadPool {
public:
    explicit ThreadPool(size_t n);
    ~ThreadPool();

    void enqueue(Job job);
    void setWorker(std::function<void(Job)> fn);

private:
    void loop();

    std::priority_queue<Job> jobs;
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    std::vector<std::thread> threads;
    std::function<void(Job)> worker;
};

template<typename Job>
ThreadPool<Job>::ThreadPool(size_t n) {
    for (size_t i = 0; i < n; ++i)
        threads.emplace_back(&ThreadPool::loop, this);
}

template<typename Job>
ThreadPool<Job>::~ThreadPool() {
    running = false;
    cv.notify_all();
    for (auto& t : threads) t.join();
}

template<typename Job>
void ThreadPool<Job>::setWorker(std::function<void(Job)> fn) {
    worker = fn;
}

template<typename Job>
void ThreadPool<Job>::enqueue(Job job) {
    {
        std::lock_guard lock(mutex);
        jobs.push(job);
    }
    cv.notify_one();
}

template<typename Job>
void ThreadPool<Job>::loop() {
    while (running) {
        Job job;
        {
            std::unique_lock lock(mutex);
            cv.wait(lock, [&]{ return !jobs.empty() || !running; });
            if (!running) return;
            job = jobs.top();
            jobs.pop();
        }
        worker(job);
    }
}

#endif