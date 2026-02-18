#pragma once

#include "file_loader.h"
#include "file_saver.h"
#include "format_converter.h"
#include "../modeling/mesh.h"
#include "../animation/skeleton.h"
#include "../animation/animation_clip.h"
#include "../audio/audio_buffer.h"
#include "../modeling/material.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>

namespace v3d {
namespace io {

enum class AssetType {
    Mesh,
    Skeleton,
    Animation,
    Audio,
    Image,
    Material,
    Scene,
    Unknown
};

enum class AssetLoadState {
    Unloaded,
    Loading,
    Loaded,
    Failed,
    Unloading
};

struct AssetMetadata {
    std::string id;
    std::string name;
    std::string path;
    AssetType type;
    size_t size;
    size_t memoryUsage;
    AssetLoadState state;
    std::string errorMessage;
    std::unordered_map<std::string, std::string> properties;
    std::chrono::system_clock::time_point lastAccessed;
    std::chrono::system_clock::time_point lastModified;
    int referenceCount;
};

struct AssetHandle {
    std::string id;
    AssetType type;
    bool isValid() const { return !id.empty(); }
    void reset() { id.clear(); type = AssetType::Unknown; }
};

class Asset {
public:
    Asset(const std::string& id, AssetType type);
    virtual ~Asset();

    const std::string& getId() const { return id_; }
    AssetType getType() const { return type_; }
    AssetLoadState getState() const { return state_; }
    void setState(AssetLoadState state) { state_ = state; }

    const AssetMetadata& getMetadata() const { return metadata_; }
    void setMetadata(const AssetMetadata& metadata) { metadata_ = metadata; }

    void incrementRefCount() { ++referenceCount_; }
    void decrementRefCount() { --referenceCount_; }
    int getRefCount() const { return referenceCount_; }

    void setLastAccessed();
    std::chrono::system_clock::time_point getLastAccessed() const { return lastAccessed_; }

    virtual size_t getMemoryUsage() const = 0;
    virtual void unload() = 0;

protected:
    std::string id_;
    AssetType type_;
    AssetLoadState state_;
    AssetMetadata metadata_;
    std::atomic<int> referenceCount_;
    std::chrono::system_clock::time_point lastAccessed_;
};

class MeshAsset : public Asset {
public:
    explicit MeshAsset(const std::string& id);
    ~MeshAsset() override;

    void setMesh(std::shared_ptr<modeling::Mesh> mesh);
    std::shared_ptr<modeling::Mesh> getMesh() const { return mesh_; }

    size_t getMemoryUsage() const override;
    void unload() override;

private:
    std::shared_ptr<modeling::Mesh> mesh_;
};

class SkeletonAsset : public Asset {
public:
    explicit SkeletonAsset(const std::string& id);
    ~SkeletonAsset() override;

    void setSkeleton(std::shared_ptr<animation::Skeleton> skeleton);
    std::shared_ptr<animation::Skeleton> getSkeleton() const { return skeleton_; }

    size_t getMemoryUsage() const override;
    void unload() override;

private:
    std::shared_ptr<animation::Skeleton> skeleton_;
};

class AnimationAsset : public Asset {
public:
    explicit AnimationAsset(const std::string& id);
    ~AnimationAsset() override;

    void setAnimationClip(std::shared_ptr<animation::AnimationClip> clip);
    std::shared_ptr<animation::AnimationClip> getAnimationClip() const { return clip_; }

    size_t getMemoryUsage() const override;
    void unload() override;

private:
    std::shared_ptr<animation::AnimationClip> clip_;
};

class AudioAsset : public Asset {
public:
    explicit AudioAsset(const std::string& id);
    ~AudioAsset() override;

    void setAudioBuffer(std::shared_ptr<audio::AudioBuffer> buffer);
    std::shared_ptr<audio::AudioBuffer> getAudioBuffer() const { return buffer_; }

    size_t getMemoryUsage() const override;
    void unload() override;

private:
    std::shared_ptr<audio::AudioBuffer> buffer_;
};

class ImageAsset : public Asset {
public:
    explicit ImageAsset(const std::string& id);
    ~ImageAsset() override;

    void setImageData(int width, int height, int channels, const std::vector<uint8_t>& data);
    std::vector<uint8_t> getImageData() const { return data_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getChannels() const { return channels_; }

    size_t getMemoryUsage() const override;
    void unload() override;

private:
    int width_;
    int height_;
    int channels_;
    std::vector<uint8_t> data_;
};

class MaterialAsset : public Asset {
public:
    explicit MaterialAsset(const std::string& id);
    ~MaterialAsset() override;

    void setMaterial(std::shared_ptr<modeling::Material> material);
    std::shared_ptr<modeling::Material> getMaterial() const { return material_; }

    size_t getMemoryUsage() const override;
    void unload() override;

private:
    std::shared_ptr<modeling::Material> material_;
};

struct AssetCacheConfig {
    size_t maxMemoryUsage = 1024 * 1024 * 1024;
    size_t maxAssetCount = 1000;
    bool enableAutoUnload = true;
    std::chrono::seconds maxIdleTime = std::chrono::seconds(300);
    float unloadThreshold = 0.9f;
};

class AssetManager {
public:
    static AssetManager& getInstance();

    AssetHandle loadAsset(const std::string& path, AssetType type);
    AssetHandle loadAssetAsync(const std::string& path, AssetType type,
                              std::function<void(const AssetHandle&)> callback);

    template<typename T>
    std::shared_ptr<T> getAsset(const AssetHandle& handle);

    void unloadAsset(const AssetHandle& handle);
    void unloadAllAssets();
    void unloadUnusedAssets();

    bool isAssetLoaded(const AssetHandle& handle) const;
    AssetLoadState getAssetState(const AssetHandle& handle) const;

    AssetMetadata getAssetMetadata(const AssetHandle& handle) const;
    std::vector<AssetMetadata> getAllAssetsMetadata() const;

    size_t getTotalMemoryUsage() const;
    size_t getLoadedAssetCount() const;

    void setCacheConfig(const AssetCacheConfig& config);
    const AssetCacheConfig& getCacheConfig() const;

    void registerAssetPath(const std::string& alias, const std::string& path);
    std::string resolveAssetPath(const std::string& path) const;

    void setProgressCallback(std::function<void(const std::string&, float)> callback);
    void setErrorCallback(std::function<void(const std::string&, const std::string&)> callback);

    void update();
    void cleanup();

private:
    AssetManager();
    ~AssetManager();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::string generateAssetId(const std::string& path) const;
    AssetType detectAssetType(const std::string& path) const;

    std::shared_ptr<Asset> createAsset(AssetType type, const std::string& id);
    bool loadAssetData(std::shared_ptr<Asset> asset, const std::string& path);

    void checkMemoryUsage();
    void unloadOldestAssets(size_t targetMemoryUsage);

    std::unordered_map<std::string, std::shared_ptr<Asset>> assets_;
    std::unordered_map<std::string, std::string> assetPathAliases_;

    mutable std::mutex assetsMutex_;
    std::atomic<size_t> totalMemoryUsage_;

    AssetCacheConfig cacheConfig_;

    std::function<void(const std::string&, float)> progressCallback_;
    std::function<void(const std::string&, const std::string&)> errorCallback_;
};

template<typename T>
std::shared_ptr<T> AssetManager::getAsset(const AssetHandle& handle) {
    std::lock_guard<std::mutex> lock(assetsMutex_);

    auto it = assets_.find(handle.id);
    if (it == assets_.end() || it->second->getState() != AssetLoadState::Loaded) {
        return nullptr;
    }

    return std::dynamic_pointer_cast<T>(it->second);
}

}
}
