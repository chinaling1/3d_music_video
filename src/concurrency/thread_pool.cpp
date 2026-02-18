#include "thread_pool.h"
#include <algorithm>

namespace v3d {
namespace concurrency {

ThreadPool::ThreadPool(size_t numThreads)
    : running_(false)
    , paused_(false)
    , activeThreads_(0)
    , maxQueueSize_(10000) {

    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
    }

    workers_.reserve(numThreads);
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {
    if (running_) {
        return;
    }

    running_ = true;
    paused_ = false;

    for (size_t i = 0; i < workers_.capacity(); ++i) {
        workers_.emplace_back(&ThreadPool::workerThread, this, i);
    }
}

void ThreadPool::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    condition_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    workers_.clear();
}

void ThreadPool::pause() {
    paused_ = true;
}

void ThreadPool::resume() {
    paused_ = false;
    condition_.notify_all();
}

void ThreadPool::workerThread(size_t threadId) {
    while (running_) {
        std::unique_ptr<Task> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex_);

            condition_.wait(lock, [this] {
                return !running_ || (!paused_ && !tasks_.empty());
            });

            if (!running_) {
                break;
            }

            if (paused_) {
                continue;
            }

            if (tasks_.empty()) {
                continue;
            }

            task = std::move(const_cast<std::unique_ptr<Task>&>(tasks_.top()));
            tasks_.pop();
        }

        if (task && !task->isCancelled()) {
            activeThreads_++;
            task->execute();
            activeThreads_--;
        }
    }
}

size_t ThreadPool::getThreadCount() const {
    return workers_.size();
}

size_t ThreadPool::getActiveThreadCount() const {
    return activeThreads_.load();
}

size_t ThreadPool::getPendingTaskCount() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return tasks_.size();
}

void ThreadPool::setMaxQueueSize(size_t maxSize) {
    maxQueueSize_ = maxSize;
}

size_t ThreadPool::getMaxQueueSize() const {
    return maxQueueSize_;
}

void ThreadPool::clearQueue() {
    std::lock_guard<std::mutex> lock(queueMutex_);

    while (!tasks_.empty()) {
        tasks_.pop();
    }
}

bool ThreadPool::isRunning() const {
    return running_;
}

bool ThreadPool::isPaused() const {
    return paused_;
}

TaskScheduler& TaskScheduler::getInstance() {
    static TaskScheduler instance;
    return instance;
}

TaskScheduler::TaskScheduler()
    : threadPool_(nullptr) {
}

TaskScheduler::~TaskScheduler() {
    clear();
}

void TaskScheduler::update() {
    auto now = std::chrono::system_clock::now();

    {
        std::lock_guard<std::mutex> lock(tasksMutex_);

        auto it = delayedTasks_.begin();
        while (it != delayedTasks_.end()) {
            if (now >= it->executeTime) {
                scheduleTask(std::unique_ptr<Task>(
                    new FunctionTask(it->func)
                ));
                it = delayedTasks_.erase(it);
            } else {
                ++it;
            }
        }

        for (auto& repeatedTask : repeatedTasks_) {
            if (!repeatedTask->cancelled && now >= repeatedTask->nextExecuteTime) {
                scheduleTask(std::unique_ptr<Task>(
                    new FunctionTask(repeatedTask->func)
                ));
                repeatedTask->nextExecuteTime = now + repeatedTask->interval;
            }
        }

        repeatedTasks_.erase(
            std::remove_if(repeatedTasks_.begin(), repeatedTasks_.end(),
                [](const std::shared_ptr<RepeatedTask>& task) {
                    return task->cancelled;
                }),
            repeatedTasks_.end()
        );
    }
}

void TaskScheduler::setThreadPool(ThreadPool* pool) {
    threadPool_ = pool;
}

ThreadPool* TaskScheduler::getThreadPool() {
    return threadPool_;
}

void TaskScheduler::clear() {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    delayedTasks_.clear();
    repeatedTasks_.clear();
}

}
}