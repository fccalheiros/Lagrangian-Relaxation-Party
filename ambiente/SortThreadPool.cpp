#include "SortThreadPool.h"
#include "Windows.h"
#include <sstream>

using namespace std;

SortThreadPool::SortThreadPool() : stop(false) {
    Initialize(std::thread::hardware_concurrency());
    std::string msg = "Número de Threads: " + std::to_string(std::thread::hardware_concurrency()) + "\r\n";
    OutputDebugStringA(msg.c_str()); // versão ANSI
   
}

SortThreadPool::SortThreadPool(size_t n) : stop(false) {
    Initialize(n);
}


void SortThreadPool::Initialize(size_t n) {
    max_queue_size = n;
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
                try {
                    task();
                }
                catch (const std::exception& e) {
                    DWORD tid = GetCurrentThreadId();
                    OutputDebugStringA((std::string("Exceção na tarefa: ") +  std::to_string(tid) +  e.what() + "\r\n").c_str());
                }
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



