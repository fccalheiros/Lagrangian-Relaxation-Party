#include "ParallelSort.h"
#include "Windows.h"
#include <sstream>


ParallelSorter::ParallelSorter() : _stop(false) {
    Initialize();
}


void ParallelSorter::Initialize() {
	auto n = 1 << _max_depth;
    for (auto i = 0; i < n; ++i) {
        workers.emplace_back([this] {
            while (true) {
				std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->_queue_mutex);
                    this->_condition.wait(lock, [this] {
                        return this->_stop || !this->tasks.empty();
                        });
                    if (this->_stop && this->tasks.empty()) return;
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

ParallelSorter::~ParallelSorter() {
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _stop = true;
    }
    _condition.notify_all();
    for (std::thread& worker : workers) {
		if ( worker.joinable() )
			worker.join();
	}
}