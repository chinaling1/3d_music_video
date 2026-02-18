#include <gtest/gtest.h>
#include "../../src/concurrency/async_executor.h"
#include <atomic>
#include <future>
#include <chrono>

using namespace v3d::concurrency;

class AsyncExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        executor_ = &AsyncExecutor::getInstance();
    }

    AsyncExecutor* executor_;
};

TEST_F(AsyncExecutorTest, BasicExecute) {
    std::atomic<int> result(0);
    
    auto future = executor_->execute([&result]() {
        result = 42;
        return result.load();
    });
    
    int value = future.get();
    
    EXPECT_EQ(value, 42);
    EXPECT_EQ(result, 42);
}

TEST_F(AsyncExecutorTest, ExecuteWithArguments) {
    auto future = executor_->execute([](int a, int b) {
        return a + b;
    }, 10, 20);
    
    EXPECT_EQ(future.get(), 30);
}

TEST_F(AsyncExecutorTest, MultipleExecutions) {
    std::atomic<int> count(0);
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(executor_->execute([&count, i]() {
            count++;
            return i;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(count, 10);
}

TEST_F(AsyncExecutorTest, ExecuteVoid) {
    std::atomic<bool> executed(false);
    
    auto future = executor_->execute([&executed]() {
        executed = true;
    });
    
    future.wait();
    
    EXPECT_TRUE(executed);
}

TEST_F(AsyncExecutorTest, ExecuteWithException) {
    auto future = executor_->execute([]() {
        throw std::runtime_error("Test exception");
        return 42;
    });
    
    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(AsyncExecutorTest, ExecuteAsync) {
    std::atomic<int> result(0);
    
    auto future = executor_->executeAsync([&result]() {
        result = 100;
        return result.load();
    });
    
    int value = future.get();
    EXPECT_EQ(value, 100);
    EXPECT_EQ(result, 100);
}

TEST_F(AsyncExecutorTest, ExecuteFireAndForget) {
    std::atomic<bool> executed(false);
    
    executor_->executeFireAndForget([&executed]() {
        executed = true;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(executed);
}

TEST_F(AsyncExecutorTest, StressTest_ManyExecutions) {
    const int numTasks = 1000;
    std::atomic<int> count(0);
    std::vector<std::future<int>> futures;
    futures.reserve(numTasks);
    
    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(executor_->execute([&count, i]() {
            count++;
            return i;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    EXPECT_EQ(count, numTasks);
}

TEST_F(AsyncExecutorTest, ParallelFor) {
    std::atomic<int> count(0);
    const int numIterations = 100;
    
    parallel_for(0, numIterations, [&count](int i) {
        count++;
    }, nullptr);
    
    EXPECT_EQ(count, numIterations);
}

TEST_F(AsyncExecutorTest, GetThreadPool) {
    ThreadPool* pool = executor_->getThreadPool();
    EXPECT_NE(pool, nullptr);
}
