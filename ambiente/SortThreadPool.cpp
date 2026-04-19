#include "SortThreadPool.h"

SortThreadPool::SortThreadPool() : stop(false) {
    Initialize(std::thread::hardware_concurrency());
}

SortThreadPool::SortThreadPool(size_t n) : stop(false) {
    Initialize(n);
}


void SortThreadPool::Initialize(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                        });
                    if (this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
            });
    }
}

SortThreadPool::~SortThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) worker.join();
}



