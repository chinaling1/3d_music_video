#include <gtest/gtest.h>
#include "../../src/core/logger.h"
#include "../utils/test_utils.h"
#include <fstream>
#include <thread>

using namespace v3d::core;
using namespace v3d::test;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = LoggerManager::getInstance().getOrCreateLogger("test_logger");
        testLogPath_ = "test_temp/test_log.txt";
    }

    void TearDown() override {
        logger_->clearAppenders();
    }

    std::string readLogFile(const std::string& path) {
        std::ifstream file(path);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }

    std::shared_ptr<Logger> logger_;
    std::string testLogPath_;
};

TEST_F(LoggerTest, LogLevels) {
    logger_->addAppender(std::make_shared<ConsoleAppender>());
    
    logger_->trace("Trace message");
    logger_->debug("Debug message");
    logger_->info("Info message");
    logger_->warning("Warning message");
    logger_->error("Error message");
    logger_->fatal("Fatal message");
    
    SUCCEED();
}

TEST_F(LoggerTest, SetLogLevel) {
    logger_->setLevel(LogLevel::Warning);
    logger_->addAppender(std::make_shared<ConsoleAppender>());
    
    logger_->trace("Trace");
    logger_->debug("Debug");
    logger_->info("Info");
    logger_->warning("Warning");
    logger_->error("Error");
    logger_->fatal("Fatal");
    
    EXPECT_EQ(logger_->getLevel(), LogLevel::Warning);
}

TEST_F(LoggerTest, FileAppender) {
    TestUtils::createDirectory("test_temp");
    
    logger_->addAppender(std::make_shared<FileAppender>(testLogPath_));
    
    logger_->info("Test message");
    logger_->warning("Warning message");
    logger_->error("Error message");
    
    logger_->flush();
    
    std::string content = readLogFile(testLogPath_);
    
    EXPECT_TRUE(content.find("Test message") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
}

TEST_F(LoggerTest, MultipleAppenders) {
    TestUtils::createDirectory("test_temp");
    
    logger_->addAppender(std::make_shared<ConsoleAppender>());
    logger_->addAppender(std::make_shared<FileAppender>(testLogPath_));
    
    logger_->info("Multi-appender test");
    
    logger_->flush();
    
    std::string fileContent = readLogFile(testLogPath_);
    
    EXPECT_TRUE(fileContent.find("Multi-appender test") != std::string::npos);
}

TEST_F(LoggerTest, RollingFileAppender) {
    TestUtils::createDirectory("test_temp");
    
    const size_t maxFileSize = 1024;
    const int maxFiles = 3;
    
    logger_->addAppender(std::make_shared<RollingFileAppender>(testLogPath_, maxFileSize, maxFiles));
    
    std::string longMessage(100, 'A');
    
    for (int i = 0; i < 10; ++i) {
        logger_->info("Message " + std::to_string(i) + ": " + longMessage);
    }
    
    logger_->flush();
    
    EXPECT_TRUE(TestUtils::fileExists(testLogPath_));
}

TEST_F(LoggerTest, ThreadSafety) {
    logger_->addAppender(std::make_shared<ConsoleAppender>());
    
    const int numThreads = 10;
    const int numLogsPerThread = 100;
    std::atomic<int> logCount(0);
    
    auto logFunc = [this, &logCount, numLogsPerThread]() {
        for (int i = 0; i < numLogsPerThread; ++i) {
            logger_->info("Thread log: " + std::to_string(i));
            logCount++;
        }
    };
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(logFunc);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(logCount, numThreads * numLogsPerThread);
}

TEST_F(LoggerTest, ClearAppenders) {
    logger_->addAppender(std::make_shared<ConsoleAppender>());
    
    logger_->info("Before clear");
    
    logger_->clearAppenders();
    
    logger_->info("After clear");
    
    SUCCEED();
}

TEST_F(LoggerTest, Flush) {
    TestUtils::createDirectory("test_temp");
    
    logger_->addAppender(std::make_shared<FileAppender>(testLogPath_));
    
    logger_->info("Before flush");
    
    logger_->flush();
    
    std::string content = readLogFile(testLogPath_);
    
    EXPECT_TRUE(content.find("Before flush") != std::string::npos);
}

TEST_F(LoggerTest, LoggerName) {
    EXPECT_EQ(logger_->getName(), "test_logger");
}

TEST_F(LoggerTest, GetLevel) {
    logger_->setLevel(LogLevel::Debug);
    EXPECT_EQ(logger_->getLevel(), LogLevel::Debug);
    
    logger_->setLevel(LogLevel::Error);
    EXPECT_EQ(logger_->getLevel(), LogLevel::Error);
}

TEST_F(LoggerTest, StressTest_ManyLogs) {
    logger_->addAppender(std::make_shared<ConsoleAppender>());
    
    const int numLogs = 1000;
    
    for (int i = 0; i < numLogs; ++i) {
        logger_->info("Log message number: " + std::to_string(i));
    }
    
    SUCCEED();
}

TEST_F(LoggerTest, StressTest_ManyThreads) {
    logger_->addAppender(std::make_shared<ConsoleAppender>());
    
    const int numThreads = 20;
    const int numLogsPerThread = 100;
    std::atomic<int> logCount(0);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, &logCount, numLogsPerThread]() {
            for (int j = 0; j < numLogsPerThread; ++j) {
                logger_->info("Thread log: " + std::to_string(j));
                logCount++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(logCount, numThreads * numLogsPerThread);
}

TEST_F(LoggerTest, Performance_FileLogging) {
    TestUtils::createDirectory("test_temp");
    
    logger_->addAppender(std::make_shared<FileAppender>(testLogPath_));
    
    const int numLogs = 1000;
    
    for (int i = 0; i < numLogs; ++i) {
        logger_->info("Performance test message: " + std::to_string(i));
    }
    
    logger_->flush();
    
    std::string content = readLogFile(testLogPath_);
    EXPECT_GT(content.length(), 0u);
}
