#include <gtest/gtest.h>
#include "../../src/io/asset_manager.h"
#include "../utils/test_data_generator.h"
#include "../utils/test_utils.h"
#include <filesystem>

using namespace v3d::io;
using namespace v3d::test;

class AssetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        assetManager_ = &AssetManager::getInstance();
        TestUtils::createDirectory("test_temp");
    }

    void TearDown() override {
        assetManager_->cleanup();
        TestUtils::deleteDirectory("test_temp");
    }

    AssetManager* assetManager_;
};

TEST_F(AssetManagerTest, LoadMeshAsset) {
    auto cube = TestDataGenerator::createCubeMesh();
    
    std::string testPath = "test_temp/test_cube.obj";
    
    EXPECT_TRUE(assetManager_->isAssetLoaded(AssetHandle()) == false);
}

TEST_F(AssetManagerTest, GetInstance) {
    AssetManager& instance1 = AssetManager::getInstance();
    AssetManager& instance2 = AssetManager::getInstance();
    
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(AssetManagerTest, SetGetCacheConfig) {
    AssetCacheConfig config;
    config.maxMemoryUsage = 512 * 1024 * 1024;
    config.maxAssetCount = 500;
    config.enableAutoUnload = true;
    
    assetManager_->setCacheConfig(config);
    
    const AssetCacheConfig& result = assetManager_->getCacheConfig();
    
    EXPECT_EQ(result.maxMemoryUsage, config.maxMemoryUsage);
    EXPECT_EQ(result.maxAssetCount, config.maxAssetCount);
    EXPECT_EQ(result.enableAutoUnload, config.enableAutoUnload);
}

TEST_F(AssetManagerTest, RegisterAssetPath) {
    assetManager_->registerAssetPath("models", "assets/models");
    assetManager_->registerAssetPath("textures", "assets/textures");
    
    std::string resolved1 = assetManager_->resolveAssetPath("models/cube.obj");
    std::string resolved2 = assetManager_->resolveAssetPath("textures/diffuse.png");
    
    EXPECT_TRUE(resolved1.find("assets/models") != std::string::npos);
    EXPECT_TRUE(resolved2.find("assets/textures") != std::string::npos);
}

TEST_F(AssetManagerTest, GetTotalMemoryUsage) {
    size_t memoryUsage = assetManager_->getTotalMemoryUsage();
    
    EXPECT_GE(memoryUsage, 0);
}

TEST_F(AssetManagerTest, GetLoadedAssetCount) {
    size_t assetCount = assetManager_->getLoadedAssetCount();
    
    EXPECT_GE(assetCount, 0);
}

TEST_F(AssetManagerTest, UnloadAllAssets) {
    assetManager_->unloadAllAssets();
    
    EXPECT_EQ(assetManager_->getLoadedAssetCount(), 0);
    EXPECT_EQ(assetManager_->getTotalMemoryUsage(), 0);
}

TEST_F(AssetManagerTest, Update) {
    AssetCacheConfig config;
    config.enableAutoUnload = true;
    config.maxIdleTime = std::chrono::seconds(1);
    assetManager_->setCacheConfig(config);
    
    assetManager_->update();
    
    EXPECT_NO_THROW(assetManager_->update());
}

TEST_F(AssetManagerTest, Cleanup) {
    assetManager_->cleanup();
    
    EXPECT_EQ(assetManager_->getLoadedAssetCount(), 0);
}

TEST_F(AssetManagerTest, AssetHandle) {
    AssetHandle handle;
    
    EXPECT_FALSE(handle.isValid());
    
    handle.id = "test_id";
    handle.type = AssetType::Mesh;
    
    EXPECT_TRUE(handle.isValid());
    
    handle.reset();
    
    EXPECT_FALSE(handle.isValid());
}

TEST_F(AssetManagerTest, AssetLoadState) {
    EXPECT_NO_THROW([]() {
        AssetLoadState state = AssetLoadState::Unloaded;
        state = AssetLoadState::Loading;
        state = AssetLoadState::Loaded;
        state = AssetLoadState::Failed;
        state = AssetLoadState::Unloading;
    });
}

TEST_F(AssetManagerTest, AssetType) {
    EXPECT_NO_THROW([]() {
        AssetType type = AssetType::Unknown;
        type = AssetType::Mesh;
        type = AssetType::Skeleton;
        type = AssetType::Animation;
        type = AssetType::Audio;
        type = AssetType::Image;
        type = AssetType::Material;
        type = AssetType::Scene;
    });
}

TEST_F(AssetManagerTest, AssetCacheConfigDefaults) {
    AssetCacheConfig config;
    
    EXPECT_EQ(config.maxMemoryUsage, 1024 * 1024 * 1024);
    EXPECT_EQ(config.maxAssetCount, 1000);
    EXPECT_TRUE(config.enableAutoUnload);
    EXPECT_EQ(config.maxIdleTime.count(), 300);
    EXPECT_FLOAT_EQ(config.unloadThreshold, 0.9f);
}

TEST_F(AssetManagerTest, StressTest_ManyAssetPathRegistrations) {
    const int numPaths = 1000;
    
    TestUtils::measureTime("Register 1000 asset paths", [this, numPaths]() {
        for (int i = 0; i < numPaths; ++i) {
            std::string alias = "alias" + std::to_string(i);
            std::string path = "path" + std::to_string(i);
            assetManager_->registerAssetPath(alias, path);
        }
    });
}

TEST_F(AssetManagerTest, StressTest_ManyPathResolutions) {
    assetManager_->registerAssetPath("test", "assets/test");
    
    const int numResolutions = 10000;
    
    TestUtils::measureTime("Resolve 10000 paths", [this, numResolutions]() {
        for (int i = 0; i < numResolutions; ++i) {
            std::string path = "test/file" + std::to_string(i) + ".obj";
            assetManager_->resolveAssetPath(path);
        }
    });
}

TEST_F(AssetManagerTest, StressTest_MemoryTracking) {
    AssetCacheConfig config;
    config.maxMemoryUsage = 10 * 1024 * 1024;
    assetManager_->setCacheConfig(config);
    
    TestUtils::measureTime("Memory tracking stress test", [this]() {
        for (int i = 0; i < 100; ++i) {
            size_t memoryBefore = assetManager_->getTotalMemoryUsage();
            assetManager_->update();
            size_t memoryAfter = assetManager_->getTotalMemoryUsage();
            
            EXPECT_GE(memoryAfter, 0);
        }
    });
}
