#include <gtest/gtest.h>
#include "../../src/concurrency/thread_pool.h"
#include "../utils/test_utils.h"
#include <atomic>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace v3d::concurrency;
using namespace v3d::test;

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        threadPool_ = std::make_unique<ThreadPool>(4);
        threadPool_->start();
    }

    void TearDown() override {
        if (threadPool_) {
            threadPool_->stop();
        }
    }

    std::unique_ptr<ThreadPool> threadPool_;
};

TEST_F(ThreadPoolTest, BasicSubmit) {
    std::atomic<int> result(0);
    
    auto future = threadPool_->submit([&result]() {
        result = 42;
        return result.load();
    });
    
    int value = future.get();
    
    EXPECT_EQ(value, 42);
    EXPECT_EQ(result, 42);
}

TEST_F(ThreadPoolTest, MultipleTasks) {
    std::atomic<int> count(0);
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(threadPool_->submit([&count, i]() {
            count++;
            return i;
        }));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
    
    EXPECT_EQ(count, 10);
}

TEST_F(ThreadPoolTest, TaskReturnValue) {
    auto future1 = threadPool_->submit([]() { return 10; });
    auto future2 = threadPool_->submit([]() { return 20.5; });
    auto future3 = threadPool_->submit([]() { return std::string("Hello"); });
    
    EXPECT_EQ(future1.get(), 10);
    EXPECT_DOUBLE_EQ(future2.get(), 20.5);
    EXPECT_EQ(future3.get(), "Hello");
}

TEST_F(ThreadPoolTest, TaskWithArguments) {
    auto future = threadPool_->submit([](int a, int b) {
        return a + b;
    }, 5, 10);
    
    EXPECT_EQ(future.get(), 15);
}

TEST_F(ThreadPoolTest, TaskException) {
    auto future = threadPool_->submit([]() {
        throw std::runtime_error("Test exception");
        return 42;
    });
    
    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(ThreadPoolTest, PriorityTasks) {
    std::vector<int> executionOrder;
    std::mutex orderMutex;
    
    auto addOrder = [&executionOrder, &orderMutex](int value) {
        std::lock_guard<std::mutex> lock(orderMutex);
        executionOrder.push_back(value);
    };
    
    auto task1 = std::make_unique<FunctionTask<std::function<void()>>>([addOrder]() { addOrder(1); }, 1);
    auto task3 = std::make_unique<FunctionTask<std::function<void()>>>([addOrder]() { addOrder(3); }, 3);
    auto task2 = std::make_unique<FunctionTask<std::function<void()>>>([addOrder]() { addOrder(2); }, 2);
    auto task5 = std::make_unique<FunctionTask<std::function<void()>>>([addOrder]() { addOrder(5); }, 5);
    auto task4 = std::make_unique<FunctionTask<std::function<void()>>>([addOrder]() { addOrder(4); }, 4);
    
    threadPool_->submitTask(std::move(task1));
    threadPool_->submitTask(std::move(task3));
    threadPool_->submitTask(std::move(task2));
    threadPool_->submitTask(std::move(task5));
    threadPool_->submitTask(std::move(task4));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(executionOrder.size(), 5);
}

TEST_F(ThreadPoolTest, StartStop) {
    auto pool = std::make_unique<ThreadPool>(2);
    
    EXPECT_FALSE(pool->isRunning());
    
    pool->start();
    
    EXPECT_TRUE(pool->isRunning());
    
    std::atomic<bool> taskExecuted(false);
    auto future = pool->submit([&taskExecuted]() {
        taskExecuted = true;
    });
    
    future.wait();
    EXPECT_TRUE(taskExecuted);
    
    pool->stop();
    
    EXPECT_FALSE(pool->isRunning());
}

TEST_F(ThreadPoolTest, PauseResume) {
    threadPool_->pause();
    
    EXPECT_TRUE(threadPool_->isPaused());
    
    std::atomic<int> count(0);
    auto future = threadPool_->submit([&count]() {
        count++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(count, 0);
    
    threadPool_->resume();
    
    future.wait();
    EXPECT_EQ(count, 1);
    
    EXPECT_FALSE(threadPool_->isPaused());
}

TEST_F(ThreadPoolTest, ThreadCount) {
    size_t threadCount = threadPool_->getThreadCount();
    
    EXPECT_EQ(threadCount, 4);
}

TEST_F(ThreadPoolTest, ActiveThreadCount) {
    std::atomic<int> activeCount(0);
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(threadPool_->submit([&activeCount]() {
            activeCount++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            activeCount--;
        }));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    size_t activeThreads = threadPool_->getActiveThreadCount();
    
    EXPECT_GT(activeThreads, 0u);
    EXPECT_LE(activeThreads, 4u);
    
    for (auto& future : futures) {
        future.wait();
    }
}

TEST_F(ThreadPoolTest, PendingTaskCount) {
    threadPool_->pause();
    
    for (int i = 0; i < 10; ++i) {
        threadPool_->submit([]() {});
    }
    
    size_t pendingCount = threadPool_->getPendingTaskCount();
    
    EXPECT_EQ(pendingCount, 10);
    
    threadPool_->resume();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    pendingCount = threadPool_->getPendingTaskCount();
    
    EXPECT_EQ(pendingCount, 0);
}

TEST_F(ThreadPoolTest, ClearQueue) {
    threadPool_->pause();
    
    for (int i = 0; i < 10; ++i) {
        threadPool_->submit([]() {});
    }
    
    EXPECT_EQ(threadPool_->getPendingTaskCount(), 10);
    
    threadPool_->clearQueue();
    
    EXPECT_EQ(threadPool_->getPendingTaskCount(), 0);
}

TEST_F(ThreadPoolTest, TaskCancellation) {
    std::atomic<bool> taskExecuted(false);
    
    auto task = std::make_unique<FunctionTask<std::function<void()>>>([&taskExecuted]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        taskExecuted = true;
    });
    
    task->cancel();
    
    threadPool_->submitTask(std::move(task));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_FALSE(taskExecuted);
}

TEST_F(ThreadPoolTest, StressTest_ManyTasks) {
    const int numTasks = 1000;
    std::atomic<int> count(0);
    
    std::vector<std::future<int>> futures;
    futures.reserve(numTasks);
    
    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(threadPool_->submit([&count, i]() {
            count++;
            return i;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(count, numTasks);
}

TEST_F(ThreadPoolTest, StressTest_LongRunningTasks) {
    const int numTasks = 100;
    std::atomic<int> count(0);
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(threadPool_->submit([&count]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            count++;
        }));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
    
    EXPECT_EQ(count, numTasks);
}

TEST_F(ThreadPoolTest, StressTest_MixedPriorities) {
    const int numTasks = 100;
    std::vector<int> executionOrder;
    std::mutex orderMutex;
    
    auto addOrder = [&executionOrder, &orderMutex](int value) {
        std::lock_guard<std::mutex> lock(orderMutex);
        executionOrder.push_back(value);
    };
    
    for (int i = 0; i < numTasks; ++i) {
        int priority = i % 10;
        auto task = std::make_unique<FunctionTask<std::function<void()>>>([addOrder, priority]() {
            addOrder(priority);
        }, priority);
        threadPool_->submitTask(std::move(task));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    EXPECT_EQ(executionOrder.size(), numTasks);
}

TEST_F(ThreadPoolTest, StressTest_RapidStartStop) {
    for (int i = 0; i < 10; ++i) {
        auto pool = std::make_unique<ThreadPool>(4);
        pool->start();
        
        std::atomic<int> count(0);
        for (int j = 0; j < 100; ++j) {
            pool->submit([&count]() { count++; });
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        pool->stop();
    }
}
