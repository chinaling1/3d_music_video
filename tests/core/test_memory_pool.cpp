#include <gtest/gtest.h>
#include "../../src/core/memory_pool.h"
#include "../utils/test_utils.h"
#include <vector>
#include <thread>
#include <atomic>

using namespace v3d::core;
using namespace v3d::test;

class MemoryPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(MemoryPoolTest, MemoryPool_AllocateDeallocate) {
    MemoryPool<int, 1024> pool;
    
    EXPECT_EQ(pool.getCapacity(), 0);
    EXPECT_EQ(pool.getUsedCount(), 0);
    
    int* ptr1 = pool.allocate();
    int* ptr2 = pool.allocate();
    int* ptr3 = pool.allocate();
    
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);
    
    EXPECT_EQ(pool.getUsedCount(), 3);
    EXPECT_GE(pool.getCapacity(), 3);
    
    pool.deallocate(ptr1);
    pool.deallocate(ptr2);
    pool.deallocate(ptr3);
    
    EXPECT_EQ(pool.getUsedCount(), 0);
}

TEST_F(MemoryPoolTest, MemoryPool_Clear) {
    MemoryPool<int, 1024> pool;
    
    for (int i = 0; i < 100; ++i) {
        pool.allocate();
    }
    
    EXPECT_GT(pool.getUsedCount(), 0);
    
    pool.clear();
    
    EXPECT_EQ(pool.getUsedCount(), 0);
}

TEST_F(MemoryPoolTest, MemoryPool_ReuseMemory) {
    MemoryPool<int, 1024> pool;
    
    int* ptr1 = pool.allocate();
    *ptr1 = 42;
    
    pool.deallocate(ptr1);
    
    int* ptr2 = pool.allocate();
    
    EXPECT_EQ(ptr1, ptr2);
    EXPECT_EQ(*ptr2, 42);
}

TEST_F(MemoryPoolTest, MemoryPool_LargeAllocation) {
    MemoryPool<int, 1024> pool;
    
    std::vector<int*> pointers;
    
    for (int i = 0; i < 5000; ++i) {
        int* ptr = pool.allocate();
        ASSERT_NE(ptr, nullptr);
        *ptr = i;
        pointers.push_back(ptr);
    }
    
    EXPECT_EQ(pool.getUsedCount(), 5000);
    
    for (size_t i = 0; i < pointers.size(); ++i) {
        EXPECT_EQ(*pointers[i], static_cast<int>(i));
    }
}

TEST_F(MemoryPoolTest, MemoryPool_ThreadSafety) {
    MemoryPool<int, 1024> pool;
    std::atomic<int> successCount(0);
    
    auto allocateFunc = [&pool, &successCount]() {
        for (int i = 0; i < 100; ++i) {
            int* ptr = pool.allocate();
            if (ptr != nullptr) {
                *ptr = i;
                successCount++;
                pool.deallocate(ptr);
            }
        }
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(allocateFunc);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successCount, 1000);
    EXPECT_EQ(pool.getUsedCount(), 0);
}

TEST_F(MemoryPoolTest, ObjectPool_AcquireRelease) {
    ObjectPool<std::string> pool(10);
    
    EXPECT_EQ(pool.getCapacity(), 10);
    EXPECT_EQ(pool.getUsedCount(), 0);
    
    std::string* str1 = pool.acquire("Hello");
    std::string* str2 = pool.acquire("World");
    
    EXPECT_NE(str1, nullptr);
    EXPECT_NE(str2, nullptr);
    EXPECT_EQ(*str1, "Hello");
    EXPECT_EQ(*str2, "World");
    
    EXPECT_EQ(pool.getUsedCount(), 2);
    
    pool.release(str1);
    pool.release(str2);
    
    EXPECT_EQ(pool.getUsedCount(), 0);
}

TEST_F(MemoryPoolTest, ObjectPool_ReuseObjects) {
    ObjectPool<std::string> pool(10);
    
    std::string* str1 = pool.acquire("Test");
    std::string* str1Address = str1;
    
    pool.release(str1);
    
    std::string* str2 = pool.acquire("New");
    
    EXPECT_EQ(str1Address, str2);
}

TEST_F(MemoryPoolTest, ObjectPool_ExpandCapacity) {
    ObjectPool<int> pool(10);
    
    EXPECT_EQ(pool.getCapacity(), 10);
    
    std::vector<int*> pointers;
    for (int i = 0; i < 20; ++i) {
        int* ptr = pool.acquire(i);
        ASSERT_NE(ptr, nullptr);
        pointers.push_back(ptr);
    }
    
    EXPECT_GE(pool.getCapacity(), 20);
    EXPECT_EQ(pool.getUsedCount(), 20);
    
    for (size_t i = 0; i < pointers.size(); ++i) {
        EXPECT_EQ(*pointers[i], static_cast<int>(i));
    }
}

TEST_F(MemoryPoolTest, ObjectPool_Clear) {
    ObjectPool<int> pool(10);
    
    for (int i = 0; i < 10; ++i) {
        pool.acquire(i);
    }
    
    EXPECT_EQ(pool.getUsedCount(), 10);
    
    pool.clear();
    
    EXPECT_EQ(pool.getUsedCount(), 0);
    EXPECT_EQ(pool.getCapacity(), 0);
}

TEST_F(MemoryPoolTest, ObjectPool_Reserve) {
    ObjectPool<int> pool(10);
    
    pool.reserve(100);
    
    EXPECT_GE(pool.getCapacity(), 100);
}

TEST_F(MemoryPoolTest, MemoryArena_Allocate) {
    MemoryArena arena(1024);
    
    EXPECT_EQ(arena.getTotalMemory(), 1024);
    EXPECT_EQ(arena.getUsedMemory(), 0);
    
    void* ptr1 = arena.allocate(100);
    void* ptr2 = arena.allocate(200);
    void* ptr3 = arena.allocate(300);
    
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);
    
    EXPECT_EQ(arena.getUsedMemory(), 600);
}

TEST_F(MemoryPoolTest, MemoryArena_Reset) {
    MemoryArena arena(1024);
    
    arena.allocate(500);
    arena.allocate(300);
    
    EXPECT_GT(arena.getUsedMemory(), 0);
    
    arena.reset();
    
    EXPECT_EQ(arena.getUsedMemory(), 0);
}

TEST_F(MemoryPoolTest, MemoryArena_Alignment) {
    MemoryArena arena(1024);
    
    void* ptr1 = arena.allocate(10, 16);
    void* ptr2 = arena.allocate(10, 32);
    void* ptr3 = arena.allocate(10, 64);
    
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr1) % 16, 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr2) % 32, 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr3) % 64, 0);
}

TEST_F(MemoryPoolTest, MemoryArena_AutoExpand) {
    MemoryArena arena(100);
    
    void* ptr1 = arena.allocate(50);
    void* ptr2 = arena.allocate(50);
    void* ptr3 = arena.allocate(50);
    
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);
    
    EXPECT_GT(arena.getTotalMemory(), 100);
}

TEST_F(MemoryPoolTest, SmartPtr_Basic) {
    SmartPtr<int> ptr(new int(42));
    
    EXPECT_TRUE(ptr);
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(ptr.get(), ptr.operator->());
}

TEST_F(MemoryPoolTest, SmartPtr_Move) {
    SmartPtr<int> ptr1(new int(42));
    SmartPtr<int> ptr2(std::move(ptr1));
    
    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(*ptr2, 42);
}

TEST_F(MemoryPoolTest, SmartPtr_Reset) {
    SmartPtr<int> ptr(new int(42));
    
    EXPECT_TRUE(ptr);
    
    ptr.reset();
    
    EXPECT_FALSE(ptr);
}

TEST_F(MemoryPoolTest, SmartPtr_Release) {
    SmartPtr<int> ptr(new int(42));
    
    int* rawPtr = ptr.release();
    
    EXPECT_FALSE(ptr);
    EXPECT_NE(rawPtr, nullptr);
    EXPECT_EQ(*rawPtr, 42);
    
    delete rawPtr;
}

TEST_F(MemoryPoolTest, StressTest_MemoryPool) {
    MemoryPool<int, 1024> pool;
    
    TestUtils::measureTime("MemoryPool stress test", [&pool]() {
        std::vector<int*> pointers;
        
        for (int i = 0; i < 10000; ++i) {
            int* ptr = pool.allocate();
            if (ptr != nullptr) {
                *ptr = i;
                pointers.push_back(ptr);
            }
        }
        
        for (auto ptr : pointers) {
            pool.deallocate(ptr);
        }
    });
    
    EXPECT_EQ(pool.getUsedCount(), 0);
}

TEST_F(MemoryPoolTest, StressTest_ObjectPool) {
    ObjectPool<std::string> pool(100);
    
    TestUtils::measureTime("ObjectPool stress test", [&pool]() {
        std::vector<std::string*> pointers;
        
        for (int i = 0; i < 10000; ++i) {
            std::string* ptr = pool.acquire("Test " + std::to_string(i));
            if (ptr != nullptr) {
                pointers.push_back(ptr);
            }
        }
        
        for (auto ptr : pointers) {
            pool.release(ptr);
        }
    });
    
    EXPECT_EQ(pool.getUsedCount(), 0);
}

TEST_F(MemoryPoolTest, StressTest_MemoryArena) {
    MemoryArena arena(1024 * 1024);
    
    TestUtils::measureTime("MemoryArena stress test", [&arena]() {
        std::vector<void*> pointers;
        
        for (int i = 0; i < 10000; ++i) {
            void* ptr = arena.allocate(100);
            if (ptr != nullptr) {
                pointers.push_back(ptr);
            }
        }
        
        arena.reset();
    });
    
    EXPECT_EQ(arena.getUsedMemory(), 0);
}
