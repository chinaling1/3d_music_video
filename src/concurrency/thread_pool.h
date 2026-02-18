/**
 * @file thread_pool.h
 * @brief 线程池和任务调度器 - 提供高效的并发任务处理
 * 
 * 该模块实现了完整的并发处理系统，包括：
 * - Task: 任务基类
 * - FunctionTask: 函数包装任务
 * - ThreadPool: 线程池
 * - TaskScheduler: 任务调度器
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <functional>
#include <future>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include <type_traits>

namespace v3d {
namespace concurrency {

/**
 * @class Task
 * @brief 任务基类 - 定义任务的执行接口
 * 
 * 所有任务都应继承此类。支持优先级和取消操作。
 * 
 * @example
 * @code
 * class MyTask : public Task {
 * public:
 *     void execute() override {
 *         // 执行任务逻辑
 *     }
 *     size_t getPriority() const override { return 10; }
 * };
 * @endcode
 */
class Task {
public:
    virtual ~Task() = default;
    
    /**
     * @brief 执行任务
     * 
     * 线程池会在工作线程中调用此方法。
     */
    virtual void execute() = 0;
    
    /**
     * @brief 取消任务
     * 
     * 标记任务为已取消状态。
     */
    virtual void cancel() {}
    
    /**
     * @brief 检查任务是否已取消
     * @return 如果已取消返回true
     */
    virtual bool isCancelled() const { return false; }
    
    /**
     * @brief 获取任务优先级
     * @return 优先级值（越大优先级越高）
     */
    virtual size_t getPriority() const { return 0; }
};

/**
 * @class FunctionTask
 * @brief 函数包装任务 - 将任意函数包装为任务
 * 
 * 模板类，可以将任意可调用对象包装为Task。
 * 
 * @tparam F 函数类型
 * 
 * @example
 * @code
 * auto task = std::make_unique<FunctionTask<std::function<void()>>>(
 *     []() { doSomething(); },
 *     5  // 优先级
 * );
 * pool.submitTask(std::move(task));
 * @endcode
 */
template<typename F>
class FunctionTask : public Task {
public:
    /**
     * @brief 构造函数任务
     * @param func 要执行的函数
     * @param priority 任务优先级（默认0）
     */
    explicit FunctionTask(F func, size_t priority = 0)
        : func_(std::move(func))
        , priority_(priority)
        , cancelled_(false) {
    }

    void execute() override {
        if (!cancelled_) {
            func_();
        }
    }

    void cancel() override {
        cancelled_ = true;
    }

    bool isCancelled() const override {
        return cancelled_;
    }

    size_t getPriority() const override {
        return priority_;
    }

private:
    F func_;                    ///< 要执行的函数
    size_t priority_;           ///< 任务优先级
    std::atomic<bool> cancelled_; ///< 取消标志
};

/**
 * @class ThreadPool
 * @brief 线程池 - 管理工作线程和任务队列
 * 
 * 提供高效的并发任务处理能力。支持：
 * - 优先级队列
 * - 任务取消
 * - 暂停/恢复
 * - 队列大小限制
 * 
 * @example
 * @code
 * // 创建并启动线程池
 * ThreadPool pool(4);  // 4个工作线程
 * pool.start();
 * 
 * // 提交任务
 * auto future = pool.submit([]() {
 *     return computeResult();
 * });
 * 
 * // 获取结果
 * auto result = future.get();
 * 
 * // 停止线程池
 * pool.stop();
 * @endcode
 */
class ThreadPool {
public:
    /**
     * @brief 构造线程池
     * @param numThreads 工作线程数量（0表示自动检测）
     */
    explicit ThreadPool(size_t numThreads = 0);
    
    ~ThreadPool();
    
    /**
     * @brief 启动线程池
     * 
     * 创建并启动所有工作线程。
     */
    void start();
    
    /**
     * @brief 停止线程池
     * 
     * 停止所有工作线程，等待当前任务完成。
     */
    void stop();
    
    /**
     * @brief 暂停线程池
     * 
     * 暂停处理新任务，当前任务继续执行。
     */
    void pause();
    
    /**
     * @brief 恢复线程池
     * 
     * 恢复处理任务。
     */
    void resume();
    
    /**
     * @brief 提交任务（模板版本）
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 要执行的函数
     * @param args 函数参数
     * @return 任务的future对象
     * 
     * 提交任务并返回可用于获取结果的future。
     * 
     * @example
     * @code
     * auto future = pool.submit([](int a, int b) {
     *     return a + b;
     * }, 10, 20);
     * 
     * int result = future.get();  // result = 30
     * @endcode
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;
    
    /**
     * @brief 提交任务对象
     * @param task 任务对象（所有权转移）
     * 
     * 提交一个Task对象到队列。
     */
    void submitTask(std::unique_ptr<Task> task);
    
    /**
     * @brief 获取工作线程数量
     * @return 线程数量
     */
    size_t getThreadCount() const;
    
    /**
     * @brief 获取活跃线程数量
     * @return 正在执行任务的线程数量
     */
    size_t getActiveThreadCount() const;
    
    /**
     * @brief 获取待处理任务数量
     * @return 队列中的任务数量
     */
    size_t getPendingTaskCount() const;
    
    /**
     * @brief 设置最大队列大小
     * @param maxSize 最大任务数（0表示无限制）
     */
    void setMaxQueueSize(size_t maxSize);
    
    /**
     * @brief 获取最大队列大小
     * @return 最大任务数
     */
    size_t getMaxQueueSize() const;
    
    /**
     * @brief 清空任务队列
     * 
     * 移除所有待处理的任务。
     */
    void clearQueue();
    
    /**
     * @brief 检查线程池是否运行中
     * @return 如果运行中返回true
     */
    bool isRunning() const;
    
    /**
     * @brief 检查线程池是否暂停
     * @return 如果暂停返回true
     */
    bool isPaused() const;

private:
    /**
     * @brief 工作线程函数
     * @param threadId 线程ID
     */
    void workerThread(size_t threadId);

    /**
     * @struct TaskComparator
     * @brief 任务比较器 - 用于优先级队列
     */
    struct TaskComparator {
        bool operator()(const std::unique_ptr<Task>& a, const std::unique_ptr<Task>& b) const {
            return a->getPriority() < b->getPriority();
        }
    };

    std::vector<std::thread> workers_;              ///< 工作线程
    std::priority_queue<std::unique_ptr<Task>, 
        std::vector<std::unique_ptr<Task>>, 
        TaskComparator> tasks_;                      ///< 任务优先级队列
    mutable std::mutex queueMutex_;                  ///< 队列互斥锁
    std::condition_variable condition_;              ///< 条件变量
    std::atomic<bool> running_;                      ///< 运行标志
    std::atomic<bool> paused_;                       ///< 暂停标志
    std::atomic<size_t> activeThreads_;              ///< 活跃线程数
    size_t maxQueueSize_;                            ///< 最大队列大小
};

/**
 * @class TaskScheduler
 * @brief 任务调度器 - 提供延迟和周期性任务调度
 * 
 * 单例模式，支持：
 * - 立即执行任务
 * - 延迟执行任务
 * - 周期性执行任务
 * 
 * @example
 * @code
 * auto& scheduler = TaskScheduler::getInstance();
 * scheduler.setThreadPool(&pool);
 * 
 * // 立即执行
 * scheduler.schedule([]() { doSomething(); });
 * 
 * // 延迟执行
 * scheduler.scheduleDelayed([]() { 
 *     doLater(); 
 * }, std::chrono::milliseconds(1000));
 * 
 * // 周期性执行
 * scheduler.scheduleRepeated([]() {
 *     update();
 * }, std::chrono::milliseconds(16));
 * 
 * // 在主循环中调用
 * scheduler.update();
 * @endcode
 */
class TaskScheduler {
public:
    /**
     * @brief 获取单例实例
     * @return TaskScheduler的单例引用
     */
    static TaskScheduler& getInstance();
    
    /**
     * @brief 调度立即执行的任务
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 要执行的函数
     * @param args 函数参数
     * @return 任务的future对象
     */
    template<typename F, typename... Args>
    auto schedule(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;
    
    /**
     * @brief 调度任务对象
     * @param task 任务对象
     */
    void scheduleTask(std::unique_ptr<Task> task);
    
    /**
     * @brief 调度延迟执行的任务
     * @tparam F 函数类型
     * @param f 要执行的函数
     * @param delay 延迟时间
     */
    template<typename F>
    void scheduleDelayed(F&& f, std::chrono::milliseconds delay);
    
    /**
     * @brief 调度周期性执行的任务
     * @tparam F 函数类型
     * @param f 要执行的函数
     * @param interval 执行间隔
     */
    template<typename F>
    void scheduleRepeated(F&& f, std::chrono::milliseconds interval);
    
    /**
     * @brief 更新调度器
     * 
     * 检查并执行到期的延迟任务和周期任务。
     * 应在主循环中定期调用。
     */
    void update();
    
    /**
     * @brief 设置线程池
     * @param pool 线程池指针
     */
    void setThreadPool(ThreadPool* pool);
    
    /**
     * @brief 获取线程池
     * @return 线程池指针
     */
    ThreadPool* getThreadPool();
    
    /**
     * @brief 清除所有待处理任务
     */
    void clear();

private:
    TaskScheduler();
    ~TaskScheduler();
    
    /**
     * @struct DelayedTask
     * @brief 延迟任务结构
     */
    struct DelayedTask {
        std::function<void()> func;                       ///< 任务函数
        std::chrono::system_clock::time_point executeTime; ///< 执行时间
    };
    
    /**
     * @struct RepeatedTask
     * @brief 周期任务结构
     */
    struct RepeatedTask {
        std::function<void()> func;                        ///< 任务函数
        std::chrono::milliseconds interval;                ///< 执行间隔
        std::chrono::system_clock::time_point nextExecuteTime; ///< 下次执行时间
        bool cancelled = false;                            ///< 取消标志
    };

    ThreadPool* threadPool_;                              ///< 线程池
    std::vector<DelayedTask> delayedTasks_;               ///< 延迟任务列表
    std::vector<std::shared_ptr<RepeatedTask>> repeatedTasks_; ///< 周期任务列表
    std::mutex tasksMutex_;                               ///< 任务互斥锁
};

template<typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    using ReturnType = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<ReturnType> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queueMutex_);

        if (tasks_.size() >= maxQueueSize_) {
            throw std::runtime_error("Task queue is full");
        }

        tasks_.push(std::unique_ptr<Task>(new FunctionTask([task]() { (*task)(); })));
    }

    condition_.notify_one();
    return result;
}

inline void ThreadPool::submitTask(std::unique_ptr<Task> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);

        if (tasks_.size() >= maxQueueSize_) {
            throw std::runtime_error("Task queue is full");
        }

        tasks_.push(std::move(task));
    }

    condition_.notify_one();
}

template<typename F, typename... Args>
auto TaskScheduler::schedule(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    if (threadPool_) {
        return threadPool_->submit(std::forward<F>(f), std::forward<Args>(args)...);
    }

    auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<decltype(f(args...))> result = task->get_future();
    (*task)();

    return result;
}

inline void TaskScheduler::scheduleTask(std::unique_ptr<Task> task) {
    if (threadPool_) {
        threadPool_->submitTask(std::move(task));
    } else {
        task->execute();
    }
}

template<typename F>
void TaskScheduler::scheduleDelayed(F&& f, std::chrono::milliseconds delay) {
    std::lock_guard<std::mutex> lock(tasksMutex_);

    DelayedTask delayedTask;
    delayedTask.func = std::forward<F>(f);
    delayedTask.executeTime = std::chrono::system_clock::now() + delay;

    delayedTasks_.push_back(std::move(delayedTask));
}

template<typename F>
void TaskScheduler::scheduleRepeated(F&& f, std::chrono::milliseconds interval) {
    std::lock_guard<std::mutex> lock(tasksMutex_);

    auto repeatedTask = std::make_shared<RepeatedTask>();
    repeatedTask->func = std::forward<F>(f);
    repeatedTask->interval = interval;
    repeatedTask->nextExecuteTime = std::chrono::system_clock::now() + interval;
    repeatedTask->cancelled = false;

    repeatedTasks_.push_back(repeatedTask);
}

}
}
