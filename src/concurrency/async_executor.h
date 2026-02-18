#pragma once

#include "thread_pool.h"
#include <future>
#include <vector>
#include <functional>
#include <type_traits>

namespace v3d {
namespace concurrency {

class AsyncExecutor {
public:
    static AsyncExecutor& getInstance();

    template<typename F, typename... Args>
    auto execute(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

    template<typename F>
    auto executeAsync(F&& f) -> std::future<decltype(f())>;

    template<typename F>
    void executeFireAndForget(F&& f);

    void setThreadPool(ThreadPool* pool);
    ThreadPool* getThreadPool();

private:
    AsyncExecutor();
    ~AsyncExecutor();

    ThreadPool* threadPool_;
};

template<typename F, typename... Args>
auto AsyncExecutor::execute(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    if (threadPool_) {
        return threadPool_->submit(std::forward<F>(f), std::forward<Args>(args)...);
    }

    using ReturnType = decltype(f(args...));
    std::promise<ReturnType> promise;
    auto future = promise.get_future();

    try {
        if constexpr (std::is_void_v<ReturnType>) {
            f(args...);
            promise.set_value();
        } else {
            promise.set_value(f(args...));
        }
    } catch (...) {
        promise.set_exception(std::current_exception());
    }

    return future;
}

template<typename F>
auto AsyncExecutor::executeAsync(F&& f) -> std::future<decltype(f())> {
    return execute(std::forward<F>(f));
}

template<typename F>
void AsyncExecutor::executeFireAndForget(F&& f) {
    if (threadPool_) {
        threadPool_->submitTask(std::unique_ptr<Task>(
            new FunctionTask(std::forward<F>(f))
        ));
    } else {
        f();
    }
}

template<typename IndexType, typename F>
void parallel_for(IndexType begin, IndexType end, F&& func, ThreadPool* pool) {
    if (!pool) {
        pool = TaskScheduler::getInstance().getThreadPool();
    }

    if (!pool || (end - begin) < 100) {
        for (IndexType i = begin; i < end; ++i) {
            func(i);
        }
        return;
    }

    const size_t numThreads = pool->getThreadCount();
    const IndexType chunkSize = (end - begin) / numThreads;

    std::vector<std::future<void>> futures;
    futures.reserve(numThreads);

    for (size_t i = 0; i < numThreads - 1; ++i) {
        IndexType chunkBegin = begin + i * chunkSize;
        IndexType chunkEnd = chunkBegin + chunkSize;

        futures.push_back(pool->submit([chunkBegin, chunkEnd, &func]() {
            for (IndexType j = chunkBegin; j < chunkEnd; ++j) {
                func(j);
            }
        }));
    }

    IndexType lastBegin = begin + (numThreads - 1) * chunkSize;
    for (IndexType i = lastBegin; i < end; ++i) {
        func(i);
    }

    for (auto& future : futures) {
        future.wait();
    }
}

template<typename Iterator, typename F, typename R>
std::vector<R> parallel_map(Iterator begin, Iterator end, F&& func, ThreadPool* pool = nullptr) {
    if (!pool) {
        pool = TaskScheduler::getInstance().getThreadPool();
    }

    std::vector<R> results(std::distance(begin, end));

    if (!pool || results.size() < 100) {
        size_t index = 0;
        for (auto it = begin; it != end; ++it) {
            results[index++] = func(*it);
        }
        return results;
    }

    const size_t numThreads = pool->getThreadCount();
    const size_t chunkSize = results.size() / numThreads;

    std::vector<std::future<void>> futures;
    futures.reserve(numThreads);

    auto current = begin;
    for (size_t i = 0; i < numThreads - 1; ++i) {
        auto next = current;
        std::advance(next, chunkSize);
        size_t startIndex = i * chunkSize;

        futures.push_back(pool->submit([current, next, &func, &results, startIndex]() {
            size_t index = startIndex;
            for (auto it = current; it != next; ++it) {
                results[index++] = func(*it);
            }
        }));

        current = next;
    }

    size_t lastIndex = (numThreads - 1) * chunkSize;
    for (auto it = current; it != end; ++it) {
        results[lastIndex++] = func(*it);
    }

    for (auto& future : futures) {
        future.wait();
    }

    return results;
}

template<typename Iterator, typename F, typename T>
T parallel_reduce(Iterator begin, Iterator end, T init, F&& func, ThreadPool* pool = nullptr) {
    if (!pool) {
        pool = TaskScheduler::getInstance().getThreadPool();
    }

    const size_t size = std::distance(begin, end);

    if (!pool || size < 1000) {
        T result = init;
        for (auto it = begin; it != end; ++it) {
            result = func(result, *it);
        }
        return result;
    }

    const size_t numThreads = pool->getThreadCount();
    const size_t chunkSize = size / numThreads;

    std::vector<std::future<T>> futures;
    futures.reserve(numThreads);

    auto current = begin;
    for (size_t i = 0; i < numThreads; ++i) {
        auto next = current;
        std::advance(next, (i == numThreads - 1) ? (end - current) : chunkSize);

        futures.push_back(pool->submit([current, next, &func, init]() {
            T result = init;
            for (auto it = current; it != next; ++it) {
                result = func(result, *it);
            }
            return result;
        }));

        current = next;
    }

    T result = init;
    for (auto& future : futures) {
        result = func(result, future.get());
    }

    return result;
}

}
}