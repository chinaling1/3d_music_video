#include "event_system.h"

namespace v3d {
namespace core {

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

EventBus::EventBus()
    : maxQueueSize_(10000)
    , nextSubscriptionId_(0) {
}

EventBus::~EventBus() {
    clear();
}

void EventBus::subscribe(const std::string& channel, EventCallback callback) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    Subscription subscription;
    subscription.channel = channel;
    subscription.callback = callback;
    subscription.id = nextSubscriptionId_++;
    subscribers_[channel].push_back(subscription);
}

void EventBus::unsubscribe(const std::string& channel) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    subscribers_.erase(channel);
}

void EventBus::publish(const std::string& channel, const Event& event) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto it = subscribers_.find(channel);
    if (it != subscribers_.end()) {
        for (const auto& subscription : it->second) {
            subscription.callback(event);
        }
    }
}

void EventBus::publishAsync(const std::string& channel, std::unique_ptr<Event> event) {
    std::lock_guard<std::mutex> lock(queueMutex_);

    if (eventQueue_.size() >= maxQueueSize_) {
        eventQueue_.pop();
    }

    QueuedEvent queuedEvent;
    queuedEvent.channel = channel;
    queuedEvent.event = std::move(event);

    eventQueue_.push(std::move(queuedEvent));
    queueCondition_.notify_one();
}

void EventBus::processEvents() {
    std::unique_lock<std::mutex> lock(queueMutex_);

    while (!eventQueue_.empty()) {
        QueuedEvent queuedEvent = std::move(eventQueue_.front());
        eventQueue_.pop();

        lock.unlock();

        publish(queuedEvent.channel, *queuedEvent.event);

        lock.lock();
    }
}

void EventBus::clear() {
    std::lock(subscribersMutex_, queueMutex_);
    std::lock_guard<std::mutex> subLock(subscribersMutex_, std::adopt_lock);
    std::lock_guard<std::mutex> queueLock(queueMutex_, std::adopt_lock);
    
    subscribers_.clear();
    while (!eventQueue_.empty()) {
        eventQueue_.pop();
    }
}

size_t EventBus::getSubscriberCount(const std::string& channel) const {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto it = subscribers_.find(channel);
    if (it != subscribers_.end()) {
        return it->second.size();
    }
    return 0;
}

size_t EventBus::getPendingEventCount() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return eventQueue_.size();
}

void EventBus::setMaxQueueSize(size_t maxSize) {
    maxQueueSize_ = maxSize;
}

size_t EventBus::getMaxQueueSize() const {
    return maxQueueSize_;
}

EventDispatcher& EventDispatcher::getInstance() {
    static EventDispatcher instance;
    return instance;
}

EventDispatcher::EventDispatcher() {
}

EventDispatcher::~EventDispatcher() {
}

void EventDispatcher::dispatch(const Event& event) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    auto it = listeners_.find(event.getType());
    if (it != listeners_.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

void EventDispatcher::addListener(const std::string& eventType, EventCallback callback) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_[eventType].push_back(callback);
}

void EventDispatcher::removeAllListeners(const std::string& eventType) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.erase(eventType);
}

void EventDispatcher::clearListeners() {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.clear();
}

}
}