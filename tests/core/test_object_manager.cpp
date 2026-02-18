#include <gtest/gtest.h>
#include "../../src/core/object_manager.h"
#include "../utils/test_utils.h"
#include <thread>

using namespace v3d::core;
using namespace v3d::test;

class TestObject : public Object {
public:
    TestObject() : value_(0) {
        type_ = "TestObject";
        category_ = "Test";
    }
    
    explicit TestObject(int value) : value_(value) {
        type_ = "TestObject";
        category_ = "Test";
    }
    
    int getValue() const { return value_; }
    void setValue(int value) { value_ = value; }
    
private:
    int value_;
};

class AnotherTestObject : public Object {
public:
    AnotherTestObject() {
        type_ = "AnotherTestObject";
        category_ = "Test";
    }
};

class ObjectManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        objectManager_ = &ObjectManager::getInstance();
        objectManager_->destroyAllObjects();
    }

    void TearDown() override {
        objectManager_->destroyAllObjects();
    }

    ObjectManager* objectManager_;
};

TEST_F(ObjectManagerTest, CreateObject) {
    TestObject* obj = objectManager_->createObject<TestObject>();
    
    ASSERT_NE(obj, nullptr);
    EXPECT_FALSE(obj->getId().empty());
    EXPECT_EQ(obj->getType(), "TestObject");
}

TEST_F(ObjectManagerTest, CreateObjectWithArgs) {
    TestObject* obj = objectManager_->createObject<TestObject>(42);
    
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->getValue(), 42);
}

TEST_F(ObjectManagerTest, DestroyObject) {
    TestObject* obj = objectManager_->createObject<TestObject>();
    std::string id = obj->getId();
    
    EXPECT_NE(objectManager_->findObjectById(id), nullptr);
    
    objectManager_->destroyObject(id);
    
    EXPECT_EQ(objectManager_->findObjectById(id), nullptr);
}

TEST_F(ObjectManagerTest, DestroyAllObjects) {
    objectManager_->createObject<TestObject>();
    objectManager_->createObject<TestObject>();
    objectManager_->createObject<TestObject>();
    
    EXPECT_EQ(objectManager_->getObjectCount(), 3);
    
    objectManager_->destroyAllObjects();
    
    EXPECT_EQ(objectManager_->getObjectCount(), 0);
}

TEST_F(ObjectManagerTest, GetObjectById) {
    TestObject* obj = objectManager_->createObject<TestObject>(100);
    std::string id = obj->getId();
    
    TestObject* retrieved = objectManager_->getObject<TestObject>(id);
    
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getValue(), 100);
}

TEST_F(ObjectManagerTest, GetObjectsByType) {
    objectManager_->createObject<TestObject>();
    objectManager_->createObject<TestObject>();
    objectManager_->createObject<AnotherTestObject>();
    
    auto testObjects = objectManager_->getObjectsByType<TestObject>();
    auto anotherObjects = objectManager_->getObjectsByType<AnotherTestObject>();
    
    EXPECT_EQ(testObjects.size(), 2);
    EXPECT_EQ(anotherObjects.size(), 1);
}

TEST_F(ObjectManagerTest, FindObjectByName) {
    TestObject* obj = objectManager_->createObject<TestObject>();
    obj->setName("TestObject1");
    
    Object* found = objectManager_->findObject("TestObject1");
    
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getId(), obj->getId());
}

TEST_F(ObjectManagerTest, FindObjectById) {
    TestObject* obj = objectManager_->createObject<TestObject>();
    std::string id = obj->getId();
    
    Object* found = objectManager_->findObjectById(id);
    
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getId(), id);
}

TEST_F(ObjectManagerTest, GetObjectsByCategory) {
    objectManager_->createObject<TestObject>();
    objectManager_->createObject<AnotherTestObject>();
    
    auto testObjects = objectManager_->getObjectsByCategory("Test");
    
    EXPECT_EQ(testObjects.size(), 2);
}

TEST_F(ObjectManagerTest, ObjectActiveState) {
    TestObject* obj = objectManager_->createObject<TestObject>();
    
    EXPECT_TRUE(obj->isActive());
    
    obj->setActive(false);
    EXPECT_FALSE(obj->isActive());
    
    obj->setActive(true);
    EXPECT_TRUE(obj->isActive());
}

TEST_F(ObjectManagerTest, GetActiveObjects) {
    TestObject* obj1 = objectManager_->createObject<TestObject>();
    TestObject* obj2 = objectManager_->createObject<TestObject>();
    
    obj2->setActive(false);
    
    auto activeObjects = objectManager_->getActiveObjects();
    
    EXPECT_EQ(activeObjects.size(), 1);
}

TEST_F(ObjectManagerTest, ObjectCount) {
    EXPECT_EQ(objectManager_->getObjectCount(), 0);
    
    objectManager_->createObject<TestObject>();
    EXPECT_EQ(objectManager_->getObjectCount(), 1);
    
    objectManager_->createObject<TestObject>();
    EXPECT_EQ(objectManager_->getObjectCount(), 2);
    
    objectManager_->destroyAllObjects();
    EXPECT_EQ(objectManager_->getObjectCount(), 0);
}

TEST_F(ObjectManagerTest, MaxObjects) {
    objectManager_->setMaxObjects(5);
    EXPECT_EQ(objectManager_->getMaxObjects(), 5);
    
    for (int i = 0; i < 5; ++i) {
        auto obj = objectManager_->createObject<TestObject>();
        ASSERT_NE(obj, nullptr);
    }
    
    auto obj = objectManager_->createObject<TestObject>();
    EXPECT_EQ(obj, nullptr);
    
    objectManager_->setMaxObjects(100);
}

TEST_F(ObjectManagerTest, ObjectComponents) {
    TestObject* obj = objectManager_->createObject<TestObject>();
    
    int* comp = obj->addComponent<int>();
    *comp = 42;
    
    int* retrieved = obj->getComponent<int>();
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(*retrieved, 42);
    
    obj->removeComponent<int>();
    
    int* afterRemove = obj->getComponent<int>();
    EXPECT_EQ(afterRemove, nullptr);
}

TEST_F(ObjectManagerTest, UpdateAllObjects) {
    class UpdatableObject : public Object {
    public:
        UpdatableObject() : updateCount(0) {
            type_ = "UpdatableObject";
        }
        
        void update(float deltaTime) override {
            updateCount++;
        }
        
        int updateCount;
    };
    
    auto obj1 = objectManager_->createObject<UpdatableObject>();
    auto obj2 = objectManager_->createObject<UpdatableObject>();
    
    objectManager_->updateAllObjects(0.016f);
    
    EXPECT_EQ(obj1->updateCount, 1);
    EXPECT_EQ(obj2->updateCount, 1);
}

TEST_F(ObjectManagerTest, StressTest_ManyObjects) {
    const int numObjects = 1000;
    
    for (int i = 0; i < numObjects; ++i) {
        auto obj = objectManager_->createObject<TestObject>(i);
        ASSERT_NE(obj, nullptr);
    }
    
    EXPECT_EQ(objectManager_->getObjectCount(), numObjects);
}

TEST_F(ObjectManagerTest, StressTest_ThreadSafety) {
    const int numThreads = 10;
    const int numObjectsPerThread = 100;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, numObjectsPerThread]() {
            for (int j = 0; j < numObjectsPerThread; ++j) {
                auto obj = objectManager_->createObject<TestObject>();
                if (obj) {
                    obj->setValue(j);
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(objectManager_->getObjectCount(), numThreads * numObjectsPerThread);
}
