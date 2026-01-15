#include "ThreadPool.h"

ThreadPool::ThreadPool(const size_t workers)
{
    for (size_t i = 0; i < workers; ++i)
        threads.emplace_back(&ThreadPool::workerLoop, this);
}

ThreadPool::~ThreadPool()
{
    running = false;
    cv.notify_all();
    for (auto& t : threads)
        t.join();
}

void ThreadPool::enqueue(std::function<void()> job)
{
    {
        std::lock_guard lock(mutex);
        jobs.push(std::move(job));
    }
    cv.notify_one();
}

void ThreadPool::workerLoop()
{
    while (running) {
        std::function<void()> job;

        {
            std::unique_lock lock(mutex);
            cv.wait(lock, [&] { return !jobs.empty() || !running; });

            if (!running)
                return;

            job = std::move(jobs.front());
            jobs.pop();
        }

        job();
    }
}