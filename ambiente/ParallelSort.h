#pragma once

#include <vector>
#include <algorithm>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>


enum class SortMode {
    AUTO,
	SEQUENTIAL,
    SEQUENTIAL_MERGE,
    PARALLEL_MERGE
};

class ParallelSorter {
public:
    explicit ParallelSorter();
    ~ParallelSorter();

	void EnableParallelExecution() { _allowParallelExecution = true; }
	void DisableParallelExecution () { _allowParallelExecution = false; }

    template<class F>
    auto enqueue(F f) -> std::future<void> {
        auto task = std::make_shared<std::packaged_task<void()>>(f);
        std::future<void> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(_queue_mutex);
            tasks.emplace([task]() { (*task)(); });
        }
        _condition.notify_one();
        return res;
     }
	 
	template< class Iterator, class Comparator>
	void Sort(Iterator begin, Iterator end, Comparator comp, SortMode mode = SortMode::AUTO)
	{

		size_t size = end - begin;
		if (mode == SortMode::AUTO) {
			mode = (size > _parallelThreshold) ? SortMode::PARALLEL_MERGE : SortMode::SEQUENTIAL_MERGE;
		}

		if (mode == SortMode::SEQUENTIAL) {
			std::sort(begin, end, comp);
			return;
		}

		if (mode == SortMode::SEQUENTIAL_MERGE || !_allowParallelExecution) {
			SequentialMergeSort(begin, end, comp , 0);
			return;
		}

		std::lock_guard<std::mutex> lock(
			_executionMutex
		);

		ThreadMergeSort( begin, end, comp, 0);
	};

private:
	 
	template <class Iterator, class StrictWeakOrdering>
    void SequentialMergeSort(Iterator begin, Iterator end, StrictWeakOrdering comp, int depth) {
		size_t size = end - begin;
	
        if ((depth < _max_recursion_depth) && (size > _min_set_size)) {
			Iterator mid = begin + size / 2;
            SequentialMergeSort( begin, mid, comp, depth + 1);
            SequentialMergeSort( mid, end, comp, depth + 1);
            auto it = std::lower_bound(begin, mid, *(mid), comp);
			std::inplace_merge(it, mid, end, comp);
        }
        else {
            std::sort(begin, end, comp);
        }
    }
	
	template <class Iterator, class StrictWeakOrdering>
    void ThreadMergeSort(Iterator begin, Iterator end, StrictWeakOrdering comp, int depth) {
      
	  size_t size = end - begin;

        if (size <= _min_set_size) {
            std::sort(begin, end, comp);
            return;
        }

        Iterator mid = begin + size / 2;

        if (size > _parallelThreshold && depth < _max_depth) {
            auto fut = enqueue([this, begin, mid, comp, depth] {
                ThreadMergeSort(begin, mid, comp, depth + 1);
                });

            ThreadMergeSort(mid, end, comp, depth + 1);
            fut.get();
        }
        else {
            SequentialMergeSort(begin, mid, comp, depth + 1);
            SequentialMergeSort(mid, end, comp, depth + 1);
        }

        Iterator it = std::lower_bound(begin, mid, *mid, comp);
        std::inplace_merge(it, mid, end, comp);
    }

private:
    void Initialize();
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex _queue_mutex;
	std::mutex _executionMutex;
    std::condition_variable _condition;
    std::atomic<bool> _allowParallelExecution = true;
    bool _stop;
	int _max_depth = 4;
	size_t _parallelThreshold = 2500;
	int _max_recursion_depth = 15;
	int _min_set_size = 128;

	
};