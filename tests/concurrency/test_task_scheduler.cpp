#include <gtest/gtest.h>
#include "../../src/concurrency/thread_pool.h"
#include "../utils/test_utils.h"
#include <atomic>
#include <chrono>

using namespace v3d::concurrency;
using namespace v3d::test;

class TaskSchedulerTest : public ::testing::Test {
protected:
    void SetUp() override {
        scheduler_ = &TaskScheduler::getInstance();
        threadPool_ = std::make_unique<ThreadPool>(4);
        threadPool_->start();
        scheduler_->setThreadPool(threadPool_.get());
    }

    void TearDown() override {
        if (threadPool_) {
            threadPool_->stop();
        }
        scheduler_->clear();
    }

    TaskScheduler* scheduler_;
    std::unique_ptr<ThreadPool> threadPool_;
};

TEST_F(TaskSchedulerTest, BasicSchedule) {
    std::atomic<int> result(0);
    
    auto future = scheduler_->schedule([&result]() {
        result = 42;
        return result.load();
    });
    
    int value = future.get();
    
    EXPECT_EQ(value, 42);
    EXPECT_EQ(result, 42);
}

TEST_F(TaskSchedulerTest, ScheduleWithArguments) {
    auto future = scheduler_->schedule([](int a, int b) {
        return a * b;
    }, 6, 7);
    
    EXPECT_EQ(future.get(), 42);
}

TEST_F(TaskSchedulerTest, ScheduleDelayed) {
    std::atomic<bool> executed(false);
    auto startTime = std::chrono::high_resolution_clock::now();
    
    scheduler_->scheduleDelayed([&executed]() {
        executed = true;
    }, std::chrono::milliseconds(100));
    
    TestUtils::sleepMs(50);
    EXPECT_FALSE(executed);
    
    TestUtils::sleepMs(100);
    EXPECT_TRUE(executed);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    EXPECT_GE(duration, 100);
}

TEST_F(TaskSchedulerTest, ScheduleRepeated) {
    std::atomic<int> count(0);
    
    scheduler_->scheduleRepeated([&count]() {
        count++;
    }, std::chrono::milliseconds(50));
    
    TestUtils::sleepMs(100);
    EXPECT_GE(count, 1);
    
    TestUtils::sleepMs(100);
    EXPECT_GE(count, 2);
}

TEST_F(TaskSchedulerTest, MultipleDelayedTasks) {
    std::vector<int> executionOrder;
    std::mutex orderMutex;
    
    auto addOrder = [&executionOrder, &orderMutex](int value) {
        std::lock_guard<std::mutex> lock(orderMutex);
        executionOrder.push_back(value);
    };
    
    scheduler_->scheduleDelayed([addOrder]() { addOrder(1); }, std::chrono::milliseconds(50));
    scheduler_->scheduleDelayed([addOrder]() { addOrder(2); }, std::chrono::milliseconds(100));
    scheduler_->scheduleDelayed([addOrder]() { addOrder(3); }, std::chrono::milliseconds(25));
    
    TestUtils::sleepMs(150);
    
    EXPECT_EQ(executionOrder.size(), 3);
    EXPECT_EQ(executionOrder[0], 3);
    EXPECT_EQ(executionOrder[1], 1);
    EXPECT_EQ(executionOrder[2], 2);
}

TEST_F(TaskSchedulerTest, MultipleRepeatedTasks) {
    std::atomic<int> count1(0);
    std::atomic<int> count2(0);
    
    scheduler_->scheduleRepeated([&count1]() { count1++; }, std::chrono::milliseconds(50));
    scheduler_->scheduleRepeated([&count2]() { count2++; }, std::chrono::milliseconds(75));
    
    TestUtils::sleepMs(200);
    
    EXPECT_GE(count1, 3);
    EXPECT_GE(count2, 2);
}

TEST_F(TaskSchedulerTest, Update) {
    std::atomic<int> count(0);
    
    scheduler_->scheduleRepeated([&count]() {
        count++;
    }, std::chrono::milliseconds(50));
    
    TestUtils::sleepMs(100);
    
    scheduler_->update();
    
    TestUtils::sleepMs(100);
    
    EXPECT_GE(count, 2);
}

TEST_F(TaskSchedulerTest, Clear) {
    std::atomic<int> count(0);
    
    scheduler_->scheduleRepeated([&count]() {
        count++;
    }, std::chrono::milliseconds(50));
    
    TestUtils::sleepMs(100);
    
    scheduler_->clear();
    
    int countBefore = count.load();
    
    TestUtils::sleepMs(100);
    
    int countAfter = count.load();
    
    EXPECT_EQ(countBefore, countAfter);
}

TEST_F(TaskSchedulerTest, SetThreadPool) {
    auto pool1 = std::make_unique<ThreadPool>(2);
    auto pool2 = std::make_unique<ThreadPool>(4);
    
    pool1->start();
    pool2->start();
    
    scheduler_->setThreadPool(pool1.get());
    
    std::atomic<int> count(0);
    auto future = scheduler_->schedule([&count]() {
        count++;
        return count.load();
    });
    
    future.get();
    EXPECT_EQ(count, 1);
    
    scheduler_->setThreadPool(pool2.get());
    
    future = scheduler_->schedule([&count]() {
        count++;
        return count.load();
    });
    
    future.get();
    EXPECT_EQ(count, 2);
    
    pool1->stop();
    pool2->stop();
}

TEST_F(TaskSchedulerTest, GetThreadPool) {
    ThreadPool* pool = scheduler_->getThreadPool();
    
    EXPECT_EQ(pool, threadPool_.get());
}

TEST_F(TaskSchedulerTest, ScheduleTask) {
    std::atomic<int> result(0);
    
    auto task = std::make_unique<FunctionTask<std::function<void()>>>([&result]() {
        result = 100;
    }, 5);
    
    scheduler_->scheduleTask(std::move(task));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_EQ(result, 100);
}

TEST_F(TaskSchedulerTest, DelayedTaskAccuracy) {
    const int delayMs = 100;
    const int toleranceMs = 20;
    
    std::atomic<bool> executed(false);
    auto startTime = std::chrono::high_resolution_clock::now();
    
    scheduler_->scheduleDelayed([&executed]() {
        executed = true;
    }, std::chrono::milliseconds(delayMs));
    
    while (!executed) {
        TestUtils::sleepMs(1);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    EXPECT_GE(duration, delayMs);
    EXPECT_LE(duration, delayMs + toleranceMs);
}

TEST_F(TaskSchedulerTest, RepeatedTaskInterval) {
    const int intervalMs = 50;
    const int toleranceMs = 20;
    
    std::vector<std::chrono::steady_clock::time_point> executionTimes;
    std::mutex timesMutex;
    
    scheduler_->scheduleRepeated([&executionTimes, &timesMutex]() {
        std::lock_guard<std::mutex> lock(timesMutex);
        executionTimes.push_back(std::chrono::steady_clock::now());
    }, std::chrono::milliseconds(intervalMs));
    
    TestUtils::sleepMs(200);
    
    EXPECT_GE(executionTimes.size(), 3);
    
    if (executionTimes.size() >= 2) {
        auto interval1 = std::chrono::duration_cast<std::chrono::milliseconds>(
            executionTimes[1] - executionTimes[0]).count();
        
        EXPECT_GE(interval1, intervalMs - toleranceMs);
        EXPECT_LE(interval1, intervalMs + toleranceMs);
    }
}

TEST_F(TaskSchedulerTest, StressTest_ManyDelayedTasks) {
    const int numTasks = 100;
    std::atomic<int> count(0);
    
    TestUtils::measureTime("Schedule 100 delayed tasks", [this, &count, numTasks]() {
        for (int i = 0; i < numTasks; ++i) {
            scheduler_->scheduleDelayed([&count]() {
                count++;
            }, std::chrono::milliseconds(i));
        }
    });
    
    TestUtils::sleepMs(numTasks + 100);
    
    EXPECT_EQ(count, numTasks);
}

TEST_F(TaskSchedulerTest, StressTest_ManyRepeatedTasks) {
    const int numTasks = 10;
    std::atomic<int> totalExecutions(0);
    
    for (int i = 0; i < numTasks; ++i) {
        scheduler_->scheduleRepeated([&totalExecutions]() {
            totalExecutions++;
        }, std::chrono::milliseconds(50 + i * 10));
    }
    
    TestUtils::sleepMs(300);
    
    EXPECT_GT(totalExecutions, 0);
}

TEST_F(TaskSchedulerTest, StressTest_MixedTasks) {
    std::atomic<int> immediateCount(0);
    std::atomic<int> delayedCount(0);
    std::atomic<int> repeatedCount(0);
    
    for (int i = 0; i < 50; ++i) {
        scheduler_->schedule([&immediateCount]() {
            immediateCount++;
        });
        
        scheduler_->scheduleDelayed([&delayedCount]() {
            delayedCount++;
        }, std::chrono::milliseconds(50));
    }
    
    scheduler_->scheduleRepeated([&repeatedCount]() {
        repeatedCount++;
    }, std::chrono::milliseconds(100));
    
    TestUtils::sleepMs(200);
    
    EXPECT_EQ(immediateCount, 50);
    EXPECT_EQ(delayedCount, 50);
    EXPECT_GE(repeatedCount, 1);
}

TEST_F(TaskSchedulerTest, StressTest_RapidScheduleClear) {
    for (int i = 0; i < 10; ++i) {
        std::atomic<int> count(0);
        
        scheduler_->scheduleRepeated([&count]() {
            count++;
        }, std::chrono::milliseconds(50));
        
        TestUtils::sleepMs(50);
        
        scheduler_->clear();
    }
}
