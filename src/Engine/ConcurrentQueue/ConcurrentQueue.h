#ifndef RE_MINECRAFT_CONCURRENTQUEUE_H
#define RE_MINECRAFT_CONCURRENTQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ConcurrentQueue {
    public:
        void push(T value) {
            std::lock_guard lock(this->mutex);

            this->queue.push(std::move(value));
            this->cv.notify_one();
        }

        bool tryPop(T& out) {
            std::lock_guard lock(this->mutex);

            if (queue.empty())
                return false;

            out = std::move(this->queue.front());
            this->queue.pop();
            return true;
        }

    private:
        std::queue<T> queue;
        std::mutex mutex;
        std::condition_variable cv;

};


#endif //RE_MINECRAFT_CONCURRENTQUEUE_H