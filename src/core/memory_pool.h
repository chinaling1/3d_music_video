#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <mutex>
#include <atomic>
#include <cstddef>

namespace v3d {
namespace core {

template<typename T, size_t BlockSize = 1024>
class MemoryPool {
public:
    MemoryPool();
    ~MemoryPool();

    T* allocate();
    void deallocate(T* ptr);

    void clear();
    size_t getUsedCount() const;
    size_t getCapacity() const;
    size_t getFreeCount() const;

private:
    struct Block {
        alignas(T) unsigned char data[sizeof(T) * BlockSize];
        std::unique_ptr<Block> next;
    };

    std::unique_ptr<Block> firstBlock_;
    std::stack<T*> freeList_;
    std::vector<std::unique_ptr<Block>> blocks_;
    mutable std::mutex mutex_;
    std::atomic<size_t> usedCount_;
    std::atomic<size_t> capacity_;

    void allocateNewBlock();
};

template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initialSize = 100);
    ~ObjectPool();

    template<typename... Args>
    T* acquire(Args&&... args);

    void release(T* obj);

    void reserve(size_t size);
    void clear();

    size_t getUsedCount() const;
    size_t getCapacity() const;

private:
    std::vector<std::unique_ptr<T>> pool_;
    std::stack<T*> freeList_;
    mutable std::mutex mutex_;
    std::atomic<size_t> usedCount_;

    void expand(size_t size);
};

class MemoryArena {
public:
    explicit MemoryArena(size_t initialSize = 1024 * 1024);
    ~MemoryArena();

    void* allocate(size_t size, size_t alignment = 16);
    void reset();

    size_t getUsedMemory() const;
    size_t getTotalMemory() const;

private:
    struct Block {
        size_t size;
        size_t used;
        std::unique_ptr<unsigned char[]> data;
        Block* next = nullptr;
    };

    Block* currentBlock_;
    std::vector<std::unique_ptr<Block>> blocks_;
    size_t blockSize_;
};

template<typename T>
class SmartPtr {
public:
    explicit SmartPtr(T* ptr = nullptr);
    ~SmartPtr();

    SmartPtr(const SmartPtr& other) = delete;
    SmartPtr& operator=(const SmartPtr& other) = delete;

    SmartPtr(SmartPtr&& other) noexcept;
    SmartPtr& operator=(SmartPtr&& other) noexcept;

    T* get() const;
    T& operator*() const;
    T* operator->() const;

    explicit operator bool() const;

    void reset();
    T* release();

private:
    T* ptr_;
};

}
}