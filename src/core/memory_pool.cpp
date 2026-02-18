#include "memory_pool.h"
#include <stdexcept>

namespace v3d {
namespace core {

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool()
    : usedCount_(0)
    , capacity_(0) {
    allocateNewBlock();
}

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() {
}

template<typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (freeList_.empty()) {
        allocateNewBlock();
    }

    T* ptr = freeList_.top();
    freeList_.pop();
    usedCount_++;

    return ptr;
}

template<typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate(T* ptr) {
    if (!ptr) return;

    std::lock_guard<std::mutex> lock(mutex_);

    ptr->~T();
    freeList_.push(ptr);
    usedCount_--;
}

template<typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    blocks_.clear();
    while (!freeList_.empty()) {
        freeList_.pop();
    }
    usedCount_ = 0;
    capacity_ = 0;

    allocateNewBlock();
}

template<typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::getUsedCount() const {
    return usedCount_.load();
}

template<typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::getCapacity() const {
    return capacity_.load();
}

template<typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::getFreeCount() const {
    return capacity_ - usedCount_;
}

template<typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateNewBlock() {
    auto block = std::make_unique<Block>();
    T* data = reinterpret_cast<T*>(block->data);

    for (size_t i = 0; i < BlockSize; ++i) {
        freeList_.push(&data[i]);
    }

    blocks_.push_back(std::move(block));
    capacity_ += BlockSize;
}

template<typename T>
ObjectPool<T>::ObjectPool(size_t initialSize)
    : usedCount_(0) {
    expand(initialSize);
}

template<typename T>
ObjectPool<T>::~ObjectPool() {
}

template<typename T>
template<typename... Args>
T* ObjectPool<T>::acquire(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (freeList_.empty()) {
        expand(pool_.size());
    }

    T* obj = freeList_.top();
    freeList_.pop();

    new (obj) T(std::forward<Args>(args)...);
    usedCount_++;

    return obj;
}

template<typename T>
void ObjectPool<T>::release(T* obj) {
    if (!obj) return;

    std::lock_guard<std::mutex> lock(mutex_);

    obj->~T();
    freeList_.push(obj);
    usedCount_--;
}

template<typename T>
void ObjectPool<T>::reserve(size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (size > pool_.size()) {
        expand(size - pool_.size());
    }
}

template<typename T>
void ObjectPool<T>::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    pool_.clear();
    while (!freeList_.empty()) {
        freeList_.pop();
    }
    usedCount_ = 0;
}

template<typename T>
size_t ObjectPool<T>::getUsedCount() const {
    return usedCount_.load();
}

template<typename T>
size_t ObjectPool<T>::getCapacity() const {
    return pool_.size();
}

template<typename T>
void ObjectPool<T>::expand(size_t size) {
    for (size_t i = 0; i < size; ++i) {
        pool_.push_back(std::make_unique<T>());
        freeList_.push(pool_.back().get());
    }
}

MemoryArena::MemoryArena(size_t initialSize)
    : blockSize_(initialSize)
    , currentBlock_(nullptr) {
    auto block = std::make_unique<Block>();
    block->size = blockSize_;
    block->used = 0;
    block->data = std::make_unique<unsigned char[]>(blockSize_);
    block->next = nullptr;
    currentBlock_ = block.get();
    blocks_.push_back(std::move(block));
}

MemoryArena::~MemoryArena() {
}

void* MemoryArena::allocate(size_t size, size_t alignment) {
    size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    if (currentBlock_->used + alignedSize > currentBlock_->size) {
        auto newBlock = std::make_unique<Block>();
        newBlock->size = std::max(blockSize_, alignedSize * 2);
        newBlock->used = 0;
        newBlock->data = std::make_unique<unsigned char[]>(newBlock->size);
        newBlock->next = currentBlock_;
        
        blocks_.push_back(std::move(newBlock));
        currentBlock_ = blocks_.back().get();
    }

    void* ptr = &currentBlock_->data[currentBlock_->used];
    currentBlock_->used += alignedSize;

    return ptr;
}

void MemoryArena::reset() {
    for (auto& block : blocks_) {
        block->used = 0;
    }
    if (!blocks_.empty()) {
        currentBlock_ = blocks_[0].get();
    }
}

size_t MemoryArena::getUsedMemory() const {
    size_t total = 0;
    for (const auto& block : blocks_) {
        total += block->used;
    }
    return total;
}

size_t MemoryArena::getTotalMemory() const {
    size_t total = 0;
    for (const auto& block : blocks_) {
        total += block->size;
    }
    return total;
}

template<typename T>
SmartPtr<T>::SmartPtr(T* ptr)
    : ptr_(ptr) {
}

template<typename T>
SmartPtr<T>::~SmartPtr() {
    if (ptr_) {
        delete ptr_;
    }
}

template<typename T>
SmartPtr<T>::SmartPtr(SmartPtr&& other) noexcept
    : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
}

template<typename T>
SmartPtr<T>& SmartPtr<T>::operator=(SmartPtr&& other) noexcept {
    if (this != &other) {
        if (ptr_) {
            delete ptr_;
        }
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }
    return *this;
}

template<typename T>
T* SmartPtr<T>::get() const {
    return ptr_;
}

template<typename T>
T& SmartPtr<T>::operator*() const {
    return *ptr_;
}

template<typename T>
T* SmartPtr<T>::operator->() const {
    return ptr_;
}

template<typename T>
SmartPtr<T>::operator bool() const {
    return ptr_ != nullptr;
}

template<typename T>
void SmartPtr<T>::reset() {
    if (ptr_) {
        delete ptr_;
        ptr_ = nullptr;
    }
}

template<typename T>
T* SmartPtr<T>::release() {
    T* temp = ptr_;
    ptr_ = nullptr;
    return temp;
}

}
}