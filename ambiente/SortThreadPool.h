#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>


class SortThreadPool {
public:
    explicit SortThreadPool();
    explicit SortThreadPool(size_t n);
    ~SortThreadPool();

    template<class F>
    auto enqueue(F f) -> std::future<void> {
        auto task = std::make_shared<std::packaged_task<void()>>(f);
        std::future<void> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
     }


private:
    void Initialize(size_t n);
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
	size_t max_queue_size; 
};
