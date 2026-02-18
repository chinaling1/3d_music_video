/**
 * @file event_system.h
 * @brief 事件系统 - 提供类型安全的事件分发机制
 * 
 * 该模块实现了完整的事件系统，包括：
 * - EventBus: 发布/订阅模式的事件总线
 * - EventDispatcher: 类型安全的事件分发器
 * - TypedEvent: 泛型事件包装器
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <typeindex>
#include <queue>
#include <condition_variable>

namespace v3d {
namespace core {

/**
 * @class Event
 * @brief 事件基类
 * 
 * 所有事件都应继承此类。
 * 提供事件类型和大小的基本信息。
 */
class Event {
public:
    virtual ~Event() = default;
    
    /**
     * @brief 获取事件类型名称
     * @return 类型名称字符串
     */
    virtual std::string getType() const = 0;
    
    /**
     * @brief 获取事件大小
     * @return 事件数据大小（字节）
     */
    virtual size_t getSize() const = 0;
};

/// @brief 事件回调函数类型
using EventCallback = std::function<void(const Event&)>;

/**
 * @class EventBus
 * @brief 事件总线 - 实现发布/订阅模式
 * 
 * 单例模式，提供全局的事件分发机制。
 * 支持多频道订阅和异步事件处理。
 * 
 * @example
 * @code
 * auto& bus = EventBus::getInstance();
 * 
 * // 订阅事件
 * bus.subscribe("ui_events", [](const Event& e) {
 *     // 处理事件
 * });
 * 
 * // 发布事件
 * MyEvent event;
 * bus.publish("ui_events", event);
 * 
 * // 异步发布
 * bus.publishAsync("ui_events", std::make_unique<MyEvent>());
 * bus.processEvents();
 * @endcode
 */
class EventBus {
public:
    /**
     * @brief 获取单例实例
     * @return EventBus的单例引用
     */
    static EventBus& getInstance();
    
    /**
     * @brief 订阅频道（模板版本）
     * @tparam T 事件类型
     * @param channel 频道名称
     * @param callback 回调函数
     */
    template<typename T>
    void subscribe(const std::string& channel, EventCallback callback);
    
    /**
     * @brief 订阅频道
     * @param channel 频道名称
     * @param callback 回调函数
     */
    void subscribe(const std::string& channel, EventCallback callback);
    
    /**
     * @brief 取消订阅频道（模板版本）
     * @tparam T 事件类型
     * @param channel 频道名称
     */
    template<typename T>
    void unsubscribe(const std::string& channel);
    
    /**
     * @brief 取消订阅频道
     * @param channel 频道名称
     * 
     * 移除该频道的所有订阅者。
     */
    void unsubscribe(const std::string& channel);
    
    /**
     * @brief 同步发布事件
     * @param channel 频道名称
     * @param event 事件对象
     * 
     * 立即调用所有订阅者的回调函数。
     */
    void publish(const std::string& channel, const Event& event);
    
    /**
     * @brief 异步发布事件
     * @param channel 频道名称
     * @param event 事件对象（所有权转移）
     * 
     * 将事件加入队列，稍后通过processEvents()处理。
     */
    void publishAsync(const std::string& channel, std::unique_ptr<Event> event);
    
    /**
     * @brief 处理所有待处理事件
     * 
     * 处理所有通过publishAsync()发布的事件。
     */
    void processEvents();
    
    /**
     * @brief 清除所有订阅和待处理事件
     */
    void clear();
    
    /**
     * @brief 获取频道订阅者数量
     * @param channel 频道名称
     * @return 订阅者数量
     */
    size_t getSubscriberCount(const std::string& channel) const;
    
    /**
     * @brief 获取待处理事件数量
     * @return 待处理事件数量
     */
    size_t getPendingEventCount() const;
    
    /**
     * @brief 设置最大队列大小
     * @param maxSize 最大事件数
     */
    void setMaxQueueSize(size_t maxSize);
    
    /**
     * @brief 获取最大队列大小
     * @return 最大事件数
     */
    size_t getMaxQueueSize() const;

private:
    EventBus();
    ~EventBus();
    
    /**
     * @struct Subscription
     * @brief 订阅信息结构
     */
    struct Subscription {
        std::string channel;    ///< 频道名称
        EventCallback callback; ///< 回调函数
        size_t id;              ///< 订阅ID
    };

    std::unordered_map<std::string, std::vector<Subscription>> subscribers_; ///< 订阅者映射
    mutable std::mutex subscribersMutex_;                                    ///< 订阅者互斥锁
    
    /**
     * @struct QueuedEvent
     * @brief 队列事件结构
     */
    struct QueuedEvent {
        std::string channel;           ///< 频道名称
        std::unique_ptr<Event> event;  ///< 事件对象
    };

    std::queue<QueuedEvent> eventQueue_;          ///< 事件队列
    mutable std::mutex queueMutex_;               ///< 队列互斥锁
    std::condition_variable queueCondition_;      ///< 条件变量
    std::atomic<size_t> maxQueueSize_;            ///< 最大队列大小
    std::atomic<size_t> nextSubscriptionId_;      ///< 下一个订阅ID
};

/**
 * @class TypedEvent
 * @brief 泛型事件包装器
 * 
 * 将任意类型包装为Event对象。
 * 
 * @tparam T 数据类型
 * 
 * @example
 * @code
 * struct PlayerData {
 *     int health;
 *     int score;
 * };
 * 
 * TypedEvent<PlayerData> event(PlayerData{100, 500});
 * bus.publish("player_updates", event);
 * @endcode
 */
template<typename T>
class TypedEvent : public Event {
public:
    /**
     * @brief 构造类型化事件
     * @param data 事件数据
     */
    explicit TypedEvent(const T& data) : data_(data) {}
    
    std::string getType() const override {
        return typeid(T).name();
    }
    
    size_t getSize() const override {
        return sizeof(T);
    }
    
    /**
     * @brief 获取事件数据（只读）
     * @return 数据的常量引用
     */
    const T& getData() const {
        return data_;
    }
    
    /**
     * @brief 获取事件数据（可修改）
     * @return 数据的引用
     */
    T& getData() {
        return data_;
    }

private:
    T data_;  ///< 事件数据
};

/**
 * @class EventDispatcher
 * @brief 事件分发器 - 类型安全的事件分发
 * 
 * 单例模式，提供按事件类型分发的能力。
 * 与EventBus不同，EventDispatcher按类型而非频道分发事件。
 * 
 * @example
 * @code
 * auto& dispatcher = EventDispatcher::getInstance();
 * 
 * // 添加监听器
 * dispatcher.addListener("MyEvent", [](const Event& e) {
 *     // 处理事件
 * });
 * 
 * // 分发事件
 * MyEvent event;
 * dispatcher.dispatch(event);
 * @endcode
 */
class EventDispatcher {
public:
    /**
     * @brief 获取单例实例
     * @return EventDispatcher的单例引用
     */
    static EventDispatcher& getInstance();
    
    /**
     * @brief 分发事件
     * @param event 事件对象
     */
    void dispatch(const Event& event);
    
    /**
     * @brief 分发类型化事件
     * @tparam T 数据类型
     * @param data 事件数据
     * 
     * 自动包装为TypedEvent后分发。
     */
    template<typename T>
    void dispatch(const T& data);
    
    /**
     * @brief 添加事件监听器
     * @param eventType 事件类型名称
     * @param callback 回调函数
     */
    void addListener(const std::string& eventType, EventCallback callback);
    
    /**
     * @brief 移除指定类型的所有监听器
     * @param eventType 事件类型名称
     */
    void removeAllListeners(const std::string& eventType);
    
    /**
     * @brief 清除所有监听器
     */
    void clearListeners();

private:
    EventDispatcher();
    ~EventDispatcher();

    std::unordered_map<std::string, std::vector<EventCallback>> listeners_; ///< 监听器映射
    mutable std::mutex listenersMutex_;                                     ///< 监听器互斥锁
};

template<typename T>
void EventBus::subscribe(const std::string& channel, EventCallback callback) {
    subscribe(channel, callback);
}

template<typename T>
void EventBus::unsubscribe(const std::string& channel) {
    unsubscribe(channel);
}

template<typename T>
void EventDispatcher::dispatch(const T& data) {
    TypedEvent<T> event(data);
    dispatch(event);
}

}
}
