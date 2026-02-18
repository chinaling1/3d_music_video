#include <gtest/gtest.h>
#include "../../src/core/event_system.h"
#include "../utils/test_utils.h"
#include <thread>
#include <atomic>
#include <chrono>

using namespace v3d::core;
using namespace v3d::test;

class TestEvent : public Event {
public:
    int value;
    std::string message;
    
    TestEvent(int v, const std::string& msg) : value(v), message(msg) {}
    
    std::string getType() const override { return "TestEvent"; }
    size_t getSize() const override { return sizeof(TestEvent); }
};

class AnotherEvent : public Event {
public:
    float data;
    
    AnotherEvent(float d) : data(d) {}
    
    std::string getType() const override { return "AnotherEvent"; }
    size_t getSize() const override { return sizeof(AnotherEvent); }
};

class EventSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventBus_ = &EventBus::getInstance();
    }

    void TearDown() override {
        eventBus_->clear();
    }

    EventBus* eventBus_;
};

TEST_F(EventSystemTest, EventBus_SubscribePublish) {
    std::atomic<int> receivedCount(0);
    int receivedValue = 0;
    std::string receivedMessage;
    
    auto handler = [&](const Event& e) {
        const TestEvent& event = static_cast<const TestEvent&>(e);
        receivedCount++;
        receivedValue = event.value;
        receivedMessage = event.message;
    };
    
    eventBus_->subscribe("test_channel", handler);
    
    TestEvent event(42, "Hello");
    eventBus_->publish("test_channel", event);
    
    EXPECT_EQ(receivedCount, 1);
    EXPECT_EQ(receivedValue, 42);
    EXPECT_EQ(receivedMessage, "Hello");
}

TEST_F(EventSystemTest, EventBus_MultipleSubscribers) {
    std::atomic<int> count1(0);
    std::atomic<int> count2(0);
    std::atomic<int> count3(0);
    
    eventBus_->subscribe("channel", [&](const Event&) { count1++; });
    eventBus_->subscribe("channel", [&](const Event&) { count2++; });
    eventBus_->subscribe("channel", [&](const Event&) { count3++; });
    
    TestEvent event(0, "");
    eventBus_->publish("channel", event);
    
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
    EXPECT_EQ(count3, 1);
}

TEST_F(EventSystemTest, EventBus_Unsubscribe) {
    std::atomic<int> count(0);
    
    eventBus_->subscribe("channel", [&](const Event&) { count++; });
    
    TestEvent event(0, "");
    eventBus_->publish("channel", event);
    
    EXPECT_EQ(count, 1);
    
    eventBus_->unsubscribe("channel");
    
    eventBus_->publish("channel", event);
    
    EXPECT_EQ(count, 1);
}

TEST_F(EventSystemTest, EventBus_MultipleChannels) {
    std::atomic<int> channel1Count(0);
    std::atomic<int> channel2Count(0);
    
    eventBus_->subscribe("channel1", [&](const Event&) { channel1Count++; });
    eventBus_->subscribe("channel2", [&](const Event&) { channel2Count++; });
    
    TestEvent event(0, "");
    
    eventBus_->publish("channel1", event);
    eventBus_->publish("channel2", event);
    eventBus_->publish("channel1", event);
    
    EXPECT_EQ(channel1Count, 2);
    EXPECT_EQ(channel2Count, 1);
}

TEST_F(EventSystemTest, EventBus_AsyncPublish) {
    std::atomic<int> receivedCount(0);
    
    eventBus_->subscribe("async_channel", [&](const Event&) { 
        receivedCount++; 
    });
    
    TestEvent event(0, "");
    eventBus_->publishAsync("async_channel", std::make_unique<TestEvent>(0, ""));
    eventBus_->publishAsync("async_channel", std::make_unique<TestEvent>(0, ""));
    eventBus_->publishAsync("async_channel", std::make_unique<TestEvent>(0, ""));
    
    eventBus_->processEvents();
    
    EXPECT_EQ(receivedCount, 3);
}

TEST_F(EventSystemTest, EventBus_Clear) {
    std::atomic<int> count(0);
    
    eventBus_->subscribe("channel", [&](const Event&) { count++; });
    
    TestEvent event(0, "");
    eventBus_->publish("channel", event);
    
    EXPECT_EQ(count, 1);
    
    eventBus_->clear();
    
    eventBus_->publish("channel", event);
    
    EXPECT_EQ(count, 1);
}

TEST_F(EventSystemTest, EventBus_SubscriberCount) {
    EXPECT_EQ(eventBus_->getSubscriberCount("channel"), 0);
    
    eventBus_->subscribe("channel", [](const Event&) {});
    EXPECT_EQ(eventBus_->getSubscriberCount("channel"), 1);
    
    eventBus_->subscribe("channel", [](const Event&) {});
    EXPECT_EQ(eventBus_->getSubscriberCount("channel"), 2);
}

TEST_F(EventSystemTest, EventBus_MaxQueueSize) {
    eventBus_->setMaxQueueSize(100);
    EXPECT_EQ(eventBus_->getMaxQueueSize(), 100);
    
    eventBus_->setMaxQueueSize(1000);
    EXPECT_EQ(eventBus_->getMaxQueueSize(), 1000);
}

TEST_F(EventSystemTest, EventBus_PendingEventCount) {
    EXPECT_EQ(eventBus_->getPendingEventCount(), 0);
    
    eventBus_->publishAsync("channel", std::make_unique<TestEvent>(0, ""));
    eventBus_->publishAsync("channel", std::make_unique<TestEvent>(0, ""));
    
    EXPECT_EQ(eventBus_->getPendingEventCount(), 2);
    
    eventBus_->processEvents();
    
    EXPECT_EQ(eventBus_->getPendingEventCount(), 0);
}

TEST_F(EventSystemTest, EventDispatcher_TypeSafety) {
    EventDispatcher& dispatcher = EventDispatcher::getInstance();
    
    std::atomic<int> testEventCount(0);
    
    dispatcher.addListener("TestEvent", [&](const Event&) { testEventCount++; });
    
    TestEvent event(42, "Test");
    
    dispatcher.dispatch(static_cast<const Event&>(event));
    dispatcher.dispatch(static_cast<const Event&>(event));
    
    EXPECT_EQ(testEventCount, 2);
}

TEST_F(EventSystemTest, EventDispatcher_RemoveListener) {
    EventDispatcher& dispatcher = EventDispatcher::getInstance();
    
    std::atomic<int> count(0);
    
    dispatcher.addListener("TestEvent2", [&](const Event&) { count++; });
    
    TestEvent event(0, "");
    dispatcher.dispatch(static_cast<const Event&>(event));
    
    EXPECT_EQ(count, 1);
    
    dispatcher.removeAllListeners("TestEvent2");
    
    dispatcher.dispatch(static_cast<const Event&>(event));
    
    EXPECT_EQ(count, 1);
}

TEST_F(EventSystemTest, EventDispatcher_Clear) {
    EventDispatcher& dispatcher = EventDispatcher::getInstance();
    
    std::atomic<int> count(0);
    
    dispatcher.addListener("TestEvent3", [&](const Event&) { count++; });
    
    TestEvent event(0, "");
    dispatcher.dispatch(static_cast<const Event&>(event));
    
    EXPECT_EQ(count, 1);
    
    dispatcher.clearListeners();
    
    dispatcher.dispatch(static_cast<const Event&>(event));
    
    EXPECT_EQ(count, 1);
}

TEST_F(EventSystemTest, Integration_EventBusAndDispatcher) {
    std::atomic<int> busCount(0);
    std::atomic<int> dispatcherCount(0);
    
    eventBus_->subscribe("channel", [&](const Event&) { busCount++; });
    
    EventDispatcher& dispatcher = EventDispatcher::getInstance();
    dispatcher.addListener("TestEvent4", [&](const Event&) { dispatcherCount++; });
    
    TestEvent event(42, "Test");
    
    eventBus_->publish("channel", event);
    dispatcher.dispatch(static_cast<const Event&>(event));
    
    EXPECT_EQ(busCount, 1);
    EXPECT_EQ(dispatcherCount, 1);
}

TEST_F(EventSystemTest, StressTest_ManyEvents) {
    const int numEvents = 10000;
    std::atomic<int> receivedCount(0);
    
    eventBus_->subscribe("stress_channel", [&](const Event&) { receivedCount++; });
    
    for (int i = 0; i < numEvents; ++i) {
        TestEvent event(i, "Event " + std::to_string(i));
        eventBus_->publish("stress_channel", event);
    }
    
    EXPECT_EQ(receivedCount, numEvents);
}

TEST_F(EventSystemTest, StressTest_ManySubscribers) {
    const int numSubscribers = 100;
    std::atomic<int> totalReceived(0);
    
    for (int i = 0; i < numSubscribers; ++i) {
        eventBus_->subscribe("many_subscribers", [&totalReceived](const Event&) { 
            totalReceived++;
        });
    }
    
    TestEvent event(0, "");
    eventBus_->publish("many_subscribers", event);
    
    EXPECT_EQ(totalReceived, numSubscribers);
}

TEST_F(EventSystemTest, StressTest_ThreadSafety) {
    const int numThreads = 10;
    const int numEventsPerThread = 100;
    std::atomic<int> receivedCount(0);
    
    eventBus_->subscribe("thread_safety", [&](const Event&) { receivedCount++; });
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, numEventsPerThread]() {
            for (int j = 0; j < numEventsPerThread; ++j) {
                TestEvent event(j, "Event");
                eventBus_->publish("thread_safety", event);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(receivedCount, numThreads * numEventsPerThread);
}
